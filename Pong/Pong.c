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

#define UP_ARROW VK_UP
#define DOWN_ARROW VK_DOWN

#define ENTER_KEY 13

const char WALL = (char)219;
const char BLANK = ' ';
const char BALL = (char)254;
const char PLAYER_TOP = (char)220;
const char PLAYER_BOT = (char)223;

#else
//Linux Libraries
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#include <curses.h>

//Linux Constants

//Controls (arrow keys for Ubuntu) 
//Originally I used constants but borland started giving me errors, so I changed to #define - I do realize that is not the best way.
#define LEFT_ARROW 260
#define RIGHT_ARROW 261

#define UP_ARROW 259
#define DOWN_ARROW 258

#define ENTER_KEY 10

const char WALL = '#';
const char BLANK = ' ';
const char BALL = 'O';
const char PLAYER_TOP = '=';
const char PLAYER_BOT = '=';

/*
//Linux Functions - These functions emulate some functions from the windows only conio header file
//Code: http://ubuntuforums.org/showthread.php?t=549023
void gotoxy(int x, int y)
{
	print("%c[%d;%df", 0x1B, y, x);
}
*/

/*
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
*/

/*
//http://www.experts-exchange.com/Programming/Languages/C/Q_10119844.html - posted by jos
char getch()
{
	char c;
	system("stty raw");
	c = getchar();
	system("stty sane");
	//print("%c",c);
	return(c);
}
*/

//End linux Functions
#endif

//This should be the same on both operating systems
#define EXIT_BUTTON 27 //ESC

// Konsolengroesse (am besten ungerade, damit Ball in der Mitte starten kann)
int CONSOLE_WIDTH = 100;
int CONSOLE_HEIGHT = 30; // Spielfeld + Header

// Anzahl an Zeilen ueber dem Spielfeld
int HEADER_HEIGHT = 2;

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

void updatePlayer(str_player* player);
void printPlayer(str_player* player, int id);
void printOhneBall();
void printSpielfeld(str_player* player, str_ball* ball, int score1, int score2);
void updateBall(str_ball* ball);
void collisionWall(str_ball* ball);
int collisionPlayer(str_ball* ball, str_player* player);
void moveBall(str_ball* ball, str_player* player);
//char waitForAnyKey();
void printUpdatedPlayer(str_player* player, int id);
void printScore(int score1, int score2);
void startscreen();
void loadGame();
void exitYN();
int menuSelector(int x, int y, int yStart);
int mainMenu();
void spielende();
int getschwierigkeitsgrad();
int getmapgrosse();
int setschwierigkeitsgrad(str_player* player);
void setmapgrosse();
void sleepProcess(int milliseconds);
void setWindowSize(int width, int height);
void setWindowTitle();
void clrscr();
void hideCursor();
void gotoxy(int x, int y);
void sysPause();
int exitgame();

// ============================================= MAIN ====================================================================== //

int main() {

#if !defined(_WIN32)
	initscr();
	keypad(stdscr, TRUE);
	noecho();
	nodelay(stdscr, TRUE);
#endif

	// Konsolengroesse aendern
	setWindowSize(CONSOLE_WIDTH, CONSOLE_HEIGHT);

	// Fenstertitel
	setWindowTitle();

	// clear screen
	clrscr();

	// Cursor unsichtbar machen
	hideCursor();

	// Startbildschirm
	startscreen();

	do {
		switch (mainMenu()) {
		case 0:
			loadGame();
			break;
		case 1:
			//displayHighScores();
			break;
		case 2:
			exitYN();
			break;
		}
	} while (1);	//

#if !defined(_WIN32)
	endwin();
#endif

	return 0;
}

// ============================================= OS SPEZIFISCHE FUNKTIONEN ================================================= //

#if defined (_WIN32) // Windows

#define print(...) printf(__VA_ARGS__)

void setWindowSize(int width, int height) {
	SMALL_RECT windowSize;

	windowSize.Top = 0;
	windowSize.Left = 0;
	windowSize.Right = width;
	windowSize.Bottom = height;

	COORD coord;
	coord.X = width + 1;
	coord.Y = height + 1;

	HANDLE Handle = GetStdHandle(STD_OUTPUT_HANDLE);

	//coord = GetLargestConsoleWindowSize(Handle);
	SetConsoleScreenBufferSize(Handle, coord);				// Textbuffergroesse setzen (kein Scrollbalken)
	SetConsoleWindowInfo(Handle, TRUE, &windowSize);		// Fenstergroesse setzen

	// https://stackoverflow.com/a/46145911 (Entfert maximieren Button)
	HWND hwnd = GetConsoleWindow();
	DWORD style = GetWindowLong(hwnd, GWL_STYLE);
	style &= ~WS_MAXIMIZEBOX;
	style &= ~WS_HSCROLL;
	style &= ~WS_VSCROLL;
	SetWindowLong(hwnd, GWL_STYLE, style);
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_SHOWWINDOW);
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

