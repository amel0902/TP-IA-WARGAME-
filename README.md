# TP-IA-War-Game

TP1 d'Intelligence Artificielle (M1 SIC). Jeu de plateau 10×10 en C, avec une IA

## Compilation et exécution

```bash
gcc -std=c99 -O2 -Wall -Wextra -o wargame wargame_patron.c
./wargame
```

Le code est aussi compatible C89 (`-std=c89 -pedantic`).

## Modes de jeu

| Choix | Mode |
| --- | --- |
| 1 | Humain (x) contre IA (o) |
| 2 | Humain contre humain |
| 3 | IA contre IA |

Un coup se saisit sous la forme `F9E8` : colonne et ligne de départ, puis d'arrivée.
Les x partent en haut du plateau et visent la ligne 0 ; les o visent la ligne 9.
On gagne en atteignant la ligne adverse ou en éliminant toutes les pièces adverses.
En mode 3, la partie est déclarée nulle après 400 coups.

## Options de compilation

| Option | Effet |
| --- | --- |
| `-DMINIMAX_PUR` | désactive les coupures alpha-bêta (pour comparer les temps) |
| `PROFONDEUR_IA` (dans le code) | profondeur de recherche, fixée à 3 pour le tournoi |
| `AFFICHE_STATS` (dans le code) | affiche nœuds, coupures et temps après chaque coup de l'IA |

## Benchmark

```bash
chmod +x run.sh
./run.sh            # utilise wargame_patron.c
./run.sh autre.c    # ou un autre fichier source
```

Le script compile les versions alpha-bêta et minimax pur, joue une partie IA contre IA
complète avec chacune, puis affiche le nombre de nœuds, le temps CPU, le coup le plus long
et les coupures. Il vérifie aussi que les deux versions jouent les mêmes coups.

Résultats obtenus (104 coups, `gcc -O2`) :

| Mesure | Alpha-bêta | Minimax pur |
| --- | --- | --- |
| Nœuds visités | 2 624 494 | 18 276 597 |
| Temps CPU cumulé | 1,27 s | 8,87 s |

Les temps absolus dépendent de la machine ; le rapport (≈ ÷7) reste stable.

## Structure du code

- `Pion`, `Coup`, `StatsIA` : structures de données (pion, coup avec score, statistiques).
- `pool_plateaux` : un plateau statique par niveau de profondeur, sans allocation pendant la recherche.
- `f_minimax_alphabeta` : recherche unique, appelée par `f_min` et `f_max`.
- `f_eval` : matériel, avancée, proximité de la ligne adverse, soutien des voisins.
- `f_IA` : choisit et joue le meilleur coup, affiche les statistiques.
