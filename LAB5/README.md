# Wątki + mutexy

Celem było stworzenie programu do symulacji kont bankowych. Program na 2 tryby:
- WITHDRAW (zmiana salda jednego konta o określoną wartość)
- TRANSFER (transfer kwoty z jednego konta na drugie)

## Budowanie:
make

## Uruchomienie (przykłady):
Program posiada 3 flagi:
  - `-N nb_threads` - liczba wątków (operacji WITHDRAW, TRANSFER); musi zostać podany jako pierwszy argument,
  - `-W account_number~cash_amount~nb_steps` - operacja wpłaty danej kwoty (`cash_amount`) z konta (`account_number`) określoną liczbę razy (`nb_steps`); w przypadku podania ujemnej kwoty, operację można interpretować jako wypłatę ,
  - `-T account_from~account_to~cash_amount~nb_steps` - operacja przelewu kwoty (`cash_amount`) z konta `account_from` na konto `account_to` określoną liczbę razy (`nb_steps`)

Np. `./threaded_bank_account_modifier -N 5 -W 0~100~5 -W 0~-100~4 -W 1~50~3 -T 0~1~20~2 -T 1~0~20~2`
