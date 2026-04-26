from __future__ import annotations

import hashlib
from dataclasses import dataclass, field, asdict
from datetime import date
from typing import Literal

Status = Literal[
    "nouveau", "a_postuler", "poste", "en_attente", "entretien", "rejet", "expire"
]


@dataclass
class Offer:
    source: str
    titre: str
    entreprise: str
    url: str
    localisation: str = ""
    contrat: str = ""
    duree: str = ""
    description: str = ""
    score: float = 0.0
    status: Status = "nouveau"
    date_trouvee: str = field(default_factory=lambda: date.today().isoformat())
    date_postule: str | None = None
    date_derniere_maj: str = field(default_factory=lambda: date.today().isoformat())
    notes: str = ""

    @property
    def id(self) -> str:
        raw = f"{self.titre.lower().strip()}|{self.entreprise.lower().strip()}|{self.source}"
        return hashlib.sha1(raw.encode()).hexdigest()[:12]

    @property
    def description_excerpt(self) -> str:
        return self.description[:500]

    def to_dict(self) -> dict:
        d = asdict(self)
        d["id"] = self.id
        d["description_excerpt"] = self.description_excerpt
        d.pop("description", None)
        return d

    @classmethod
    def from_dict(cls, d: dict) -> Offer:
        d = dict(d)
        d.pop("id", None)
        d["description"] = d.pop("description_excerpt", "")
        return cls(**d)
