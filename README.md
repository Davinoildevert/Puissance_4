# Puissance 4 - Projet Réseaux

## Présentation
Ce projet implémente un jeu Puissance 4 en réseau, permettant à deux joueurs de s'affronter via un serveur centralisé. Le projet est découpé en deux parties principales : un serveur et un client, avec une interface utilisateur en ligne de commande claire et un protocole robuste.

## Structure du projet
- `server/` : code source du serveur, gestion des connexions, logique du jeu, protocole, exécutable `serveur`.
- `client/` : code source du client, gestion du protocole, interface utilisateur, exécutable `cli`.
- `utils/` : gestion du flux replay et utilitaires partagés.
- `doc/protocole.md` : documentation détaillée du protocole d'échange client-serveur.
- `makefile` : compilation centralisée de tout le projet.
- `README.md` : ce fichier.

## Compilation
Pour compiler l'ensemble du projet (client et serveur) :

```bash
make
```

Pour nettoyer les exécutables :
```bash
make clean
```

Pour recompiler complètement :
```bash
make rebuild
```

## Lancement
### Serveur
```bash
./server/serveur [-p PORT] [-L LARGEUR] [-H HAUTEUR]
```
- `-p PORT` : port d'écoute (défaut 5000)
- `-L LARGEUR` : largeur de la grille (défaut 7)
- `-H HAUTEUR` : hauteur de la grille (défaut 6)

### Client
```bash
./client/cli <IP serveur> <PORT>
```
- `<IP serveur>` : adresse IP du serveur (obligatoire)
- `<PORT>` : port du serveur (défaut 5000)

## Fonctionnalités principales
- Interface utilisateur moderne en ligne de commande (affichage grille, couleurs, prompts clairs)
- Gestion de parties à deux joueurs en réseau
- Protocole robuste et documenté (voir `doc/protocole.md`)
- Gestion du replay (rejouer une partie sans relancer le client)
- Gestion des erreurs et des cas limites (déconnexions, entrées invalides, etc.)
- Compilation et nettoyage centralisés via un seul Makefile

## Conseils d'utilisation
- Lancer d'abord le serveur, puis les clients.
- Le client n'accepte la saisie d'un coup que lorsqu'il reçoit le prompt `/play`.
- Après chaque partie, le client propose automatiquement de rejouer.
- En cas de problème réseau, relancer le client ou le serveur.

## Dépannage
- Si le client ou le serveur ne se lance pas, vérifier :
  
- Si le port est déjà utilisé, choisir un autre port avec `-p`.
- Pour toute erreur de compilation, utiliser `make clean` puis `make`.

## Auteur
Davino Ildevert ANDRIANARIVONY
