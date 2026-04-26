from __future__ import annotations

from datetime import date, timedelta

from rich.console import Console
from rich.table import Table

console = Console()

STATUS_LABEL = {
    "nouveau": "🆕 Nouveau",
    "a_postuler": "⏳ À postuler",
    "poste": "✅ Posté",
    "en_attente": "📧 En attente",
    "entretien": "🎯 Entretien",
    "rejet": "❌ Rejet",
    "expire": "💀 Expiré",
}


def render_hot(db: dict, threshold: float = 7.0) -> None:
    hot = [o for o in db["offres"]
           if o["score"] >= threshold and o["status"] not in ("rejet", "expire")]
    hot.sort(key=lambda o: o["score"], reverse=True)

    title = f"🔴 OFFRES HOT (score ≥ {threshold}) — À postuler en priorité"
    table = Table(title=title, show_lines=True, expand=True)
    table.add_column("Score", justify="right", style="bold red", width=6)
    table.add_column("Titre", style="cyan", overflow="fold")
    table.add_column("Entreprise", style="magenta")
    table.add_column("Loc", style="green")
    table.add_column("URL", style="blue", overflow="fold")

    if not hot:
        table.add_row("-", "[dim]Aucune offre HOT pour le moment[/dim]", "", "", "")
    else:
        for o in hot:
            table.add_row(
                f"{o['score']:.1f}",
                o["titre"],
                o["entreprise"],
                o["localisation"] or "-",
                o["url"],
            )
    console.print(table)


def render_suivi(db: dict, days: int = 30) -> None:
    cutoff = date.today() - timedelta(days=days)
    rows = []
    for o in db["offres"]:
        if o["status"] == "rejet":
            continue
        d = o.get("date_derniere_maj") or o.get("date_trouvee")
        if d and date.fromisoformat(d) >= cutoff:
            rows.append(o)
    rows.sort(key=lambda o: (o["status"] != "poste", -o["score"]))

    title = f"📊 SUIVI CANDIDATURES (derniers {days} jours)"
    table = Table(title=title, show_lines=False, expand=True)
    table.add_column("#", justify="right", style="dim", width=3)
    table.add_column("Entreprise", style="magenta")
    table.add_column("Score", justify="right", width=6)
    table.add_column("Status", width=14)
    table.add_column("Date postule", style="cyan", width=12)

    if not rows:
        table.add_row("-", "[dim]Rien à afficher[/dim]", "", "", "")
    else:
        for i, o in enumerate(rows, 1):
            table.add_row(
                str(i),
                o["entreprise"],
                f"{o['score']:.1f}",
                STATUS_LABEL.get(o["status"], o["status"]),
                o.get("date_postule") or "-",
            )
    console.print(table)


def render_distribution(db: dict) -> None:
    actives = [o for o in db["offres"] if o["status"] not in ("rejet", "expire")]
    buckets = {"≥8.5": 0, "7-8.5": 0, "5-7": 0, "<5": 0}
    for o in actives:
        s = o["score"]
        if s >= 8.5:
            buckets["≥8.5"] += 1
        elif s >= 7:
            buckets["7-8.5"] += 1
        elif s >= 5:
            buckets["5-7"] += 1
        else:
            buckets["<5"] += 1

    table = Table(title="📈 RÉPARTITION", show_header=False, expand=False)
    table.add_column("Tranche", style="bold")
    table.add_column("Count", justify="right")
    for k, v in buckets.items():
        table.add_row(k, str(v))
    console.print(table)


def render_footer(db: dict) -> None:
    actives = [o for o in db["offres"] if o["status"] != "rejet"]
    n_post = sum(1 for o in actives if o["status"] == "poste")
    n_wait = sum(1 for o in actives if o["status"] == "en_attente")
    n_new = sum(1 for o in actives if o["status"] == "nouveau")
    last = db.get("last_run") or "-"
    console.print(
        f"\n[bold]{n_post}[/] postées | "
        f"[bold]{n_wait}[/] en attente | "
        f"[bold]{n_new}[/] nouvelles | "
        f"last_run = [dim]{last}[/]\n"
    )


def render_all(db: dict) -> None:
    render_hot(db)
    console.print()
    render_suivi(db)
    console.print()
    render_distribution(db)
    render_footer(db)
