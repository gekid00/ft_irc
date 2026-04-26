"""HelloWork scraper — SSR HTML, httpx + selectolax."""
from __future__ import annotations

import asyncio
import json
import logging
from urllib.parse import quote_plus

from src import cache as cache_mod
from src.http import client, parse
from src.models import Offer

log = logging.getLogger(__name__)

QUERIES = ["pentest", "red+team", "cybersécurité"]
SEARCH_URL = ("https://www.hellowork.com/fr-fr/emploi/recherche.html"
              "?k={q}&c=Stage&d=france_entiere")

CARD_SEL = "li[data-id-storage-target='item']"
TITLE_SEL = "h3"
COMPANY_SEL = "[data-cy='companyName']"
LOCATION_SEL = "[data-cy='localisationCard']"
LINK_SEL = "a[href*='/emplois/']"
PER_REQ_DELAY = 1.0


async def _search(c, query: str) -> list[Offer]:
    url = SEARCH_URL.format(q=quote_plus(query))
    log.info("HelloWork search: %s", query)
    try:
        resp = await c.get(url)
    except Exception as e:
        log.error("HelloWork req failed for %r: %s", query, e)
        return []
    if resp.status_code >= 400:
        log.warning("HelloWork status %s for %r", resp.status_code, query)
        return []
    tree = parse(resp.text)
    offers: list[Offer] = []
    for card in tree.css(CARD_SEL):
        try:
            title_n = card.css_first(TITLE_SEL)
            link_n = card.css_first(LINK_SEL)
            if not title_n or not link_n:
                continue
            href = link_n.attributes.get("href", "")
            if href and not href.startswith("http"):
                href = "https://www.hellowork.com" + href
            comp_n = card.css_first(COMPANY_SEL)
            loc_n = card.css_first(LOCATION_SEL)
            offers.append(Offer(
                source="hellowork",
                titre=(title_n.text() or "").strip(),
                entreprise=(comp_n.text() if comp_n else "").strip(),
                url=href.split("?")[0],
                localisation=(loc_n.text() if loc_n else "").strip(),
                contrat="stage", description="",
            ))
        except Exception as e:
            log.debug("HelloWork card error: %s", e)
    log.info("HelloWork %r: %d offers", query, len(offers))
    return offers


async def fetch(use_cache: bool = True) -> list[Offer]:
    cache_key = "hellowork"
    if use_cache:
        cached = cache_mod.read(cache_key, ext="json")
        if cached:
            return [Offer(**d) for d in json.loads(cached)]
    all_offers: list[Offer] = []
    seen: set[str] = set()
    async with client() as c:
        for q in QUERIES:
            try:
                offers = await _search(c, q)
            except Exception as e:
                log.error("HelloWork %r failed: %s", q, e)
                continue
            for o in offers:
                if o.url in seen:
                    continue
                seen.add(o.url)
                all_offers.append(o)
            await asyncio.sleep(PER_REQ_DELAY)
    if all_offers:
        cache_mod.write(cache_key,
                        json.dumps([o.__dict__ for o in all_offers], ensure_ascii=False),
                        ext="json")
    return all_offers
