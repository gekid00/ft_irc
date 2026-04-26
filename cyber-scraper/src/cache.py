from __future__ import annotations

from datetime import date
from pathlib import Path

CACHE_DIR = Path("cache")


def cache_path(source: str, ext: str = "html") -> Path:
    CACHE_DIR.mkdir(exist_ok=True)
    return CACHE_DIR / f"{source}_{date.today().isoformat()}.{ext}"


def read(source: str, ext: str = "html") -> str | None:
    p = cache_path(source, ext)
    if p.exists():
        return p.read_text(encoding="utf-8")
    return None


def write(source: str, content: str, ext: str = "html") -> None:
    cache_path(source, ext).write_text(content, encoding="utf-8")
