#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "Puissance4_server.h"
#include "../utils/replay.h"

#define MSG_LEN_MAX 256

// Variables globales pour la taille de la grille
int LARG = 7; // Largeur par défaut
int HAUT = 6; // Hauteur par défaut

void init_grille(char **grille, int haut, int larg) {
    for (int i = 0; i < haut; i++)
        for (int j = 0; j < larg; j++)
            grille[i][j] = '_';
}

// Retourne 0 si le login n'est pas déjà utilisé
int login_unique(joueur_t *joueurs, const char *login) {
    joueur_t *tmp = joueurs;
    while (tmp) {
        if (strcmp(tmp->login, login) == 0) return 0;
        tmp = tmp->next;
    }
    return 1;
}

// Crée la string du plateau dans le format du protocole
void grille_to_str(char **grille, char *out, int haut, int larg) {
    int pos = 0;
    for (int i = 0; i < haut; i++) {
        for (int j = 0; j < larg; j++)
            out[pos++] = grille[i][j];
        if (i < haut-1) out[pos++] = '/';
    }
    out[pos] = '\0';
}


// Place le pion du joueur, retourne 1 si OK, 0 si colonne pleine
int jouer_coup(char **grille, int col, char pion, int haut, int larg) {
    if (col < 0 || col >= larg)
        return -1; // colonne invalide
    for (int i = 0; i < haut; i++) {
        if (grille[i][col] == '_') {
            grille[i][col] = pion;
            return i; // retourne la ligne où le pion a été mis
        }
    }
    return -2; // colonne pleine
}

