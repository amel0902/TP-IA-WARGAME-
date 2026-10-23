#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NB_LIGNES 10
#define NB_COLONNES 10
#define INFINI 100000          /* valeur d'une victoire (+ profondeur restante) */
#define BORNE 1000000          /* strictement > INFINI + profondeur : sentinelle alpha/beta */
#define PROFONDEUR_IA 3        /* fixee a 3 pour le tournoi */
#define MAX_TOURS 400          /* garde-fou IA vs IA (partie nulle) */
#define AFFICHE_STATS 1

/* Compiler avec -DMINIMAX_PUR pour desactiver les coupures (mesures du rapport) */

typedef struct pion_s
{
    int couleur;
    int valeur;
} Pion;

typedef struct coup_s
{
    int l1, c1;
    int l2, c2;
    int score;
} Coup;

typedef struct stats_s
{
    long noeuds_visites;
    long coupures_alpha;
    long coupures_beta;
    double temps_cpu;
} StatsIA;

Pion *plateauDeJeu;
StatsIA g_stats;

/* Un plateau par niveau de profondeur : aucun malloc pendant la recherche */
static Pion pool_plateaux[PROFONDEUR_IA + 1][NB_LIGNES * NB_COLONNES];

void f_affiche_plateau(Pion *plateau);
int f_convert_char2int(char c);
char f_convert_int2char(int i);
int f_min(Pion *jeu, int joueur, int profondeur, int alpha, int beta);
int f_max(Pion *jeu, int joueur, int profondeur, int alpha, int beta);

int f_convert_char2int(char c)
{
    if(c >= 'A' && c <= 'Z')
        return (int)(c - 'A');
    if(c >= 'a' && c <= 'z')
        return (int)(c - 'a');
    return -1;
}

char f_convert_int2char(int i)
{
    return (char)(i + 'A');
}

