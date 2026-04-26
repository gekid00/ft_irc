"""httpx async client partagé + helpers selectolax."""
from __future__ import annotations

import logging
from contextlib import asynccontextmanager
from typing import AsyncIterator

import httpx
from selectolax.parser import HTMLParser

log = logging.getLogger(__name__)

UA = (
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
    "(KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36"
)
DEFAULT_HEADERS = {
    "User-Agent": UA,
    "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
    "Accept-Language": "fr-FR,fr;q=0.9,en;q=0.8",
}


@asynccontextmanager
async def client(timeout: float = 20.0) -> AsyncIterator[httpx.AsyncClient]:
    async with httpx.AsyncClient(
        headers=DEFAULT_HEADERS,
        timeout=timeout,
        follow_redirects=True,
        verify=False,
    ) as c:
        yield c


def parse(html: str) -> HTMLParser:
    return HTMLParser(html)