int* checkKeysPressed() {
	int pressed[4];
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

		if (GetKeyState(UP_ARROW) & 0x8000) {
			pressed[2] = UP_ARROW;
		}
		else if (GetKeyState(DOWN_ARROW) & 0x8000) {
			pressed[2] = DOWN_ARROW;
		}

		if (GetKeyState(VK_RETURN) & 0x8000) {
			pressed[3] = ENTER_KEY;
		}

	}

	return pressed;
}

void updatePlayer(str_player* player) {
	// alte Position speichern
	player[0].prev_pos = player[0].pos;
	player[1].prev_pos = player[1].pos;

	// Tastendruck
	int* pressed = checkKeysPressed();

	// Position Spieler 1 aktualisieren
	switch (pressed[0]) {
	case LEFT_ARROW:
		if (player[0].pos > 0) {
			player[0].pos--;
		}
		break;
	case RIGHT_ARROW:
		if (player[0].pos + player[0].length < CONSOLE_WIDTH) {
			player[0].pos++;
		}
		break;
	}

	// Position Spieler 2 aktualisieren
	switch (pressed[1]) {
	case 'a':
		if (player[1].pos > 0) {
			player[1].pos--;
		}
		break;
	case 'd':
		if (player[1].pos + player[1].length < CONSOLE_WIDTH) {
			player[1].pos++;
		}
		break;
	}
}

// Zeit seit Prozessstart
inline void sleepProcess(int milliseconds) {
	Sleep(milliseconds);
}

// Im Menu bewegen 
int menuSelector(int x, int y, int yStart) {
	int enter = 0;
	int i = 0;
	x = x - 2;
	gotoxy(x, yStart);

	print(">");

	while (enter == 0) {
		int* pressed;
		pressed = checkKeysPressed();

		if (pressed[2] == UP_ARROW) {
			gotoxy(x, yStart + i);
			print(" ");

			if (yStart >= yStart + i) {
				i = y - yStart - 2;
			}
			else {
				i--;
			}
			gotoxy(x, yStart + i);
			print(">");
		}
		else
			if (pressed[2] == DOWN_ARROW)
			{
				gotoxy(x, yStart + i);
				print(" ");

				if (i + 2 >= y - yStart) {
					i = 0;
				}
				else {
					i++;
				}
				gotoxy(x, yStart + i);
				print(">");
			}
		if (pressed[3] == ENTER_KEY) {
			enter = 1;
		}
		sleepProcess(100);
	}
	return i;
}


// https://stackoverflow.com/a/55635979
void gotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void sysPause() {
	system("pause");
}

#else // Linux

#define print(...) printw(__VA_ARGS__)

inline void clrscr()
{
	clear();
	refresh();
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

	pressed = getch();

	if (pressed != EXIT_BUTTON) {
		flushinp();
		return pressed;
	}

	flushinp();
	return 0;
}

void updatePlayer(str_player* player) {
	// alte Position speichern
	player[0].prev_pos = player[0].pos;
	player[1].prev_pos = player[1].pos;

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
		if (player[0].pos + player[0].length < CONSOLE_WIDTH) {
			player[0].pos++;
		}
		break;
	case 'a':
		if (player[1].pos > 0) {
			player[1].pos--;
		}
		break;
	case 'd':
		if (player[1].pos + player[1].length < CONSOLE_WIDTH) {
			player[1].pos++;
		}
		break;
	}
}

void sleepProcess(int milliseconds) {
	usleep(milliseconds * 1000);
}