Pion *f_init_plateau()
{
    /* Plateau initial du sujet : {ligne, colonne, couleur, valeur}
       couleur : 1 = joueur x, -1 = joueur o ; 0 = case vide */
    static const int init[][4] = {
        {9,5, 1,1},{9,6, 1,2},{9,7, 1,3},{9,8, 1,2},{9,9, 1,1},
        {8,0, 1,1},{8,1, 1,3},{8,2, 1,3},{8,3, 1,1},{8,6, 1,1},{8,7, 1,1},{8,8, 1,1},
        {7,1, 1,1},{7,2, 1,1},
        {2,7,-1,1},{2,8,-1,1},
        {1,1,-1,1},{1,2,-1,1},{1,3,-1,1},{1,6,-1,1},{1,7,-1,3},{1,8,-1,3},{1,9,-1,1},
        {0,0,-1,1},{0,1,-1,2},{0,2,-1,3},{0,3,-1,2},{0,4,-1,1}
    };
    int i;
    Pion *plateau = (Pion *)calloc(NB_LIGNES * NB_COLONNES, sizeof(Pion));
    if(plateau == NULL)
    {
        fprintf(stderr, "error: unable to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    for(i = 0; i < (int)(sizeof(init) / sizeof(init[0])); i++)
    {
        plateau[init[i][0] * NB_COLONNES + init[i][1]].couleur = init[i][2];
        plateau[init[i][0] * NB_COLONNES + init[i][1]].valeur = init[i][3];
    }
    return plateau;
}

void f_affiche_plateau(Pion *plateau)
{
    int i, j, k;

    printf("\n    ");
    for(k = 0; k < NB_COLONNES; k++)
        printf("%2c ", f_convert_int2char(k));
    printf("\n    ");
    for(k = 0; k < NB_COLONNES; k++)
        printf("-- ");
    printf("\n");
    for(i = NB_LIGNES - 1; i >= 0; i--)
    {
        printf("%2d ", i);
        for(j = 0; j < NB_COLONNES; j++)
        {
            printf("|");
            switch(plateau[i * NB_COLONNES + j].couleur)
            {
            case -1:
                printf("%do", plateau[i * NB_COLONNES + j].valeur);
                break;
            case 1:
                printf("%dx", plateau[i * NB_COLONNES + j].valeur);
                break;
            default:
                printf("  ");
            }
        }
        printf("|\n    ");
        for(k = 0; k < NB_COLONNES; k++)
            printf("-- ");
        printf("\n");
    }
    printf("    ");
}

/* Gagnant d'un plateau quelconque : 1, -1 ou 0 */
static int f_gagnant_plateau(Pion *jeu)
{
    int i, j;
    int joueur1 = 0, joueur2 = 0;

    for(j = 0; j < NB_COLONNES; j++)
    {
        if(jeu[j].couleur == 1)
            return 1;
        if(jeu[(NB_LIGNES - 1) * NB_COLONNES + j].couleur == -1)
            return -1;
    }
    for(i = 0; i < NB_LIGNES; i++)
    {
        for(j = 0; j < NB_COLONNES; j++)
        {
            if(jeu[i * NB_COLONNES + j].couleur == 1)
                joueur1++;
            else if(jeu[i * NB_COLONNES + j].couleur == -1)
                joueur2++;
        }
    }
    if(joueur1 == 0)
        return -1;
    if(joueur2 == 0)
        return 1;
    return 0;
}

/* Gagnant du plateau de jeu courant : 1, -1 ou 0 */
int f_gagnant()
{
    return f_gagnant_plateau(plateauDeJeu);
}

/* Bataille sur la case (l,c) du plateau global plateauDeJeu */
int f_bataille(int l, int c)
{
    int i, j, mini, maxi, minj, maxj;
    int somme = 0;

    mini = l - 1 < 0 ? 0 : l - 1;
    maxi = l + 1 > NB_LIGNES - 1 ? NB_LIGNES - 1 : l + 1;
    minj = c - 1 < 0 ? 0 : c - 1;
    maxj = c + 1 > NB_COLONNES - 1 ? NB_COLONNES - 1 : c + 1;

    for(i = mini; i <= maxi; i++)
        for(j = minj; j <= maxj; j++)
            somme += plateauDeJeu[i * NB_COLONNES + j].couleur * plateauDeJeu[i * NB_COLONNES + j].valeur;
    somme -= plateauDeJeu[l * NB_COLONNES + c].couleur * plateauDeJeu[l * NB_COLONNES + c].valeur;

    if(somme < 0)
        return -1;
    if(somme > 0)
        return 1;
    return plateauDeJeu[l * NB_COLONNES + c].couleur;
}

/* Renvoie 1 si le mouvement est invalide, 0 sinon */
int f_test_mouvement(Pion *plateau, int l1, int c1, int l2, int c2, int couleur)
{
    if(l1 < 0 || l1 >= NB_LIGNES || l2 < 0 || l2 >= NB_LIGNES ||
       c1 < 0 || c1 >= NB_COLONNES || c2 < 0 || c2 >= NB_COLONNES)
        return 1;
    if(plateau[l1 * NB_COLONNES + c1].valeur == 0 || plateau[l1 * NB_COLONNES + c1].couleur != couleur)
        return 1;
    if(plateau[l2 * NB_COLONNES + c2].couleur == plateau[l1 * NB_COLONNES + c1].couleur)
        return 1;
    if(l1 - l2 > 1 || l2 - l1 > 1 || c1 - c2 > 1 || c2 - c1 > 1 || (l1 == l2 && c1 == c2))
        return 1;
    return 0;
}

/* Renvoie 1 en cas d'erreur, 0 sinon.
 * ATTENTION : f_bataille lit le plateau global ; plateau doit etre plateauDeJeu
 * (f_joue_coup se charge de le rediriger pour la recherche). */
int f_bouge_piece(Pion *plateau, int l1, int c1, int l2, int c2, int couleur)
{
    int gagnant = 0;

    if(f_test_mouvement(plateau, l1, c1, l2, c2, couleur) != 0)
        return 1;

    if(plateau[l2 * NB_COLONNES + c2].valeur == 0)
    {
        plateau[l2 * NB_COLONNES + c2] = plateau[l1 * NB_COLONNES + c1];
        plateau[l1 * NB_COLONNES + c1].couleur = 0;
        plateau[l1 * NB_COLONNES + c1].valeur = 0;
    }
    else
    {
        gagnant = f_bataille(l2, c2);
        if(gagnant == couleur)
        {
            plateau[l2 * NB_COLONNES + c2] = plateau[l1 * NB_COLONNES + c1];
            plateau[l1 * NB_COLONNES + c1].couleur = 0;
            plateau[l1 * NB_COLONNES + c1].valeur = 0;
        }
        else if(gagnant != 0)
        {
            plateau[l1 * NB_COLONNES + c1].couleur = 0;
            plateau[l1 * NB_COLONNES + c1].valeur = 0;
        }
    }
    return 0;
}

void f_copie_plateau(Pion *source, Pion *destination)
{
    memcpy(destination, source, NB_LIGNES * NB_COLONNES * sizeof(Pion));
}

/* Nombre de pions d'un joueur */
int f_nbPions(Pion *jeu, int joueur)
{
    int i, j, nb = 0;
    for(i = 0; i < NB_LIGNES; i++)
        for(j = 0; j < NB_COLONNES; j++)
            if(jeu[i * NB_COLONNES + j].couleur == joueur)
                nb++;
    return nb;
}

/* Somme des valeurs des pions d'un joueur */
int f_valeur(Pion *jeu, int joueur)
{
    int i, j, valeur = 0;
    for(i = 0; i < NB_LIGNES; i++)
        for(j = 0; j < NB_COLONNES; j++)
            if(jeu[i * NB_COLONNES + j].couleur == joueur)
                valeur += jeu[i * NB_COLONNES + j].valeur;
    return valeur;
}

/* Plateau vide (a liberer avec free) */
Pion *f_raz_plateau()
{
    Pion *jeu = (Pion *)calloc(NB_LIGNES * NB_COLONNES, sizeof(Pion));
    if(jeu == NULL)
    {
        fprintf(stderr, "error: unable to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    return jeu;
}

/* Copie source dans destination puis joue le coup sur destination.
 * Le mouvement doit avoir ete valide avant (f_test_mouvement). */
static int f_joue_coup(Pion *source, Pion *destination, int l1, int c1, int l2, int c2, int joueur)
{
    Pion *ancien_plateau = plateauDeJeu;
    int resultat;

    f_copie_plateau(source, destination);
    plateauDeJeu = destination;
    resultat = f_bouge_piece(destination, l1, c1, l2, c2, joueur);
    plateauDeJeu = ancien_plateau;
    return resultat;
}

/* Evaluation du point de vue de 'joueur' : materiel, avancee, proximite du but, soutien */
int f_eval(Pion *jeu, int joueur)
{
    int i, j, di, dj, ni, nj;
    int score = 0;
    int couleur, dist_but, poids;

    for(i = 0; i < NB_LIGNES; i++)
    {
        for(j = 0; j < NB_COLONNES; j++)
        {
            couleur = jeu[i * NB_COLONNES + j].couleur;
            if(couleur == 0)
                continue;

            dist_but = (couleur == 1) ? i : (NB_LIGNES - 1 - i);
            poids = 200 * jeu[i * NB_COLONNES + j].valeur + ((NB_LIGNES - 1) - dist_but) * 25;

            if(dist_but == 1)
                poids += 500;

            for(di = -1; di <= 1; di++)
            {
                for(dj = -1; dj <= 1; dj++)
                {
                    if(di == 0 && dj == 0)
                        continue;
                    ni = i + di;
                    nj = j + dj;
                    if(ni >= 0 && ni < NB_LIGNES && nj >= 0 && nj < NB_COLONNES &&
                       jeu[ni * NB_COLONNES + nj].couleur == couleur)
                        poids += 8 * jeu[ni * NB_COLONNES + nj].valeur;
                }
            }

            score += (couleur == joueur) ? poids : -poids;
        }
    }
    return score;
}

static int f_minimax_alphabeta(Pion *jeu, int joueur, int profondeur, int alpha, int beta, int maximisant);

int f_min(Pion *jeu, int joueur, int profondeur, int alpha, int beta)
{
    return f_minimax_alphabeta(jeu, joueur, profondeur, alpha, beta, 0);
}

int f_max(Pion *jeu, int joueur, int profondeur, int alpha, int beta)
{
    return f_minimax_alphabeta(jeu, joueur, profondeur, alpha, beta, 1);
}

/* Minimax avec coupures alpha-beta (variante fail-soft).
 * 'joueur' = camp pour lequel l'IA calcule ; maximisant = 1 si c'est a lui de jouer. */
static int f_minimax_alphabeta(Pion *jeu, int joueur, int profondeur, int alpha, int beta, int maximisant)
{
    int l1, c1, l2, c2, valeur;
    int camp_actuel = maximisant ? joueur : -joueur;
    int meilleur = maximisant ? -BORNE : BORNE;
    int coup_trouve = 0;
    int gagnant = f_gagnant_plateau(jeu);
    Pion *fils = pool_plateaux[profondeur];

    g_stats.noeuds_visites++;

    if(gagnant != 0)
        return gagnant == joueur ? (INFINI + profondeur) : (-INFINI - profondeur);
    if(profondeur == 0)
        return f_eval(jeu, joueur);

    for(l1 = 0; l1 < NB_LIGNES; l1++)
    {
        for(c1 = 0; c1 < NB_COLONNES; c1++)
        {
            if(jeu[l1 * NB_COLONNES + c1].couleur != camp_actuel)
                continue;

            for(l2 = l1 - 1; l2 <= l1 + 1; l2++)
            {
                for(c2 = c1 - 1; c2 <= c1 + 1; c2++)
                {
                    /* on valide AVANT de copier le plateau */
                    if(f_test_mouvement(jeu, l1, c1, l2, c2, camp_actuel) != 0)
                        continue;
                    f_joue_coup(jeu, fils, l1, c1, l2, c2, camp_actuel);
                    coup_trouve = 1;

                    if(maximisant)
                    {
                        valeur = f_min(fils, joueur, profondeur - 1, alpha, beta);
                        if(valeur > meilleur)
                            meilleur = valeur;
                        if(meilleur > alpha)
                            alpha = meilleur;
#ifndef MINIMAX_PUR
                        if(alpha >= beta)
                        {
                            g_stats.coupures_beta++;
                            return meilleur;
                        }
#endif
                    }
                    else
                    {
                        valeur = f_max(fils, joueur, profondeur - 1, alpha, beta);
                        if(valeur < meilleur)
                            meilleur = valeur;
                        if(meilleur < beta)
                            beta = meilleur;
#ifndef MINIMAX_PUR
                        if(beta <= alpha)
                        {
                            g_stats.coupures_alpha++;
                            return meilleur;
                        }
#endif
                    }
                }
            }
        }
    }

    if(!coup_trouve)                 /* camp bloque : on evalue la position */
        return f_eval(jeu, joueur);
    return meilleur;
}

/* Calcule et joue le meilleur coup pour 'joueur' */
void f_IA(int joueur)
{
    int l1, c1, l2, c2, score;
    int alpha = -BORNE, beta = BORNE;
    clock_t debut, fin;
    Pion *fils = pool_plateaux[PROFONDEUR_IA];
    Coup meilleur_coup = {-1, -1, -1, -1, -BORNE};

    g_stats.noeuds_visites = 0;
    g_stats.coupures_alpha = 0;
    g_stats.coupures_beta = 0;
    debut = clock();

    for(l1 = 0; l1 < NB_LIGNES; l1++)
    {
        for(c1 = 0; c1 < NB_COLONNES; c1++)
        {
            if(plateauDeJeu[l1 * NB_COLONNES + c1].couleur != joueur)
                continue;

            for(l2 = l1 - 1; l2 <= l1 + 1; l2++)
            {
                for(c2 = c1 - 1; c2 <= c1 + 1; c2++)
                {
                    if(f_test_mouvement(plateauDeJeu, l1, c1, l2, c2, joueur) != 0)
                        continue;
                    f_joue_coup(plateauDeJeu, fils, l1, c1, l2, c2, joueur);

                    score = f_min(fils, joueur, PROFONDEUR_IA - 1, alpha, beta);

                    /* le 1er coup legal est toujours retenu (coup de secours) */
                    if(meilleur_coup.l1 == -1 || score > meilleur_coup.score)
                    {
                        meilleur_coup.l1 = l1;
                        meilleur_coup.c1 = c1;
                        meilleur_coup.l2 = l2;
                        meilleur_coup.c2 = c2;
                        meilleur_coup.score = score;
                    }
                    if(score > alpha)
                        alpha = score;
                }
            }
        }
    }

    fin = clock();
    g_stats.temps_cpu = ((double)(fin - debut)) / CLOCKS_PER_SEC;

#if AFFICHE_STATS
    printf("\n--- Stats IA (%s) ---\n", joueur == 1 ? "x" : "o");
    if(meilleur_coup.l1 >= 0)
        printf("Coup joue : %c%d -> %c%d (score %d)\n",
               f_convert_int2char(meilleur_coup.c1), meilleur_coup.l1,
               f_convert_int2char(meilleur_coup.c2), meilleur_coup.l2, meilleur_coup.score);
    printf("Noeuds visites : %ld\n", g_stats.noeuds_visites);
    printf("Coupures Alpha : %ld | Beta : %ld\n", g_stats.coupures_alpha, g_stats.coupures_beta);
    printf("Temps de calcul : %.3f s\n--------------------\n", g_stats.temps_cpu);
#endif

    if(meilleur_coup.l1 >= 0)
        f_bouge_piece(plateauDeJeu, meilleur_coup.l1, meilleur_coup.c1,
                      meilleur_coup.l2, meilleur_coup.c2, joueur);
}

void f_humain(int joueur)
{
    char c1, c2;
    char buffer[64];
    int l1, l2;

    printf("joueur %s joue (ex: F9E8) : ", joueur == 1 ? "x" : "o");
    while(1)
    {
        if(fgets(buffer, sizeof(buffer), stdin) == NULL)   /* EOF : on quitte, pas de boucle infinie */
        {
            printf("\nFin de l'entree, abandon.\n");
            exit(EXIT_SUCCESS);
        }
        if(sscanf(buffer, " %c%d%c%d", &c1, &l1, &c2, &l2) == 4)
        {
            if(f_bouge_piece(plateauDeJeu, l1, f_convert_char2int(c1), l2, f_convert_char2int(c2), joueur) == 0)
                break;
        }
        printf("Coup invalide, recommencez : ");
    }
}

int main(void)
{
    int fin = 0, mode = 0, ret, joueur = 1, tours = 0, c;

    printf("1. Humain vs IA\n2. Humain vs Humain\n3. IA vs IA\nChoix : ");
    if(scanf("%d", &mode) != 1)
        return 1;
    while((c = getchar()) != '\n' && c != EOF)
        ;

    plateauDeJeu = f_init_plateau();
    while(!fin)
    {
        f_affiche_plateau(plateauDeJeu);
        if(mode == 1)
        {
            if(joueur > 0)
                f_humain(joueur);
            else
                f_IA(joueur);
        }
        else if(mode == 2)
            f_humain(joueur);
        else
            f_IA(joueur);

        if((ret = f_gagnant()) != 0)
        {
            f_affiche_plateau(plateauDeJeu);
            printf("\n>>> Le joueur %s a gagne ! <<<\n", ret == 1 ? "x" : "o");
            fin = 1;
        }
        else if(mode == 3 && ++tours >= MAX_TOURS)
        {
            f_affiche_plateau(plateauDeJeu);
            printf("\n>>> Partie nulle (%d coups) <<<\n", MAX_TOURS);
            fin = 1;
        }
        joueur = -joueur;
    }

    free(plateauDeJeu);
    return 0;
}