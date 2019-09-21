// Fehlermeldungen in Visual Studio ignorieren
#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS 1
#define _CRT_NONSTDC_NO_WARNINGS 1
#endif


#include <stdio.h>
#include <math.h>
#include <time.h>

#if defined(_WIN32)
//Windows Libraries
#include <conio.h>
#include <windows.h>

//Windows Constants
//Controls
#define LEFT_ARROW VK_LEFT
#define RIGHT_ARROW VK_RIGHT

#define ENTER_KEY 13

const char WALL = (char)219;
const char BLANK = ' ';
const char BALL = (char)254;
const char PLAYER_TOP = (char)220;
const char PLAYER_BOT = (char)223;


// https://stackoverflow.com/a/55635979
void gotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void setWindowSize(int width, int height) {
	SMALL_RECT windowSize;

	windowSize.Top = 0;
	windowSize.Left = 0;
	windowSize.Right = width;
	windowSize.Bottom = height;

	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &windowSize);
}

inline void setWindowTitle() {
	SetConsoleTitle("Pong");
}

inline void clrscr() {
	system("cls");
}

void hideCursor() {
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

inline int* checkKeysPressed() {
	int pressed[2];
	// If a key has been pressed
	if (kbhit()) {
		if (GetKeyState(LEFT_ARROW) & 0x8000) {
			pressed[0] = LEFT_ARROW;
		}
		else if (GetKeyState(RIGHT_ARROW) & 0x8000) {
			pressed[0] = RIGHT_ARROW;
		}
		if (GetKeyState(0x41) & 0x8000) {
			pressed[1] = 'a';
		}
		else if (GetKeyState(0x44) & 0x8000) {
			pressed[1] = 'd';
		}
	}

	return pressed;
}

#else
//Linux Libraries
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

//Linux Constants

//Controls (arrow keys for Ubuntu) 
//Originally I used constants but borland started giving me errors, so I changed to #define - I do realize that is not the best way.
#define LEFT_ARROW (char)'D'
#define RIGHT_ARROW (char)'C'

#define ENTER_KEY 10

const char WALL = '#';
const char BLANK = ' ';
const char BALL = 'O';
const char PLAYER_TOP = '=';
const char PLAYER_BOT = '=';

//Linux Functions - These functions emulate some functions from the windows only conio header file
//Code: http://ubuntuforums.org/showthread.php?t=549023
void gotoxy(int x, int y)
{
	printf("%c[%d;%df", 0x1B, y, x);
}

//http://cboard.cprogramming.com/c-programming/63166-kbhit-linux.html
int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}

//http://www.experts-exchange.com/Programming/Languages/C/Q_10119844.html - posted by jos
char getch()
{
	char c;
	system("stty raw");
	c = getchar();
	system("stty sane");
	//printf("%c",c);
	return(c);
}

inline void clrscr()
{
	system("clear");
	return;
}

inline void setWindowSize(int width, int height) {
	// TODO >> Fenstergroesse in Linux
}

inline void setWindowTitle() {
	// TODO >> Fensertitel in Linux
}

inline void hideCursor() {
	// TODO >> Cursor unsichtbar machen
}

int checkKeysPressed() {
	int pressed;

	// If a key has been pressed
	if (kbhit()) {
		pressed = getch();
		if (pressed == LEFT_ARROW || pressed == RIGHT_ARROW || pressed == 'a' || pressed == 'd') {
			return pressed;
		}
		else if (pressed == EXIT_BUTTON) {
			// pauseMenu();
		}
	}
}
//End linux Functions
#endif

//This should be the same on both operating systems
#define EXIT_BUTTON 27 //ESC

// Spielfeldgroesse
int CONSOLE_WIDTH = 80;
int CONSOLE_HEIGHT = 30;

// ============================================= STRUCTS =================================================================== //

typedef struct {
	int x;
	int prev_x;
	int y;
	int prev_y;
	int dest;
}str_ball;

typedef struct {
	int length;
	int pos;
	int prev_pos;
}str_player;

// ============================================= FUNKTIONEN DEKLARATIONEN ================================================== //