// Im Menue bewegen
int menuSelector(int x, int y, int yStart) {
	int enter = 0;
	int i = 0;
	x = x - 2;
	gotoxy(x, yStart);

	print(">");

	while (enter == 0) {
		int pressed;
		pressed = checkKeysPressed();

		if (pressed == UP_ARROW) {
			gotoxy(x, yStart + i);
			print(" ");

			if (yStart >= yStart + i) {
				i = y - yStart - 2;
			}
			else {
				i--;
			}
			gotoxy(x, yStart + i);
			print(">");
		}
		else
			if (pressed == DOWN_ARROW)
			{
				gotoxy(x, yStart + i);
				print(" ");

				if (i + 2 >= y - yStart) {
					i = 0;
				}
				else {
					i++;
				}
				gotoxy(x, yStart + i);
				print(">");
			}
		if (pressed == ENTER_KEY) {
			enter = 1;
		}
		sleepProcess(100);
	}
	return i;
}

void gotoxy(int x, int y) {
	move(y, x);
}

void sysPause() {
	while (getch() == -1) {
		usleep(100);
	}
}

#endif

// ============================================= ALLGEMEINE FUNKTIONEN ===================================================== //

void printPlayer(str_player* player, int id) {
	// linke Wand
	print("%c", WALL);

	// blank space bis player
	for (int i = 1; i < player[id].pos; i++) {
		print("%c", BLANK);
	}

	// id == 0 -> Player top
	if (id == 0) {
		for (int i = 0; i < player[id].length; i++) {
			print("%c", PLAYER_TOP);
		}
	}
	// id == 1 -> Player bot
	else {
		for (int i = 0; i < player[id].length; i++) {
			print("%c", PLAYER_BOT);
		}
	}

	// blank space bis rechte Wand
	for (int i = player[id].pos + player[id].length; i < CONSOLE_WIDTH; i++) {
		print("%c", BLANK);
	}

	// rechte Wand
	print("%c", WALL);
}

void printOhneBall() {
	// linke Wand
	print("%c", WALL);

	// blank bis rechte Wand
	for (int j = 0; j < CONSOLE_WIDTH - 1; j++) {
		print("%c", BLANK);
	}

	// rechte Wand
	print("%c", WALL);
}

void printSpielfeld(str_player* player, str_ball* ball, int score1, int score2) {

	// clear screen
	clrscr();

	// Text fuer Punkte
	print("Punkte Spieler 1:");
	gotoxy(CONSOLE_WIDTH - 20, 0);
	print("Punkte Spieler 2:");

	// Punktezahlen ausgeben
	printScore(score1, score2);

	print("\n\n");

	// Spieler oben
	printPlayer(player, 0);

	// naechste Zeile
	print("\n");

	// Spielfeld
	for (int i = HEADER_HEIGHT; i < CONSOLE_HEIGHT - 1; i++) {
		// Zeile ohne Ball
		printOhneBall();

		// naechste Zeile
		print("\n");
	}

	// Spieler unten
	printPlayer(player, 1);

	// Ball
	moveBall(ball, player);
}

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

int collisionPlayer(str_ball* ball, str_player* player) {
	// Ball am oberen Rand
	if (ball->y <= HEADER_HEIGHT) {
		// Kollision mit Spieler 1
		if (ball->x >= player[0].pos && ball->x <= player[0].pos + (player[0].length / 5)) {
			// Aus welcher Richtung kommt der Ball
			if (ball->dest < 3) {
				ball->dest = 3;
				return 1;
			}
			else {
				ball->dest = 8;
				return 1;
			}
		}
		else if (ball->x >= player[0].pos + (player[0].length / 5) && ball->x <= player[0].pos + 2 * (player[0].length / 5)) {
			if (ball->dest < 3) {
				ball->dest = 4;
				return 1;
			}
			else {
				ball->dest = 7;
				return 1;
			}
		}
		else if (ball->x >= player[0].pos + 2 * (player[0].length / 5) && ball->x <= player[0].pos + 3 * (player[0].length / 5)) {
			if (ball->dest < 3) {
				ball->dest = 5;
				return 1;
			}
			else {
				ball->dest = 6;
				return 1;
			}
		}
		else if (ball->x >= player[0].pos + 3 * (player[0].length / 5) && ball->x <= player[0].pos + 4 * (player[0].length / 5)) {
			if (ball->dest < 3) {
				ball->dest = 4;
				return 1;
			}
			else {
				ball->dest = 7;
				return 1;
			}
		}
		else if (ball->x >= player[0].pos + 4 * (player[0].length / 5) && ball->x <= player[0].pos + 5 * (player[0].length / 5)) {
			if (ball->dest < 3) {
				ball->dest = 3;
				return 1;
			}
			else {
				ball->dest = 8;
				return 1;
			}

		}
		else {
			return -1;
		}
	}

	// Ball am unteren Rand
	else if (ball->y >= CONSOLE_HEIGHT) {
		// Kollision mit Spieler 2
		if (ball->x >= player[1].pos && ball->x <= player[1].pos + (player[1].length / 5)) {
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 2;
				return 2;
			}
			else {
				ball->dest = 9;
				return 2;
			}
		}
		else if (ball->x >= player[1].pos + (player[1].length / 5) && ball->x <= player[1].pos + 2 * (player[1].length / 5)) {
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 1;
				return 2;
			}
			else {
				ball->dest = 10;
				return 2;
			}
		}
		else if (ball->x >= player[1].pos + 2 * (player[1].length / 5) && ball->x <= player[1].pos + 3 * (player[1].length / 5)) {
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 0;
				return 2;
			}
			else {
				ball->dest = 11;
				return 2;
			}
		}
		else if (ball->x >= player[1].pos + 3 * (player[1].length / 5) && ball->x <= player[1].pos + 4 * (player[1].length / 5)) {
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 1;
				return 2;
			}
			else {
				ball->dest = 10;
				return 2;
			}
		}
		else if (ball->x >= player[1].pos + 4 * (player[1].length / 5) && ball->x <= player[1].pos + 5 * (player[1].length / 5)) {
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 2;
				return 2;
			}
			else {
				ball->dest = 9;
				return 2;
			}
		}
		else {
			return -1;
		}
	}
	else {
		return 0;
	}
}

