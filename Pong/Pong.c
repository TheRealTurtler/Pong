/*	Code von Michael Diedler und Kai Frisch
	Anfaenge wurden aus dem Snake-Code uebernommen, sind aber stark abgeaendert worden
	Sonstige Quellen sind im Code vermerkt
	Bitte lesen Sie unser Readme, um die Logik hinter dem Spiel besser zu verstehen,
	sowie Informationen zu verwendeten Linux- oder Windows-spezifischen Sonderheiten zu erhalten
*/

// ============================================= Header ============================================================== //

// Fehlermeldungen in Visual Studio ignorieren
#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS 1
#define _CRT_NONSTDC_NO_WARNINGS 1
#endif

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

#if defined(_WIN32)
// Windows Bibliotheken
#include <conio.h>
#include <windows.h>

// Windows Konstanten
// Steuerung
#define LEFT_ARROW VK_LEFT
#define RIGHT_ARROW VK_RIGHT
#define UP_ARROW VK_UP
#define DOWN_ARROW VK_DOWN
#define ENTER_KEY VK_RETURN

// Zeichen
const char WALL = (char)219;
const char BLANK = ' ';
const char BALL = (char)254;
const char PLAYER_TOP = (char)220;
const char PLAYER_BOT = (char)223;

#else	// Linux
// Linux Bibliotheken
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

// Anleitung fuer ncurses: http://openbook.rheinwerk-verlag.de/linux_unix_programmierung/Kap13-002.htm
#include <curses.h>

// Linux Konstanten

// Steuerung
#define LEFT_ARROW 260
#define RIGHT_ARROW 261
#define UP_ARROW 259
#define DOWN_ARROW 258
#define ENTER_KEY 10

// Zeichen
const char WALL = '#';
const char BLANK = ' ';
const char BALL = 'O';
const char PLAYER_TOP = '=';
const char PLAYER_BOT = '=';
#endif

// Konsolengroesse (am besten gerade, damit Ball in der Mitte starten kann)
int CONSOLE_WIDTH = 100;
int CONSOLE_HEIGHT = 32; // Spielfeld + Header

// Anzahl an Zeilen ueber dem Spielfeld
int HEADER_HEIGHT = 2;

// ============================================= STRUCTS =================================================================== //

// Ball
typedef struct {
	int x;
	int prev_x;
	int y;
	int prev_y;
	int dest;
}str_ball;

// Spieler
typedef struct {
	int length;
	int pos;
	int prev_pos;
}str_player;

// Eintrag Highscore
typedef struct {
	int score;
	char name[21];
}str_highscore;

// ============================================= FUNKTIONEN DEKLARATIONEN ================================================== //

void updatePlayer(str_player* player);
void printPlayer(str_player* player, int id);
void printOhneBall();
void printSpielfeld(str_player* player, str_ball* ball, int score1, int score2);
void updateBall(str_ball* ball);
void collisionWall(str_ball* ball);
int collisionPlayer(str_ball* ball, str_player* player);
void moveBall(str_ball* ball, str_player* player);
void printUpdatedPlayer(str_player* player, int id);
void printScore(int score1, int score2);
void startscreen();
void loadGame();
int menuSelector(int x, int y, int yStart);
int mainMenu();
void spielende();
int getschwierigkeitsgrad();
int getmapgrosse();
int setschwierigkeitsgrad(str_player* player, int grosse);
int setmapgrosse();
void sleepProcess(int milliseconds);
void setWindowSize(int width, int height);
void setWindowTitle();
void clrscr();
void hideCursor();
void gotoxy(int x, int y);
void sysPause();
int exitgame();
void printHighscore();
void addHighscore(int score);
int checkHighscore(int score);
int compareScore(const void* a, const void* b);
void sortHighscore(str_highscore* highscore);
int getGeschwindigkeit();
int setGeschwindigkeit();

// ============================================= MAIN ====================================================================== //

int main() {

	// Linux -- ncurses initialisieren
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

	int running = 1;

	while (running == 1) {
		switch (mainMenu()) {
		case 0:	// New Game
			loadGame();
			break;
		case 1:	// Highscore
			printHighscore();
			break;
		case 2:	// Exit
			running = exitgame();
			break;
		}
	}

	// Linux -- ncurses beenden
#if !defined(_WIN32)
	endwin();
#endif

	return 0;
}

// ============================================= OS SPEZIFISCHE FUNKTIONEN ================================================= //

