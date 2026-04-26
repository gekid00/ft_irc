from __future__ import annotations

import json
import logging
import shutil
from datetime import date, datetime, timedelta
from pathlib import Path

from .models import Offer

log = logging.getLogger(__name__)

SCHEMA_VERSION = 1
DEFAULT_PATH = Path("candidatures.json")
EXPIRE_DAYS = 14


def _empty_db() -> dict:
    return {
        "schema_version": SCHEMA_VERSION,
        "last_run": None,
        "offres": [],
    }


def load(path: Path = DEFAULT_PATH) -> dict:
    if not path.exists():
        return _empty_db()
    try:
        with path.open("r", encoding="utf-8") as f:
            return json.load(f)
    except (json.JSONDecodeError, OSError) as e:
        ts = datetime.now().strftime("%Y%m%d-%H%M%S")
        backup = path.with_suffix(f".json.bak.{ts}")
        shutil.copy2(path, backup)
        log.error("DB corrompue (%s), backup -> %s, repart vide", e, backup)
        return _empty_db()


def save(db: dict, path: Path = DEFAULT_PATH) -> None:
    db["last_run"] = datetime.now().isoformat(timespec="seconds")
    tmp = path.with_suffix(".json.tmp")
    with tmp.open("w", encoding="utf-8") as f:
        json.dump(db, f, ensure_ascii=False, indent=2)
    tmp.replace(path)


def merge(db: dict, scraped: list[Offer]) -> tuple[int, int]:
    by_id = {o["id"]: o for o in db["offres"]}
    today = date.today().isoformat()
    seen_ids = set()
    new_count = 0
    upd_count = 0

    for offer in scraped:
        seen_ids.add(offer.id)
        d = offer.to_dict()
        if offer.id in by_id:
            existing = by_id[offer.id]
            existing["score"] = d["score"]
            existing["date_derniere_maj"] = today
            existing["description_excerpt"] = d["description_excerpt"]
            upd_count += 1
        else:
            by_id[offer.id] = d
            new_count += 1

    _expire_stale(by_id, seen_ids)
    db["offres"] = list(by_id.values())
    return new_count, upd_count


def _expire_stale(by_id: dict, seen_ids: set[str]) -> None:
    cutoff = date.today() - timedelta(days=EXPIRE_DAYS)
    for oid, off in by_id.items():
        if oid in seen_ids:
            continue
        if off.get("status") not in ("nouveau", "a_postuler"):
            continue
        last_maj = off.get("date_derniere_maj")
        if last_maj and date.fromisoformat(last_maj) < cutoff:
            off["status"] = "expire"


def update_status(db: dict, company_substring: str, new_status: str,
                  note: str | None = None) -> Offer | None:
    needle = company_substring.lower()
    for off in db["offres"]:
        if needle in off["entreprise"].lower():
            off["status"] = new_status
            off["date_derniere_maj"] = date.today().isoformat()
            if new_status == "poste":
                off["date_postule"] = date.today().isoformat()
            if note:
                off["notes"] = (off.get("notes", "") + " | " + note).strip(" |")
            return Offer.from_dict(off)
    return None
