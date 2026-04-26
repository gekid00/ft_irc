"""APEC scraper — API JSON interne /cms/webservices/rechercheOffre."""
from __future__ import annotations

import asyncio
import json
import logging

from src import cache as cache_mod
from src.http import client
from src.models import Offer

log = logging.getLogger(__name__)

QUERIES = ["pentest", "red team", "cybersécurité"]
API_URL = "https://www.apec.fr/cms/webservices/rechercheOffre"
DETAIL_URL = "https://www.apec.fr/candidat/recherche-emploi.html/emploi/detail-offre/{id}"

PAYLOAD_BASE = {
    "typesContrat": ["101888"],
    "lieux": [], "fonctions": [], "secteursActivite": [], "experience": [],
    "salaireMinimum": 0, "salaireMaximum": 200,
    "pagination": {"sortsType": "SCORE", "page": 0, "size": 50},
}
PER_REQ_DELAY = 1.0


async def _search(c, query: str) -> list[Offer]:
    payload = dict(PAYLOAD_BASE)
    payload["motsCles"] = query
    log.info("APEC search: %s", query)
    try:
        resp = await c.post(API_URL, json=payload, headers={
            "Content-Type": "application/json", "Accept": "application/json"})
    except Exception as e:
        log.error("APEC req failed for %r: %s", query, e)
        return []
    if resp.status_code >= 400:
        log.warning("APEC status %s for %r", resp.status_code, query)
        return []
    try:
        data = resp.json()
    except Exception as e:
        log.error("APEC JSON parse failed: %s", e)
        return []
    items = data.get("resultats") or data.get("results") or []
    offers: list[Offer] = []
    for it in items:
        offer_id = it.get("numeroOffre") or it.get("id") or ""
        if not offer_id:
            continue
        offers.append(Offer(
            source="apec",
            titre=(it.get("intitule") or "").strip(),
            entreprise=(it.get("nomCommercial") or it.get("entreprise") or "").strip(),
            url=DETAIL_URL.format(id=offer_id),
            localisation=(it.get("lieuTexte") or it.get("lieu") or "").strip(),
            contrat="stage",
            description=(it.get("texte") or it.get("description") or "")[:1000],
        ))
    log.info("APEC %r: %d offers", query, len(offers))
    return offers


async def fetch(use_cache: bool = True) -> list[Offer]:
    cache_key = "apec"
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
                log.error("APEC %r failed: %s", q, e)
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