#if defined (_WIN32) // Windows

// print umdefinieren, damit es fuer Windows und Linux kompatibel ist
#define print(...) printf(__VA_ARGS__)

// Fensergroesse aendern
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

	SetConsoleScreenBufferSize(Handle, coord);				// Textbuffergroesse setzen (kein Scrollbalken)
	SetConsoleWindowInfo(Handle, TRUE, &windowSize);		// Fenstergroesse setzen

	// https://stackoverflow.com/a/46145911 (Entfert maximieren Button)
	HWND hwnd = GetConsoleWindow();
	DWORD style = GetWindowLong(hwnd, GWL_STYLE);
	style &= ~WS_MAXIMIZEBOX;
	SetWindowLong(hwnd, GWL_STYLE, style);
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_SHOWWINDOW);
}

// Fenstername
inline void setWindowTitle() {
	SetConsoleTitle("Pong");
}

// Konsole leeren
inline void clrscr() {
	system("cls");
}

// Cursor ausschalten
void hideCursor() {
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

// Tastendruck abfragen
int* checkKeysPressed() {
	int pressed[4];
	// Wenn eine Taste gedrueckt wurde
	if (kbhit()) {

		// links, rechts
		if (GetKeyState(LEFT_ARROW) & 0x8000) {
			pressed[0] = LEFT_ARROW;
		}
		else if (GetKeyState(RIGHT_ARROW) & 0x8000) {
			pressed[0] = RIGHT_ARROW;
		}

		// a, d
		if (GetKeyState(0x41) & 0x8000) {
			pressed[1] = 'a';
		}
		else if (GetKeyState(0x44) & 0x8000) {
			pressed[1] = 'd';
		}

		// oben, unten
		if (GetKeyState(UP_ARROW) & 0x8000) {
			pressed[2] = UP_ARROW;
		}
		else if (GetKeyState(DOWN_ARROW) & 0x8000) {
			pressed[2] = DOWN_ARROW;
		}

		// Enter
		if (GetKeyState(ENTER_KEY) & 0x8000) {
			pressed[3] = ENTER_KEY;
		}

	}

	return pressed;
}

// Spielerposition aktualisieren
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


// https://stackoverflow.com/a/55635979 (Zu Koordinaten in Konsole springen)
void gotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Pause (Press any key to continue...)
void sysPause() {
	system("pause");
}

#else // Linux

// print umdefinieren, damit es fuer Windows und Linux kompatibel ist
#define print(...) printw(__VA_ARGS__)

// Konsole leeren
inline void clrscr()
{
	clear();
	refresh();
}

// Fensergroesse aendern
void setWindowSize(int width, int height) {
	// TODO >> Fenstergroesse in Linux
}

// Fenstername
void setWindowTitle() {
	// TODO >> Fensertitel in Linux
}

// Cursor ausschalten
void hideCursor() {
	curs_set(0);
}

// Tastendruck abfragen
int checkKeysPressed() {
	int pressed = 0;

	// Tastendruck abfragen (-1, wenn keine Taste gedrueckt)
	pressed = getch();
	
	// Key buffer leeren
	flushinp();

	return pressed;
}

// Spielerposition aktualisieren
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

// Zeit seit Prozessstart
void sleepProcess(int milliseconds) {
	// usleep normalerweise in Mikrosekunden
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

// Zu Koordinaten in Konsole springen
void gotoxy(int x, int y) {
	move(y, x);
}

// Pause (Press any key to continue...)
void sysPause() {
	print("Druecken Sie eine beliebige Taste...");

	while (getch() == -1) {
		usleep(100);
	}
}

#endif

// ============================================= ALLGEMEINE FUNKTIONEN ===================================================== //

// Spieler zum ersten Mal zeichnen
void printPlayer(str_player* player, int id) {
	// linke Wand
	print("%c", WALL);

	// BLANK bis Spieler
	for (int i = 1; i < player[id].pos; i++) {
		print("%c", BLANK);
	}

	// id == 0 -> Spieler oben
	if (id == 0) {
		for (int i = 0; i < player[id].length; i++) {
			print("%c", PLAYER_TOP);
		}
	}

	// id == 1 -> Spieler unten
	else {
		for (int i = 0; i < player[id].length; i++) {
			print("%c", PLAYER_BOT);
		}
	}

	// BLANK bis rechte Wand
	for (int i = player[id].pos + player[id].length; i < CONSOLE_WIDTH; i++) {
		print("%c", BLANK);
	}

	// rechte Wand
	print("%c", WALL);
}

// leere Spielfeldzeile zeichnen
void printOhneBall() {
	// linke Wand
	print("%c", WALL);

	// BLANK bis rechte Wand
	for (int j = 0; j < CONSOLE_WIDTH - 1; j++) {
		print("%c", BLANK);
	}

	// rechte Wand
	print("%c", WALL);
}

// Spielfeld zeichnen
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

// Ballposition aktualisieren
void updateBall(str_ball* ball) {
	// alte Koordinaten speichern
	ball->prev_x = ball->x;
	ball->prev_y = ball->y;

	// Ball bewegen
	switch (ball->dest) {
		// oben rechts
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
		// unten rechts
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
		// unten links
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
		// oben links
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

// Kollision mit Wand abfragen
void collisionWall(str_ball* ball) {
	if (ball->x <= 1 || ball->x >= CONSOLE_WIDTH - 1) {
		ball->dest = abs(ball->dest - 11);
	}
}

// Kollision mit Spieler abfragen
int collisionPlayer(str_ball* ball, str_player* player) {
	// Ball am oberen Rand
	if (ball->y <= HEADER_HEIGHT && ball->prev_y > HEADER_HEIGHT) {	// zweites vermeidet, dass Kollision zu oft abgefragt wird
		// Kollision mit Spieler 1

		// Erstes Segment (von links)
		if (ball->x >= player[0].pos && ball->x <= player[0].pos + (player[0].length / 5)) {
			// Ball von unten links
			if (ball->dest < 3) {
				ball->dest = 3;
				return 1;
			}
			// Ball von unten rechts
			else {
				ball->dest = 8;
				return 1;
			}
		}

		// Zweites Segment (von links)
		else if (ball->x >= player[0].pos + (player[0].length / 5) && ball->x <= player[0].pos + 2 * (player[0].length / 5)) {
			// Ball von unten links
			if (ball->dest < 3) {
				ball->dest = 4;
				return 1;
			}
			// Ball von unten rechts
			else {
				ball->dest = 7;
				return 1;
			}
		}

		// Drittes Segment (von links)
		else if (ball->x >= player[0].pos + 2 * (player[0].length / 5) && ball->x <= player[0].pos + 3 * (player[0].length / 5)) {
			// Ball von unten links
			if (ball->dest < 3) {
				ball->dest = 5;
				return 1;
			}
			// Ball von unten rechts
			else {
				ball->dest = 6;
				return 1;
			}
		}

		// Viertes Segment (von links)
		else if (ball->x >= player[0].pos + 3 * (player[0].length / 5) && ball->x <= player[0].pos + 4 * (player[0].length / 5)) {
			// Ball von unten links
			if (ball->dest < 3) {
				ball->dest = 4;
				return 1;
			}
			// Ball von unten rechts
			else {
				ball->dest = 7;
				return 1;
			}
		}

		// Fuenftes Segment (von links)
		else if (ball->x >= player[0].pos + 4 * (player[0].length / 5) && ball->x <= player[0].pos + 5 * (player[0].length / 5)) {
			// Ball von unten links
			if (ball->dest < 3) {
				ball->dest = 3;
				return 1;
			}
			// Ball von unten rechts
			else {
				ball->dest = 8;
				return 1;
			}

		}

		// Spieler 1 nicht getroffen
		else {
			return -1;
		}
	}

	// Ball am unteren Rand
	else if (ball->y >= CONSOLE_HEIGHT && ball->prev_y < CONSOLE_HEIGHT) {	// zweites vermeidet, dass Kollision zu oft abgefragt wird
		// Kollision mit Spieler 2

		// Erstes Segment (von links)
		if (ball->x >= player[1].pos && ball->x <= player[1].pos + (player[1].length / 5)) {
			// Ball von oben links
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 2;
				return 2;
			}
			// Ball von oben rechts
			else {
				ball->dest = 9;
				return 2;
			}
		}

		// Erstes Segment (von links)
		else if (ball->x >= player[1].pos + (player[1].length / 5) && ball->x <= player[1].pos + 2 * (player[1].length / 5)) {
			// Ball von oben links
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 1;
				return 2;
			}
			// Ball von oben rechts
			else {
				ball->dest = 10;
				return 2;
			}
		}

		// Erstes Segment (von links)
		else if (ball->x >= player[1].pos + 2 * (player[1].length / 5) && ball->x <= player[1].pos + 3 * (player[1].length / 5)) {
			// Ball von oben links
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 0;
				return 2;
			}
			// Ball von oben rechts
			else {
				ball->dest = 11;
				return 2;
			}
		}

		// Erstes Segment (von links)
		else if (ball->x >= player[1].pos + 3 * (player[1].length / 5) && ball->x <= player[1].pos + 4 * (player[1].length / 5)) {
			// Ball von oben links
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 1;
				return 2;
			}
			// Ball von oben rechts
			else {
				ball->dest = 10;
				return 2;
			}
		}

		// Erstes Segment (von links)
		else if (ball->x >= player[1].pos + 4 * (player[1].length / 5) && ball->x <= player[1].pos + 5 * (player[1].length / 5)) {
			// Ball von oben links
			if (ball->dest > 2 && ball->dest < 6) {
				ball->dest = 2;
				return 2;
			}
			// Ball von oben rechts
			else {
				ball->dest = 9;
				return 2;
			}
		}

		// Spieler 2 nicht getroffen
		else {
			return -1;
		}
	}

	// Ball nicht am oberen oder unteren Rand
	else {
		return 0;
	}
}

