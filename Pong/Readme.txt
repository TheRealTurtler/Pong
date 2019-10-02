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

Das Spielprinzip ist relativ selbsterklaerend: 2 Spieler versuchen einen Ball mithilfe ihres Schlaegers im Spielfeld zu halten. Fuer jeden
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

Hierbei stellt '=' den Schlaeger dar und '|' die unterteilung der einzelnen Segmente (diese ist im Spiel nicht zu sehen). Die Zahlen darunter
geben die Richtung an, die der Ball bei beruehrung mit dem jeweiligen Segment einschlaegt.

Bewegt sich der Ball nun von rechts unten nach links oben, dann wuerde die Richtungsaenderung des Balls folgendermassen aussehen (Spieler 2 analog):

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