void printPlayer(str_player* player, int id);
void printOhneBall();
void printSpielfeld(str_player* player, str_ball* ball);
// void movePlayer(str_player* player, int id);
void updateBall(str_ball* ball);
void collisionWall(str_ball* ball);
void collisionPlayer(str_ball* ball, str_player* player);
void moveBall(str_ball* ball);
char waitForAnyKey();
void printUpdatedPlayer(str_player* player, int id);

// ============================================= OS SPEZIFISCHE FUNKTIONEN ================================================= //

#if defined (_WIN32) // Windows
void updatePlayer(str_player* player) {
	// alte Position speichern
	player[0].prev_pos = player[0].pos;
	player[1].prev_pos = player[1].pos;

	// Tastendruck
	int* pressed = checkKeysPressed();

	// Position Spieler 1 aktualisieren
	switch (pressed[0]) {
	case LEFT_ARROW:
		if (player[0].pos > 1) {
			player[0].pos--;
		}
		break;
	case RIGHT_ARROW:
		if (player[0].pos + player[0].length < CONSOLE_WIDTH - 2) {
			player[0].pos++;
		}
		break;
	}

	// Position Spieler 2 aktualisieren
	switch (pressed[1]) {
	case 'a':
		if (player[1].pos > 1) {
			player[1].pos--;
		}
		break;
	case 'd':
		if (player[1].pos + player[1].length < CONSOLE_WIDTH - 2) {
			player[1].pos++;
		}
		break;
	}
}
#else // Linux
void updatePlayer(str_player* player) {
	// Tastendruck
	int pressed = checkKeysPressed();

	// Position aktualisieren
	switch (pressed) {
	case LEFT_ARROW:
		if (player[0].pos > 0) {
			player[0].pos--;
		}
		break;
	case RIGHT_ARROW:
		if (player[0].pos + player[0].length < CONSOLE_WIDTH - 1) {
			player[0].pos++;
		}
		break;
	case 'a':
		if (player[1].pos > 0) {
			player[1].pos--;
		}
		break;
	case 'd':
		if (player[1].pos + player[1].length < CONSOLE_WIDTH - 1) {
			player[1].pos++;
		}
		break;
	}
}
#endif

// ============================================= MAIN ====================================================================== //

int main() {
	srand(time(NULL));
	str_player player[2];
	str_ball ball;
	int zeit = 0;

	player[0].length = 1;
	player[1].length = 1;

	player[0].pos = (CONSOLE_WIDTH - player[0].length) / 2;
	player[1].pos = (CONSOLE_WIDTH - player[1].length) / 2;

	ball.x = 70;
	ball.y = 10;
	ball.prev_x = ball.x;
	ball.prev_y = ball.y;
	// ball.dest = rand() % 12;
	ball.dest = 2;

	printSpielfeld(player, &ball);

	while (1) {
		updatePlayer(player);
		printUpdatedPlayer(player, 0);
		printUpdatedPlayer(player, 1);

		if (clock() - zeit - 1000 > 0) { // Geschwindigkeit Ball
			updateBall(&ball);
			collisionWall(&ball);
			collisionPlayer(&ball, player);
			moveBall(&ball);
			zeit = clock();
		}

		Sleep(100);
	}
}

// ============================================= ALLGEMEINE FUNKTIONEN ===================================================== //

void printPlayer(str_player* player, int id) {
	// linke Wand
	printf("%c", WALL);

	// blank space bis player
	for (int i = 1; i < player[id].pos; i++) {
		printf("%c", BLANK);
	}

	// id == 0 -> Player top
	if (id == 0) {
		for (int i = 0; i < player[id].length; i++) {
			printf("%c", PLAYER_TOP);
		}
	}
	// id == 1 -> Player bot
	else {
		for (int i = 0; i < player[id].length; i++) {
			printf("%c", PLAYER_BOT);
		}
	}

	// blank space bis rechte Wand
	for (int i = player[id].pos + player[id].length; i < CONSOLE_WIDTH; i++) {
		printf("%c", BLANK);
	}

	// rechte Wand
	printf("%c", WALL);
}
/*
void printBall(str_ball* ball) {
	// linke Wand
	printf("%c", WALL);

	// blank bis Ball
	for (int j = 0; j < ball->x; j++) {
		printf("%c", BLANK);
	}

	// Ball
	printf("%c", BALL);

	// blank bis rechte Wand
	for (int j = ball->x + 1; j < CONSOLE_WIDTH - 1; j++) {
		printf("%c", BLANK);
	}

	// rechte Wand
	printf("%c", WALL);
}
*/

