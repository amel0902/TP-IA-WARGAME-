#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NB_LIGNES 10
#define NB_COLONNES 10
#define INFINI 10000
#define PROFONDEUR_IA 3

/*#define DEBUG*/

typedef struct pion_s
{
    int couleur;
    int valeur;
} Pion;

Pion *plateauDeJeu;

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
    if(c>='A' && c<='Z')
        return (int)(c-'A');
    if(c>='a' && c<='z')
        return (int)(c-'a');
    return -1;
}

char f_convert_int2char(int i)
{
#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    return (char)i+'A';
}

Pion *f_init_plateau()
{
    int i, j;
    Pion *plateau=NULL;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    plateau = (Pion *)malloc(NB_LIGNES*NB_COLONNES*sizeof(Pion));
    if(plateau == NULL)
    {
        printf("error: unable to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    for(i=0; i<NB_LIGNES; i++)
    {
        for(j=0; j<NB_COLONNES; j++)
        {
            plateau[i*NB_COLONNES+j].couleur = 0;
            plateau[i*NB_COLONNES+j].valeur = 0;
        }
    }

    plateau[9*NB_COLONNES+5].couleur = 1;
    plateau[9*NB_COLONNES+5].valeur = 1;

    plateau[9*NB_COLONNES+6].couleur = 1;
    plateau[9*NB_COLONNES+6].valeur = 2;

    plateau[9*NB_COLONNES+7].couleur = 1;
    plateau[9*NB_COLONNES+7].valeur = 3;

    plateau[9*NB_COLONNES+8].couleur = 1;
    plateau[9*NB_COLONNES+8].valeur = 2;

    plateau[9*NB_COLONNES+9].couleur = 1;
    plateau[9*NB_COLONNES+9].valeur = 1;

    plateau[8*NB_COLONNES+0].couleur = 1;
    plateau[8*NB_COLONNES+0].valeur = 1;

    plateau[8*NB_COLONNES+1].couleur = 1;
    plateau[8*NB_COLONNES+1].valeur = 3;

    plateau[8*NB_COLONNES+2].couleur = 1;
    plateau[8*NB_COLONNES+2].valeur = 3;

    plateau[8*NB_COLONNES+3].couleur = 1;
    plateau[8*NB_COLONNES+3].valeur = 1;

    plateau[8*NB_COLONNES+6].couleur = 1;
    plateau[8*NB_COLONNES+6].valeur = 1;

    plateau[8*NB_COLONNES+7].couleur = 1;
    plateau[8*NB_COLONNES+7].valeur = 1;

    plateau[8*NB_COLONNES+8].couleur = 1;
    plateau[8*NB_COLONNES+8].valeur = 1;

    plateau[7*NB_COLONNES+1].couleur = 1;
    plateau[7*NB_COLONNES+1].valeur = 1;

    plateau[7*NB_COLONNES+2].couleur = 1;
    plateau[7*NB_COLONNES+2].valeur = 1;

    plateau[2*NB_COLONNES+7].couleur = -1;
    plateau[2*NB_COLONNES+7].valeur = 1;

    plateau[2*NB_COLONNES+8].couleur = -1;
    plateau[2*NB_COLONNES+8].valeur = 1;

    plateau[1*NB_COLONNES+1].couleur = -1;
    plateau[1*NB_COLONNES+1].valeur = 1;

    plateau[1*NB_COLONNES+2].couleur = -1;
    plateau[1*NB_COLONNES+2].valeur = 1;

    plateau[1*NB_COLONNES+3].couleur = -1;
    plateau[1*NB_COLONNES+3].valeur = 1;

    plateau[1*NB_COLONNES+6].couleur = -1;
    plateau[1*NB_COLONNES+6].valeur = 1;

    plateau[1*NB_COLONNES+7].couleur = -1;
    plateau[1*NB_COLONNES+7].valeur = 3;

    plateau[1*NB_COLONNES+8].couleur = -1;
    plateau[1*NB_COLONNES+8].valeur = 3;

    plateau[1*NB_COLONNES+9].couleur = -1;
    plateau[1*NB_COLONNES+9].valeur = 1;

    plateau[0*NB_COLONNES+0].couleur = -1;
    plateau[0*NB_COLONNES+0].valeur = 1;

    plateau[0*NB_COLONNES+1].couleur = -1;
    plateau[0*NB_COLONNES+1].valeur = 2;

    plateau[0*NB_COLONNES+2].couleur = -1;
    plateau[0*NB_COLONNES+2].valeur = 3;

    plateau[0*NB_COLONNES+3].couleur = -1;
    plateau[0*NB_COLONNES+3].valeur = 2;

    plateau[0*NB_COLONNES+4].couleur = -1;
    plateau[0*NB_COLONNES+4].valeur = 1;

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif

    return plateau;
}

void f_affiche_plateau(Pion *plateau)
{
    int i,j,k;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    printf("\n    ");
    for(k=0; k<NB_COLONNES; k++)
        printf("%2c ",f_convert_int2char(k));
    printf("\n    ");
    for(k=0; k<NB_COLONNES; k++)
        printf("-- ");
    printf("\n");
    for(i=NB_LIGNES-1; i>=0; i--)
    {
        printf("%2d ", i);
        for(j=0; j<NB_COLONNES; j++)
        {
            printf("|");
            switch(plateau[i*NB_COLONNES+j].couleur)
            {
            case -1:
                printf("%do",plateau[i*NB_COLONNES+j].valeur);
                break;
            case 1:
                printf("%dx",plateau[i*NB_COLONNES+j].valeur);
                break;
            default:
                printf("  ");
            }
        }
        printf("|\n    ");
        for(k=0; k<NB_COLONNES; k++)
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
    int i, j, somme1=0, somme2=0;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    /* Quelqu'un est-il arrive sur la ligne de l'autre */
    for(i=0; i<NB_COLONNES; i++)
    {
        if(plateauDeJeu[i].couleur == 1)
            return 1;
        if(plateauDeJeu[(NB_LIGNES-1)*NB_COLONNES+i].couleur == -1)
            return -1;
    }

    /* taille des armees */
    for(i=0; i<NB_LIGNES; i++)
    {
        for(j=0; j<NB_COLONNES; j++)
        {
            if(plateauDeJeu[i*NB_COLONNES+j].couleur == 1)
                somme1++;
            if(plateauDeJeu[i*NB_COLONNES+j].couleur == -1)
                somme2++;
        }
    }
    if(somme1==0)
        return -1;
    if(somme2==0)
        return 1;

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
    return 0;
}

int f_bataille(int l, int c)
{
    int i, j, mini, maxi, minj, maxj;
    int somme=0;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
    mini = l-1<0?0:l-1;
    maxi = l+1>NB_LIGNES-1?NB_LIGNES-1:l+1;
    minj = c-1<0?0:c-1;
    maxj = c+1>NB_COLONNES-1?NB_COLONNES-1:c+1;

    for(i=mini; i<=maxi; i++)
    {
        for(j=minj; j<=maxj; j++)
        {
            somme += plateauDeJeu[i*NB_COLONNES+j].couleur*plateauDeJeu[i*NB_COLONNES+j].valeur;
        }
    }
    somme -= plateauDeJeu[l*NB_COLONNES+c].couleur*plateauDeJeu[l*NB_COLONNES+c].valeur;

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
    if(somme < 0)
        return -1;
    if(somme > 0)
        return 1;

    return plateauDeJeu[l*NB_COLONNES+c].couleur;
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
    if(plateau[l1*NB_COLONNES+c1].valeur == 0 || plateau[l1*NB_COLONNES+c1].couleur != couleur)
        return 1;
    if(plateau[l2*NB_COLONNES+c2].couleur == plateau[l1*NB_COLONNES+c1].couleur)
        return 1;

    if(l1-l2 >1 || l2-l1 >1 || c1-c2 >1 || c2-c1 >1 || (l1==l2 && c1==c2))
        return 1;
#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
    return 0;
}

int f_bouge_piece(Pion *plateau, int l1, int c1, int l2, int c2, int couleur)
{
    int gagnant=0;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    if(f_test_mouvement(plateau, l1, c1, l2, c2, couleur) != 0)
        return 1;

    if(plateau[l2*NB_COLONNES+c2].valeur == 0)
    {
        plateau[l2*NB_COLONNES+c2].couleur = plateau[l1*NB_COLONNES+c1].couleur;
        plateau[l2*NB_COLONNES+c2].valeur = plateau[l1*NB_COLONNES+c1].valeur;
        plateau[l1*NB_COLONNES+c1].couleur = 0;
        plateau[l1*NB_COLONNES+c1].valeur = 0;
    }
    else
    {
        gagnant=f_bataille(l2, c2);
        if(gagnant == couleur)
        {
            plateau[l2*NB_COLONNES+c2].couleur = plateau[l1*NB_COLONNES+c1].couleur;
            plateau[l2*NB_COLONNES+c2].valeur = plateau[l1*NB_COLONNES+c1].valeur;
            plateau[l1*NB_COLONNES+c1].couleur = 0;
            plateau[l1*NB_COLONNES+c1].valeur = 0;
        }
        else if(gagnant != 0)
        {
            plateau[l1*NB_COLONNES+c1].couleur = 0;
            plateau[l1*NB_COLONNES+c1].valeur = 0;
        }
    }

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
    return 0;
}

int f_nbPions(Pion* jeu, int joueur)
{
    int nbPion=0;
    int i, j;
    for (i = 0; i < NB_COLONNES; ++i)
    {
        for (j = 0; j < NB_LIGNES; ++j)
        {
            if (jeu[i * NB_COLONNES + j].couleur == joueur)
            {
                ++nbPion;
            }
        }
    }
    return nbPion;
}

int f_valeur(Pion* jeu, int joueur)
{
    int i, j;
    int valeur=0;
    for (i = 0; i < NB_COLONNES; ++i)
    {
        for (j = 0; j < NB_LIGNES; ++j)
        {
            if (jeu[i * NB_COLONNES + j].couleur == joueur)
            {
                valeur += jeu[i * NB_COLONNES + j].valeur;
            }
        }
    }
    return valeur;
}

int f_eval(Pion* jeu, int joueur)
{
    int i, j;
    int score = 0;
    int progression;

    for(i=0; i<NB_LIGNES; i++)
    {
        for(j=0; j<NB_COLONNES; j++)
        {
            if(jeu[i*NB_COLONNES+j].couleur == joueur)
            {
                progression = joueur == 1 ? i : NB_LIGNES-1-i;
                score += 100 * jeu[i*NB_COLONNES+j].valeur;
                score += progression * jeu[i*NB_COLONNES+j].valeur;
            }
            else if(jeu[i*NB_COLONNES+j].couleur == -joueur)
            {
                progression = joueur == 1 ? NB_LIGNES-1-i : i;
                score -= 100 * jeu[i*NB_COLONNES+j].valeur;
                score -= progression * jeu[i*NB_COLONNES+j].valeur;
            }
        }
    }

    return score;
}

void f_copie_plateau(Pion* source, Pion* destination)
{
    int i, j;
    for (i = 0; i < NB_LIGNES; i++)
    {
        for (j = 0; j < NB_COLONNES; j++)
        {
            destination[i * NB_COLONNES + j].couleur = source[i * NB_COLONNES + j].couleur;
            destination[i * NB_COLONNES + j].valeur = source[i * NB_COLONNES + j].valeur;
        }
    }
}

Pion* f_raz_plateau()
{
    Pion* jeu = NULL;
    int i, j;
    jeu = (Pion *) malloc(NB_LIGNES * NB_COLONNES * sizeof (Pion));
    if(jeu == NULL)
    {
        fprintf(stderr, "error: unable to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < NB_LIGNES; i++)
    {
        for (j = 0; j < NB_COLONNES; j++)
        {
            jeu[i * NB_COLONNES + j].couleur = 0;
            jeu[i * NB_COLONNES + j].valeur = 0;
        }
    }
    return jeu;
}

static int f_gagnant_plateau(Pion *jeu)
{
    int i, j;
    int joueur1 = 0, joueur2 = 0;

    for(i=0; i<NB_LIGNES; i++)
    {
        for(j=0; j<NB_COLONNES; j++)
        {
            if(jeu[i*NB_COLONNES+j].couleur == 1)
                joueur1++;
            else if(jeu[i*NB_COLONNES+j].couleur == -1)
                joueur2++;
        }
    }

    for(j=0; j<NB_COLONNES; j++)
    {
        if(jeu[j].couleur == -1)
            return -1;
        if(jeu[(NB_LIGNES-1)*NB_COLONNES+j].couleur == 1)
            return 1;
    }

    if(joueur1 == 0)
        return -1;
    if(joueur2 == 0)
        return 1;
    return 0;
}

static int f_joue_coup(Pion *source, Pion *destination,
    int l1, int c1, int l2, int c2, int joueur)
{
    Pion *ancien_plateau = plateauDeJeu;
    int resultat;

    f_copie_plateau(source, destination);
    plateauDeJeu = destination;
    resultat = f_bouge_piece(destination, l1, c1, l2, c2, joueur);
    plateauDeJeu = ancien_plateau;
    return resultat;
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
    int coupure = 0;
    Pion *fils;
    int gagnant = f_gagnant_plateau(jeu);

    if(gagnant != 0)
        return gagnant == joueur ? INFINI + profondeur : -INFINI - profondeur;
    if(profondeur == 0)
        return f_eval(jeu, joueur);

    fils = f_raz_plateau();

    for(l1=0; l1<NB_LIGNES && !coupure; l1++)
    {
        for(c1=0; c1<NB_COLONNES && !coupure; c1++)
        {
            if(jeu[l1*NB_COLONNES+c1].couleur != (maximisant ? joueur : -joueur))
                continue;

            for(l2=l1-1; l2<=l1+1 && !coupure; l2++)
            {
                for(c2=c1-1; c2<=c1+1 && !coupure; c2++)
                {
                    if(f_joue_coup(jeu, fils, l1, c1, l2, c2,
                        maximisant ? joueur : -joueur) != 0)
                        continue;

                    if(maximisant)
                    {
                        valeur = f_min(fils, joueur, profondeur-1, alpha, beta);
                        if(valeur > alpha)
                            alpha = valeur;
                        if(alpha >= beta)
                            coupure = 1;
                    }
                    else
                    {
                        valeur = f_max(fils, joueur, profondeur-1, alpha, beta);
                        if(valeur < beta)
                            beta = valeur;
                        if(beta <= alpha)
                            coupure = 1;
                    }
                }
            }
        }
    }

    free(fils);
    return maximisant ? alpha : beta;
}

void f_IA(int joueur)
{
    int l1, c1, l2, c2;
    int score;
    int alpha = -INFINI;
    int beta = INFINI;
    int meilleur_l1 = -1, meilleur_c1 = -1;
    int meilleur_l2 = -1, meilleur_c2 = -1;
    Pion *fils = f_raz_plateau();
    Pion *ancien_plateau = plateauDeJeu;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    for(l1=0; l1<NB_LIGNES; l1++)
    {
        for(c1=0; c1<NB_COLONNES; c1++)
        {
            if(plateauDeJeu[l1*NB_COLONNES+c1].couleur != joueur)
                continue;

            for(l2=l1-1; l2<=l1+1; l2++)
            {
                for(c2=c1-1; c2<=c1+1; c2++)
                {
                    if(f_joue_coup(ancien_plateau, fils, l1, c1, l2, c2, joueur) != 0)
                        continue;

                    score = f_min(fils, joueur, PROFONDEUR_IA-1, alpha, beta);
                    if(score > alpha)
                    {
                        alpha = score;
                        meilleur_l1 = l1;
                        meilleur_c1 = c1;
                        meilleur_l2 = l2;
                        meilleur_c2 = c2;
                    }
                }
            }
        }
    }

    if(meilleur_l1 >= 0)
        f_bouge_piece(ancien_plateau, meilleur_l1, meilleur_c1,
            meilleur_l2, meilleur_c2, joueur);
    free(fils);

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
}

void f_humain(int joueur)
{
    char c1, c2;
    char buffer[32];
    int l1, l2;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    printf("joueur ");
    switch(joueur)
    {
    case -1:
        printf("o ");
        break;
    case 1:
        printf("x ");
        break;
    default:
        printf("inconnu ");
    }
    printf("joue:\n");
    while(1)
    {
        fgets(buffer, 32, stdin);
        if(sscanf(buffer, "%c%i%c%i\n", &c1, &l1, &c2, &l2) == 4)
        {
            if(f_bouge_piece(plateauDeJeu, l1, f_convert_char2int(c1), l2, f_convert_char2int(c2), joueur) == 0)
                break;
        }
        fflush(stdin);
        printf("mauvais choix\n");
    }

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
}

int main(int argv, char *argc[])
{
    int fin = 0, mode = 0, ret, joueur = 1;
    (void)argv;
    (void)argc;
    printf("1 humain vs IA\n2 humain vs humain\n3 IA vs IA\n");
    scanf("%d", &mode);

    plateauDeJeu = f_init_plateau();
    while (!fin)
    {
        f_affiche_plateau(plateauDeJeu);
        if(mode==1)
        {
            if(joueur>0)
                f_humain(joueur);
            else
                f_IA(joueur);
        }
        else if(mode==2)
        {
            f_humain(joueur);
        }
        else
        {
            f_IA(joueur);
        }

        if ((ret = f_gagnant()) != 0)
        {
            switch (ret)
            {
            case 1:
                f_affiche_plateau(plateauDeJeu);
                printf("joueur x gagne!\n");
                fin = 1;
                break;
            case -1:
                f_affiche_plateau(plateauDeJeu);
                printf("joueur o gagne!\n");
                fin = 1;
                break;
            }
        }
        joueur = -joueur;
    }

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif

    return 0;
}