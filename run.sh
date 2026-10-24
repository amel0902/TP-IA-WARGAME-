#!/bin/sh
# Compare alpha-beta et minimax pur sur une partie complete IA vs IA.
# Usage : ./run.sh [fichier.c]   (defaut : wargame_patron.c)

SRC="${1:-wargame_patron.c}"
OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

if [ ! -f "$SRC" ]; then
    echo "fichier introuvable : $SRC" >&2
    exit 1
fi

gcc -O2 -Wall -Wextra -o "$OUT/ab" "$SRC" || exit 1
gcc -O2 -Wall -Wextra -DMINIMAX_PUR -o "$OUT/pur" "$SRC" || exit 1

stats()
{
    awk -v nom="$1" '
        /Noeuds visites/  { n += $4; k++ }
        /Temps de calcul/ { t += $5; if ($5 > m) m = $5 }
        /Coupures Alpha/  { a += $4; b += $8 }
        /gagne|nulle/     { fin = $0 }
        END {
            printf "%-12s coups=%d noeuds=%d noeuds/coup=%.0f cpu=%.3fs max_coup=%.3fs coupures(a/b)=%d/%d\n",
                   nom, k, n, n / k, t, m, a, b
            print "             " fin
        }' "$2"
}

echo "Partie IA vs IA complete (profondeur 3)..."
echo 3 | "$OUT/ab"  > "$OUT/ab.txt"
echo 3 | "$OUT/pur" > "$OUT/pur.txt"

stats "alpha-beta"  "$OUT/ab.txt"
stats "minimax pur" "$OUT/pur.txt"

if diff -q "$OUT/ab.txt" "$OUT/pur.txt" >/dev/null 2>&1 ||
   [ "$(grep 'Coup joue' "$OUT/ab.txt")" = "$(grep 'Coup joue' "$OUT/pur.txt")" ]; then
    echo "Memes coups joues dans les deux versions : les coupures ne changent pas le choix."
else
    echo "ATTENTION : les coups joues different entre les deux versions." >&2
    exit 1
fi
