# Matrix LED — Masque Daft Punk

Contrôleur de matrice LED WS2812B pour masque style Thomas Bangalter (Daft Punk).  
Piloté via une app mobile Bluetooth BLE.

> App mobile : [Daft-Punk-LED-Controller](https://github.com/skankydev/Daft-Punk-LED-Controller)

---

## Hardware

| Composant | Détail |
|-----------|--------|
| Carte | ESP32-S3-N16R8 |
| LEDs | WS2812B flexible 32×8 (256 LEDs) |
| Pin LEDs | GPIO 15 (défini dans `setting.h`) |
| Communication | BLE (pas de WiFi) |

**Serpentin vertical** : colonne paire haut→bas, colonne impaire bas→haut.

---

## Stack

- **FastLED** — pilotage WS2812B
- **BLE Arduino** — communication mobile
- **PlatformIO** + framework Arduino

---

## Structure

```
src/
├── setting.h           — constantes globales (MATRIX_W, MATRIX_H, PIN_LEDS)
├── LedManager.cpp/.h   — singleton LEDs, registre des effets, timing
├── effects/
│   ├── Effect.h        — classe de base + XY() mapping serpentin
│   └── Effet*.h        — un fichier par effet
├── ble/
│   └── MyBle.cpp/.h    — singleton BLE, queue de commandes
├── Terminal.cpp/.h     — CLI série debug
└── function.cpp/.h     — utilitaires (formatage, couleurs terminal, sysinfo)

archive/                — effets désactivés (hors compilation)
partitions.csv          — partition table 16MB custom
```

---

## Effets disponibles

| # | Nom | Couleur | Texte |
|---|-----|---------|-------|
| 0 | Rainbow | — | — |
| 1 | Text | ✓ | ✓ |
| 2 | Fire | ✓ | — |
| 3 | VisorFire | — | — |
| 4 | Rain | ✓ | — |
| 5 | Matrix | ✓ | — |
| 6 | Fireworks | — | — |
| 7 | Scanner | ✓ | — |
| 8 | Cylon | ✓ | — |
| 9 | HeartBeat | ✓ | — |
| 10 | Bounce | ✓ | — |
| 11 | GameOfLife | ✓ | — |
| 12 | PacmanGame | — | — |
| 13 | Sauron | — | — |
| 14 | Kawaii | ✓ | — |

### Ajouter un effet

Une seule ligne dans `EFFETS[]` dans `LedManager.cpp` :
```cpp
{ "MonEffet", []() -> Effect* { return new EffetMonEffet(); }, 50, "#FF0000", nullptr },
```

---

## Protocol BLE

**Device** : `Daft Punk`  
**MTU** : une caractéristique READ/WRITE/NOTIFY

### Commandes (WRITE)

```
setEffectFull:idx|speed|color|text   — tout en une fois (color et text optionnels)
setEffect:idx                        — changer d'effet
setSpeed:ms                          — vitesse en ms
setBrightness:0-255                  — luminosité
setColor:FF0000                      — couleur hex sans #
setText:mon texte                    — texte (si supporté par l'effet)
next                                 — effet suivant
default                              — reset général
getEffects                           — demande la liste des effets
REBOOT                               — redémarre l'ESP32
```

### Récupérer la liste des effets

Envoyer `getEffects` → reçoit `{"count":N}` puis N notifications JSON :
```json
{"idx":0,"name":"Rainbow","speed":50}
{"idx":1,"name":"Text","speed":80,"color":"#FF0000","text":"Daft Punk"}
```

---

## Partitions (16MB)

| Partition | Taille | Usage |
|-----------|--------|-------|
| nvs | 20 KB | config persistante |
| otadata | 8 KB | OTA metadata |
| app0 | 6 MB | firmware actif |
| app1 | 6 MB | firmware OTA |
| spiffs | 3.87 MB | stockage fichiers |
| coredump | 64 KB | debug crash |
