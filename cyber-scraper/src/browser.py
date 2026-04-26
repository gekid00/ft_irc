"""Playwright browser helpers — context manager async unique pour tout le run."""
from __future__ import annotations

import logging
import os
from contextlib import asynccontextmanager
from typing import AsyncIterator

from playwright.async_api import Browser, BrowserContext, async_playwright

log = logging.getLogger(__name__)

UA = (
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
    "(KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36"
)

_EXEC_CANDIDATES = [
    os.environ.get("PLAYWRIGHT_CHROMIUM"),
]


def _executable_path() -> str | None:
    for p in _EXEC_CANDIDATES:
        if p and os.path.exists(p):
            return p
    return None


@asynccontextmanager
async def browser_context() -> AsyncIterator[BrowserContext]:
    async with async_playwright() as p:
        kwargs = {"headless": True, "args": ["--no-sandbox"]}
        exe = _executable_path()
        if exe:
            kwargs["executable_path"] = exe
        b: Browser = await p.chromium.launch(**kwargs)
        try:
            ctx = await b.new_context(
                user_agent=UA,
                ignore_https_errors=True,
                locale="fr-FR",
                viewport={"width": 1366, "height": 900},
            )
            yield ctx
        finally:
            await b.close()
