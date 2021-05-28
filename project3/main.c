#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include "functions.h"

struct game{
	int turn; //how many turns there have been
	int playerID;
	int opID;
	int gameID;
	uint8_t row;
	uint8_t column;
	int whoseTurn;
	char symbol; //x or o
	int sock;
	char board[5][5];
};

struct game myGame;

void winner(){
	displayBoard();
	int temp;
	uint8_t buf;
	temp = read(myGame.sock, &buf, sizeof(buf));
	if(buf == 16){
		printf("X won\n");
	}
	if(buf == 17){
		printf("O won\n");
	}
	if(buf == 18){
		printf("Game ended in a draw\n");
	}
	exit(0);
}

void initBoard(){
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			myGame.board[i][j] = '.';
		}
	}
}

void displayBoard(){
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			printf("%c", myGame.board[i][j]);
		}
		printf("\n");
	}
}

void updateBoard(){
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			if(i == (myGame.column-1) && j == (myGame.row-1)){
				if(myGame.whoseTurn == 1){
					myGame.board[i][j] = myGame.symbol;
				}
				if(myGame.whoseTurn == 0){
					if(myGame.symbol == 'X'){
						myGame.board[i][j] = 'O';
					}else{
						myGame.board[i][j] = 'X';
					}
				}
			}
		}
	}

}

void oppMove(){
	displayBoard();
	myGame.whoseTurn = 0;
	int temp;
	uint8_t buf;
	temp = read(myGame.sock, &buf, sizeof(buf));
	myGame.row = buf;
	temp = read(myGame.sock, &buf, sizeof(buf));
	myGame.column = buf;
	temp = read(myGame.sock, &buf, sizeof(buf));
	printf("Opponent has placed %d, %d\n", myGame.row, myGame.column);
	updateBoard();
	myGame.turn = myGame.turn + 1;
	if(buf == 32){
		gameMove();
	}
	if(buf == 34){
		printf("Game over\n");
		winner();
	}
	if(buf == 35){
		printf("Opponent has terminated the game\n");
		exit(0);
		
	}

}			

void gameMove(){
	displayBoard();
	myGame.whoseTurn = 1;
	int temp;
	uint8_t column;
	uint8_t row;
	char input[10];
	while(1){
		printf("Enter Command\n");
		scanf("%s", input);
		if(strcmp(input, "place") == 0){
			scanf("%d, %d", &row, &column);
			//printf("%d, %d\n", row, column);
			temp = write(myGame.sock, &row, sizeof(row));
			//printf("write: %d\n", temp);
			temp = write(myGame.sock, &column, sizeof(column));
			//printf("write: %d\n", temp);
			uint8_t buf;
			read(myGame.sock, &buf, sizeof(buf));
			//printf("buffer: %d\n", buf);
			if(buf == 0){
				printf("Invalid move, try again\n");
			}
			if(buf == 33){
				printf("Move ok\n");
				myGame.row = row;
				myGame.column = column;
				updateBoard();
				myGame.turn = myGame.turn + 1;
				oppMove();
				break;
			}
			if(buf == 34){
				printf("Game over\n");
				myGame.row = row;
				myGame.column = column;
				updateBoard();
				winner();
				break;
			}
			if(buf == 35){
				printf("Opponent has terminated the game\n");	
				exit(0);
			}
		}
		if(strcmp(input, "whose") == 0){
			if(myGame.whoseTurn == 1){
				printf("Your turn\n");
			}else{
				printf("opponent's turn\n");
			}	
		}
		if(strcmp(input, "board") == 0){
			displayBoard();
		}
		if(strcmp(input, "stats") == 0){
			printf("Player ID: %d\n", myGame.playerID);
			printf("Opponent ID: %d\n", myGame.opID);
			printf("Game ID: %d\n", myGame.gameID);
			printf("Number of turns: %d\n", myGame.turn);
		}
	}

}

int main(int argc, char **argv){
	//struct game myGame;
	uint8_t move = 0;
	char *port = argv[2];
	char *machine = argv[1];
	unsigned short gameID = atoi(argv[3]);
	//printf("preferred gameID: %d\n", gameID);

	struct addrinfo* addr = NULL;
	getaddrinfo(machine, port, NULL, &addr);

	int sock;
	int connection;
	for(struct addrinfo* rp = addr; rp != NULL; rp = rp->ai_next){
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		//printf("socket: %d\n", sock);
		myGame.sock = sock;
		//printf("socket: %d\n", myGame.sock);
		if(sock < 0){
			perror("ERROR socket");
			close(sock);
			continue;
		}
		/*if(sock == -1){
			continue;
		}*/
		connection = connect(sock, rp->ai_addr, rp->ai_addrlen);
		if(connection < 0){
			perror("ERROR connect");
			close(sock);
			continue;
		}
		/*else{
			break;
		}*/
		printf("connected\n");
		while(1){
			//printf("reading...\n");
			int temp;
			unsigned short buffer;
			temp = read(sock, &buffer, sizeof(buffer));
			if(temp < 0){
				perror("ERROR read");
				close(temp);
			}
			if(temp == 0){
				close(sock);
				break;
			}
			int playerID = ntohs(buffer);
			myGame.playerID = playerID;
			
			//printf("playerID: ");
			//printf("%d\n", myGame.playerID);

			if(gameID != 0){
				gameID = htons(gameID);
				//printf("preferred gameID: %d\n", gameID);
			}
			
			temp = write(sock, &gameID, sizeof(gameID));
			if(temp < 0){
				perror("ERROR write");
				close(temp);
			}
			
			char *buf[4];
			temp = read(sock, &buf, sizeof(buf));
			if(temp < 0){
				perror("ERROR read");
				close(temp);
			}
			if(temp == 0){
				close(sock);
				break;
			}
			printf("%s\n", buf);

			temp = read(sock, &buffer, sizeof(buffer));
			gameID = ntohs(buffer);
			myGame.gameID = gameID;
			if(gameID == 0){
				printf("No game available\n");
				exit(0);
			}

			//printf("gameID: ");
			//printf("%d\n", myGame.gameID);

			temp = read(sock, &buffer, sizeof(buffer));
			playerID = ntohs(buffer);
			myGame.playerID = playerID;

			//printf("playerId: ");
			//printf("%d\n", myGame.playerID);

			temp = read(sock, &buffer, sizeof(buffer));
			int opID = ntohs(buffer);
			myGame.opID = opID;

			//printf("opponentID: ");
			//printf("%d\n", myGame.opID);

			//uint8_t move;
			temp = read(sock, &move, 1);
			//move = ntohs(move);
			//printf("move: ");
			//printf("%d\n", move);
			break;
		}
		while(1){
			int temp;
			if(move == 16){
				printf("You are x\n");
				myGame.symbol = 'X';
				initBoard();
				gameMove();
				
			}
			if(move == 17){
				printf("You are O\n");
				myGame.symbol = 'O';
				initBoard();
				oppMove();
			}
			
			break;
		}
	}
	return 0;
}
