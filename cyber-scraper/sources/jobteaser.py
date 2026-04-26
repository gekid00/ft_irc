"""JobTeaser stub — auth SSO 42 requise. Renvoie un rappel manuel."""
from __future__ import annotations

import logging

from src.models import Offer

log = logging.getLogger(__name__)


async def fetch(use_cache: bool = True) -> list[Offer]:
    log.info("JobTeaser: stub (auth SSO 42 requise)")
    return [Offer(
        source="jobteaser",
        titre="[REMINDER] Check JobTeaser 42 manuellement",
        entreprise="—",
        url="https://42-jobteaser.com",
        localisation="—",
        contrat="stage",
        description=("JobTeaser 42 nécessite l'auth SSO. Connecte-toi et filtre "
                     "Stage + cyber/pentest. À faire 1× par semaine."),
    )]
