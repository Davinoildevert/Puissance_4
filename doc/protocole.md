Protocole Puissance 4 Version 0.2
Lancement de l'application serveur
./serveur [-p PORT] [-L LARGUEUR] [-H HAUTEUR]

Lancement de l'application client
./client -s IP [-p PORT]

Commandes serveur
/login => demande au client son login (voir /login client pour la réponse)
/ret ACTION:CODE => CODE de retour suite à la commande ACTION (voir codes de retour ci-dessous)
/info ... => information du serveur sans réponse attendue
/info ID:server_id => description libre du serveur (sans :)
/info LOGIN:n/N:login:[ro] => le joueur "login" est le (n)ième connecté sur N attendus et il jouera les x ou les o
/info MATRIX:_xo_/__x_/____/____ => description de la grille suite à une action de jeu (la grille est représentée ligne par ligne, séparées par des /, sans espaces, et en partant du bas à gauche avec les x ou o représentant les pièces des joueurs et les _ représentant les cases vides
/info END:WIN:LOGIN => le joueur "LOGIN" a gagné (l'info MATRIX précédente contient les X ou les O en majuscules pour montrer la combinaison gagnante)
/info END:DRAW:NONE => match nul, pas de gagnant
/play => demande au joueur de donner la colonne où il souhaite glisser son pion (première colonne à gauche est en position 0)
Réponses client
/login LOGIN
Retour : /ret LOGIN:CODE
/play X
Retour : /ret PLAY:CODE
autre
Retour : /ret PROTO:201
Codes de retour
000:OK
101:login already in use
102:not your turn
103:unknown column
104:column is full
105:invalid login
201:unknown command
202:command not awaited
Options et Limites
PORT
Port d'écoute du serveur
Défaut 5000
Attention si <1024 nécessite les droits root
LOGIN
Nom de joueur
Caractères interdits : (deux points)
Min 3 caractères, Max 16 caractères
HAUTEUR
Hauteur de la grille
Min 4, Max 10, Défaut 6
LARGEUR
Largeur de la grille
Min 5, Max 10, Défaut 7
SÉQUENCEMENT

Exemple de séquence de dialogue entre un client (C) et le serveur (S) juste après la connexion :

S: /info ID:Mon super serveur v3.6
S: /login

C: /login Julien

S: /ret LOGIN:101
S: /login

C: /login The Goat

S: /ret LOGIN:000
S: /info LOGIN:1/2:The Goat:x
🕑
S: /info LOGIN:2/2:Willy:o
S: /info MATRIX:_____/_____/_____/_____/_____
S: /play

C: /play 3

S: /ret PLAY:000
S: /info MATRIX:__x__/_____/_____/_____/_____
🕑
S: /info MATRIX:__xo__/_____/_____/_____/_____
S: /play

...

C: /play 3

S: /ret PLAY:000
S: /info MATRIX:ooXoo/__X__/__X__/__X__/_____
S: /info END:WIN:The Goat