void moveBall(str_ball* ball, str_player* player) {
	if ((ball->prev_x > 1 && ball->prev_x < CONSOLE_WIDTH - 1) && (ball->prev_y > HEADER_HEIGHT && ball->prev_y < CONSOLE_HEIGHT)) {
		gotoxy(ball->prev_x, ball->prev_y);
		print("%c", BLANK);
	}

	if (ball->x > 1 && ball->x < CONSOLE_WIDTH - 1) {
		if (ball->y <= HEADER_HEIGHT) {
			if (ball->x < player[0].pos || ball->x > player[0].pos + player[0].length) {
				gotoxy(ball->x, ball->y);
				print("%c", BALL);
			}
		}
		else if (ball->y >= CONSOLE_HEIGHT) {
			if (ball->x < player[1].pos || ball->x > player[1].pos + player[1].length) {
				gotoxy(ball->x, ball->y);
				print("%c", BALL);
			}
		}
		else {
			gotoxy(ball->x, ball->y);
			print("%c", BALL);
		}
	}
}

// system(pause);
/*char getkwaitForAnyKey() {
	while (!kbhit()) {
		// TODO >> evtl noch interrupt einfuegen
	}
	return getch();
}*/

void printUpdatedPlayer(str_player* player, int id) {
	switch (id) {
	case 0:
		if (player[id].pos > player[id].prev_pos) {
			if (player[id].prev_pos > 0) {
				gotoxy(player[id].prev_pos, HEADER_HEIGHT);
				print("%c", BLANK);
			}
			gotoxy(player[id].pos + player[id].length - 1, HEADER_HEIGHT);
			print("%c", PLAYER_TOP);
		}
		else if (player[id].pos < player[id].prev_pos) {
			gotoxy(player[id].pos + 1, HEADER_HEIGHT);
			print("%c", PLAYER_TOP);
			if (player[id].prev_pos + player[id].length < CONSOLE_WIDTH) {
				gotoxy(player[id].prev_pos + player[id].length, HEADER_HEIGHT);
				print("%c", BLANK);
			}
		}
		break;
	case 1:
		if (player[id].pos > player[id].prev_pos) {
			if (player[id].prev_pos > 0) {
				gotoxy(player[id].prev_pos, CONSOLE_HEIGHT);
				print("%c", BLANK);
			}
			gotoxy(player[id].pos + player[id].length - 1, CONSOLE_HEIGHT);
			print("%c", PLAYER_BOT);
		}
		else if (player[id].pos < player[id].prev_pos) {
			gotoxy(player[id].pos + 1, CONSOLE_HEIGHT);
			print("%c", PLAYER_BOT);
			if (player[id].prev_pos + player[id].length < CONSOLE_WIDTH) {
				gotoxy(player[id].prev_pos + player[id].length, CONSOLE_HEIGHT);
				print("%c", BLANK);
			}
		}
		break;
	}
}

