#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "piece.h"
#include "game.h"

#define RANDOMIZE 50

#define SIZE_GAME 6 // A enlever, doit être présent dans game.h à la place

char *dir_c[] = {"up", "left", "down", "right"}; //Uniquement utile pour comparer avec l'input

/*
Retourne une nombre aléatoire en a et b (compris)
*/
int rand_ab(int a, int b) {
	return (rand()%((b+1)-a)) + a;
}

/*
Retourne true si les 2 chaines sont exactement identiques.
*/
bool streq(char* s1, char* s2) {
	int i = 1;
	if (s1[0] != s2[0]) return false;
	while (s1[i] != '\0' && s2[i] != '\0' && i < 20) {
		if (s1[i] != s2[i]) return false;
		i++;
	}
	return true;
}

/*
décompose le char* cmd pour y retrouver les 3 arguments utiles :
la piece ciblé, la direction du mouvement et la distance a parcourir.
Renvoie vrai si elle trouve les 3 arguments correctement, faux sinon.
*/
bool readCommand(char *cmd, int* target, int* direction, int* distance) {
	char str[3][20]; // char** pour stocker les 3 aguments en string
	for (int i = 0; i < 3; i++) { 		// initialisation à '\0' sur tout les char
		for (int j = 0; j < 10; j++) {	// |
			str[i][j] = '\0';			// |
		}								// |
	}									// -
	int i = 0, j = 0, k = 0;
	while (cmd[i] != '\0' && i < 20 && j < 3) { // parcours cmd et le stock dans str[j]. A chaque espace j++ donc nouvel argument
		if (cmd[i] == ' ') {					// |
			j++;								// |
			k = -1;//car k incrémenté plus bas	// |
		} else {								// |
			str[j][k] = cmd[i];					// |
		}										// |
		k++;									// |
		i++;									// |
	}											// -
	if (j != 2) return false;// si il n'y a pas 2 espaces retourne faux
	*target = atoi(str[0]);
	*distance = atoi(str[2]);
	*direction = -1;
	for (int i = 0; i < 4; i++) {
		if (streq(str[1], dir_c[i])) *direction = i;
	}
	if (*direction == -1 || *distance == 0) return false; // si la direction ou le mouvement ne sont pas trouvé (ou distance = 0) renvoie faux
	return true;
}

void DEBUG_display(game currentGame, int nb_pieces) {
	printf("Pieces :\n");
	for (int i = 0; i < nb_pieces; ++i) {
		printf("piece %d : x:%d y:%d small:%d hori:%d\n",i ,get_x(game_piece(currentGame, i)), get_y(game_piece(currentGame, i)), 
			is_small(game_piece(currentGame, i)), is_horizontal(game_piece(currentGame, i)));
	}
}
/*
genere une piece placé aléatoirement sur la grille, qui ne dépasse pas de la grille et qui ne chevauche pas d'autres pieces
Attention, generer trop de pieces sur une grille trop petite peut faire boucler cette fonction !
*/
void randomizePiece(piece pieces[], int n, int m) {
	bool 	finished = false;
	int 	x;
	int 	y;
	bool	small;
	bool	hori;
	while (!finished) {
		finished = true;
		x = rand_ab(0,SIZE_GAME-1);
		y = rand_ab(0,SIZE_GAME-1);
		small = rand_ab(0,1);
		if (x == SIZE_GAME-1) hori = false;
		else if (x == SIZE_GAME-2 && !small) hori = false;
		else if (y == SIZE_GAME-1) hori = true;
		else if (y == SIZE_GAME-2 && !small) hori = true;
		else hori = rand_ab(0,1);
		if ((x == 5 && y == 5) || (!small && x >= 4 && y >= 4)) finished = false;
		pieces[n] = new_piece_rh(x,y,small,hori);
		for (int i = 0; i < n; ++i) { // si cette piece en chevauche une autre déja placé, on recommence
			if (intersect(pieces[n], pieces[i])) finished = false; 
		}
		if (!finished) {
			delete_piece(pieces[n]);
		}
	}
}

int main(int argc, char* argv[]) {
	char 	cmd[20]; // contiendra l'input de l'utilisateur
	int 	cmd_target;
	int 	cmd_direction;
	int 	cmd_distance;
	int 	nb_pieces;
	game	currentGame;
	piece*	pieces;
	
	srand((unsigned)time(NULL)); 

	newGame:
	// Generation du niveaux 
			/// Certaine piece semble se chevaucher dans certain cas (Après les play_move ?), il faudra faire des tests lorsque la fonction d'affichage sera finie
	nb_pieces = 6;
	pieces = malloc(nb_pieces*(sizeof(piece)));
	pieces[0] = new_piece_rh(3,4,true,true); // la 1ere piece est à l'arrivé, en position de partie finie
	for (int i = 1; i < nb_pieces; ++i) { // on genere les autres pieces aléatoirement
		randomizePiece(pieces, i, nb_pieces);
	}
	currentGame = new_game_hr(nb_pieces, pieces);
	for (int r = 0; r < RANDOMIZE; ++r) { // et pour finir on fait faire a chaque piece un certain nombre de mouvements aléatoires pour mélanger la grille
		for (int i = 0; i < nb_pieces; ++i) {
			if (is_horizontal(game_piece(currentGame, i))) play_move(currentGame, i, (rand_ab(0,1)?1:3), rand_ab(0,5));
			else play_move(currentGame, i, (rand_ab(0,1)?0:2), rand_ab(0,5));
		}
	}
	
	// Debut du jeu
	while (!streq(cmd,"exit")) { // tant qu'on n'entre pas "exit", boucle le programme
		// Affichage de la grille
		DEBUG_display(currentGame, nb_pieces); // Fonction d'affichage sommaire uniquement pour vérifier que le programme fonctionne
		
		// attente de la demande de l'utilisateur
		for (int j = 0; j < 20; j++) cmd[j] = '\0'; // vidage de cmp (inutile?)
		printf("Entrez votre commande : ");
		fgets(cmd, 20, stdin);
		//
		//printf("%c[2J", 0x1B); A revoir, pour rafraichir la console a chaque affichage
		if (readCommand(cmd, &cmd_target, &cmd_direction, &cmd_distance)) { //si la commande est correcte
			printf("Commande : deplacer la piece %d de %d case(s) dans la direction %i\n", cmd_target, cmd_distance, cmd_direction);
			/*Déroulement d'un tour du jeu*/
			if (play_move(currentGame, cmd_target, cmd_direction, cmd_distance)) {	
				if (game_over_hr(currentGame)) {
					printf("Partie finie !\n"); // peut-etre ajouter un timer ou une action avant de relancer une partie
					goto newGame;
				}
			} else {
				printf("Action impossible\n");
			}
			/*Fin du tour*/
		} else if (cmd[0] != '\0' && !streq(cmd,"exit") && !streq(cmd,"new")) { //usage si la commande est incorrecte et qu'elle n'est pas un "exit"
			printf("usage : <numeros piece> <up/left/down/right> <distance>\nAttention certaines touches, commes les flèches, peuvent changer la commande envoyée\n");
		} else if (streq(cmd,"new")) {
			printf("Nouvelle partie\n");
			goto newGame;
		}
	}
	return EXIT_SUCCESS;
}