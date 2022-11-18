-------------------------------------------------------------------------------
Nume: Iordache Madalina Gabriela
Grupa: 333CA

                                 Tema1 - Map-Reduce

Pentru rezolvarea temei s-au executat urmatorii pasi pe care ii voi detalia
ulterior:
 - procesarea argumentelor si citirea datelor din fisierul principal de intrare
 (test.txt)
 - initializarea mutexilor si a barierei pentru sincronizarea procesarii datelor
 - alocarea de memorie dinamica pentru structurile transmise in functiile
 threadurilor
 - pornirea threadurilor de procesare cu parametrii: adresa functiei specifice
 procesarii si adresa datelor ce reprezinta parametrii acesteia
 - asteptarea procesarii thread-urilor (cu join) si dezalocarea memoriei dinamice
 folosite

1.Procesarea argumentelor si citirea datelor consta initializarea numarului de
thread-uri pentru mapper si reducer si setarea numarului de fisiere de intrare
pentru procesare de catre mapper.
2.Parametrii de sincronizare sunt initializati, bariera avand un numar de
thread-uri in asteptare de: nr_threaduri_maping + nr_threaduri_reduce.
3.Alocarea de memorie si initializarea structurilor este realizata in functia:
allocMemory. In cadrul acesteia se aloca memorie pe heap si se initializeaza
vectorul de adrese de start al listelor maperilor.
Acest vector este necesar pentru reducer, deoarece fiecare reducer are nevoie
de acces la cel putin o lista de la fiecare mapper.

4.Threadurile de mapping sunt pornite cu urmatoare functie: paralel_process_map.
Acestea se sincronizeaza la "alegerea unui fisier de procesat" printr-un mutex. 
Dupa ce thread-ul stie ce fisier proceseaza, se elibereaza mutex-ul pentru a
permite altor thread-uri sa continue rularea. Fisierul este deschis si transmis
in functia process_input_file.
In cadrul functiei de "process_input_file" se extrag datele si sunt filtrate
astfel: se cauta radacina cea mai apropiata pentru toate puterile testate
(prin formula lui Newton -> link: 
https://en.wikipedia.org/wiki/Nth_root#Computing_principal_roots). 

In continuare, radacinile ce sunt apropiate de un numar natural sunt luate in
considerare (se verifica daca numarul rotunjit minus numarul initial este mai mic
decat o eroare prestabilita - in cazul acesta 0.1).
Toate numerele ce corespund acestui criteriu sunt verificate prin impartiri
repetate la radacina incat se confirme daca baza(radacina) este intr-adevar
corecta (nu a fost afectata de operatiile in virgula mobila).
Odata depistat un astfel de numar, acesta este introdus in lista exponentului
corespunzator.

Toate threadurile de maping trebuie sa se intalneasca la bariera pentru a
sincroniza pornirea threadurilor reducer.
Threadurile reducer asteapta la randul lor finalizarea procesului de mapare
prin intermediul aceleiasi bariere.
Dupa ce se finalizeaza maparea, acestea sunt sincronizate in alegerea
exponentului. Odata ales se construieste fisierul de iesire pentru fiecare
exponent.
Se parcurg listele maperilor specifice exponentului curent, iar elementele
acestuia sunt adaugate intr-un set pentru eliminarea duplicatelor.
Dupa finalizarea procesarii unui exponent de catre un thread se scrie in
fiserul de iesire dimensiunea setului.

5.In final, se asteapta terminarea procesarii de catre thread-urile de map
si cele de reduce pentru a dezaloca memoria de pe heap.
