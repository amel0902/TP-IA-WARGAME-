#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NB_LIGNES 10
#define NB_COLONNES 10
#define INFINI 100000
#define PROFONDEUR_IA 3

/*#define DEBUG*/

typedef struct pion_s
{
    int couleur;
    int valeur;
} Pion;

/* Structure pour encapsuler la definition et le score d'un coup */
typedef struct coup_s
{
    int l1, c1;
    int l2, c2;
    int score;
} Coup;

/* Structure de statistiques pour le rapport et le profilage */
typedef struct stats_s
{
    long noeuds_visites;
    long coupures_alpha;
    long coupures_beta;
    double temps_cpu;
} StatsIA;

Pion *plateauDeJeu;
StatsIA g_stats;

/* Buffers memoire statiques par profondeur pour eviter les malloc/free repetitifs */
static Pion pool_plateaux[PROFONDEUR_IA + 1][NB_LIGNES * NB_COLONNES];

void f_affiche_plateau(Pion *plateau);
int f_convert_char2int(char c);
char f_convert_int2char(int i);
int f_min(Pion *jeu, int joueur, int profondeur, int alpha, int beta);
int f_max(Pion *jeu, int joueur, int profondeur, int alpha, int beta);

int f_convert_char2int(char c)
{
#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
    if(c >= 'A' && c <= 'Z')
        return (int)(c - 'A');
    if(c >= 'a' && c <= 'z')
        return (int)(c - 'a');
    return -1;
}

char f_convert_int2char(int i)
{
#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
    return (char)(i + 'A');
}

