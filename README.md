# Puissance 4 — Projet réseau client/serveur

Implémentation en C d’un **Puissance 4 jouable à distance par deux clients reliés à un serveur central via sockets TCP**.

## Ma contribution

J’ai réalisé **toute la partie serveur** du projet. Le client a été développé par un autre membre de l’équipe.

- **Développé** le serveur TCP en C : ouverture du socket, acceptation des connexions et gestion de deux joueurs.
- **Implémenté** la logique de partie côté serveur : attribution des pions, gestion des tours, validation des coups et détection victoire / match nul.
- **Géré** les cas d’erreur : login déjà utilisé, mauvais tour, colonne invalide ou pleine, partie complète et déconnexion d’un joueur.
- **Intégré** le système de replay afin de relancer une partie sans redémarrer le serveur.
- **Participé** au protocole d’échange client/serveur et à l’intégration globale du projet.
- **Contribué** à l’organisation et à la compilation via Makefile.

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
