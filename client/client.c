#include "client.h"
// ...en-têtes standard...
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>
#include <locale.h>

#define MSG_LEN_MAX 256

int sockfd = -1;

void handle_sigint(int sig) {
    (void)sig;
    if (sockfd != -1) close(sockfd);
    printf("\nDéconnexion du serveur.\n");
    exit(0);
}

// Nouvelle fonction d'affichage de la grille façon Puissance 4
void afficher_grille(const char *matrix) {
    // On suppose que la grille est envoyée sous forme: ligne1/ligne2/ligne3...
    char lignes[20][20]; // max 20 lignes, 20 colonnes
    int nb_lignes = 0;
    const char *p = matrix;
    while (*p && nb_lignes < 20) {
        int c = 0;
        while (*p && *p != '/' && c < 20) {
            lignes[nb_lignes][c++] = *p;
            p++;
        }
        lignes[nb_lignes][c] = '\0';
        nb_lignes++;
        if (*p == '/') p++;
    }
    // Affichage des numéros de colonnes
    printf("\n      ");
    size_t j;
    for (j = 0; lignes[0][j]; j++) printf(" %d  ", (int)j);
    printf("\n    ╔");
    for (j = 0; lignes[0][j]; j++) printf("═══%s", (j == strlen(lignes[0])-1) ? "╗" : "╦");
    printf("\n");
    for (int i = nb_lignes - 1; i >= 0; i--) {
        printf("%3d ║", i);
        for (size_t j = 0; lignes[i][j]; j++) {
            char c = lignes[i][j];
            if (c == 'x' || c == 'X')
                printf(" \033[1;31m●\033[0m ║"); // Rouge
            else if (c == 'o' || c == 'O')
                printf(" \033[1;33m●\033[0m ║"); // Jaune
            else
                printf(" · ║"); // Vide
        }
        printf("\n");
        if (i > 0) {
            printf("    ╠");
            for (j = 0; lignes[0][j]; j++) printf("═══%s", (j == strlen(lignes[0])-1) ? "╣" : "╬");
            printf("\n");
        }
    }
    printf("    ╚");
    for (j = 0; lignes[0][j]; j++) printf("═══%s", (j == strlen(lignes[0])-1) ? "╝" : "╩");
    printf("\n\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP serveur> <port>\n", argv[0]);
        return 1;
    }
    signal(SIGINT, handle_sigint);
    const char *ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in serv_addr;
    char buffer[MSG_LEN_MAX];
    char recvbuf[4096] = ""; // Buffer d'accumulation pour lecture partielle
    int attente_login = 0;
    int attente_replay = 0;
    char mon_login[32] = "";

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("Adresse IP invalide\n");
        return 1;
    }
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return 1;
    }
    printf("Connecté au serveur %s:%d\n", ip, port);
    fd_set readfds;
    int maxfd = (sockfd > fileno(stdin)) ? sockfd : fileno(stdin);
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(fileno(stdin), &readfds);
        int ready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ready < 0) {
            perror("select");
            break;
        }
        // Message du serveur
        if (FD_ISSET(sockfd, &readfds)) {
            ssize_t n = read(sockfd, buffer, MSG_LEN_MAX - 1);
            if (n <= 0) {
                printf("\nConnexion fermée par le serveur.\n");
                break;
            }
            buffer[n] = '\0';
            // Accumule dans recvbuf
            size_t recvlen = strlen(recvbuf);
            if (recvlen + n >= sizeof(recvbuf) - 1) {
                // Trop de données, on flush
                recvbuf[0] = '\0';
                recvlen = 0;
            }
            strncat(recvbuf, buffer, sizeof(recvbuf) - recvlen - 1);
            // Découpe et traite chaque ligne
            char *line_start = recvbuf;
            char *newline = NULL;
            while ((newline = strchr(line_start, '\n')) != NULL) {
                size_t linelen = newline - line_start + 1;
                char oneline[512];
                if (linelen >= sizeof(oneline)) linelen = sizeof(oneline) - 1;
                strncpy(oneline, line_start, linelen);
                oneline[linelen] = '\0';
                // Affiche la ligne reçue
                // printf("%s", oneline);
                // fflush(stdout);
                // Affichage de la grille si /info MATRIX:
                if (strncmp(oneline, "/info MATRIX:", 13) == 0) {
                    afficher_grille(oneline + 13);
                }
                // Tour du joueur (nouvelle gestion)
                if (strncmp(oneline, "/play", 5) == 0) {
                    printf("─────────────────────────────\n");
                    printf("\033[1;32m    C'est votre tour !\033[0m\n");
                    printf(" Entrez le numéro de colonne :\n");
                    printf("─────────────────────────────\n");
                    // Lecture bloquante du coup
                    char input[64];
                    while (1) {
                        
                        fflush(stdout);
                        if (!fgets(input, sizeof(input), stdin)) {
                            printf("Erreur de lecture.\n");
                            close(sockfd);
                            exit(1);
                        }
                        input[strcspn(input, "\n")] = 0;
                        char *endptr;
                        long col = strtol(input, &endptr, 10);
                        if (*endptr != '\0' || col < 0 || col > 99) {
                            printf("[ERREUR] Veuillez entrer un numéro de colonne valide.\n");
                        } else {
                            char msg[32];
                            snprintf(msg, sizeof(msg), "/play %ld\n", col);
                            write(sockfd, msg, strlen(msg));
                            break;
                        }
                    }
                }
                // Détection de la demande de login
                if (strstr(oneline, "/login") != NULL) {
                    printf("Veuillez entrer votre login: ");
                    fflush(stdout);
                    attente_login = 1;
                }
                // Détection de la partie pleine
                if (strstr(oneline, "/info SERVER:full") != NULL) {
                    printf("[INFO] La partie est pleine. Veuillez réessayer plus tard.\n");
                    close(sockfd);
                    exit(0);
                }
                // En attente adversaire
                if (strstr(oneline, "/info WAIT:") != NULL ||
                    strstr(oneline, "En attente d'un second joueur") != NULL ||
                    strstr(oneline, "En attente d'un nouvel adversaire") != NULL ||
                    strstr(oneline, "En attente d’un adversaire") != NULL) {
                    printf("\033[1;34mEn attente d’un adversaire…\033[0m\n");
                }
                // Gestion des retours de coup
                if (strncmp(oneline, "/ret PLAY:103", 13) == 0) {
                    printf("\033[1;31m! Colonne invalide, choisissez-en une autre.\033[0m\n");
                    printf("─────────────────────────────\n");
                    printf("    Entrez le numéro de colonne :\n");
                    printf("─────────────────────────────\n");
                    // Relance la saisie du coup localement
                    char input[64];
                    while (1) {
                        fflush(stdout);
                        if (!fgets(input, sizeof(input), stdin)) {
                            printf("Erreur de lecture.\n");
                            close(sockfd);
                            exit(1);
                        }
                        input[strcspn(input, "\n")] = 0;
                        char *endptr;
                        long col = strtol(input, &endptr, 10);
                        if (*endptr != '\0' || col < 0 || col > 99) {
                            printf("[ERREUR] Veuillez entrer un numéro de colonne valide.\n");
                        } else {
                            char msg[32];
                            snprintf(msg, sizeof(msg), "/play %ld\n", col);
                            write(sockfd, msg, strlen(msg));
                            break;
                        }
                    }
                }
                if (strncmp(oneline, "/ret PLAY:104", 13) == 0) {
                    printf("\033[1;31m! Colonne pleine, choisissez-en une autre:\033[0m\n");
                    printf("─────────────────────────────\n");
                    printf("    Entrez le numéro de colonne :\n");
                    printf("─────────────────────────────\n");
                    // Relance la saisie du coup localement
                    char input[64];
                    while (1) {
                        fflush(stdout);
                        if (!fgets(input, sizeof(input), stdin)) {
                            printf("Erreur de lecture.\n");
                            close(sockfd);
                            exit(1);
                        }
                        input[strcspn(input, "\n")] = 0;
                        char *endptr;
                        long col = strtol(input, &endptr, 10);
                        if (*endptr != '\0' || col < 0 || col > 99) {
                            printf("[ERREUR] Veuillez entrer un numéro de colonne valide.\n");
                        } else {
                            char msg[32];
                            snprintf(msg, sizeof(msg), "/play %ld\n", col);
                            write(sockfd, msg, strlen(msg));
                            break;
                        }
                    }
                }
                if (strncmp(oneline, "/ret PLAY:102", 13) == 0) {
                    printf("\033[1;33m[INFO] Ce n’est pas votre tour, veuillez patienter...\033[0m\n");
                }
                if (strncmp(oneline, "/ret PLAY:202", 13) == 0) {
                    printf("[ERREUR] Partie non en cours, veuillez patienter.\n");
                }
                // Récupération du login du joueur (pour savoir si on a gagné)
                
                if (strncmp(oneline, "/info LOGIN:1/2:", 15) == 0 || strncmp(oneline, "/info LOGIN:2/2:", 15) == 0) {
                    // Ne rien faire ici : mon_login est déjà défini à la saisie utilisateur
                }
                
                // Gestion de la fin de partie
                if (strncmp(oneline, "/info END:WIN:", 14) == 0) {
                    char gagnant[32];
                    if (sscanf(oneline, "/info END:WIN:%31s", gagnant) == 1) {
                        // Nettoyage des éventuels caractères parasites
                        gagnant[strcspn(gagnant, "\r\n ")] = 0;
                        mon_login[strcspn(mon_login, "\r\n ")] = 0;
                        // Affichage debug
                     
                        mon_login[strcspn(mon_login, "\r\n ")] = 0;
                        // Affichage debug
                        printf("[DEBUG] mon_login='%s', gagnant='%s'\n", mon_login, gagnant);
                        if (strcmp(gagnant, mon_login) == 0) {
                            printf("\033[1;42;37m╔════════════════════════════╗\033[0m\n");
                            printf("\033[1;42;37m║   BRAVO, VOUS AVEZ GAGNÉ ! ║\033[0m\n");
                            printf("\033[1;42;37m╚════════════════════════════╝\033[0m\n");
                        } else {
                            printf("\033[1;41;37m╔══════════════════════════════════╗\033[0m\n");
                            printf("\033[1;41;37m║   Dommage, l’adversaire a gagné. ║\033[0m\n");
                            printf("\033[1;41;37m╚══════════════════════════════════╝\033[0m\n");
                        }
                        fflush(stdout);
                    }
                    // Prépare l'attente du replay
                    attente_replay = 1;
                }
                if (strncmp(oneline, "/info END:DRAW:NONE", 19) == 0) {
                    printf("\033[1;43mMatch nul, personne ne gagne cette manche.\033[0m\n");
                    fflush(stdout);
                    attente_replay = 1;
                }
                // Détection de la demande de replay
                if (strstr(oneline, "/info REPLAY:") != NULL) {
                    printf("\033[1;36mVoulez-vous rejouer ? (o/n) : \033[0m");
                    fflush(stdout);
                    attente_replay = 1;
                }
                // Détection de la déconnexion de l'adversaire
                if (strstr(oneline, "/info SERVER:L'autre joueur a quitté") != NULL) {
                    printf("\033[1;34m[INFO] L'autre joueur a quitté. En attente d'un nouvel adversaire...\033[0m\n");
                    // Ne pas mettre attente_replay = 0 ici : l'utilisateur doit pouvoir répondre
                }
                // Gestion de la fermeture propre
                if (strncmp(oneline, "/info SERVER:full", 18) == 0) {
                    printf("[INFO] La partie est pleine. Fermeture de la connexion.\n");
                    close(sockfd);
                    exit(0);
                }
                // Gestion des erreurs serveur inattendues
                if (strncmp(oneline, "/ret PROTO:", 11) == 0) {
                    printf("[ERREUR] Erreur protocole serveur : %s\n", oneline+11);
                }
                // Passe à la ligne suivante
                line_start = newline + 1;
            }
            // Décale le buffer pour ne garder que l'éventuel reste non traité
            if (line_start != recvbuf) {
                memmove(recvbuf, line_start, strlen(line_start) + 1);
            }
        }
        // Saisie utilisateur (login ou replay uniquement)
        if ((attente_login || attente_replay) && FD_ISSET(fileno(stdin), &readfds)) {
            char input[64];
            if (fgets(input, sizeof(input), stdin) != NULL) {
                input[strcspn(input, "\n")] = 0;
                if (attente_login) {
                    if (strlen(input) > 0) {
                        char msg[80];
                        snprintf(msg, sizeof(msg), "/login %s\n", input);
                        write(sockfd, msg, strlen(msg));
                        strncpy(mon_login, input, sizeof(mon_login)-1);
                        mon_login[sizeof(mon_login)-1] = '\0';
                    }
                    attente_login = 0;
                } else if (attente_replay) {
                    if (strcmp(input, "o") == 0 || strcmp(input, "O") == 0) {
                        write(sockfd, "/replay o\n", 10);
                        printf("[INFO] Demande de replay envoyée (oui).\n");
                        attente_replay = 0;
                    } else if (strcmp(input, "n") == 0 || strcmp(input, "N") == 0) {
                        write(sockfd, "/replay n\n", 10);
                        printf("[INFO] Demande de replay envoyée (non).\n");
                        attente_replay = 0;
                    } else {
                        printf("[ERREUR] Répondez par 'o' ou 'n' : ");
                        fflush(stdout);
                    }
                }
            }
        }
    }
    close(sockfd);
    return 0;
}
// ...fin du fichier...