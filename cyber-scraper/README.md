# cyber-scraper

Agrégateur de stages cybersécurité offensive en France. Scrape WTTJ, HelloWork, APEC, choisirleservicepublic.gouv.fr, et les pages carrière des pure-players (Synacktiv, Almond, Akerva, Lexfo, Wavestone, Quarkslab, etc.). Filtre + score selon `PROFILE.md`. Suivi de candidatures local en JSON.

## Setup

```bash
cd cyber-scraper
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
playwright install chromium
```

## Usage

```bash
# Scrape + affiche tableau + suivi
python scraper.py

# Voir uniquement le suivi de candidatures
python scraper.py --suivi-only

# Marquer une offre comme postée / rejet / en attente
python scraper.py --marquer-postule "Synacktiv" --note "envoyé via mail RH"
python scraper.py --marquer-rejet "Akerva"
python scraper.py --marquer-attente "Almond"

# Forcer un refresh (ignore le cache 24h)
python scraper.py --full-refresh
```

## Structure

- `scraper.py` — entry point + CLI
- `src/` — core (models, scoring, persistence, output, http, browser, cache)
- `sources/` — un module par source (`wttj`, `hellowork`, `apec`, `jobteaser`, `gov`, `pure_players`)
- `fixtures/` — données de test pour dev offline
- `cache/` — cache HTTP 24h (auto-créé, gitignored)
- `candidatures.json` — base SQLite-like en JSON (auto-créée, gitignored)

## Notes

- **WTTJ / pure-players** : sélecteurs DOM susceptibles de changer. Vérifier `scraper.log` si une source remonte 0 offre. Sélecteurs centralisés en haut de chaque module dans `sources/`.
- **JobTeaser** : stub. Auth SSO 42 requise, génère juste un rappel quotidien dans l'output.
- **Sandbox HTTPS** : `verify=False` dans `src/http.py` pour tolérer les MITM. À retirer en prod si tu peux.
- **Profil** : `PROFILE.md` contient le filtrage perso (mots-clés requis/exclus, préférences de score). À éditer librement.