void printScore(int score1, int score2) {
	gotoxy(18, 0);
	print("%3d", score1);
	gotoxy(CONSOLE_WIDTH - 2, 0);
	print("%3d", score2);
}

void loadGame() {
	srand(time(NULL));
	str_player player[2];
	str_ball ball;
	int zeit = 0;
	long vergangeneZeit;
	int score1 = 0;
	int score2 = 0;
	int gameover = 0;

#if !defined(_WIN32)
	struct timeval start, end;
	long secs, usecs;

	gettimeofday(&start, NULL);
#endif

	int difficulty = setschwierigkeitsgrad(player);
	setmapgrosse();

	player[0].pos = (CONSOLE_WIDTH - player[0].length) / 2;
	player[1].pos = (CONSOLE_WIDTH - player[1].length) / 2;
	// TODO >> player.length gerade/ ungerade (Spielfeldbreite ungerade -> Ball in der Mitte, aber Spieler nicht)

	// Debug
	ball.x = 50;
	ball.y = 10;
	ball.dest = 0;
	// ball.prev_x = ball.x;
	// ball.prev_y = ball.y;

	// Release
	// ball.x = 1 + (CONSOLE_WIDTH - 2) / 2;
	// ball.y = HEADER_HEIGHT + (CONSOLE_HEIGHT - HEADER_HEIGHT) / 2;
	// ball.dest = rand() % 12;


	printSpielfeld(player, &ball, score1, score2);

	while (gameover == 0) {
		updatePlayer(player);
		printUpdatedPlayer(player, 0);
		printUpdatedPlayer(player, 1);

#if defined(_WIN32)
		vergangeneZeit = clock();
#else // https://stackoverflow.com/a/9871230 (Vergangene Zeit seit Prozessstart im Millisekunden
		gettimeofday(&end, NULL);
		secs = end.tv_sec - start.tv_sec;
		usecs = end.tv_usec - start.tv_usec;
		vergangeneZeit = ((secs) * 1000 + usecs / 1000.0) + 0.5;
#endif

		if ((vergangeneZeit - zeit - 1000) > 0) { // Geschwindigkeit Ball
			updateBall(&ball);
			collisionWall(&ball);
			switch (collisionPlayer(&ball, player)) {
			case -1:
				// Game Over
				spielende();
				gameover = 1;
				break;
			case 0:
				// Nichts, Ball nicht in Spielernaehe
				break;
			case 1:
				// Punkt fuer Spieler 1
				score1++;
				break;
			case 2:
				// Punkt fuer Spieler 2
				score2++;
				break;
			}
			printScore(score1, score2);
			moveBall(&ball, player);

#if defined(_WIN32)
			zeit = clock();
#else // https://stackoverflow.com/a/9871230 (Vergangene Zeit seit Prozessstart im Millisekunden
			gettimeofday(&end, NULL);
			secs = end.tv_sec - start.tv_sec;
			usecs = end.tv_usec - start.tv_usec;
			zeit = ((secs) * 1000 + usecs / 1000.0) + 0.5;
#endif
		}

		sleepProcess(100);
	}
}

int mainMenu() {

	int x = 10, y = 5;
	int yStart = y;

	int selected;

	clrscr(); //clear the console

	gotoxy(x, y++);
	print("New Game\n");
	gotoxy(x, y++);
	print("HighScore\n");
	gotoxy(x, y++);
	print("Exit\n");
	gotoxy(x, y++);

	selected = menuSelector(x, y, yStart);

	return selected;
}

//	Alternatives ASCII Art
/*void startscreen(){
	//	ASCII Art Quelle: http://pong.ascii.uk/
	clrscr();

	for(int i=0; i<((CONSOLE_HEIGHT/2)-6); i++){
	print("\n");
	}

	print("\t\t\t88888b.  .d88b. 88888b.  .d88b.  \n");
	print("\t\t\t888 \"88bd88\"\"88b888 \"88bd88P\"88b\n");
	print("\t\t\t888  888888  888888  888888  888 \n");
	print("\t\t\t888 d88PY88..88P888  888Y88b 888 \n");
	print("\t\t\t88888P\"  \"Y88P\" 888  888 \"Y88888 \n");
	print("\t\t\t888                          888 \n");
	print("\t\t\t888                     Y8b d88P \n");
	print("\t\t\t888                      \"Y88P\"  \n");
	print("\t\t\tDruecke irgendeinen Key...... ");

	sytem("pause");
	return;
}*/

