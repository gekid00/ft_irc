# cyber-scraper

Agrégateur de stages cybersécurité offensive en France. Scrape WTTJ, HelloWork, APEC, choisirleservicepublic.gouv.fr, et les pages carrière des pure-players (Synacktiv, Almond, Akerva, Lexfo, Wavestone, Quarkslab, etc.). Filtre + score selon `PROFILE.md`. Suivi de candidatures local en JSON.

## Setup

```bash
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
- **Sandbox HTTPS** : `verify=False` dans `src/http.py` pour tolérer les MITM de certains environnements. À retirer en prod si tu peux.
- **Profil** : `PROFILE.md` contient le filtrage perso (mots-clés requis/exclus, préférences de score). À éditer librement.

## Extraction en repo standalone

Le code vit dans `cyber-scraper/` du repo `ft_irc` (limitation du scope MCP au moment du développement). Pour le sortir en repo indépendant :

```bash
git fetch origin claude/cybersec-job-scraper-Mxt06
git checkout origin/claude/cybersec-job-scraper-Mxt06 -- cyber-scraper/
mv cyber-scraper ~/projets/cyber-scraper
cd ~/projets/cyber-scraper
git init && git add . && git commit -m "init from ft_irc"
# puis créer le repo privé sur GitHub via UI et :
git remote add origin git@github.com:gekid00/cyber-scraper.git
git push -u origin main
```
