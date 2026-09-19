# Puissance 4 — Projet réseau client/serveur

Implémentation en C d’un **Puissance 4 jouable à distance par deux clients reliés à un serveur central via sockets TCP**.

## En bref — contribution & valeur

- **Implémenté** une architecture client/serveur utilisant des sockets TCP.
- **Conçu** un protocole d’échange dédié pour synchroniser les deux joueurs et l’état de la partie.
- **Géré** les entrées invalides, déconnexions et cas limites afin d’éviter de bloquer une partie.
- **Ajouté** un système de replay permettant de relancer une partie sans redémarrer les clients.
- **Centralisé** compilation et nettoyage du projet via Makefile.

## Stack technique

- **C**
- **Sockets TCP**
- **Client / serveur**
- **Makefile**
- **Linux / terminal**

## Architecture

```text
Puissance_4/
├── server/        # serveur, logique de jeu, connexions
├── client/        # client TCP et interface CLI
├── utils/         # utilitaires et replay
├── doc/           # documentation du protocole
└── makefile
```

## Fonctionnalités

- partie réseau à deux joueurs ;
- serveur centralisé ;
- protocole documenté ;
- grille configurable ;
- replay ;
- validation des entrées ;
- gestion des déconnexions et erreurs ;
- interface en ligne de commande.

## Compilation

```bash
make
```

Nettoyage :

```bash
make clean
```

Rebuild :

```bash
make rebuild
```

## Lancement

### Serveur

```bash
./server/serveur [-p PORT] [-L LARGEUR] [-H HAUTEUR]
```

### Client

```bash
./client/cli <IP_SERVEUR> <PORT>
```

Le port par défaut est `5000`.

## Compétences démontrées

**C • programmation réseau • sockets TCP • protocole client/serveur • gestion d’erreurs • Makefile**

## Auteur

**Davino Ildevert ANDRIANARIVONY**
