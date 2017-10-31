# Semafory

Celem było stworzenie programu do symulacji kont bankowych. Program na 2 tryby:
- WITHDRAW (zmiana salda jednego konta o określoną wartość)
- TRANSFER (transfer kwoty z jednego konta na drugie)

Budowanie:
make

Uruchomienie (przykłady):
Dla opcji WITHDRAW: ./bank_account_modifier WITHDRAW account_index cash_amount nb_steps
./bank_account_modifier WITHDRAW 0 100 5
./bank_account_modifier WITHDRAW 0 -500 2

Dla opcji TRANSFER: ./bank_account_modifier TRANSFER account_from account_to cash_amount nb_steps
./bank_account_modifier TRANSFER 0 1 100 5

