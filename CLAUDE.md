# Cyber Offensive Job Tracker

> Workflow simple : Claude scanne le web à la demande, filtre selon `PROFILE.md`, te donne les meilleures offres, et tient le suivi de tes candidatures dans `candidatures.md`.

## Comment l'utiliser

### "Alors du nouveau ?"
Claude :
1. Lance plusieurs WebSearch ciblées (pentest / red team / reverse / exploit / ANSSI / pure-players)
2. Filtre selon ton profil (`PROFILE.md`) : Paris/IDF prio, stage 6 mois, offensif uniquement
3. Déduplique avec `vu.json` (n'affiche que les **nouvelles** offres)
4. Te présente top 5-10 avec : titre, boîte, lien, pourquoi ça matche
5. Met à jour `vu.json`

### Suivre tes candidatures
- « j'ai postulé chez Synacktiv le 28/04 » → Claude ajoute à `candidatures.md`
- « rejet Akerva » → Claude marque comme rejet
- « entretien le 5/05 chez Quarkslab » → Claude met à jour
- « où j'en suis ? » → Claude lit `candidatures.md` et résume

## Règles de filtrage (utilisées par Claude)

**Garde si** : titre/desc contient `pentest`, `red team`, `offensive`, `exploit`, `reverse`, `vulnerability research`, `binary exploitation`, `tool development offensif`.

**Écarte si** : `SOC analyst`, `blue team only`, `gouvernance`, `compliance`, `RGPD`, `audit ISO 27001`, `helpdesk`, `niveau 1/2`.

**Bonus prio** :
- Boîte dans Tier 1 pure-players (Synacktiv, Quarkslab, Ambionics, Lexfo, Akerva)
- Lieu Paris/IDF (sinon malus)
- Stack mentionnée match (C/C++, Python, Linux, Bash, Assembly, gdb/Ghidra)
- Stage 6 mois explicite

## Sources scannées à chaque "alors du nouveau ?"

- WTTJ (via Google search `site:welcometothejungle.com`)
- HelloWork, APEC, Indeed, Glassdoor (idem)
- Pages carrière directes : Synacktiv, Quarkslab, Lexfo, Akerva, Almond, Ambionics, HarfangLab, Tehtris, Intrinsec, YesWeHack, Yogosha
- Sites gov : COMCYBER, ANSSI, choisirleservicepublic.gouv.fr
- JobTeaser 42 (manuel — auth SSO requise, Claude te le rappelle)

## Fallback : code Python (dossier `cyber-scraper/`)

Si tu veux un jour automatiser sans dépendre de Claude (cron, GitHub Actions), le scraper Python complet est dans `cyber-scraper/`. Setup dans son `README.md`.

Sinon — ignore le, le workflow Claude+WebSearch est plus simple et plus fiable.
