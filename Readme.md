# IPK Projekt 1

C++ server komunikující prostřednictvím protokolu HTTP poskytujcí informace o systému.

Konkrétně lze od serveru získat následující informace:
 * doménové jméno 
 * název CPU 
 * aktuální zatížení CPU

## Kompilace

Použitím příkazu __make__ ve složce se zdrojovým souborem __server.cpp__ dojde k vytvoření spustitelného souboru serveru __hinfosvc__.

```
$ make
```

## Spuštění

Při spuštění serveru je nutné zadat jako argument příkazové řádky __číslo portu__,na kterém bude server naslouchat. Validí hodnoty čísla portu jsou v rozmezí 1 - 65535.  
__POZOR:__ Porty 1 - 1023 jsou rezervovány systémem, při jejich použití pravděpodobně dojde k chybě.

```
$ ./hinfosvc 4444
SERVER: listening on port 4444
```

## Použití
Se serverem je možno komunikovat pomocí webového prohlížeče nebo pomocí nástrojů typu wget a curl.
Server umí zpracovat následující dotazy zaslané příkazem __GET__:

1. Získání doménového jména

    Vrací __síťové jméno počítače__ včetně domény, například:
    ```
    $ curl http://localhost:4444/hostname
    vilem-VirtualBox
    ```

2. Získání informací o CPU 

    Vrací název __modelu procesoru__, například:
    ```
    $ curl http://localhost:4444/cpu-name
    AMD Ryzen 5 5600H with Radeon Graphics
    ```

3. Aktuální zátěž 

    Vrací aktuální informace o __zátěži CPU__, například:
    ```
    $ curl http://localhost:4444/load
    26%
    ```
 ### Neznámé dotazy
V případě __neznámého dotazu__ vrací __400 Bad Request__, například:
```
$ curl http://localhost:4444/nonsense
400 Bad Request
```
 ### Interní chyba serveru
V případě __interní chyby__ serveru při zpracování dotazu vrací __500 Internal Server Error__, například:
```
$ curl http://localhost:4444/hostname
500 Internal Server Error
```
