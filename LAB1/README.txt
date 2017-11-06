Zostały zrealizowane następujące zadania:

1. Wykorzystać funkcję fork() i  wypisac w obu procesach getpid, getppid
   Uruchomienie: ./zad1

2. Połaczyć procesy za pomocą pipe()
   Wysłać coś z jednego do drugiego procesu
   Uruchomienie: ./zad2 some_text

3. exec() --> potomny "cat -n", drugi proces odczytac i wypisac na ekran
   Uruchomienie: ./zad3 hello

4. ./prog p1 arg1 p2 arg2 p3 arg3 ... --> separatory, łańcuch procesów
   Uruchomienie: ./zad4 ls -lah + cat -n + grep zad + tac

Budowanie (dla każdego zadania):
make all

Czyszczenie:
make clean

Spakowanie w archiwum zip (w katalogu głównym):
make zip
