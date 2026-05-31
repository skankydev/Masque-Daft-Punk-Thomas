
# Masque Daft Punk Thomas

Contrôleur de matrice LED WS2812B pour masque style Thomas Bangalter (Daft Punk).  
Piloté via une app mobile Bluetooth BLE. Micro INMP441 pour les effets audio-réactifs.

> App mobile : [Daft-Punk-LED-Controller](https://github.com/skankydev/Daft-Punk-LED-Controller)

---

## Hardware

### Carte & communication

| Composant | Détail |
|-----------|--------|
| Carte | ESP32-S3-N16R8 |
| Communication | BLE (pas de WiFi) |

### Matrice principale

| Composant | Pin | Détail |
|-----------|-----|--------|
| Matrice LED | GPIO 15 | WS2812B flexible 32×8 (256 LEDs) |

**Serpentin vertical** : colonne paire haut→bas, colonne impaire bas→haut.

### Bandes latérales — "oreilles" et bas

4 strips WS2812B de 10 LEDs chacune (40 LEDs au total) :

| Strip | Pin | Position |
|-------|-----|----------|
| `STRIP_RIGHT_TOP` | GPIO 11 | oreille droite |
| `STRIP_RIGHT_BOT` | GPIO 12 | bande basse droite |
| `STRIP_LEFT_TOP` | GPIO 17 | oreille gauche |
| `STRIP_LEFT_BOT` | GPIO 18 | bande basse gauche |

Chaque effet peut overrider `stepStripsTop()` et `stepStripsBot()` pour les animer.

### Micro INMP441 (I2S)

| Signal | Pin |
|--------|-----|
| SCK (bit clock) | GPIO 41 |
| WS (word select) | GPIO 42 |
| SD (data) | GPIO 2 |

Sample rate 22050 Hz, FFT 512 samples → 32 bandes log de 100 Hz à 8 kHz.

### Alimentation

Deux alimentations séparées :
- **ESP32** : USB ou 5V via Vin
- **LEDs** : alim 5V externe dédiée . GND commun avec l'ESP32 obligatoire.
--  ⚠️  à pleine puissance les 296 LEDs peuvent tirer jusqu'à ~18A pic théorique (60mA × 296). En usage musical à brightness modérée (10/255 par défaut), on reste sous 3A. Prévoir une alim 5V externe suffisante.

---

## Stack

- **FastLED** — pilotage WS2812B
- **arduinoFFT** — analyse spectrale du micro
- **BLE Arduino** — communication mobile
- **PlatformIO** + framework Arduino

---

## Structure

```
src/
├── setting.h           — constantes globales (pins, MATRIX_W, MATRIX_H, mic)
├── LedManager.cpp/.h   — singleton LEDs, registre effets, timing, mode auto random
├── effects/
│   ├── Effect.h        — classe de base + XY() serpentin + audioColor() helper
│   └── Effet*.h        — un fichier par effet
├── mic/
│   └── MicManager.cpp/.h — singleton micro I2S, FFT, beat detection, centroid spectral
├── ble/
│   └── MyBle.cpp/.h    — singleton BLE, queue de commandes
├── Terminal.cpp/.h     — CLI série debug
└── function.cpp/.h     — utilitaires (formatage, couleurs terminal, sysinfo)

archive/                — effets désactivés (hors compilation)
partitions.csv          — partition table 16MB custom
```

---

## Effets disponibles

| # | Nom | Couleur | Texte | Audio |
|---|-----|---------|-------|-------|
| 0 | Rainbow | — | — | — |
| 1 | Text | ✓ | ✓ | — |
| 2 | Fire | ✓ | — | — |
| 3 | VisorFire | — | — | — |
| 4 | Rain | ✓ | — | — |
| 5 | Matrix | ✓ | — | — |
| 6 | Fireworks | — | — | ✓ (beat) |
| 7 | Scanner | ✓ | — | — |
| 8 | Cylon | ✓ | — | — |
| 9 | HeartBeat | ✓ | — | — |
| 10 | Bounce | ✓ | — | — |
| 11 | GameOfLife | ✓ | — | — |
| 12 | PacmanGame | — | — | — |
| 13 | Sauron | — | — | — |
| 14 | Kawaii | ✓ | — | — |
| 15 | Audio | ✓ | — | ✓ (spectre) |
| 16 | Audio2 | — | — | ✓ (spectre) |
| 17 | Audio3 | ✓ | — | ✓ (spectre) |
| 18 | Audio4 | — | — | ✓ (spectre + couleur réactive) |
| 19 | Waterfall | ✓ | — | ✓ (spectrogramme défilant) |
| 20 | Particles | ✓ | — | ✓ (explosions sur beat) |

### Services audio (disponibles pour tous les effets)

- `MicManager::getBand(i)` — 32 bandes spectrales lissées [0, 1]
- `MicManager::getBass()` / `getTreble()` — moyennes basses/aigus
- `MicManager::getBeatLevel()` — 0→1, pulse à 1 sur beat puis retombe
- `MicManager::getSpectralCentroid()` — 0 (basses) → 1 (aigus)
- `audioColor(centroid)` (fonction libre dans `Effect.h`) — palette HSV rouge→vert→bleu→violet

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
setSens:0.1-5.0                      — sensibilité micro (défaut 1.0)
setRandom:0-30                       — mode random en secondes (0 = OFF)
next                                 — effet suivant
default                              — reset général
getEffects                           — demande la liste des effets
REBOOT                               — redémarre l'ESP32
```

### Mode random

`setRandom:N` avec N en secondes :
- `0` = désactivé
- `1-30` = change d'effet toutes les N secondes, avec couleur HSV aléatoire si l'effet supporte `setColor()`

### Récupérer la liste des effets

Envoyer `getEffects` → reçoit `{"count":N}` puis N notifications JSON :
```json
{"idx":0,"name":"Rainbow","speed":50}
{"idx":1,"name":"Text","speed":80,"color":"#FF0000","text":"Daft Punk"}
```

Les champs `color` et `text` ne sont présents que si l'effet les supporte.

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