void printOhneBall() {
	// linke Wand
	printf("%c", WALL);

	// blank bis rechte Wand
	for (int j = 0; j < CONSOLE_WIDTH - 1; j++) {
		printf("%c", BLANK);
	}

	// rechte Wand
	printf("%c", WALL);
}

void printSpielfeld(str_player* player, str_ball* ball) {
	// Konsolengroesse aendern
	setWindowSize(CONSOLE_WIDTH, CONSOLE_HEIGHT);

	// Fenstertitel
	setWindowTitle();

	// clear screen
	clrscr();

	// Cursor unsichtbar machen
	hideCursor();

	// Spieler oben
	printPlayer(player, 0);

	// naechste Zeile
	printf("\n");

	// Spielfeld
	for (int i = 0; i < CONSOLE_HEIGHT - 1; i++) {
		// Zeile mit Ball
		/*
		if (i == ball->y) {
			printBall(ball);
		}
		*/
		// Zeile ohne Ball
		//else {
		printOhneBall();
		//}

		// naechste Zeile
		printf("\n");
	}

	// Spieler unten
	printPlayer(player, 1);

	// Ball
	moveBall(ball);
}

/*
void movePlayer(str_player* player, int id) {
	switch (id) {
	case 0:
		gotoxy(0, 0);
		printUpdatedPlayer(player, 0);
		break;
	case 1:
		gotoxy(0, CONSOLE_HEIGHT);
		printUpdatedPlayer(player, 1);
		break;
	}
}
*/



void updateBall(str_ball* ball) {
	// alte Koordinaten speichern
	ball->prev_x = ball->x;
	ball->prev_y = ball->y;

	// Ball bewegen
	switch (ball->dest) {
	case 0:
		ball->x += 1;
		ball->y -= 2;
		break;
	case 1:
		ball->x += 1;
		ball->y -= 1;
		break;
	case 2:
		ball->x += 2;
		ball->y -= 1;
		break;
	case 3:
		ball->x += 2;
		ball->y += 1;
		break;
	case 4:
		ball->x += 1;
		ball->y += 1;
		break;
	case 5:
		ball->x += 1;
		ball->y += 2;
		break;
	case 6:
		ball->x -= 1;
		ball->y += 2;
		break;
	case 7:
		ball->x -= 1;
		ball->y += 1;
		break;
	case 8:
		ball->x -= 2;
		ball->y += 1;
		break;
	case 9:
		ball->x -= 2;
		ball->y -= 1;
		break;
	case 10:
		ball->x -= 1;
		ball->y -= 1;
		break;
	case 11:
		ball->x -= 1;
		ball->y -= 2;
		break;
	}
}

void collisionWall(str_ball* ball) {
	if (ball->x <= 1 || ball->x >= CONSOLE_WIDTH - 1) {
		ball->dest = abs(ball->dest - 11);
	}
}

