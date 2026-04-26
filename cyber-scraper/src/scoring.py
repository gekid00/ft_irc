from __future__ import annotations

from .models import Offer

INCLUDE_KEYWORDS = [
    "pentest", "test d'intrusion", "test de pénétration", "test de penetration",
    "red team", "offensive", "exploitation", "vulnerability", "vulnérabilité",
    "vulnerabilite", "outil offensif", "tool development", "reverse engineering",
    "reverse", "buffer overflow", "binary exploitation", "exploit dev",
]

EXCLUDE_KEYWORDS = [
    "soc analyst", "blue team only", "gouvernance", "compliance", "rgpd",
    "iso 27001", "support", "helpdesk", " n1 ", " n2 ", "niveau 1", "niveau 2",
]

HOT = ["pentest", "red team", "exploitation", "offensive", "exploit dev", "reverse"]
WARM = ["security audit", "vulnerability", "vulnérabilité", "audit sécurité",
        "audit securite"]
SKILLS = ["c++", " c ", "python", "linux", "bash", "playwright", "assembly", "asm"]
PURE_PLAYERS = [
    "synacktiv", "akerva", "wavestone", "almond", "intrinsec", "lexfo",
    "quarkslab", "ambionics", "yeswehack", "yogosha", "harfanglab", "tehtris",
]


def is_relevant(offer: Offer) -> bool:
    blob = f"{offer.titre} {offer.description}".lower()
    if any(k in blob for k in EXCLUDE_KEYWORDS):
        return False
    return any(k in blob for k in INCLUDE_KEYWORDS)


def score_offer(offer: Offer) -> float:
    desc = f"{offer.titre} {offer.description}".lower()
    loc = offer.localisation.lower()
    contract = offer.contrat.lower()
    company = offer.entreprise.lower()

    s = 0.0
    if any(k in desc for k in HOT):
        s += 4
    elif any(k in desc for k in WARM):
        s += 2
    else:
        return 0.0

    if "paris" in loc or " 75" in loc or loc.startswith("75"):
        s += 3
    elif "île-de-france" in loc or "ile-de-france" in loc:
        s += 2
    elif "france" in loc:
        s += 0.5
    else:
        s -= 1

    if "stage" in contract or "internship" in contract:
        s += 2
    elif "alternance" in contract or "apprenticeship" in contract:
        s += 1

    if any(k in desc for k in SKILLS):
        s += 1

    if any(c in company for c in PURE_PLAYERS):
        s += 1

    return max(0.0, min(10.0, s))