Pion *f_init_plateau()
{
    int i, j;
    Pion *plateau = NULL;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    plateau = (Pion *)malloc(NB_LIGNES * NB_COLONNES * sizeof(Pion));
    if(plateau == NULL)
    {
        fprintf(stderr, "error: unable to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < NB_LIGNES; i++)
    {
        for(j = 0; j < NB_COLONNES; j++)
        {
            plateau[i * NB_COLONNES + j].couleur = 0;
            plateau[i * NB_COLONNES + j].valeur = 0;
        }
    }

    plateau[9 * NB_COLONNES + 5].couleur = 1;
    plateau[9 * NB_COLONNES + 5].valeur = 1;
    plateau[9 * NB_COLONNES + 6].couleur = 1;
    plateau[9 * NB_COLONNES + 6].valeur = 2;
    plateau[9 * NB_COLONNES + 7].couleur = 1;
    plateau[9 * NB_COLONNES + 7].valeur = 3;
    plateau[9 * NB_COLONNES + 8].couleur = 1;
    plateau[9 * NB_COLONNES + 8].valeur = 2;
    plateau[9 * NB_COLONNES + 9].couleur = 1;
    plateau[9 * NB_COLONNES + 9].valeur = 1;

    plateau[8 * NB_COLONNES + 0].couleur = 1;
    plateau[8 * NB_COLONNES + 0].valeur = 1;
    plateau[8 * NB_COLONNES + 1].couleur = 1;
    plateau[8 * NB_COLONNES + 1].valeur = 3;
    plateau[8 * NB_COLONNES + 2].couleur = 1;
    plateau[8 * NB_COLONNES + 2].valeur = 3;
    plateau[8 * NB_COLONNES + 3].couleur = 1;
    plateau[8 * NB_COLONNES + 3].valeur = 1;
    plateau[8 * NB_COLONNES + 6].couleur = 1;
    plateau[8 * NB_COLONNES + 6].valeur = 1;
    plateau[8 * NB_COLONNES + 7].couleur = 1;
    plateau[8 * NB_COLONNES + 7].valeur = 1;
    plateau[8 * NB_COLONNES + 8].couleur = 1;
    plateau[8 * NB_COLONNES + 8].valeur = 1;

    plateau[7 * NB_COLONNES + 1].couleur = 1;
    plateau[7 * NB_COLONNES + 1].valeur = 1;
    plateau[7 * NB_COLONNES + 2].couleur = 1;
    plateau[7 * NB_COLONNES + 2].valeur = 1;

    plateau[2 * NB_COLONNES + 7].couleur = -1;
    plateau[2 * NB_COLONNES + 7].valeur = 1;
    plateau[2 * NB_COLONNES + 8].couleur = -1;
    plateau[2 * NB_COLONNES + 8].valeur = 1;

    plateau[1 * NB_COLONNES + 1].couleur = -1;
    plateau[1 * NB_COLONNES + 1].valeur = 1;
    plateau[1 * NB_COLONNES + 2].couleur = -1;
    plateau[1 * NB_COLONNES + 2].valeur = 1;
    plateau[1 * NB_COLONNES + 3].couleur = -1;
    plateau[1 * NB_COLONNES + 3].valeur = 1;
    plateau[1 * NB_COLONNES + 6].couleur = -1;
    plateau[1 * NB_COLONNES + 6].valeur = 1;
    plateau[1 * NB_COLONNES + 7].couleur = -1;
    plateau[1 * NB_COLONNES + 7].valeur = 3;
    plateau[1 * NB_COLONNES + 8].couleur = -1;
    plateau[1 * NB_COLONNES + 8].valeur = 3;
    plateau[1 * NB_COLONNES + 9].couleur = -1;
    plateau[1 * NB_COLONNES + 9].valeur = 1;

    plateau[0 * NB_COLONNES + 0].couleur = -1;
    plateau[0 * NB_COLONNES + 0].valeur = 1;
    plateau[0 * NB_COLONNES + 1].couleur = -1;
    plateau[0 * NB_COLONNES + 1].valeur = 2;
    plateau[0 * NB_COLONNES + 2].couleur = -1;
    plateau[0 * NB_COLONNES + 2].valeur = 3;
    plateau[0 * NB_COLONNES + 3].couleur = -1;
    plateau[0 * NB_COLONNES + 3].valeur = 2;
    plateau[0 * NB_COLONNES + 4].couleur = -1;
    plateau[0 * NB_COLONNES + 4].valeur = 1;

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
    return plateau;
}

void f_affiche_plateau(Pion *plateau)
{
    int i, j, k;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
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

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
}

int f_gagnant()
{
    int i, j, somme1 = 0, somme2 = 0;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
    for(i = 0; i < NB_COLONNES; i++)
    {
        if(plateauDeJeu[i].couleur == 1)
            return 1;
        if(plateauDeJeu[(NB_LIGNES - 1) * NB_COLONNES + i].couleur == -1)
            return -1;
    }

    for(i = 0; i < NB_LIGNES; i++)
    {
        for(j = 0; j < NB_COLONNES; j++)
        {
            if(plateauDeJeu[i * NB_COLONNES + j].couleur == 1)
                somme1++;
            if(plateauDeJeu[i * NB_COLONNES + j].couleur == -1)
                somme2++;
        }
    }
    if(somme1 == 0)
        return -1;
    if(somme2 == 0)
        return 1;

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
    return 0;
}

int f_bataille(int l, int c)
{
    int i, j, mini, maxi, minj, maxj;
    int somme = 0;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
    mini = l - 1 < 0 ? 0 : l - 1;
    maxi = l + 1 > NB_LIGNES - 1 ? NB_LIGNES - 1 : l + 1;
    minj = c - 1 < 0 ? 0 : c - 1;
    maxj = c + 1 > NB_COLONNES - 1 ? NB_COLONNES - 1 : c + 1;

    for(i = mini; i <= maxi; i++)
    {
        for(j = minj; j <= maxj; j++)
        {
            somme += plateauDeJeu[i * NB_COLONNES + j].couleur * plateauDeJeu[i * NB_COLONNES + j].valeur;
        }
    }
    somme -= plateauDeJeu[l * NB_COLONNES + c].couleur * plateauDeJeu[l * NB_COLONNES + c].valeur;

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
    if(somme < 0)
        return -1;
    if(somme > 0)
        return 1;

    return plateauDeJeu[l * NB_COLONNES + c].couleur;
}

int f_test_mouvement(Pion *plateau, int l1, int c1, int l2, int c2, int couleur)
{
#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
    printf("de (%d,%d) vers (%d,%d)\n", l1, c1, l2, c2);
#endif
    if(l1 < 0 || l1 >= NB_LIGNES || l2 < 0 || l2 >= NB_LIGNES ||
       c1 < 0 || c1 >= NB_COLONNES || c2 < 0 || c2 >= NB_COLONNES)
        return 1;
    if(plateau[l1 * NB_COLONNES + c1].valeur == 0 || plateau[l1 * NB_COLONNES + c1].couleur != couleur)
        return 1;
    if(plateau[l2 * NB_COLONNES + c2].couleur == plateau[l1 * NB_COLONNES + c1].couleur)
        return 1;

    if(l1 - l2 > 1 || l2 - l1 > 1 || c1 - c2 > 1 || c2 - c1 > 1 || (l1 == l2 && c1 == c2))
        return 1;
#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
    return 0;
}

int f_bouge_piece(Pion *plateau, int l1, int c1, int l2, int c2, int couleur)
{
    int gagnant = 0;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
    if(f_test_mouvement(plateau, l1, c1, l2, c2, couleur) != 0)
        return 1;

    if(plateau[l2 * NB_COLONNES + c2].valeur == 0)
    {
        plateau[l2 * NB_COLONNES + c2].couleur = plateau[l1 * NB_COLONNES + c1].couleur;
        plateau[l2 * NB_COLONNES + c2].valeur = plateau[l1 * NB_COLONNES + c1].valeur;
        plateau[l1 * NB_COLONNES + c1].couleur = 0;
        plateau[l1 * NB_COLONNES + c1].valeur = 0;
    }
    else
    {
        gagnant = f_bataille(l2, c2);
        if(gagnant == couleur)
        {
            plateau[l2 * NB_COLONNES + c2].couleur = plateau[l1 * NB_COLONNES + c1].couleur;
            plateau[l2 * NB_COLONNES + c2].valeur = plateau[l1 * NB_COLONNES + c1].valeur;
            plateau[l1 * NB_COLONNES + c1].couleur = 0;
            plateau[l1 * NB_COLONNES + c1].valeur = 0;
        }
        else if(gagnant != 0)
        {
            plateau[l1 * NB_COLONNES + c1].couleur = 0;
            plateau[l1 * NB_COLONNES + c1].valeur = 0;
        }
    }

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
    return 0;
}

void f_copie_plateau(Pion *source, Pion *destination)
{
    memcpy(destination, source, NB_LIGNES * NB_COLONNES * sizeof(Pion));
}

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

/* Evaluation strategique prenant en compte materiel, distance et soutien tactique */
int f_eval(Pion *jeu, int joueur)
{
    int i, j, di, dj;
    int score = 0;
    int progression;
    int c_adv, dist_but;

    for(i = 0; i < NB_LIGNES; i++)
    {
        for(j = 0; j < NB_COLONNES; j++)
        {
            c_adv = jeu[i * NB_COLONNES + j].couleur;
            if(c_adv == 0)
                continue;

            dist_but = (c_adv == 1) ? i : (NB_LIGNES - 1 - i);
            progression = (NB_LIGNES - 1) - dist_but;

            /* Bonus materiel et avancee */
            int poids = 200 * jeu[i * NB_COLONNES + j].valeur + progression * 25;

            /* Bonus de proximite immédiate de la ligne adverse */
            if(dist_but == 1)
                poids += 500;

            /* Bonus de soutien local pour les batailles */
            for(di = -1; di <= 1; di++)
            {
                for(dj = -1; dj <= 1; dj++)
                {
                    if(di == 0 && dj == 0)
                        continue;
                    int ni = i + di;
                    int nj = j + dj;
                    if(ni >= 0 && ni < NB_LIGNES && nj >= 0 && nj < NB_COLONNES)
                    {
                        if(jeu[ni * NB_COLONNES + nj].couleur == c_adv)
                            poids += 8 * jeu[ni * NB_COLONNES + nj].valeur;
                    }
                }
            }

            if(c_adv == joueur)
                score += poids;
            else
                score -= poids;
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

static int f_minimax_alphabeta(Pion *jeu, int joueur, int profondeur, int alpha, int beta, int maximisant)
{
    int l1, c1, l2, c2;
    int valeur;
    int gagnant = f_gagnant_plateau(jeu);
    Pion *fils = pool_plateaux[profondeur];

    g_stats.noeuds_visites++;

    if(gagnant != 0)
        return gagnant == joueur ? (INFINI + profondeur) : (-INFINI - profondeur);

    if(profondeur == 0)
        return f_eval(jeu, joueur);

    int camp_actuel = maximisant ? joueur : -joueur;

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
                    if(f_joue_coup(jeu, fils, l1, c1, l2, c2, camp_actuel) != 0)
                        continue;

                    if(maximisant)
                    {
                        valeur = f_min(fils, joueur, profondeur - 1, alpha, beta);
                        if(valeur > alpha)
                            alpha = valeur;
                        if(alpha >= beta)
                        {
                            g_stats.coupures_beta++;
                            return alpha;
                        }
                    }
                    else
                    {
                        valeur = f_max(fils, joueur, profondeur - 1, alpha, beta);
                        if(valeur < beta)
                            beta = valeur;
                        if(beta <= alpha)
                        {
                            g_stats.coupures_alpha++;
                            return beta;
                        }
                    }
                }
            }
        }
    }
    return maximisant ? alpha : beta;
}

void f_IA(int joueur)
{
    int l1, c1, l2, c2;
    int score;
    int alpha = -INFINI;
    int beta = INFINI;
    clock_t debut, fin;
    Pion *ancien_plateau = plateauDeJeu;
    Pion *fils = pool_plateaux[PROFONDEUR_IA];

    Coup meilleur_coup;
    meilleur_coup.l1 = -1;
    meilleur_coup.c1 = -1;
    meilleur_coup.l2 = -1;
    meilleur_coup.c2 = -1;
    meilleur_coup.score = -INFINI;

    g_stats.noeuds_visites = 0;
    g_stats.coupures_alpha = 0;
    g_stats.coupures_beta = 0;
    debut = clock();

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

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
                    if(f_joue_coup(ancien_plateau, fils, l1, c1, l2, c2, joueur) != 0)
                        continue;

                    /* Coup par defaut de securite si aucun ne depasse -INFINI */
                    if(meilleur_coup.l1 == -1)
                    {
                        meilleur_coup.l1 = l1;
                        meilleur_coup.c1 = c1;
                        meilleur_coup.l2 = l2;
                        meilleur_coup.c2 = c2;
                    }

                    score = f_min(fils, joueur, PROFONDEUR_IA - 1, alpha, beta);
                    if(score > alpha)
                    {
                        alpha = score;
                        meilleur_coup.score = score;
                        meilleur_coup.l1 = l1;
                        meilleur_coup.c1 = c1;
                        meilleur_coup.l2 = l2;
                        meilleur_coup.c2 = c2;
                    }
                }
            }
        }
    }

    fin = clock();
    g_stats.temps_cpu = ((double)(fin - debut)) / CLOCKS_PER_SEC;

    printf("\n--- Stats IA (%s) ---\n", joueur == 1 ? "x" : "o");
    printf("Coup joue : %c%d -> %c%d\n",
           f_convert_int2char(meilleur_coup.c1), meilleur_coup.l1,
           f_convert_int2char(meilleur_coup.c2), meilleur_coup.l2);
    printf("Noeuds visites : %ld\n", g_stats.noeuds_visites);
    printf("Coupures Alpha : %ld | Beta : %ld\n", g_stats.coupures_alpha, g_stats.coupures_beta);
    printf("Temps de calcul : %.3f s\n--------------------\n", g_stats.temps_cpu);

    if(meilleur_coup.l1 >= 0)
    {
        f_bouge_piece(ancien_plateau, meilleur_coup.l1, meilleur_coup.c1,
                      meilleur_coup.l2, meilleur_coup.c2, joueur);
    }

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
}

