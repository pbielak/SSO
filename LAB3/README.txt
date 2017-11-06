Laboratorium 3:
1) Zadanie polegało na zbadaniu zachowania FIFO:
   - 2 procesy
   - jeden szybko pisze
   - drugi wolno odbiera (na początku jeden długi sleep, następnie po każdej odebranej paczce danych krótki sleep)

2) Kontroler (front-end) do mplayera za pomocą FIFO.

Budowanie:
make all

Czyszczenie:
make clean

Pakowanie do zipa:
make zip

Testowanie:
1) ./zad1 create
   ./zad1 reader 70000 (w jednej konsoli)
   ./zad1 writer 70000 (w drugiej konsoli)

2) mkfifo /tmp/mplayer_fifo
   mplayer -input file=/tmp/mplayer_fifo plik.mp3 (w jednej konsoli)
   ./frontend /tmp/mplayer_fifo (w drugiej konsoli)

Wnioski do zadania 1):
- Bufor FIFO "zapycha" się po włożeniu 8192 bajtów (proces piszący się blokuje)
- Aby odblokować możliwość pisania do FIFO, należy odczytać 1 bajt.
  (system operacyjny OS X 15.6.0 Darwin Kernel Version 15.6.0)