// Neue Ballposition zeichnen und alte Position loeschen
void moveBall(str_ball* ball, str_player* player) {
	// Wand und Spieler nicht mit BLANK ueberschreiben
	if ((ball->prev_x > 1 && ball->prev_x < CONSOLE_WIDTH - 1) && (ball->prev_y > HEADER_HEIGHT && ball->prev_y < CONSOLE_HEIGHT)) {
		gotoxy(ball->prev_x, ball->prev_y);
		print("%c", BLANK);
	}

	// Ball ist innerhalb der Waende
	if (ball->x > 1 && ball->x < CONSOLE_WIDTH - 1) {
		// Ball ist am oberen Rand
		if (ball->y <= HEADER_HEIGHT) {
			// Ball ist nicht auf dem Spieler
			if (ball->x < player[0].pos || ball->x > player[0].pos + player[0].length) {
				gotoxy(ball->x, ball->y);
				print("%c", BALL);
			}
		}

		// Ball ist am unteren Rand
		else if (ball->y >= CONSOLE_HEIGHT) {
			// Ball ist nicht auf dem Spieler
			if (ball->x < player[1].pos || ball->x > player[1].pos + player[1].length) {
				gotoxy(ball->x, ball->y);
				print("%c", BALL);
			}
		}

		// Ball ist nicht am oberen oder unteren Rand
		else {
			gotoxy(ball->x, ball->y);
			print("%c", BALL);
		}
	}
}

