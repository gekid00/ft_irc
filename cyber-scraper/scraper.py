#!/usr/bin/env python3
"""Cyber offensive job scraper — entry point."""
from __future__ import annotations

import argparse
import asyncio
import logging
import sys
from pathlib import Path

from src import persistence
from src.models import Offer
from src.output import console, render_all, render_suivi, STATUS_LABEL
from src.scoring import is_relevant, score_offer

LOG_FILE = Path("scraper.log")
LOG_FORMAT = "%(asctime)s [%(levelname)s] %(name)s: %(message)s"


def setup_logging(verbose: bool) -> None:
    level = logging.DEBUG if verbose else logging.INFO
    logging.basicConfig(
        level=level,
        format=LOG_FORMAT,
        handlers=[
            logging.FileHandler(LOG_FILE, encoding="utf-8"),
            logging.StreamHandler(sys.stderr),
        ],
    )


async def run_all_sources(use_cache: bool) -> list[Offer]:
    """Lance toutes les sources en parallèle, fusionne les résultats."""
    from sources import wttj, hellowork, apec, jobteaser, gov, pure_players

    tasks = [
        wttj.fetch(use_cache=use_cache),
        hellowork.fetch(use_cache=use_cache),
        apec.fetch(use_cache=use_cache),
        jobteaser.fetch(use_cache=use_cache),
        gov.fetch(use_cache=use_cache),
        pure_players.fetch(use_cache=use_cache),
    ]
    results = await asyncio.gather(*tasks, return_exceptions=True)
    offers: list[Offer] = []
    for r in results:
        if isinstance(r, Exception):
            logging.error("source failed: %s", r)
            continue
        offers.extend(r)
    return offers


def filter_and_score(offers: list[Offer]) -> list[Offer]:
    out = []
    for o in offers:
        if not is_relevant(o):
            continue
        o.score = score_offer(o)
        if o.score == 0:
            continue
        out.append(o)
    return out


def cmd_scrape(args: argparse.Namespace) -> int:
    db = persistence.load()
    raw = asyncio.run(run_all_sources(use_cache=not args.full_refresh))
    logging.info("scraped %d raw offers", len(raw))
    relevant = filter_and_score(raw)
    logging.info("kept %d relevant offers after filter+score", len(relevant))
    new_n, upd_n = persistence.merge(db, relevant)
    persistence.save(db)
    console.print(f"[green]✓[/] {new_n} nouvelles, {upd_n} mises à jour\n")
    render_all(db)
    return 0


def cmd_suivi(_: argparse.Namespace) -> int:
    db = persistence.load()
    render_suivi(db)
    return 0


def cmd_status(args: argparse.Namespace, new_status: str) -> int:
    db = persistence.load()
    off = persistence.update_status(db, args.entreprise, new_status, note=args.note)
    if not off:
        console.print(f"[red]✗[/] Aucune offre matchant '{args.entreprise}'")
        return 1
    persistence.save(db)
    console.print(
        f"[green]✓[/] {off.entreprise} → {STATUS_LABEL.get(new_status, new_status)}"
    )
    return 0


def main() -> int:
    p = argparse.ArgumentParser(description="Cyber offensive job scraper")
    p.add_argument("--verbose", action="store_true")
    p.add_argument("--full-refresh", action="store_true",
                   help="Ignore le cache 24h")
    p.add_argument("--suivi-only", action="store_true",
                   help="N'affiche que le suivi, pas de scrape")
    p.add_argument("--marquer-postule", metavar="ENTREPRISE",
                   help="Marque une offre comme postée")
    p.add_argument("--marquer-rejet", metavar="ENTREPRISE",
                   help="Marque une offre comme rejet")
    p.add_argument("--marquer-attente", metavar="ENTREPRISE",
                   help="Marque une offre comme en attente de réponse")
    p.add_argument("--note", metavar="TEXTE", default=None,
                   help="Ajoute une note (avec --marquer-*)")

    args = p.parse_args()
    setup_logging(args.verbose)

    if args.suivi_only:
        return cmd_suivi(args)
    if args.marquer_postule:
        args.entreprise = args.marquer_postule
        return cmd_status(args, "poste")
    if args.marquer_rejet:
        args.entreprise = args.marquer_rejet
        return cmd_status(args, "rejet")
    if args.marquer_attente:
        args.entreprise = args.marquer_attente
        return cmd_status(args, "en_attente")

    return cmd_scrape(args)


if __name__ == "__main__":
    raise SystemExit(main())
