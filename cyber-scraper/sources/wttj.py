"""Welcome To The Jungle scraper (Playwright)."""
from __future__ import annotations

import asyncio
import json
import logging
import re
from pathlib import Path
from urllib.parse import quote

from playwright.async_api import BrowserContext, TimeoutError as PWTimeout

from src import cache as cache_mod
from src.browser import browser_context
from src.models import Offer

log = logging.getLogger(__name__)

QUERIES = ["pentest", "red team", "cybersécurité"]
BASE = "https://www.welcometothejungle.com"
SEARCH_URL = (
    BASE + "/fr/jobs?query={q}"
    "&refinementList%5Bcontract_type%5D%5B%5D=internship"
    "&refinementList%5Boffices.country_code%5D%5B%5D=FR"
)

JOB_LINK_SEL = 'a[href*="/companies/"][href*="/jobs/"]'
DETAIL_TITLE_SEL = 'h1, h2'
DETAIL_COMPANY_SEL = 'a[href^="/fr/companies/"]'
DETAIL_DESC_SEL = '[data-testid="job-section-description"], section'
DETAIL_LOC_SEL = '[data-testid="job-metadata-location"], [class*="location" i]'

PAGE_TIMEOUT = 30000
PER_REQ_DELAY = 1.0
FIXTURE_FALLBACK = Path(__file__).parent.parent / "fixtures" / "wttj_sample.json"


async def _collect_links(ctx: BrowserContext, query: str) -> set[str]:
    url = SEARCH_URL.format(q=quote(query))
    page = await ctx.new_page()
    links: set[str] = set()
    try:
        log.info("WTTJ search: %s", query)
        resp = await page.goto(url, wait_until="domcontentloaded", timeout=PAGE_TIMEOUT)
        if not resp or resp.status >= 400:
            log.warning("WTTJ status %s for query %r", resp.status if resp else "?", query)
            return links
        try:
            await page.wait_for_selector(JOB_LINK_SEL, timeout=10000)
        except PWTimeout:
            log.warning("no job links found for %r", query)
            return links
        for _ in range(2):
            await page.mouse.wheel(0, 4000)
            await page.wait_for_timeout(800)
        hrefs = await page.locator(JOB_LINK_SEL).evaluate_all(
            "els => els.map(e => e.getAttribute('href'))"
        )
        for h in hrefs:
            if not h:
                continue
            full = h if h.startswith("http") else BASE + h
            if "/jobs/" in full:
                links.add(full.split("?")[0])
    finally:
        await page.close()
    return links


async def _fetch_detail(ctx: BrowserContext, url: str) -> Offer | None:
    page = await ctx.new_page()
    try:
        resp = await page.goto(url, wait_until="domcontentloaded", timeout=PAGE_TIMEOUT)
        if not resp or resp.status >= 400:
            return None
        await page.wait_for_timeout(500)
        title = (await page.locator(DETAIL_TITLE_SEL).first.text_content() or "").strip()
        company = ""
        try:
            company = (await page.locator(DETAIL_COMPANY_SEL).first.text_content() or "").strip()
        except Exception:
            pass
        if not company:
            m = re.search(r"/companies/([^/]+)/jobs/", url)
            company = m.group(1).replace("-", " ").title() if m else ""
        location = ""
        try:
            location = (await page.locator(DETAIL_LOC_SEL).first.text_content() or "").strip()
        except Exception:
            pass
        description = ""
        try:
            description = (await page.locator(DETAIL_DESC_SEL).first.text_content() or "").strip()
        except Exception:
            pass
        if not title:
            return None
        return Offer(source="wttj", titre=title, entreprise=company, url=url,
                     localisation=location, contrat="stage", description=description)
    finally:
        await page.close()


async def fetch(use_cache: bool = True) -> list[Offer]:
    cache_key = "wttj"
    if use_cache:
        cached = cache_mod.read(cache_key, ext="json")
        if cached:
            return [Offer(**d) for d in json.loads(cached)]

    offers: list[Offer] = []
    try:
        async with browser_context() as ctx:
            all_links: set[str] = set()
            for q in QUERIES:
                try:
                    all_links |= await _collect_links(ctx, q)
                except Exception as e:
                    log.error("WTTJ search failed for %r: %s", q, e)
                await asyncio.sleep(PER_REQ_DELAY)
            log.info("WTTJ: %d unique job links", len(all_links))
            for href in all_links:
                try:
                    off = await _fetch_detail(ctx, href)
                    if off:
                        offers.append(off)
                except Exception as e:
                    log.error("WTTJ detail failed for %s: %s", href, e)
                await asyncio.sleep(PER_REQ_DELAY)
    except Exception as e:
        log.error("WTTJ browser failed: %s — fallback fixture", e)
        return _load_fixture()

    if not offers:
        log.warning("WTTJ 0 offers — fallback fixture")
        return _load_fixture()

    cache_mod.write(cache_key,
                    json.dumps([_to_dict(o) for o in offers], ensure_ascii=False),
                    ext="json")
    return offers


def _to_dict(o: Offer) -> dict:
    return {"source": o.source, "titre": o.titre, "entreprise": o.entreprise,
            "url": o.url, "localisation": o.localisation, "contrat": o.contrat,
            "duree": o.duree, "description": o.description}


def _load_fixture() -> list[Offer]:
    if not FIXTURE_FALLBACK.exists():
        return []
    return [Offer(**d) for d in json.loads(FIXTURE_FALLBACK.read_text(encoding="utf-8"))]