// Neue Spielerposition zeichnen und alte loeschen
void printUpdatedPlayer(str_player* player, int id) {
	switch (id) {
		// Spieler 1
	case 0:
		// Spieler hat sich nach rechts bewegt
		if (player[id].pos > player[id].prev_pos) {
			// Spieler ist nicht an der Wand (ansonsten wird Wand mit BLANK ueberschrieben)
			if (player[id].prev_pos > 0) {
				gotoxy(player[id].prev_pos, HEADER_HEIGHT);
				print("%c", BLANK);
			}
			gotoxy(player[id].pos + player[id].length - 1, HEADER_HEIGHT);
			print("%c", PLAYER_TOP);
		}

		// Spieler hat sich nach links bewegt
		else if (player[id].pos < player[id].prev_pos) {
			gotoxy(player[id].pos + 1, HEADER_HEIGHT);
			print("%c", PLAYER_TOP);
			// Spieler ist nicht an der Wand (ansonsten wird Wand mit BLANK ueberschrieben)
			if (player[id].prev_pos + player[id].length < CONSOLE_WIDTH) {
				gotoxy(player[id].prev_pos + player[id].length, HEADER_HEIGHT);
				print("%c", BLANK);
			}
		}
		break;

		// Spieler 2
	case 1:
		// Spieler hat sich nach rechts bewegt
		if (player[id].pos > player[id].prev_pos) {
			// Spieler ist nicht an der Wand (ansonsten wird Wand mit BLANK ueberschrieben)
			if (player[id].prev_pos > 0) {
				gotoxy(player[id].prev_pos, CONSOLE_HEIGHT);
				print("%c", BLANK);
			}
			gotoxy(player[id].pos + player[id].length - 1, CONSOLE_HEIGHT);
			print("%c", PLAYER_BOT);
		}

		// Spieler hat sich nach links bewegt
		else if (player[id].pos < player[id].prev_pos) {
			gotoxy(player[id].pos + 1, CONSOLE_HEIGHT);
			print("%c", PLAYER_BOT);
			// Spieler ist nicht an der Wand (ansonsten wird Wand mit BLANK ueberschrieben)
			if (player[id].prev_pos + player[id].length < CONSOLE_WIDTH) {
				gotoxy(player[id].prev_pos + player[id].length, CONSOLE_HEIGHT);
				print("%c", BLANK);
			}
		}
		break;
	}
}

