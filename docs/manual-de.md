# El Banquos Radiowecker - Bedienungsanleitung

## Die Idee
Mangels eines ausgereiften Produktes in den hiesigen Elektronikmärkten habe ich mir einen eigenen Radiowecker gebaut,
der folgende Eigenschaften in sich vereint:

* Leicht einstellbare Weckzeit (Vor- und Zurückstellbar)
* "Einblenden" des Radios zur Weckzeit (statt brachialem "Radio an")
* Eine Anzeige die ich auch ohne Brille ablesen kann, aber nicht gleich riesig ist
* "entspannte" Zeitanzeige (Ich brauche es nicht auf die Minute genau) 
* Anzeige ist nur so hell, wie gerade nötig und nicht mit geschlossenen Augen warnehmbar
* Stellt sich selber überdie im Radiosignal verfügbare Zeitinformation (RDS)
* Deaktivieren des Alarms kann nicht aus Versehen passieren
* Sender und Lautstärke können nicht aus Versehen verstellt werden

## Bestandteile des Weckers
* 8x8 Punkmatrix Display zur Anzeige aller Informationen
* Snooze Taster
* Einstelldrehknopf und Taster
* Schalter zum aktivieren der Alarmfunktion
* Lautsprecher an der Seite
* Lautstärkeregler an der Seite, sehr weit hinten

## Inbetriebnahme und Schnellstart
1. Wecker an das Stromnetz anschließen -> Auf dem Display erscheit kurz ein "Bestätigungssymbol" und danach die "Zeitsuchanzeige". Der Wecker 
wartet nun auf zeitinformationen im Radiosignal. Da diese nur 1 mal pro Minute gesendet werden und der Wecker
außerdem mindestens auf 10 plausible Werte waret, wird dieser Vorgang mindestens 10 Minuten dauern. Trotzdem kann jetzt schon die Weckzeit eingestellt werden
2. Einstellen der Weckzeit einleiten durch einmaliges Drücken auf den Drehschalter
3. Drehschalter vor oder zurückdrehen bis die gewünschte Weckzeit angezeigt wird (siehe gleich, lesen der Zeitanzeige)
4. Drehschalter dürcken um die gewählte Zeit als Weckzeit zu speichern -  Auf dem Display erscheit kurz ein "Bestätigungssymbol" 
und danach die Zeitanzeige bzw. die Zeitsuchanzeige
5. ggf. muss noch der Schalter zum aktivieren der Alarmfunktion auf 1 gestellt werden -> In der Zeitanzeige wird der Status entsprechend angezeigt.

Der Wecker schaltete sich nun zu der gesetzten Uhrzeit ein.
* Um den Alarm für 10 minuten zu pausieren, Snooze Taste drücken
* Um den Alarm abzustellen, den Drehknopf drehen-> Auf dem Display wird mit der Drehbewegung ein Quadrat gezeichnet. 
Hat man weit geng gedreht um das Quadrat zu komplettieren, wird der Alarm beendet
* Um zu verhindern, das der Alarm am nächsten Tag erneut aktiv wird, muss die Funktion mit dem Schalter komplett ausgeschaltet werden.

Hinweis: Ein aktiver Alarm wird durch Betätitigung des Schalters nicht beendet. Dazu ist die Bedienung mit dem Drehknopf notnwedig
Alternativ schaltet sich das Radio nach 1 Stunde automatisch ab.


## Die Zeitanzeige
Die Zeitanzeige wurde entworfen um in der Nacht auch von einem kurzsichtigen Menschen ohne Brille mit moderatem Abstand abgelesen werden zu können. Sie 
ist deshalb mit einfachen Linienmustern konzipiert, die für alle Situationen eindeutig und gut differenzierbar sind. 
Die Anzeige teilt sich in zwei Elemente:
* Stundenrahmen
* Viertelstundenkreis
Da die Zeit insgesamt mit einer Unschärfe von 15 minuten angezeigt wird. , ist sie gegenüber der präzisen Zeit um 5 Minuten
versetzt, d.h. die volle Stunde wird 5 Minuten vor dem erreichen der vollen Stunde angezeigt. 
Der gleiche Versatz für alle weiteren Abschnitte der Stunde. (Dieser Verstatz gilt nur für die Echtzeitanzeige. Die Einstellung der 
Alarmzeit hat eine exakte Anzeige)
### Der Stundenrahmen
Der Stundenrahmen wird am äußeren Rand des Displays angezeigt. Eine kleine Lücke aus 2 Pixeln zeigt die aktuelle Stunde an, und zwar auf den
gleichen Positionen wie bei einer Zeigeruhr (12 Uhr oben mittig, 3 uhr rechts mittig usw). Ab 20 Minuten vor der Folgestunde rückt diese Lücke um ein Pixel vor. um die relative Nähe
zur Kommenden Stunde anzuzeigen. Der Rahmen ist immer an der, der Stundenlücke gegenüber liegenden Seite, offen (große Lücke). Dadurch ist 
eine sehr grobe Einschätzung der Zeit ohne erkennen der kleinen Lücke möglich. Gerade die allgemein "weckkrittischen" Stunden 5-7 Uhr sind 
damit gut von den davor liegenden Stunden 11-4  unterscheidbar.
Solange der Alarm über den Schalter abgeschaltet ist, wird dies durch zwei Lücken in den "seitlichen" Rahmenkanten erkennbar gemacht.
### Viertelstundenkreis
In der Mitte des Displays wird der Fortschritt innerhalb einer Stunde in 15 Minutenschritten angezeigt.
* -5 Minuten bis + 5 Minuten zur vollen Stunde: Voller kreis
* 5 Minuten- 10 Minuten: keine Anzeige
* 10 - 25 Minuten: 2 Pixel rechts der Mitte
* 25-40 Minuten: zusätzlich 2 Pixel unterhalbt der Mitte
40-55 Minuten: zusätzlich 2 Pixel links der Mitte

## Arbeitszustände des Weckers
Über die ####

### Weckzeit anzeigen
Wenn sich das der Wecker im Ruhezustand befindet 
1. Drehregler drehen

## Hintergründe für das Design
### Anzeige
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











