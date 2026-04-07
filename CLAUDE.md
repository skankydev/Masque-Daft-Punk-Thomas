# CLAUDE.md — Module/Matrix

## Contexte
Projet pour un ami — contrôleur de matrice LED WS2812B monté sur un masque style Daft Punk (Thomas).
Piloté via une app mobile Bluetooth BLE.

## Cible hardware
- **Carte** : ESP32 dev board classic
- **LEDs** : WS2812B, matrice 10×7 (70 LEDs), serpentin option A
  - Ligne paire : gauche → droite
  - Ligne impaire : droite → gauche
- **Pin LED** : variable (défini dans `setting.h`, ne pas hardcoder ailleurs)
- **Communication** : BLE (pas de WiFi, pas de MQTT)

## Objectif final
Reproduire l'affichage du masque de Thomas Bangalter (Daft Punk) — animations sur matrice LED, contrôlées depuis une app mobile via BLE. La matrice finale sera probablement plus grande que 10×7.

## Conventions de code
- **Code** : anglais (noms de classes, méthodes, variables)
- **Commentaires** : français
- **Pas de tests** pour l'instant

## Architecture
- `setting.h` — constantes globales : `MATRIX_W`, `MATRIX_H`, `PIN_LEDS`. Scalable, tout se base dessus.
- `effects/Effect.h` — classe de base abstraite + fonction libre `XY(x, y)` pour le mapping serpentin
- Une classe par effet dans `effects/` — chaque effet gère son propre état interne
- `LedManager` — singleton, gère le timing (`_speed` en ms), instancie les effets à la demande via le registre `EFFETS[]` (factory pattern). Un seul effet en mémoire à la fois.
- `Terminal` — CLI série pour le debug et les tests

## Registre des effets
Ajouter un effet = **une ligne dans `EFFETS[]`** dans `LedManager.cpp`, rien d'autre.
```cpp
{ "MonEffet", []() -> Effect* { return new EffetMonEffet(); } },
```

## Ce qui est hors scope
- WiFi, MQTT, persistance — volontairement retirés
- Texte défilant — prévu plus tard, pas maintenant

## Règles de collaboration
- Si une demande est ambiguë ou qu'un choix archi a plusieurs options valables → discuter avant de coder
- Relire les fichiers avant d'éditer — Simon modifie souvent le code entre deux messages