// Punktezahl schreiben
void printScore(int score1, int score2) {
	gotoxy(18, 0);
	print("%3d", score1);
	gotoxy(CONSOLE_WIDTH - 2, 0);
	print("%3d", score2);
}

// Spiel starten
void loadGame() {
	srand(time(NULL));
	str_player player[2];
	str_ball ball;
	int zeit = 0;
	long vergangeneZeit;
	int score1 = 0;
	int score2 = 0;
	int gameover = 0;

	// Linux -- Startzeit festlegen
#if !defined(_WIN32)
	struct timeval start, end;
	long secs, usecs;

	gettimeofday(&start, NULL);
#endif

	// Spielfeldgroesse
	int grosse = setmapgrosse();

	// Schwierigkeitsgrad
	setschwierigkeitsgrad(player, grosse);

	// Geschwindigkeit
	int speed = setGeschwindigkeit();

	// Spielerposition
	player[0].pos = (CONSOLE_WIDTH - player[0].length) / 2;
	player[1].pos = (CONSOLE_WIDTH - player[1].length) / 2;

	// Ballposition
		// Debug
	//ball.x = 50;
	//ball.y = 10;
	//ball.dest = 0;
	
		// Release
	ball.x = 1 + (CONSOLE_WIDTH - 2) / 2;
	ball.y = HEADER_HEIGHT + (CONSOLE_HEIGHT - HEADER_HEIGHT) / 2;
	ball.dest = rand() % 12;

	// Spielfeld zeichnen
	printSpielfeld(player, &ball, score1, score2);

	// Spiel
	while (gameover == 0) {
		updatePlayer(player);
		printUpdatedPlayer(player, 0);
		printUpdatedPlayer(player, 1);

		// Vergangene Zeit seit Prozessstart
#if defined(_WIN32)
		vergangeneZeit = clock();
#else // https://stackoverflow.com/a/9871230 (Vergangene Zeit seit Prozessstart in Millisekunden)
		gettimeofday(&end, NULL);
		secs = end.tv_sec - start.tv_sec;
		usecs = end.tv_usec - start.tv_usec;
		vergangeneZeit = ((secs) * 1000 + usecs / 1000.0) + 0.5;
#endif

		if ((vergangeneZeit - zeit - speed) > 0) { // Geschwindigkeit Ball
			updateBall(&ball);
			collisionWall(&ball);

			switch (collisionPlayer(&ball, player)) {
				// Game Over
			case -1:
				if (score1 >= score2 && checkHighscore(score1) == 1) {
					addHighscore(score1);
				}
				else if (score2 > score1 && checkHighscore(score2) == 1) {
					addHighscore(score2);
				}
				spielende();
				gameover = 1;
				break;
				// Nichts, Ball nicht in Spielernaehe
			case 0:
				break;
				// Punkt fuer Spieler 1
			case 1:
				score1++;
				break;
				// Punkt fuer Spieler 2
			case 2:
				score2++;
				break;
			}

			printScore(score1, score2);
			moveBall(&ball, player);

			// Zeit reset
#if defined(_WIN32)
			zeit = clock();
#else // https://stackoverflow.com/a/9871230 (Vergangene Zeit seit Prozessstart in Millisekunden)
			gettimeofday(&end, NULL);
			secs = end.tv_sec - start.tv_sec;
			usecs = end.tv_usec - start.tv_usec;
			zeit = ((secs) * 1000 + usecs / 1000.0) + 0.5;
#endif
		}

		sleepProcess(100);
	}
}

