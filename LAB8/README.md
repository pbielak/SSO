# Komunikacja UDP (czat internetowy)

Celem było stworzenie prostego czatu internetowego (serwer asynchroniczny + aplikacja kliencka),
podobnych do poprzedniego laboratorium, przy czym komunikacja powinna odbywać się za pomocą UDP. 
Aplikacje powinny:
- korzystać z komunikacji UDP
- (serwer) asynchronicznie obsługiwać klientów
- (klient) asynchronicznie reagować na zdarzenia użytkownika i serwera

## Budowanie
make

## Uruchomienie:
- serwer `./chat-server 7777`
- klient `./chat-client 127.0.0.1 7777 MyNickname`
