// replay.c
// Module de gestion du flux replay pour Puissance 4
#include "replay.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// Exemple de structure pour stocker l'état du replay (à adapter selon besoin)
void envoyer_demande_replay(joueur_t *joueurs) {
    joueur_t *j = joueurs;
    while (j) {
        if (j->etat == 1) {
            write(j->sockfd, "/info REPLAY:Voulez-vous rejouer ? (o/n)\n", strlen("/info REPLAY:Voulez-vous rejouer ? (o/n)\n"));
        }
        j = j->next;
    }
}

void reset_replay_etat(joueur_t *joueurs) {
    joueur_t *j = joueurs;
    while (j) {
        j->veut_rejouer = 0;
        j = j->next;
    }
}

// Traite la réponse d'un joueur au replay
// Retourne :
// 0 = attente d'autres réponses
// 1 = relancer partie (2 oui)
// 2 = un non, l'autre oui (attente nouvel adversaire)
// 3 = les deux non (vider la liste)
// 4 = un seul joueur restant (attente nouvel adversaire)
int traiter_reponse_replay(joueur_t **pjoueurs, joueur_t *repondeur, int reponse, char **grille, int haut, int larg) {
    // reponse: 1 = oui, -1 = non
    repondeur->veut_rejouer = reponse;
    int total = 0, ok = 0, nok = 0;
    joueur_t *j = *pjoueurs;
    while (j) {
        if (j->etat == 1) {
            total++;
            if (j->veut_rejouer == 1) ok++;
            if (j->veut_rejouer == -1) nok++;
        }
        j = j->next;
    }
    // On attend que les deux aient répondu
    if (ok + nok < 2) {
        return 0; // On attend encore
    }
    if (ok == 2) {
        // Les deux veulent rejouer
        reset_replay_etat(*pjoueurs);
        if (grille) {
            // Réinitialise la grille
            for (int i = 0; i < haut; i++)
                for (int k = 0; k < larg; k++)
                    grille[i][k] = '_';
        }
        return 1;
    } else if (nok > 0) {
        // Au moins un joueur a refusé
        // Déconnecter et retirer tous les "non"
        joueur_t *j = *pjoueurs, *prev = NULL;
        while (j) {
            if (j->etat == 1 && j->veut_rejouer == -1) {
                close(j->sockfd);
                if (prev == NULL) {
                    *pjoueurs = j->next;
                    free(j);
                    j = *pjoueurs;
                } else {
                    prev->next = j->next;
                    free(j);
                    j = prev->next;
                }
            } else {
                prev = j;
                j = j->next;
            }
        }
        // Vérifier combien il reste de joueurs
        int restants = 0;
        joueur_t *j2 = *pjoueurs;
        while (j2) {
            if (j2->etat == 1) restants++;
            j2 = j2->next;
        }
        if (restants == 1) {
            // Un seul joueur reste, il attend un nouvel adversaire
            joueur_t *jrest = *pjoueurs;
            while (jrest && jrest->etat != 1) jrest = jrest->next;
            if (jrest) {
                jrest->etat = 1;
                jrest->pion = 'x';
                reset_replay_etat(*pjoueurs);
                if (grille) for (int i = 0; i < haut; i++) for (int k = 0; k < larg; k++) grille[i][k] = '_';
                write(jrest->sockfd, "/info SERVER:L'autre joueur a quitté. En attente d'un nouvel adversaire...\n", strlen("/info SERVER:L'autre joueur a quitté. En attente d'un nouvel adversaire...\n"));
            }
            return 2;
        } else if (restants == 0) {
            // Plus de joueurs, serveur attend de nouveaux clients
            return 3;
        }
    }
    // Sinon, on attend encore des réponses
    return 0;
}
