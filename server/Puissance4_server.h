#ifndef PUISSANCE4_SERVER_H
#define PUISSANCE4_SERVER_H

#include <netinet/in.h>

#define MSG_LEN_MAX 256

// Variables globales pour la taille de la grille
extern int LARG;
extern int HAUT;

typedef struct joueur_s {
    int sockfd;
    int numero;
    struct sockaddr_in addr;
    char login[17];
    char pion; // 'x' ou 'o'
    int etat;  // 0=connecté, 1=loggé, 2=pret, 3=en jeu
    int veut_rejouer; // 0 = pas répondu, 1 = oui, -1 = non
    struct joueur_s *next;
} joueur_t;

void init_grille(char **grille, int haut, int larg);
int login_unique(joueur_t *joueurs, const char *login);
void grille_to_str(char **grille, char *out, int haut, int larg);
int jouer_coup(char **grille, int col, char pion, int haut, int larg);
int check_victoire(char **grille, int ligne, int col, char pion, int haut, int larg);
int grille_pleine(char **grille, int haut, int larg);
void send_all(joueur_t *joueurs, const char *msg);
joueur_t *trouve_par_pion(joueur_t *joueurs, char pion);

#endif // PUISSANCE4_SERVER_H
