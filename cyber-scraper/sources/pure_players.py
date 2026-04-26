"""Pure-players cyber offensive — pages carrière directes."""
from __future__ import annotations

import asyncio
import json
import logging
import re
from urllib.parse import urlparse

from src import cache as cache_mod
from src.http import client, parse
from src.models import Offer

log = logging.getLogger(__name__)

COMPANIES = [
    ("Synacktiv", "https://www.synacktiv.com/jobs"),
    ("Almond", "https://www.almond.consulting/recrutement-et-vie-en-entreprise/"),
    ("Akerva", "https://akerva.com/rejoignez-nous/"),
    ("Lexfo", "https://www.lexfo.fr/jobs.html"),
    ("Wavestone", "https://www.wavestone.com/fr/carriere/"),
    ("Quarkslab", "https://www.quarkslab.com/career.html"),
    ("Sopra Steria", "https://www.soprasteria.com/fr/carrieres/postuler"),
    ("Orange Cyberdefense", "https://orangecyberdefense.com/global/careers/"),
]
PER_REQ_DELAY = 1.5
STAGE_RE = re.compile(r"\bstage\b", re.IGNORECASE)


async def _scan(c, name: str, url: str) -> list[Offer]:
    log.info("pure-players: %s", name)
    try:
        resp = await c.get(url)
    except Exception as e:
        log.error("pure-players req failed for %s: %s", name, e)
        return []
    if resp.status_code >= 400:
        log.warning("pure-players status %s for %s", resp.status_code, name)
        return []
    tree = parse(resp.text)
    offers: list[Offer] = []
    seen: set[str] = set()
    for link in tree.css("a[href]"):
        try:
            txt = (link.text() or "").strip()
            if not txt or len(txt) < 5 or len(txt) > 200:
                continue
            if not STAGE_RE.search(txt):
                continue
            href = link.attributes.get("href", "")
            if href.startswith("/"):
                p = urlparse(url)
                href = f"{p.scheme}://{p.netloc}{href}"
            elif not href.startswith("http"):
                continue
            if href in seen:
                continue
            seen.add(href)
            offers.append(Offer(
                source=f"pure:{name.lower().replace(' ', '_')}",
                titre=txt, entreprise=name, url=href.split("?")[0],
                localisation="", contrat="stage", description="",
            ))
        except Exception as e:
            log.debug("pure-players link error: %s", e)
    log.info("pure-players %s: %d offers", name, len(offers))
    return offers


async def fetch(use_cache: bool = True) -> list[Offer]:
    cache_key = "pure_players"
    if use_cache:
        cached = cache_mod.read(cache_key, ext="json")
        if cached:
            return [Offer(**d) for d in json.loads(cached)]
    all_offers: list[Offer] = []
    async with client() as c:
        for name, url in COMPANIES:
            try:
                all_offers.extend(await _scan(c, name, url))
            except Exception as e:
                log.error("pure-players %s failed: %s", name, e)
            await asyncio.sleep(PER_REQ_DELAY)
    if all_offers:
        cache_mod.write(cache_key,
                        json.dumps([o.__dict__ for o in all_offers], ensure_ascii=False),
                        ext="json")
    return all_offers