// Hauptmenue
int mainMenu() {

	// Startpunkt
	int x = 10, y = 5;
	int yStart = y;

	int selected;

	// Konsole leeren
	clrscr();

	// Menue Text
	gotoxy(x, y++);
	print("New Game\n");
	gotoxy(x, y++);
	print("HighScore\n");
	gotoxy(x, y++);
	print("Exit\n");
	gotoxy(x, y++);

	//Im Menue navigieren
	selected = menuSelector(x, y, yStart);

	return selected;
}

// Startbildschirm
void startscreen() {
	// ASCII Art Quelle: http://pong.ascii.uk/
	// Note: "\\" schreibt nur einmal "\"

	clrscr();

	gotoxy((CONSOLE_WIDTH / 2) - 13, (CONSOLE_HEIGHT / 2) - 4); // Mein Versuch das ganze irgendwie mittig zu machen...
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

	gotoxy((CONSOLE_WIDTH / 2) - 13, (CONSOLE_HEIGHT / 2) + 3);

	// Warte auf Benutzereingabe
	sysPause();
}

// Spiel beenden
int exitgame() {
	// Gleiche Strucktur wie im Hauptmenu
	int x = 10, y = 5;
	int yStart = y;

	int selected;

	// Konsole leeren
	clrscr();

	// Menue Text
	gotoxy(x - 3, y - 1);
	print("Willst du das Spiel wirklich beenden?");
	gotoxy(x, y++);
	print("Ja\n");
	gotoxy(x, y++);
	print("Nein\n");
	gotoxy(x, y++);
	
	selected = menuSelector(x, y, yStart);

	return selected;	// 0 bei Ja, 1 bei Nein
}

