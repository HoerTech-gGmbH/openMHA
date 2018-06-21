MatlabUnit -- Unit Testing framework f�r Matlab
Copyright (C) 2003-2005 Medizinische Physik, Universit�t Oldenburg
Autor: Tobias Herzke
File Version: $Id: README.txt,v 1.2 2005/04/13 16:17:09 tobiasl Exp $

Dies ist das Unit Testing Framework f�r Matlab, das im Medi-Aku-
Kolloquium vorgestellt wurde. Die Folien dieses Vortrags sind in
test_driven_development.pdf verf�gbar. Es folgen kurze Hinweise zum
Testen mit Matlab.

Im folgenden unterscheide ich zwischen dem Code, der getestet werden soll (Program Logic) und dem Code, der testet (Test Cases).

Die einzelnen Test Cases werden als Matlab-Funktionen in m-Files mit der
Namenskonvention "test_*.m" implementiert.

In diesen Test Cases ruft ihr die zu testenden Matlab-Funktionen
(Program Logic) auf und pr�ft, ob die Ergebnisse den Erwartungen
entsprechen. Dazu werden die "assert_*.m" Matlab Funktionen verwendet,
die auf Gleichheit, Ungleichheit, Kleinheit der Abweichungen, und
Matlab-Wahrheitswert testen. Wenn eine dieser Assertions nicht erf�llt
ist, oder wenn w�hrend der Abarbeitung des Tests ein Fehler auftritt,
gilt der Test als fehlgeschlagen. Wenn der Test komplett ohne Fehler
ausgef�hrt wird, dann war er erfolgreich.

Um alle Test Cases in einem bestimmten Dateisystemverzeichnis
nacheinander abzuarbeiten, wird die Funktion "runtests.m"
aufgerufen. "runtests.m" ruft nacheinander alle Tests Cases auf, die
sich im aktuellen Verzeichnis oder in einem als Parameter �bergebenen
Verzeichnis befinden. Test Cases werden dabei am Namen
(Namenskonvention "test_*.m") erkannt. "runtests.m" f�hrt Buch �ber
erfolgreiche und fehlgeschlagene Tests und gibt am Ende eine
Zusammenfassung der Testl�ufe aus.

Eine solche Zusammenfassung sieht so aus:

  33 tests: 133 assertions, 0 failures, 0 errors, 0 teardown errors

Hier wurden 33 "test_*.m" mfiles aufgerufen. Diese m-files haben
insgesamt 133 mal mit Hilfe einer der "assert_*.m" Funktionen zum pr�fen von
Werten aufgerufen. Keine dieser Assertions ist fehlgeschlagen ("0
failures"), es gab keinen Matlab Fehler ("0 errors") und "0 teardown
errors".



F�r Fortgeschrittene: Was sind teardown errors?

Manche Test Cases werden globale Resourcen erzeugen, die nach dem Test
besser wieder freigegeben werden. Das kann zum Beispiel eine Datei
sein, die w�hrend des Tests geschrieben wird, und die hinterher wieder
gel�scht werden soll, oder es kann soundmex sein, das w�hrend des
Tests initialisiert wird und hinterher wieder beendet werden soll.

Beispiel:
  soundmex init;
  testing_mem2mem;
  soundmex exit;

Wenn w�hrend testing_mem2mem ein Fehler auftritt, wird "soundmex exit"
nicht mehr ausgef�hrt.

Deshalb gibt es die M�glichkeit, Aufr�umfunktionen zu registrieren,
die nach Ende des Test Cases auf jeden Fall ausgef�hrt werden, egal ob
der Test Case durchl�uft oder abbricht. Die Funktion, bei der man die
Aufr�umfunktionen registriert, hei�t unittest_teardown. Sie wird
aufgerufen wie feval (sie ruft n�mlich sp�ter selbst feval auf). Ein
Test Case kann beliebig viele Aufr�umfunktionen registrieren,
ausgef�hrt werden sie in umgekerhter Reihenfolge (last in, first out).
