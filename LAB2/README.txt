Laboratorium 2:
1) Prezentacja zadania 4 z ostatnich laboratoriow ("minishell", symulacja pipe |, wiele procesów, wiele argumentów)
2) Zadanie polegało na zbadaniu zachowania pipe'a:
   - 2 procesy
   - jeden szybko pisze
   - drugi wolno odbiera (na początku jeden długi sleep, następnie po każdej odebranej paczce danych krótki sleep)

Budowanie:
make all

Czyszczenie:
make clean

Pakowanie do zipa:
make zip

Testowanie:
./zad1 65999

Wnioski:
- Bufor pipe'a "zapycha" się po włożeniu 65536 bajtów (proces piszący się blokuje)
- Aby odblokować możliwość pisania do pipe'a, należy odczytać 2 bajty
  (system operacyjny OS X 15.6.0 Darwin Kernel Version 15.6.0)
  lub 2 kB (na serwerach LAKu).