// Note: "\\" schreibt nur einmal "\"
void startscreen() {
	//	ASCII Art Quelle: http://pong.ascii.uk/

	clrscr();

	gotoxy((CONSOLE_WIDTH / 2) - 13, (CONSOLE_HEIGHT / 2) - 4); //	Mein Versuch das ganze irgendwie mittig zu machen...
															//	Alternative: vgl. oben
	print(" _ __   ___  _ __   __ _ \n");

	gotoxy((CONSOLE_WIDTH / 2) - 13, (CONSOLE_HEIGHT / 2) - 3);
	print("| '_ \\ / _ \\| '_ \\ / _` |\n");

	gotoxy((CONSOLE_WIDTH / 2) - 13, (CONSOLE_HEIGHT / 2) - 2);
	print("| |_) | (_) | | | | (_| |\n");

	gotoxy((CONSOLE_WIDTH / 2) - 13, (CONSOLE_HEIGHT / 2) - 1);
	print("| .__/ \\___/|_| |_|\\__, |\n");

	gotoxy((CONSOLE_WIDTH / 2) - 13, (CONSOLE_HEIGHT / 2));
	print("| |                 __/ |\n");

	gotoxy((CONSOLE_WIDTH / 2) - 13, (CONSOLE_HEIGHT / 2) + 1);
	print("|_|                |___/ \n");

	gotoxy((CONSOLE_WIDTH / 2) - 13, (CONSOLE_HEIGHT / 2) + 2);

	sysPause();
}

int exitgame() {

	int x = 10, y = 5;
	int yStart = y;

	int selected;

	clrscr(); //clear the console
	gotoxy(x - 3, y - 1);
	print("Willst du das Spiel wirklich beenden?");
	gotoxy(x, y++);
	print("Ja\n");
	gotoxy(x, y++);
	print("Nein\n");
	gotoxy(x, y++);


	selected = menuSelector(x, y, yStart);

	return selected;
}

void exitYN() {
	clrscr();

	int ende = exitgame();

	if (ende == 0)
	{
		clrscr(); //clear the console
		exit(1);
	}
}

int getschwierigkeitsgrad() {

	int x = 10, y = 5;
	int yStart = y;

	int selected;

	clrscr(); //clear the console
	gotoxy(x - 3, y - 1);
	print("Waehle deinen Schwierigkeitsgrad");
	gotoxy(x, y++);
	print("leicht\n");
	gotoxy(x, y++);
	print("mittel\n");
	gotoxy(x, y++);
	print("schwer\n");
	gotoxy(x, y++);

	selected = menuSelector(x, y, yStart);

	return selected;
}

int getmapgrosse() {

	int x = 10, y = 5;
	int yStart = y;

	int selected;

	clrscr(); //clear the console
	gotoxy(x - 3, y - 1);
	print("Waehle deine Map groesse");
	gotoxy(x, y++);
	print("klein\n");
	gotoxy(x, y++);
	print("mittel\n");
	gotoxy(x, y++);
	print("gross\n");
	gotoxy(x, y++);

	selected = menuSelector(x, y, yStart);

	return selected;
}

int setschwierigkeitsgrad(str_player* player) {

	int difficulty = getschwierigkeitsgrad() + 1;

	switch (difficulty) {
	case 1:
		player[0].length = (CONSOLE_WIDTH / 5);
		player[1].length = (CONSOLE_WIDTH / 5);
		break;
	case 2:
		player[0].length = (CONSOLE_WIDTH / 10);
		player[1].length = (CONSOLE_WIDTH / 10);
		break;
	case 3:
		player[0].length = (CONSOLE_WIDTH / 20);
		player[1].length = (CONSOLE_WIDTH / 20);
		break;
	}

	return difficulty;
}

void setmapgrosse() {
	int grosse = getmapgrosse() + 1;

	CONSOLE_HEIGHT = grosse * CONSOLE_HEIGHT;
	CONSOLE_WIDTH = grosse * CONSOLE_WIDTH;

	setWindowSize(CONSOLE_WIDTH, CONSOLE_HEIGHT);
}

void spielende() {
	clrscr();

	gotoxy(CONSOLE_WIDTH / 2, CONSOLE_HEIGHT / 2);
	print("Gameover\n");

	CONSOLE_HEIGHT = 30;
	CONSOLE_WIDTH = 100;

	setWindowSize(CONSOLE_WIDTH, CONSOLE_HEIGHT);

	sysPause();
}