// Schwierigkeitsgrad auswaehlen
int getschwierigkeitsgrad() {
	// vgl. MainMenu
	int x = 10, y = 5;
	int yStart = y;

	int selected;

	// Konsole leeren
	clrscr();

	// Menue Text
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

// Spielfeldgroesse auswaehlen
int getmapgrosse() {
	// vgl. MainMenu
	int x = 10, y = 5;
	int yStart = y;

	int selected;

	// Konsole leeren
	clrscr();

	// Menue Text
	gotoxy(x - 3, y - 1);
	print("Waehle deine Spielfeldgroesse");
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

// Schwierigkeitsgrad festlegen
int setschwierigkeitsgrad(str_player* player, int grosse) {
	// Benutzer waehlt Schwierigkeit
	int difficulty = getschwierigkeitsgrad();

	// Spielerlaenge muss durch 5 teilbar sein, da wir diese in der Kollision in 5 Bereiche aufteilen
	switch (difficulty) {
	case 0:
		player[0].length = (CONSOLE_WIDTH - 1) / 5;			// Schwiereigkeitsgrad nicht von Spielfeldgroesse abhaengig
		player[1].length = (CONSOLE_WIDTH - 1) / 5;
		break;
	case 1:
		player[0].length = (CONSOLE_WIDTH - 1) / 10;
		player[1].length = (CONSOLE_WIDTH - 1) / 10;
		break;
	case 2:
		if (grosse == 1) {
			player[0].length = (CONSOLE_WIDTH - 1) / 15;	// Hier waere sonst Spielerlaenge 150/20 = 7.5!
			player[1].length = (CONSOLE_WIDTH - 1) / 15;
		}
		else {
			player[0].length = (CONSOLE_WIDTH - 1) / 20;
			player[1].length = (CONSOLE_WIDTH - 1) / 20;
		}
		break;
	}

	return difficulty;
}

// Spielfeldgroesse festlegen
int setmapgrosse() {
	// Benutzer waehlt Groesse
	int grosse = getmapgrosse();

	switch (grosse) {
	case 0:
		// Standartwerte
		// CONSOLE_HEIGHT = 32;
		// CONSOLE_WIDTH = 100;
		break;
	case 1:
		CONSOLE_HEIGHT = 48;
		CONSOLE_WIDTH = 150;
		break;
	case 2:
		CONSOLE_HEIGHT = 62;
		CONSOLE_WIDTH = 200;
		break;
	}

	// Konsolengroesse aendern
	setWindowSize(CONSOLE_WIDTH, CONSOLE_HEIGHT);

	return grosse;
}


// Game Over screen
void spielende() {
	// vgl. Startscreen

	// Konsole leeren
	clrscr();

	// Konsolengroesse zuruecksetzen
	CONSOLE_HEIGHT = 32;
	CONSOLE_WIDTH = 100;

	setWindowSize(CONSOLE_WIDTH, CONSOLE_HEIGHT);

	//Ascii Art: http://patorjk.com/software/taag/#p=display&f=Big&t=Game%20Over

	gotoxy((CONSOLE_WIDTH / 2) - 27, (CONSOLE_HEIGHT / 2) - 4);
	print("   _____                         ____                 \n");

	gotoxy((CONSOLE_WIDTH / 2) - 27, (CONSOLE_HEIGHT / 2) - 3);
	print("  / ____|                       / __ \\                \n");

	gotoxy((CONSOLE_WIDTH / 2) - 27, (CONSOLE_HEIGHT / 2) - 2);
	print(" | |  __  __ _ _ __ ___   ___  | |  | |_   _____ _ __ \n");

	gotoxy((CONSOLE_WIDTH / 2) - 27, (CONSOLE_HEIGHT / 2) - 1);
	print(" | | |_ |/ _` | '_ ` _ \\ / _ \\ | |  | \\ \\ / / _ \\ '__|\n");

	gotoxy((CONSOLE_WIDTH / 2) - 27, (CONSOLE_HEIGHT / 2));
	print(" | |__| | (_| | | | | | |  __/ | |__| |\\ V /  __/ |  \n");

	gotoxy((CONSOLE_WIDTH / 2) - 27, (CONSOLE_HEIGHT / 2) + 1);
	print("  \\_____|\\__,_|_| |_| |_|\\___|  \\____/  \\_/ \\___|_|  \n");

	gotoxy((CONSOLE_WIDTH / 2) - 13, (CONSOLE_HEIGHT / 2) + 3);

	// Warte auf Benutzereingabe
	sysPause();
}

// Highscoreliste ausgeben
void printHighscore() {
	// Oeffne Datei mit Leserechten
	FILE* datei = fopen("Highscore.txt", "r");

	char text[25];
	int x = 10, y = 5;

	// Konsole leeren
	clrscr();

	// Pruefe, ob datei geoeffnet werden konnte
	if (!datei) {
		gotoxy(0, 0);
		print("Datei kann nicht geoeffnet werden!\n");

		// Linux -- ncurses (ansonsten wird nichts auf dem Bildschirm ausgegeben)
#if !defined(_WIN32)
		refresh();
#endif

		sysPause();
		return;
	}

	// Ueberschrift
	gotoxy(x - 3, y - 1);
	print("Highscore");

	// erste Zeile aus der Datei
	fgets(text, 25, datei);

	// Rest der Datei
	while (!feof(datei)) {
		// schreibe die erste Zeile, dann zweite usw.
		gotoxy(x, y++);
		print("%s", text);

		// naechste Zeile
		fgets(text, 25, datei);
	}	//	vermeidet Fehler, da sich in der Datei elf Zeilen befinden, aber nur zehn mit Text befuellt sind

	print("\n");

	// Linux -- ncurses (ansonsten wird nichts auf dem Bildschirm ausgegeben)
#if !defined(_WIN32)
	refresh();
#endif

	// Warte auf Benutzereingabe
	sysPause();

	// Datei schliessen
	fclose(datei);
}

// Neuen Highscore hinzufuegen
void addHighscore(int score) {
	// Linux -- ncurses (damit Eingabe wieder sichtabr wird)
#if !defined(_WIN32)
	curs_set(1);
	echo();
	nodelay(stdscr, FALSE);
	keypad(stdscr, FALSE);
#endif

	// Oeffne Datei mit Lese- und Schreibrechten
	FILE* datei = fopen("Highscore.txt", "r+");

	str_highscore highscore[11] = { 0 };
	int a;

	// Konsole leeren
	clrscr();

	// Pruefe, ob datei geoeffnet werden konnte
	if (!datei) {
		gotoxy(0, 0);
		print("Datei kann nicht geoeffnet werden!\n");

		// Linux -- ncurses (ansonsten wird nichts auf dem Bildschirm ausgegeben)
#if !defined(_WIN32)
		refresh();
#endif

		sysPause();
		return;
	}

	// Neuer Highscore: Benutzereingabe
	gotoxy(7, 4);
	print("Bitte Namen eingeben (max 20 Zeichen, kein Leerzeichen):\n");

	// Linux -- ncurses (ansonsten wird nichts auf dem Bildschirm ausgegeben)
#if !defined(_WIN32)
	refresh();
#endif

#if defined(_WIN32)
	gotoxy(0, 2);	// in Kombination mit scanf ist gotoxy nicht mehr absolut und der x-Wert wird ignoriert 
	scanf(" %20s", highscore[10].name);
#else
	gotoxy(7, 5);
	getnstr(highscore[10].name, 20);
#endif

	highscore[10].name[strlen(highscore[10].name)] = '\n';
	highscore[10].score = score;

	// lade die gespeicherten Highscores
	for (int i = 0; i < 10; i++) {
		// Punktezahl einlesen
		fscanf(datei, "%d", &highscore[i].score);
		a = 0;

		// Leerzeichen einlesen und direkt mit dem naechsten Buchstaben ueberschreiben
		fscanf(datei, "%c", &highscore[i].name[a]);
		while (highscore[i].name[a] == ' ') {
			fscanf(datei, "%c", &highscore[i].name[a]);
		}

		// Name einlesen
		while (highscore[i].name[a] != '\n' && !feof(datei)) {
			a++;
			fscanf(datei, "%c", &highscore[i].name[a]);
		}
	}

	// Sortieren der eingelesenen Daten mit qsort
	sortHighscore(highscore);

	// Schreibe neue Highscoreliste in Datei
	fseek(datei, 0, SEEK_SET);

	for (int i = 0; i < 10; i++) {
		fprintf(datei, "%d %s", highscore[i].score, highscore[i].name);
	}

	// Datei schliessen
	fclose(datei);

	// Linux -- ncurses (Eingabe wieder unsichtbar)
#if !defined(_WIN32)
	curs_set(0);
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
#endif
}

// Ueberpruefe, ob die erreichte Punktzahl ueber dem Minimum in der Highscoreliste ist
int checkHighscore(int score) {
	// Oeffne Datei mit Leserechten
	FILE* datei = fopen("Highscore.txt", "r");

	int lowestScore = 1000;
	int zahl[10];
	int i = 0;
	char buffer;

	// Pruefe, ob datei geoeffnet werden konnte
	if (!datei) {
		clrscr();
		gotoxy(0, 0);
		print("Datei kann nicht geoeffnet werden!\n");
		sysPause();
		return 0;
	}

	// Suche die niedrigste Punktzahl
	while (!feof(datei)) {
		fscanf(datei, "%d", &zahl[i]);
		buffer = 0;

		while (buffer != '\n' && !feof(datei)) {
			fscanf(datei, "%c", &buffer);
		}

		if (lowestScore > zahl[i]) {
			lowestScore = zahl[i];
		}

		i++;
	}

	// Datei schliessen
	fclose(datei);

	// Vergleiche erreichte Punktzahl mit dem niedrigsten Highscore
	if (score > lowestScore) {
		// erreichte Punktzahlk ist groesser
		return 1;
	}

	// erreichte Punktzahlk ist kleiner
	return 0;
}

// Highscoreliste sortieren
void sortHighscore(str_highscore* highscore) {
	qsort(highscore, 11, sizeof(*highscore), compareScore);
}

// Zwei Elemente aus der Highscoreliste miteinander fuer qsort vergleichen (sortiere absteigend)
int compareScore(const void* a, const void* b) {
	str_highscore player1 = *(str_highscore*)a;
	str_highscore player2 = *(str_highscore*)b;

	if (player1.score < player2.score) return 1;
	if (player1.score > player2.score) return -1;
	return 0;
}

// Geschwindigkeit auswaehlen
int getGeschwindigkeit() {
	// vgl. MainMenu
	int x = 10, y = 5;
	int yStart = y;

	int selected;

	// Konsole leeren
	clrscr();

	// Menue Text
	gotoxy(x - 3, y - 1);
	print("Waehle deine Geschwindigkeit");
	gotoxy(x, y++);
	print("langsam");
	gotoxy(x, y++);
	print("mittel");
	gotoxy(x, y++);
	print("schnell");
	gotoxy(x, y++);

	selected = menuSelector(x, y, yStart);

	return selected;
}

// Spielfeldgroesse festlegen
int setGeschwindigkeit() {
	// Benutzer waehlt Geschwindigkeit
	int selected = getGeschwindigkeit();
	int speed = 1000;

	switch (selected) {
	case 0:
		speed = 600;
		break;
	case 1:
		speed = 400;
		break;
	case 2:
		speed = 200;
		break;
	}

	return speed;
}