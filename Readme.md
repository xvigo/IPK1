# VUT FIT - IPK Projekt 1
## Autor : Vilém Gottwald (xgottw07)
## Popis
Server v jazyce C++ komunikující prostřednictvím protokolu HTTP, který na základě dotazů poskytuje informace o systému.

Konkrétně lze od serveru získat následující informace:
 * doménové jméno 
 * název modelu CPU 
 * aktuální zatížení CPU

## Kompilace

Použitím příkazu __make__ ve složce se zdrojovým souborem __server.cpp__ dojde k vytvoření spustitelného souboru serveru __hinfosvc__.

```
$ make
```

## Spuštění

Při spuštění serveru je nutné zadat jako argument příkazové řádky __číslo portu__, na kterém bude server naslouchat. Validí hodnoty čísla portu jsou v rozmezí 1 - 65535.  
__POZOR:__ Porty 1 - 1023 jsou rezervovány systémem, při jejich použití pravděpodobně dojde k chybě.

```
$ ./hinfosvc 4444
SERVER: listening on port 4444
```

## Použití
Se serverem je možno komunikovat pomocí webového prohlížeče nebo pomocí nástrojů typu wget a curl.  
Server umí zpracovat následující dotazy zaslané příkazem __GET__ (případně příkazem __HEAD__):

1. Získání doménového jména - __hostname__

    Vrací __síťové jméno počítače__ včetně domény, například:
    ```
    $ curl http://localhost:4444/hostname
    vilem-VirtualBox
    ```

2. Získání informací o CPU - __cpu-name__

    Vrací název __modelu procesoru__, například:
    ```
    $ curl http://localhost:4444/cpu-name
    AMD Ryzen 5 5600H with Radeon Graphics
    ```

3. Aktuální zátěž - __load__

    Vrací aktuální informace o __zátěži CPU__, například:
    ```
    $ curl http://localhost:4444/load
    26%
    ```

## Další možné odpovědi serveru  
 ### Ostatní HTTP příkazy
V případě dotazů __ostatními HTTP příkazy__ vrací __501 Not Implemented__, například:
```
$ curl -X  DELETE http://localhost:4444/hostname

501 Not Implemented: Server only supports HEAD and GET
```
 ### Neplatná cesta
V případě dotazu s __neplatnou cestou__ vrací __404 Not Found__, například:
```
$ curl http://localhost:4444/nonsense
404 Not Found
```
 ### Interní chyba serveru
V případě __interní chyby__ serveru při zpracování dotazu vrací __500 Internal Server Error__.

 ### Neplatný dotaz
V případě __neplatného dotazu__ vrací __400 Bad Request__.