void f_humain(int joueur)
{
    char c1, c2;
    char buffer[64];
    int l1, l2;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
    printf("joueur %s joue (ex: B8B7) : ", joueur == 1 ? "x" : "o");
    while(1)
    {
        if(fgets(buffer, sizeof(buffer), stdin) == NULL)
            continue;

        if(sscanf(buffer, " %c%d%c%d", &c1, &l1, &c2, &l2) == 4)
        {
            int col1 = f_convert_char2int(c1);
            int col2 = f_convert_char2int(c2);
            if(f_bouge_piece(plateauDeJeu, l1, col1, l2, col2, joueur) == 0)
                break;
        }
        printf("Coup invalide, recommencez : ");
    }

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
}

int main(int argc, char *argv[])
{
    int fin = 0, mode = 0, ret, joueur = 1;
    (void)argc;
    (void)argv;

    printf("1. Humain vs IA\n2. Humain vs Humain\n3. IA vs IA\nChoix : ");
    if(scanf("%d", &mode) != 1)
        return 1;

    /* Vidage du buffer d'entree apres le scanf */
    while(getchar() != '\n');

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
        {
            f_humain(joueur);
        }
        else
        {
            f_IA(joueur);
        }

        if((ret = f_gagnant()) != 0)
        {
            f_affiche_plateau(plateauDeJeu);
            printf("\n>>> Le joueur %s a gagne ! <<<\n", ret == 1 ? "x" : "o");
            fin = 1;
        }
        joueur = -joueur;
    }

    free(plateauDeJeu);
    return 0;
}