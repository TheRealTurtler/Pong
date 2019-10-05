##################################################################################################################################################### 
 
 _ __   ___  _ __   __ _ 
| '_ \ / _ \| '_ \ / _` |
| |_) | (_) | | | | (_| |
| .__/ \___/|_| |_|\__, |
| |                 __/ |
|_|                |___/ 

von Michael Diedler (Matr. Nr. 2020406) und Kai Frisch (Matr. Nr. 2033963)

#####################################################################################################################################################

Spielanleitung:

Das Spielprinzip ist relativ selbsterklaerend: 2 Spieler versuchen einen Ball mit Hilfe ihrer Schlaeger im Spielfeld zu halten. Fuer jeden
zurueckgeschlagenen Ball gibt es fuer den entsprechenden Spieler einen Punkt. Verfehlt der Ball einen Schlaeger und fliegt ins Aus, so ist das Spiel
vorbei und der Spieler mit der hoechsten Punktzahl kann sich (sofern die erreichte Punktzahl hoch genug ist) in die Highscoreliste eintragen.

#####################################################################################################################################################

Steuerung:

Spieler 1 steuert den oberen Schlaeger mit den Pfeiltasten (links, rechts).
Spieler 2 steuert den unteren Schlaeger mit A und D.

Fuer die Navigation im Menue werden die Pfeiltasten (oben, unten) verwendet und die Auswahl wird mit Enter bestaetigt.

#####################################################################################################################################################

Besonderheiten:

Der Ball kann sich in 12 verschiedene Richtungen bewegen. Diese sind im Code folgendermaﬂen numeriert:

  11   0
9 10   1 2
     B
8  7   4 3
   6   5

Hierbei steht B fuer die aktuelle Position des Balls und die Zahlen drum herum jeweils fuer die naechste Position, auf die sich der Ball bewegen kann.

Auch die Schlaeger haben eine Besonderheit, denn diese sind in 5 Bereiche unterteilt. Je nachdem auf welchen Abschnitt des Schlaegers der Ball
trifft, springt er anders zurueck.

Die Aufteilung fuer Spieler 1 und einen Ball, der sich von links unten nach rechts oben bewegt (also Richtung 0, 1, 2) ist folgendermassen
(Spieler 2 analog):

===|===|===|===|===
 3   4   5   4   3

Hierbei stellt '=' den Schlaeger dar und '|' die Unterteilung der einzelnen Segmente (diese ist im Spiel nicht zu sehen). Die Zahlen darunter
geben die Richtung an, die der Ball bei Beruehrung mit dem jeweiligen Segment einschlaegt.

Bewegt sich der Ball nun von rechts unten nach links oben (also Richtung 9, 10, 11), dann wuerde die Richtungsaenderung des Balls folgendermassen
aussehen (Spieler 2 analog):

===|===|===|===|===
 8   7   6   7   8

Trifft der Ball auf eine Wand, so wird er nur reflektiert, ohne seinen Winkel zu dieser zu aendern, also an der linken Wand folgendermaﬂen:

#
# 2
#
# 9
#

Hier bewegt sich der Ball von unten rechts nach oben links mit der Richtung 9. Nach der Kollision mit der Wand aendert er die Richtung und bewegt
sich nun von links unten nach rechts oben mit der Richtung 2.

#####################################################################################################################################################

Windows-/ Linux-Kompatibilitaet:

Der Code kann sowohl unter Windows, als auch unter Linux kompiliert werden. Hierzu sind die Funktionen, die OS-spezifisch sind, doppelt geschrieben
und auf das jeweilige Betriebssystem angepasst.

Dies fuehrt allerdings zu leichten Unterschieden:

1. Zeichen
Die verwendeten Zeichen sind fuer Windows und Linux unterschiedlich.

2. Tastendruckerkennung
Unter Windows kann mit der Funktion GetKeyState() aus der Bibliothek "Windows.h" direkt abgefragt werden, ob die spezifizierte Taste gedrueckt ist
oder nicht. Somit koennen auch mehrere gleichzeitige Tastendruecke erkannt werden und beide Spieler koennen ihre Schlaeger simultan bewegen. Dies
ist in Linux nicht der Fall. In Linux wird hier die Bibliothek "curses.h" (auch ncurses genannt) verwendet. Diese beinhalet die Funktion getch(),
welche die zuletzt gedrueckte Taste zurueckgibt. Dies hat allerdings den Nachteil, dass nur ein Tastendruck erkannt werden kann. Zusaetzlich muss
hier immer der Keybuffer geleert werden, da ansonsten immer alle Tastenanschlaege ausgefuehrt werden und nicht nur der aktuelle, was zu sehr
verzoegerten Inputs fuehrt.
Da Windows mehrere simultane Tastendruecke erkennen kann, werden diese in einem Array gespeichert und die Funktionen, die die Tastendruecke
verarbeiten, muessen somit auch fuer Windows und Linux extra geschrieben werden.

3. Konsole
Die Windows-Konsole ist anders zu manipulieren, als eine Linux-Konsole. Somit sind die Funktionen SetWindowSize(), SetWindowTitle(), clrscr() und
hideCursor() OS-spezifisch. Unter Windows bietet die Bibliothek "Windows.h" Moeglichkeiten die Groesse und den Titel Konsole zu veraendern. Auch
kann damit der blinkende Cursor ausgeschaltet werden. Die Funktion clrscr() besteht nur aus dem Befehl system("cls"), welcher die Konsole leert.
Unter Linux defieniert die "curses.h" Bibliothek die Funktion clear() und refresh(). Die Funktion gotoxy() setzt den Cursor an die angegebene
Position. Diese wird unter Windows mit Hilfe der "Windows.h" Bibliothek definiert, unter Linux verwendet diese die Funktion move() aus der
"curses.h" Bibliothek. Auch die Ausgabe auf der Konsole muss extra behandelt werden, denn unter Linux muss die Konsole auf raw input gesetzt werden,
um Tastendruecke zu erkennen, ohne diese sofort auf dem Bildschirm auszugeben. Somit funktioniert allerdings die Funktion printf() nicht mehr und es
muss stattdessen printw() verwendet werden. Da der Syntax genau gleich ist, wurden beide Funktionen unter print() zusammengefasst.

4. Etc
Die Funktion sysPause() soll auf einen Tastendruck warten. Unter Windows wird dies durch system("pause") erreicht, unter Linux durch Warten auf
einen Tastendruck mit getch().
Unter Windows gibt es den Befehl Sleep(), welcher den Prozess fuer die angegebene Zeit (in Millisekunden) pausiert und somit bei Endlosschleifen
eine hohe CPU-Auslastung verhindert. Unter Linux wird hier die Funktion usleep() aus der "unistd.h" Bibliothek verwendet. Diese Funktion erwartet
allerdings eine Eingabe in Mikrosekunden. Zusammengefasst wurden diese beiden Funktionen unter sysPause().

#####################################################################################################################################################

Kompilieren unter Linux:

Beim kompilieren unter Linux sind einige Besonderheiten zu beachten:

1. -Werror
Die Option -Werror darf NICHT verwendet werden, da usleep eine Warnung ausgibt, fuer die wir keine Loesung gefunden haben.

2. -lncurses
Die Option -lncurses muss noch am Schluss des Compiler-Aufrufs hinzugefuegt werden, um die "curses.h" Bibliothek zu verwenden. Diese muss auch
seperat installiert werden: https://www.cyberciti.biz/faq/linux-install-ncurses-library-headers-on-debian-ubuntu-centos-fedora/

Das beigefuegte Skript cr_ncurses ist eine abgeaenderte Version des im Kurs zur Verfuegung gestellten cr Skripts mit eben genau diesen Aenderungen.

#####################################################################################################################################################

Sonstiges:

Falls ein Kompilieren bei Ihnen nicht moeglich sein sollte, oder wenn Sie wissen wollen, wie das Programm unter Windows aussehen wuerde, haben wir
ein paar Bilder beigefuegt.

Damit die Highscore-Datei unter Linux richtig gelesen und ausgegeben werden kann, muessen die Zeilenumbrueche in dieser vom Typ LF sein. Wenn es die
Windows-Zeilenumbrueche sind, also CR LF, dann kann die Datei zwar gelesen und beschrieben werden, allerdings werden bei der Auswahl von "Highscore"
im Hauptmenue nur die Zeilen mit den korrekten Linux-Zeilenumbruechen, also LF, ausgegeben. Wenn die Datei also urspruenglich falsh formatiert sein
sollte, werden nur neu hinzugefuegte Highscores ausgegeben.

Falls das cr_ncurses Skript nicht funktionieren sollte, kann dies evtl. daran liegen, dass in diesem Windows-Zeilenumbrueche vorhanden sind. Da es
allerdings Linux-Zeilenumbrueche sein muessen, muss die Datei evtl. neu formatiert werden. Unter Windows ist dies mit Notepad++ leicht umzustellen,
allerdings wissen wir nicht wie dies unter Linux umzustellen ist.

#####################################################################################################################################################