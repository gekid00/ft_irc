"""Choisir Le Service Public (ANSSI, COMCYBER, etc.)."""
from __future__ import annotations

import asyncio
import json
import logging
from urllib.parse import quote_plus

from src import cache as cache_mod
from src.http import client, parse
from src.models import Offer

log = logging.getLogger(__name__)

QUERIES = ["cybersécurité", "sécurité informatique", "pentest"]
SEARCH_URL = ("https://choisirleservicepublic.gouv.fr/nos-offres/"
              "?keywords={q}&type-contrat=stage")

CARD_SEL = "article.card-offre, article[class*='card'], li.offer-item"
TITLE_SEL = "h3, h2, .card-title"
LINK_SEL = "a[href]"
EMPLOYER_SEL = ".card-employer, .employer, [class*='employeur']"
LOC_SEL = ".card-location, [class*='location'], [class*='lieu']"
PER_REQ_DELAY = 1.0


async def _search(c, query: str) -> list[Offer]:
    url = SEARCH_URL.format(q=quote_plus(query))
    log.info("gov search: %s", query)
    try:
        resp = await c.get(url)
    except Exception as e:
        log.error("gov req failed for %r: %s", query, e)
        return []
    if resp.status_code >= 400:
        log.warning("gov status %s for %r", resp.status_code, query)
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
                href = "https://choisirleservicepublic.gouv.fr" + href
            comp_n = card.css_first(EMPLOYER_SEL)
            loc_n = card.css_first(LOC_SEL)
            offers.append(Offer(
                source="gov",
                titre=(title_n.text() or "").strip(),
                entreprise=(comp_n.text() if comp_n else "Service public").strip(),
                url=href.split("?")[0],
                localisation=(loc_n.text() if loc_n else "").strip(),
                contrat="stage", description="",
            ))
        except Exception as e:
            log.debug("gov card error: %s", e)
    log.info("gov %r: %d offers", query, len(offers))
    return offers


async def fetch(use_cache: bool = True) -> list[Offer]:
    cache_key = "gov"
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
                log.error("gov %r failed: %s", q, e)
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
