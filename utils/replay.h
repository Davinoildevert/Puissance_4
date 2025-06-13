// replay.h
#ifndef REPLAY_H
#define REPLAY_H

#include "../server/Puissance4_server.h"

// Envoie la demande de replay à tous les joueurs loggés

void envoyer_demande_replay(joueur_t *joueurs);
void reset_replay_etat(joueur_t *joueurs);
int traiter_reponse_replay(joueur_t **pjoueurs, joueur_t *repondeur, int reponse, char **grille, int haut, int larg);

// À compléter : autres fonctions utiles pour le flux replay

#endif // REPLAY_H