// Vérifie victoire après coup joué à (ligne, col)
int check_victoire(char **grille, int ligne, int col, char pion, int haut, int larg) {
    int directions[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
    char pion_maj = (pion == 'x') ? 'X' : 'O';
    for (int d = 0; d < 4; d++) {
        int coords[4][2] = { {ligne, col}, {0,0}, {0,0}, {0,0} };
        int count = 1;

        // Sens positif
        for (int k = 1; k < 4; k++) {
            int ni = ligne + k*directions[d][0], nj = col + k*directions[d][1];
            if (ni < 0 || ni >= haut || nj < 0 || nj >= larg) break;
            if (grille[ni][nj] == pion) {
                coords[count][0] = ni;
                coords[count][1] = nj;
                count++;
            } else break;
        }
        // Sens négatif
        for (int k = 1; k < 4 && count < 4; k++) {
            int ni = ligne - k*directions[d][0], nj = col - k*directions[d][1];
            if (ni < 0 || ni >= haut || nj < 0 || nj >= larg) break;
            if (grille[ni][nj] == pion) {
                coords[count][0] = ni;
                coords[count][1] = nj;
                count++;
            } else break;
        }
        if (count >= 4) {
            // Met les 4 en majuscule
            for (int i = 0; i < 4; i++)
                grille[coords[i][0]][coords[i][1]] = pion_maj;
            return 1;
        }
    }
    return 0;
}

int grille_pleine(char **grille, int haut, int larg) {
    for (int j = 0; j < larg; j++)
        if (grille[haut-1][j] == '_')
            return 0;
    return 1;
}

// Envoie à tous les joueurs
void send_all(joueur_t *joueurs, const char *msg) {
    joueur_t *j = joueurs;
    while (j) {
        write(j->sockfd, msg, strlen(msg));
        j = j->next;
    }
}


// Trouve le joueur ayant le pion donné
joueur_t *trouve_par_pion(joueur_t *joueurs, char pion) {
    joueur_t *j = joueurs;
    while (j) {
        if (j->pion == pion) return j;
        j = j->next;
    }
    return NULL;
}

int main(int argc, char *argv[]){
    int serveur_sock, ret, maxfd;
    int compteur_clients = 1;
    int etat_jeu = 0;
    char pion_courant = 'x';
    socklen_t taille_addr_serveur, taille_addr_client;
    struct sockaddr_in addr_serveur, addr_client;
    char message_recu[MSG_LEN_MAX];
    fd_set readfds;
    joueur_t *joueurs = NULL;
    int port = 5000;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        }
        if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) {
            LARG = atoi(argv[++i]);
            if (LARG < 5) LARG = 5;
            if (LARG > 10) LARG = 10;
        }
        if (strcmp(argv[i], "-H") == 0 && i + 1 < argc) {
            HAUT = atoi(argv[++i]);
            if (HAUT < 4) HAUT = 4;
            if (HAUT > 10) HAUT = 10;
        }
    }
    // Allocation dynamique de la grille
    char **grille = (char **)malloc(HAUT * sizeof(char *));
    for (int i = 0; i < HAUT; i++)
        grille[i] = (char *)malloc(LARG * sizeof(char));
    // Initialisation de la grille
    init_grille(grille, HAUT, LARG);

    // socket TCP
    serveur_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serveur_sock < 0) {
        perror("socket");
        exit(-1);
    }
    printf("socket()... OK (%d)\n", serveur_sock);

    // adresse d'écoute
    taille_addr_serveur = sizeof(struct sockaddr_in);
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_serveur.sin_port = htons(port);

    // Bind
    ret = bind(serveur_sock, (struct sockaddr *)&addr_serveur, taille_addr_serveur);
    if (ret < 0) {
        perror("bind");
        exit(-2);
    }
    printf("bind()... OK\n");

    // listen
    ret = listen(serveur_sock, 5);
    if (ret < 0) {
        perror("listen");
        exit(-3);
    }
    printf("listen()... OK\n");

    printf("Serveur à l'écoute sur le port %d\n", ntohs(addr_serveur.sin_port));
    printf("Attente d'une demande de connexion (Ctrl + C pour quitter)...\n\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(serveur_sock, &readfds);
        maxfd = serveur_sock;
        joueur_t *c = joueurs;
        while (c != NULL) {
            FD_SET(c->sockfd, &readfds);
            if (c->sockfd > maxfd)
                maxfd = c->sockfd;
            c = c->next;
        }

        int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(serveur_sock, &readfds)) {
            taille_addr_client = sizeof(addr_client);
            int new_sock = accept(serveur_sock, (struct sockaddr *)&addr_client, &taille_addr_client);
            if (new_sock < 0) {
                perror("accept");
                continue;
            }

            // 1. Compter les joueurs loggés (etat == 1)
            int logged = 0;
            joueur_t *j = joueurs;
            while (j) {
                if (j->etat == 1) logged++;
                j = j->next;
            }
            if (logged >= 2) {
                char msg[] = "/info SERVER:full\n";
                write(new_sock, msg, strlen(msg));
                printf("[WARN] Connexion refusée (partie pleine)\n");
                close(new_sock);
                continue;
            }

            joueur_t *new_joueur = (joueur_t *)malloc(sizeof(joueur_t));
            new_joueur->sockfd = new_sock;
            new_joueur->numero = compteur_clients++;
            new_joueur->addr = addr_client;
            strcpy(new_joueur->login, "");
            new_joueur->pion = '_';
            new_joueur->etat = 0;
            new_joueur->next = joueurs;
            joueurs = new_joueur;

            printf("Connexion acceptée de client %d from %s:%d (fd %d)\n",
                new_joueur->numero,
                inet_ntoa(addr_client.sin_addr),
                ntohs(addr_client.sin_port),
                new_sock);

            // Protocole : /info ID d'abord
            char info_id[] = "/info ID: Puissance 4 server v0.2\n";
            write(new_joueur->sockfd, info_id, strlen(info_id));
            // Puis demande login
            write(new_joueur->sockfd, "/login\n", 7);
        }

        // Messages reçus des joueurs
        joueur_t *prev = NULL;
        c = joueurs;
        while (c != NULL) {
            if (FD_ISSET(c->sockfd, &readfds)) {
                memset(message_recu, 0, MSG_LEN_MAX);
                int ret = read(c->sockfd, message_recu, MSG_LEN_MAX-1);
                if (ret <= 0) {
                    printf("[INFO] Client %d déconnecté.\n", c->numero);
                    close(c->sockfd);
                    if (prev == NULL) {
                        joueurs = c->next;
                        free(c);
                        c = joueurs;
                    } else {
                        prev->next = c->next;
                        free(c);
                        c = prev->next;
                    }
                    // --- Ajout : notifier le joueur restant et réinitialiser la grille ---
                    int nb_joueurs_restants = 0;
                    joueur_t *jtmp = joueurs;
                    while (jtmp) {
                        if (jtmp->etat == 1) nb_joueurs_restants++;
                        jtmp = jtmp->next;
                    }
                    if (etat_jeu == 1 && nb_joueurs_restants == 1) {
                        joueur_t *jrest = joueurs;
                        while (jrest && jrest->etat != 1) jrest = jrest->next;
                        if (jrest) {
                            write(jrest->sockfd, "/info SERVER:L'autre joueur a quitté. En attente d'un nouvel adversaire...\n", strlen("/info SERVER:L'autre joueur a quitté. En attente d'un nouvel adversaire...\n"));
                            jrest->pion = 'x';
                            init_grille(grille, HAUT, LARG);
                        }
                    }
                    continue;
                } else {
                    message_recu[ret] = '\0';
                    if (etat_jeu == 3 && strncmp(message_recu, "/replay ", 8) == 0) {
                        int reponse = (message_recu[8] == 'o' || message_recu[8] == 'O') ? 1 : -1;
                        int res = traiter_reponse_replay(&joueurs, c, reponse, grille, HAUT, LARG);
                        if (res == 1) {
                            // Les deux veulent rejouer, on relance la partie
                            pion_courant = 'x';
                            etat_jeu = 1;
                            char matrix[HAUT*LARG + HAUT + 1], msg2[512];
                            grille_to_str(grille, matrix, HAUT, LARG);
                            snprintf(msg2, sizeof(msg2), "/info MATRIX:%s\n", matrix);
                            send_all(joueurs, msg2);
                            joueur_t *jx = trouve_par_pion(joueurs, 'x');
                            if (jx) write(jx->sockfd, "/play\n", 6);
                        } else if (res == 2) {
                            // Un joueur refuse, l'autre attend un nouvel adversaire (déjà notifié dans replay)
                            // Rien à faire ici
                        } else if (res == 3) {
                            // Plus de joueurs, serveur attend de nouveaux clients
                            // Rien à faire ici
                        }
                        // Si res == 0, on attend encore des réponses, rien à faire
                        // Correction : si la liste des joueurs est vide, sortir de la boucle
                        if (joueurs == NULL) {
                            goto end_clients_loop;
                        }
                        goto nextc;
                    }


                    if (strncmp(message_recu, "/login ", 7) == 0) {
                        char login[17] = {0};
                        sscanf(message_recu + 7, "%16s", login);

                        // Vérification protocole
                        if (strchr(login, ':') != NULL || strlen(login) < 3 || strlen(login) > 16) {
                            write(c->sockfd, "/ret LOGIN:105\n", 15);
                            write(c->sockfd, "/login\n", 7);
                            goto nextc;
                        }

                        // Vérifier si déjà 2 joueurs loggés
                        int nb_logged = 0;
                        joueur_t *jtmp = joueurs;
                        while (jtmp) {
                            if (jtmp->etat == 1) nb_logged++;
                            jtmp = jtmp->next;
                        }
                        if (nb_logged >= 2) {
                            // On accepte le login pour la forme, mais on informe que c'est plein et on ferme
                            write(c->sockfd, "/ret LOGIN:000\n", 16);
                            write(c->sockfd, "/info SERVER:full\n", strlen("/info SERVER:full\n"));
                            close(c->sockfd);
                            // Suppression du client de la liste chaînée
                            if (prev == NULL) {
                                joueurs = c->next;
                                free(c);
                                c = joueurs;
                            } else {
                                prev->next = c->next;
                                free(c);
                                c = prev->next;
                            }
                            continue;
                        }

                        if (!login_unique(joueurs, login)) {
                            printf("[DEBUG] Login déjà pris: %s\n", login);
                            write(c->sockfd, "/ret LOGIN:101\n", 15);
                            write(c->sockfd, "/login\n", 7);
                        } else {
                            strncpy(c->login, login, 16);
                            c->etat = 1; // loggé
                            // Attribution du pion
                            int pion_x_taken = 0, pion_o_taken = 0;
                            joueur_t *j = joueurs;
                            while (j) {
                                if (j->etat == 1) {
                                    if (j->pion == 'x') pion_x_taken = 1;
                                    if (j->pion == 'o') pion_o_taken = 1;
                                }
                                j = j->next;
                            }
                            if (!pion_x_taken) c->pion = 'x';
                            else if (!pion_o_taken) c->pion = 'o';
                            else c->pion = '_';

                            // Protocole : d'abord le code retour
                            write(c->sockfd, "/ret LOGIN:000\n", 16);
                            printf("[DEBUG] /ret LOGIN:000 envoyé au client %d\n", c->numero);

                            // Recompter les joueurs loggés
                            int nb_logged2 = 0;
                            joueur_t *j1 = NULL, *j2 = NULL;
                            j = joueurs;
                            while (j) {
                                if (j->etat == 1) {
                                    nb_logged2++;
                                    if (!j1) j1 = j;
                                    else j2 = j;
                                }
                                j = j->next;
                            }
                            if (nb_logged2 == 1) {
                                // Premier joueur : ne reçoit que son info et le message d'attente
                                char msg[128];
                                snprintf(msg, sizeof(msg), "/info LOGIN:1/2:%s:%c\n", c->login, c->pion);
                                write(c->sockfd, msg, strlen(msg));
                                printf("[DEBUG] /info LOGIN:1/2 envoyé au client %d\n", c->numero);
                                write(c->sockfd, "/info WAIT:En attente d'un second joueur...\n", strlen("/info WAIT:En attente d'un second joueur...\n"));
                                printf("[DEBUG] /info WAIT envoyé au client %d\n", c->numero);
                                sleep(1); // Pause pour garantir l'affichage côté client
                            } else if (nb_logged2 == 2 && j1 && j2) {
                                // Deux joueurs loggés : chacun reçoit les infos dans l'ordre du protocole
                                char msg1[128], msg2[128];
                                snprintf(msg1, sizeof(msg1), "/info LOGIN:1/2:%s:%c\n", j1->login, j1->pion);
                                snprintf(msg2, sizeof(msg2), "/info LOGIN:2/2:%s:%c\n", j2->login, j2->pion);
                                write(j1->sockfd, msg1, strlen(msg1));
                                write(j1->sockfd, msg2, strlen(msg2));
                                write(j2->sockfd, msg1, strlen(msg1));
                                write(j2->sockfd, msg2, strlen(msg2));
                                // Envoie la grille initiale
                                char matrix[HAUT*LARG + HAUT + 1], msgm[512];
                                grille_to_str(grille, matrix, HAUT, LARG);
                                snprintf(msgm, sizeof(msgm), "/info MATRIX:%s\n", matrix);
                                write(j1->sockfd, msgm, strlen(msgm));
                                write(j2->sockfd, msgm, strlen(msgm));
                                // Donne la main au joueur 'x'
                                if (j1->pion == 'x') write(j1->sockfd, "/play\n", 6);
                                else if (j2->pion == 'x') write(j2->sockfd, "/play\n", 6);
                                etat_jeu = 1;
                                pion_courant = 'x';
                            }
                            printf("[DEBUG] joueur %d connecté avec login '%s' et pion '%c'\n", c->numero, c->login, c->pion);
                        }
                    }
                    else if (strncmp(message_recu, "/play ", 6) == 0) {
                        if (etat_jeu != 1) {
                            write(c->sockfd, "/ret PLAY:202\n", 14);
                            printf("[WARN] Partie non en cours.\n");
                            goto nextc;
                        }
                        if (c->pion != pion_courant) {
                            write(c->sockfd, "/ret PLAY:102\n", 14);
                            printf("[WARN] Pas le tour de ce joueur (%s)\n", c->login);
                            goto nextc;
                        }
                        int col = atoi(message_recu + 6);
                        int ligne = jouer_coup(grille, col, c->pion, HAUT, LARG);
                        if (ligne == -1) {
                            write(c->sockfd, "/ret PLAY:103\n", 14);
                            printf("[WARN] Colonne inconnue (%d)\n", col);
                            goto nextc;
                        }
                        if (ligne == -2) {
                            write(c->sockfd, "/ret PLAY:104\n", 14);
                            printf("[WARN] Colonne pleine (%d)\n", col);
                            goto nextc;
                        }
                        write(c->sockfd, "/ret PLAY:000\n", 14);

                        int victoire = check_victoire(grille, ligne, col, c->pion, HAUT, LARG);
                     
                        char matrix[HAUT*LARG + HAUT + 1], msg2[512];
                        if (victoire) {
                            // Envoie D’ABORD la grille gagnante avec majuscules
                            grille_to_str(grille, matrix, HAUT, LARG);
                            snprintf(msg2, sizeof(msg2), "/info MATRIX:%s\n", matrix);
                            send_all(joueurs, msg2);

                            // Puis le message de victoire
                            snprintf(msg2, sizeof(msg2), "/info END:WIN:%s\n", c->login);
                            send_all(joueurs, msg2);

                            printf("[INFO] Partie terminée : victoire de %s\n", c->login);
                            // --- NOUVEAU FLUX REPLAY ---
                            envoyer_demande_replay(joueurs);
                            reset_replay_etat(joueurs);
                            etat_jeu = 3;
                            goto nextc;
                        } else if (grille_pleine(grille, HAUT, LARG)) {
                            send_all(joueurs, "/info END:DRAW:NONE\n");
                            grille_to_str(grille, matrix, HAUT, LARG);
                            snprintf(msg2, sizeof(msg2), "/info MATRIX:%s\n", matrix);
                            send_all(joueurs, msg2);
                            printf("[INFO] Partie terminée : match nul\n");
                            // --- NOUVEAU FLUX REPLAY ---
                            envoyer_demande_replay(joueurs);
                            reset_replay_etat(joueurs);
                            etat_jeu = 3;
                            goto nextc;
                        } else {
                            // Toujours envoyer la grille après chaque coup, même sans victoire
                            grille_to_str(grille, matrix, HAUT, LARG);
                            snprintf(msg2, sizeof(msg2), "/info MATRIX:%s\n", matrix);
                            send_all(joueurs, msg2);
                            pion_courant = (pion_courant == 'x') ? 'o' : 'x';
                            joueur_t *jnext = trouve_par_pion(joueurs, pion_courant);
                            if (jnext) write(jnext->sockfd, "/play\n", 6);
                            
                        }
                    }

                    else {
                        printf("[DEBUG] Message inconnu du client %d: %s\n", c->numero, message_recu);
                        write(c->sockfd, "/ret PROTO:201\n", 15);
                    }
                }
            }
            nextc:
            prev = c;
            if (c) c = c->next;
        }
    }
    end_clients_loop:
    
    joueur_t *tmp=NULL;

    while (joueurs != NULL) {
        close(joueurs->sockfd);
        tmp = joueurs;
        joueurs = joueurs->next;
        free(tmp);
    }
    for (int i = 0; i < HAUT; i++)
        free(grille[i]);
    free(grille);
    close(serveur_sock);

    return 0;
}
