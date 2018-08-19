# El Banquos Radiowecker - Bedienungsanleitung

Mangels eines ausgereiften Produktes in den hiesigen Elektronikmärkten habe ich mir einen eigenen Radiowecker gebaut,
der folgende Eigenschaften in sich vereint:

* Leicht einstellbare Weckzeit (Vor- und Zurückstellbar)
* "Einblenden" des Radios zur Weckzeit (statt brachialem "Radio an")
* Eine Anzeige die ich auch ohne Brille ablesen kann, aber nicht gleich riesig ist
* "entspannte" Zeitanzeige (Ich brauche es nicht auf die Minute genau) 
* Anzeige ist nur so hell, wie gerade nötig und nicht mit geschlossenen Augen warnehmbar
* Stellt sich selbst mittels der im Radiosignal verfügbare Zeitinformation (RDS)
* Nicherchenfunktion (Radio an nach definiertem Interval (z.B. 20 Min für PowerNap)
* Deaktivieren des Alarms kann nicht aus Versehen passieren
* Sender und Lautstärke können nicht aus Versehen verstellt werden

# Bestandteile des Weckers
* 8x8 LED Punkmatrix Display zur Anzeige aller Informationen
* Schlummer Taster
* Drehregler und Taster
* Schalter zum aktivieren der Alarmfunktion
* Lautsprecher an der Seite
* Lautstärkeregler an der Seite, sehr weit hinten

# Inbetriebnahme und Schnellstart
1. Wecker an das Stromnetz anschließen -> Auf dem Display erscheit kurz ein "Bestätigungssymbol" und danach die "Zeitsuchanzeige". Der Wecker 
wartet nun auf zeitinformationen im Radiosignal. Da diese nur 1 mal pro Minute gesendet werden und der Wecker
außerdem mindestens auf 10 plausible Werte waret, wird dieser Vorgang mindestens 10 Minuten dauern. Trotzdem kann jetzt schon die Weckzeit eingestellt werden
2. Einstellen der Weckzeit einleiten durch einmaliges Drücken auf den Drehschalter
3. Drehregler vor oder zurückdrehen bis die gewünschte Weckzeit angezeigt wird (siehe gleich, lesen der Zeitanzeige)
4. Drehregler dürcken um die gewählte Zeit als Weckzeit zu speichern -  Auf dem Display erscheit kurz ein "Bestätigungssymbol" 
und danach die Zeitanzeige bzw. die Zeitsuchanzeige
5. ggf. muss noch der Schalter zum aktivieren der Alarmfunktion auf 1 gestellt werden -> In der Zeitanzeige wird der Status entsprechend angezeigt.

Der Wecker schaltete sich nun zu der gesetzten Uhrzeit ein.
* Um den Alarm für 10 minuten zu pausieren, Schlummer Taste drücken
* Um den Alarm abzustellen, den Drehregler drehen-> Auf dem Display wird mit der Drehbewegung ein Quadrat gezeichnet. 
Hat man weit geng gedreht um das Quadrat zu komplettieren, wird der Alarm beendet
* Um zu verhindern, das der Alarm am nächsten Tag erneut aktiv wird, muss die Funktion mit dem Schalter komplett ausgeschaltet werden.

Hinweis: Ein aktiver Alarm wird durch Betätitigung des Schalters nicht beendet. Dazu ist die Bedienung mit dem Drehknopf notnwedig
Alternativ schaltet sich das Radio nach 1 Stunde automatisch ab.


# Die Uhrzeitanzeige
Die Zeitanzeige wurde entworfen um in der Nacht auch von einem kurzsichtigen Menschen ohne Brille mit moderatem Abstand abgelesen werden zu können. Sie 
ist deshalb mit einfachen Linienmustern konzipiert, die für alle Situationen eindeutig und gut differenzierbar sind. 
Die Anzeige teilt sich in zwei Elemente:
* Stundenrahmen
* Viertelstundenkreis
Da die Zeit insgesamt mit einer Unschärfe von 15 minuten angezeigt wird. , ist sie gegenüber der präzisen Zeit um 5 Minuten
versetzt, d.h. die volle Stunde wird 5 Minuten vor dem erreichen der vollen Stunde angezeigt. 
Der gleiche Versatz für alle weiteren Abschnitte der Stunde. (Dieser Verstatz gilt nur für die Echtzeitanzeige. Die Einstellung der 
Alarmzeit hat eine exakte Anzeige)
## Der Stundenrahmen
Der Stundenrahmen wird am äußeren Rand des Displays angezeigt. Eine kleine Lücke aus 2 Pixeln zeigt die aktuelle Stunde an, analog wie bei einer Zeigeruhr (12 Uhr oben mittig, 3 uhr rechts mittig usw).

Ab 20 Minuten vor der Folgestunde rückt diese Lücke um ein Pixel vor. um die relative Nähe
zur Kommenden Stunde anzuzeigen. 

Der Rahmen ist immer an der, der Stundenlücke gegenüber liegenden Seite, offen (große Lücke). Dadurch ist 
eine sehr grobe Einschätzung der Zeit ohne erkennen der kleinen Lücke möglich. Gerade die allgemein "weckkrittischen" Stunden 5-7 Uhr sind damit gut von den davor liegenden Stunden 11-4  unterscheidbar.

Solange der Alarm über den Schalter abgeschaltet ist, wird dies durch zwei Lücken in den "seitlichen" Rahmenkanten erkennbar gemacht.
## Viertelstundenkreis
In der Mitte des Displays wird der Fortschritt innerhalb einer Stunde in 15 Minutenschritten angezeigt.
* -5 Minuten bis + 5 Minuten zur vollen Stunde: Voller kreis
* 5 Minuten- 10 Minuten: keine Markierung in der Mitte
* 10 - 25 Minuten: 2 Pixel rechts der Mitte
* 25-40 Minuten: zusätzlich 2 Pixel unterhalbt der Mitte
* 40-55 Minuten: zusätzlich 2 Pixel links der Mitte

# Die Weckzeitanzeige
Die Weckzeitanzeige ist ähnlich wie die Uhrzeitanzeige abzulesen. Es gibt folgende Unterschiede:
* Minuten werden in mit einem kleinen Quadreat(4x4) in der Mitte des Displays dargestellt. Jeder Pixel des Quardrates steht für 5 Minuten.
* Uhrzeiten am Nachmittag und Abend  (12-23 Uhr) werden durch einen ausgefüllten Mittelpunkt der Anzeige kenntlich gemacht.
* Die sonst offene Kante des Stundenrahmens ist geschlossen (Anzeige) oder blinkt (Einstellung)
* Die angezeigte Zeit entspricht exakt dem Wert, der gerade gewählt wurde.

# Bedienung

## Weckzeit anzeigen
Wenn sich das der Wecker im Ruhezustand befindet (Anzeige der Uhrzeit, keine laufender Alarm) kann man sich die eingestellte Weckzeit wie folgt anzeigen lassen:
1. Drehregler drehen -> Die Weckzeit wird angezeigt
2. Schlummer Taster drücken -> Es wird wieder die Uhrzeit angezeigt

Alternativ springt die Anzeige auch nach einer kurzen Wartezeit selbständig zurück auf die Uhrzeit.

## Weckzeit stellen
Wenn sich der Wecker im Ruhezustand befinden (Anzeige der Uhrzeit, kein laufender Alarm) kann die Weckzeit wie folgt eingestellt werden:
1. Drehregler drücken -> Die Weckzeit wird angezeigt, wobei die offene Kante des Stundenrahmens blinkt
2. Drehregler vor oder zurückdrehen bis die gewünschte Weckzeit angezeigt wird (siehe gleich, lesen der Zeitanzeige)
3. Drehregler drücken oder 10 Sekunden keine Bedienung vornehmen um die gewählte Zeit als Weckzeit festzulegen->-  Auf dem Display erscheint kurz ein "Bestätigungssymbol" 
und danach die Zeitanzeige bzw. die Zeitsuchanzeige

Alternativ kann mit dem "Schlummer" Taster die Änderung der Weckzeit abgebrochen werden

## Alarmfunktion ein/ausshalten
Die grundsätzliche Aktivierung der Alarmzeit erfolgt mit dem Alarmschalter. Damit der Alarm bei Erreichen der Weckzeit aktiviert wird, muss dieser Schalter auf "1" gestellt sein. Ist er auf "0" gestellt und der Alarm damit deaktiviert, wird dies durch zwei kleine Lücken in den seitlichen Kanten des Stundenrahmens im Display angezeigt.

## Alarm pausieren ("Schlummer")
Wenn das Radio aufgrund des erreichens der Alarmzeit eingeschaltet wurde kann man den Alarm pausieren:
1. Schlummer Taster betätigen -> Das Radio wird abgeschaltet und nach 10 Minuten erneut aktiviert

## Aktuellen Alarm beenden
Um einen laufenden  oder pausierten Alarm zu beenden:
1. Drehregler drehen-> Auf dem Display erscheint mit der Drehbewegung ein Quadrat 
2. Drehregler weiter drehen bis das Quadrat komplettiert ist ->  der Alarm wird beendet

## Nickerchenfunktion
Für ein Nickerchen, kann der Wecker unabhängig von der Weckzeit auf ein Intervall eingestellt werden, nach dem er dann die Weckfunktion aktiviert.
1. Schlummer Taste drücken -> Auf dem Display erscheint ein  "Minutenquadrat", voreingestellt auf 20 Minuten (oder bei laufender Nickerchenfunktion mit der noch verbleibenden Laufzeit)
2. Mit dem Drehregler kann die Dauer in 5 Minutenschritten zwischen 0 und 120 Minuten(großes komplettes Quadrat) geändert werden 
3. Mit drücken auf den Drehregler (oder aber nach 10 Sekunden ohne Bedienhandlung) wird die gewählte Dauer bestätigen -> Bestätigtungshaken erscheint und danach die Uhrzeit

Der Wecker alarmiert nach Ablauf der eingestellten Zeit.

## Nickerchenfunktion ausschalten
Um eine laufende Nickerchenfunktion abzuschalten
1. Schlummer Taste drücken -> Auf dem Display erscheint ein  "Minutenquadrat",  mit der noch verbleibenden Laufzeit
2. Mit dem Drehregler die Zeit auf 0 verringern
3. Mit drücken auf den Drehreger wird die Nickerchenfunktion abgeschaltet.

## Einschlaffunktion / Radio an
Über die Schlaffunktion kann das Radio spontan für eine definierte Zeit eingeschaltet werden.
1. Schlummer Taste drücken ->-> Auf dem Display erscheint ein  "Minutenquadrat", voreingestellt auf 20 Minuten für ein Nickerchen
2. Mit dem Drehregler die Dauer verringern und über den Nullpunkt drehen-> das Radio schaltet sich ein.
3. Durch Weiterdrehen kann die Einschaltdauer in 5 Minutenschritten geändert werden 
4. Mit Drücken auf den Drehregler die gewählte Dauer bestätigen -> Das Radio wird sich nach Ablaufen der Dauer abschalten
Alternativ: Wird nach der Anpassung der Zeit keine weitere Bedienung vorgenommen, wird die angezeigte Zeit als Laufzeit übernommen

## Einschlaffunktion / Radio abschalten
Wenn das Radio über die Schlaffunktion aktiviert wurde, kann es wie folgt abgeschaltet werden:
1. Schlummer Taste drücken -> Auf dem Display erscheint das  "Minutenquadrat, mit der Restzeit der Schlaffunktion
2. Schlummer Taste drücken -> die Schlaffunktion wird abgeschaltet

# Zeitsuchanzeige
Die Zeitsichanzeige wird angezeigt, solange der Wecker nach dem Einschalten keine zuverlässige Uhrzeitinformation besitzt.
Sie besteht aus folgenden Elementen:
* Linker Balken von unten nach oben: Empfangsstärke des laufenden Senders 
* Mittlerer Balken von oben nach unten: Vertrauen in den aktuell empfangenden RDS Wert (Voll = ausreichend um als Uhrzeit übernommen zu werden) 
* Rechter Balken von unten nach oben: Vertrauen in die Uhrzeit

Die Uhr übernimmt die aktuelle  RDS Uhrzeit nur dann, wenn sie das gleiche oder bessere Vertrauen in die RDS zeit als in die laufende Uhrzeit hat. 

# Verhalten bei einem Neustart
Ein Neustart der Uhr erfolgt
* Nach einem Stromausfall
* bei internen Reboot aufgrund eines nicht behebbaren Fehlers
Um zu verhindern, dass eine Weckzeit durch einen Reboot/Stromausfall verpasst wird, schaltet sich der der Alarm unter folgenden Bedingungen unverzüglich ein
* (noch zu implementieren) Der Schlummermodus war aktiv
* Eine "Nickerchenzeit" war gesetzt
* Sowie die Zeit über RDS als korrekt bewertet wurde und die eingestellte Weckzeit weniger als 60 Minuten her ist 

# Sender Preset auswählen
Das Radio hat 4 Senderpresets, die auch bei der Suche der RDS Zeit durchsucht werden. Das Preset, das zum Wecken benutzt wird, kann wie folgt ausgewählt werde,
1. Schlummer Taste drücken -> Auf dem Display erscheint das  "Minutenquadrat, mit der Restzeit der Schlaffunktion oder Nickerchenfunktion"
2. Mit dem Drehregler die Zeit über den größten Wert hinaus drehen bis ein "P" auf dem Display erscheint.
3. Drehregler drücken-> Presetauswahlmodus wird gestartet
4. Mit dem Drehregler das gewünschte Preset auswählen
Zusatz: Mit der Snooze Taste kann das Radio in diesem Modus ein und ausgeschaltet werden.

# Diagnosefunktion(Trace)
Um die Funktionstüchtigkeit des Weckers zu beurteilen können über die Trace Funktion interne Werte der Uhr abgerufen werden.
1. Schlummer Taste drücken -> Auf dem Display erscheint das  "Minutenquadrat, mit der Restzeit der Schlaffunktion oder Nickerchenfunktion"
2. Mit dem Drehregler die Zeit über den größen Wert hinaus drehen bis ein "T" auf dem Display erscheint.
3. Drehregler drücken-> Diagnosemodus wird gestartet
4. Mit den Drehregler die gewünscht Diagnoseseite wählen
     1. Stundenwert der Uhr
     2. Minutenwert der Uhr
     3. Zeitsuchanzeige
     4. Empfangsstärke 
     5. 10er und 1 Ziffer der Frequenz des aktiven Senders
     6. Kommastelle der Frequenz des aktiven Senders
     7. Anzeige der Systemlaufzeit in BCD (Zeile 8 Sekunden, Zeile 7 Minuten, Zeile 6 Stunden, Zeile 5 Tage). Die Systemlaufzeit beginnt nach ca. 50 Tagen von vorne bzw. nach jedem "Reboot".

# Demo Funktion

#

# Hintergründe für das Design
## Anzeige
Die übliche Anzeige mit Ziffern, hat den Nachteil, dass einige Zahlen nur schwer unterscheidbar sind, wenn man nicht über die komplette
Sehschärfe verfügt. So ist eine 5 schwer von eine 6 zu unterscheiden (was am Morgen den Unterschied zwischen Weiterschlafen und Panik 
ausmachen kann)
Analoge Uhrenziffernblätter sind diesbezüglich einfacher, haben aber immer noch das Problem der eindeutigen Orientierung. Eine leichte 
Kopfneigung kann aus einer 5:20  eine 4:15 machen (oder umgekehrt).
Das jetzige Konzept löst beide Probleme:
* für eine Lageverwechslung ist eine Fehleinschätzung von 45 Grad notwendig
* Die Anzeige der Viertelstunden is komplett unterschiedliche zu den Stunden und bietet eine zusätzliche Richtungsorientierung

Beim Ausprobieren verschiedener Darstellungen hat sich außerdem gezeigt, dass es unumgänglich ist, dem Nutzer immer eine Orientierung bzgl
der Gesamtgröße des Disyplays zu geben. Auch hat sich gezeigt, dass eine Unterbrechung von Linien immer 
zu Ableseproblemen führt. Daraus ergab sich am Ende der Ansatz mit dem Rahmen.

## Anordnung der Bedienelemente
* Die Hauptbedienelmente liegen oben und werden vertikal betätigt, damit der Wecker beim Betätigen nicht aus versehen verschoben wird bzw. mit einer zweiten Hand festgehalten werden muss
* der Lauststärkeregler liegt an der Seite unten, da er nach der Optimalen einstellung nicht aus Versehen verstellt werden soll
* Die Hauptbedienelmente liegen hintereinander, entsprechend ihrer Priorität der Nutzung beim Wecken. Schlummer zuerst, Alarm beenden als nächstes, Alarm aus/an als drittes

## Bedienungsabfolge
Die Bedienabfolge ist so gestaltet, dass die Bedienung beim zu Bett gehen(auch für ein Nickerchen) so wenig Geräusche wie möglich verursacht um "akustische Trigger", die den Körper wieder aufwecken, zu vermeiden.
Akustische Trigger sind: Drücken des Drehreglers, aktivierung des Radios. 

* Bedienungen ohne Geräusche: Alarmzeit anschauen, Nickerchenzeit setzen, Alarm pausieren, Alarm beenden
, Sender prüfen/ändern
* Bedienung mit 1 Klick: Alarmzeit ändern,Radiosender wählen