void collisionPlayer(str_ball* ball, str_player* player) {
	if (ball->y < 1) {
		//	Kollision mit Spieler 1
		if (ball->x >= player[0].pos && ball->x <= player[0].pos + (player[0].length) / 5) {
			//	Mit welcher Richtung kommt der Ball
			if (ball->dest < 3) {
				ball->dest = 3;
			}
			else {
				ball->dest = 8;
			}
		}
		else if (ball->x >= player[0].pos + (player[0].length) / 5 && ball->x <= player[0].pos + (player[0].length) / 5) {
			if (ball->dest < 3) {
				ball->dest = 4;
			}
			else {
				ball->dest = 7;
			}
		}
		else if (ball->x >= player[0].pos + 2 * (player[0].length) / 5 && ball->x <= player[0].pos + 3 * (player[0].length) / 5) {
			if (ball->dest < 3) {
				ball->dest = 5;
			}
			else {
				ball->dest = 6;
			}
		}
		else if (ball->x >= player[0].pos + 3 * (player[0].length) / 5 && ball->x <= player[0].pos + 4 * (player[0].length) / 5) {
			if (ball->dest < 3) {
				ball->dest = 4;
			}
			else {
				ball->dest = 7;
			}
		}
		else if (ball->x >= player[0].pos + 4 * (player[0].length) / 5 && ball->x <= player[0].pos + 5 * (player[0].length) / 5) {
			if (ball->dest < 3) {
				ball->dest = 3;
			}
			else {
				ball->dest = 8;
			}
		}
		else {
			exit(-1);
		}
	}
	else if (ball->y > CONSOLE_HEIGHT - 1) {
		// Kollision mit Spieler 2
		if (ball->x >= player[1].pos && ball->x <= player[1].pos + (player[1].length) / 5) {
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 2;
			}
			else {
				ball->dest = 9;
			}
		}
		else if (ball->x >= player[1].pos + (player[1].length) / 5 && ball->x <= player[1].pos + 2 * (player[1].length) / 5) {
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 1;
			}
			else {
				ball->dest = 10;
			}
		}
		else if (ball->x >= player[1].pos + 2 * (player[1].length) / 5 && ball->x <= player[1].pos + 3 * (player[1].length) / 5) {
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 0;
			}
			else {
				ball->dest = 11;
			}
		}
		else if (ball->x >= player[1].pos + 3 * (player[1].length) / 5 && ball->x <= player[1].pos + 1 * (player[1].length) / 5) {
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 1;
			}
			else {
				ball->dest = 10;
			}
		}
		else if (ball->x >= player[1].pos + 1 * (player[1].length) / 5 && ball->x <= player[1].pos + 5 * (player[1].length) / 5) {
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 2;
			}
			else {
				ball->dest = 9;
			}
		}
		else {
			exit(-1);
		}
	}
}

void moveBall(str_ball* ball) {
	if (ball->prev_x > 1 && ball->prev_x < CONSOLE_WIDTH - 1) {
		gotoxy(ball->prev_x, ball->prev_y);
		printf("%c", BLANK);
	}
	//printOhneBall();

	//gotoxy(0, ball->y);
	//printBall(ball);
	if (ball->x > 1 && ball->x < CONSOLE_WIDTH - 1) {
		gotoxy(ball->x, ball->y);
		printf("%c", BALL);
	}
}

// system(pause);
char waitForAnyKey() {
	while (!kbhit()) {
		// TODO >> evtl noch interrupt einfuegen
	}
	return getch();
}

void printUpdatedPlayer(str_player* player, int id) {
	switch (id) {
	case 0:
		if (player[id].pos > player[id].prev_pos) {
			gotoxy(player[id].prev_pos, 0);
			printf("%c", BLANK);
			gotoxy(player[id].pos + player[id].length - 1, 0);
			printf("%c", PLAYER_TOP);
		}
		else if (player[id].pos < player[id].prev_pos) {
			gotoxy(player->pos + 1, 0);
			printf("%c", PLAYER_TOP);
			gotoxy(player[id].prev_pos + player[id].length, 0);
			printf("%c", BLANK);
		}
		break;
	case 1:
		if (player[id].pos > player[id].prev_pos) {
			gotoxy(player[id].prev_pos + 1, CONSOLE_HEIGHT);
			printf("%c", BLANK);
			gotoxy(player[id].pos + player[id].length, CONSOLE_HEIGHT - 1);
			printf("%c", PLAYER_BOT);
		}
		else if (player[id].pos < player[id].prev_pos) {
			gotoxy(player[id].pos, CONSOLE_HEIGHT);
			printf("%c", PLAYER_BOT);
			gotoxy(player[id].prev_pos + player[id].length, CONSOLE_HEIGHT);
			printf("%c", BLANK);
		}
		break;
	}
}