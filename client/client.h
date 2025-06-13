// client.h - En-têtes pour client Puissance 4
#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>
#include <locale.h>

#define MSG_LEN_MAX 256

// Déclaration de la fonction d'affichage de la grille
void afficher_grille(const char *matrix);

// Déclaration du handler SIGINT
void handle_sigint(int sig);

#endif // CLIENT_H
