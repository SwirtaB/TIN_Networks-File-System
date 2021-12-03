# TIN - NFS - Dokumentacja wstępna

## Skład zespołu
* Świrta Bartosz - lider
* Wojno Maciej
* Woźniak Szymon
* Hebda Jakub

Data przekazania dokumentacji wstępnej: 03.12.2021

## Temat projektu
<p align="justify">
Napisać program obsługujący uproszczona wersję protokołu NFS (Network File System). Należy zaimplementować: serwer, bibliotekę kliencką (jako plik .a lub .so) oraz testowe programy klienckie realizujące funkcje operujące na zdalnych plikach (zlokalizowanych na serwerze sieciowym). Biblioteka klienca ma udostępniać zdalne odpowiedniki niektórych funkcji systemowych na plikach (open, close, read, write, lseek, unlink, fstat, flock).
</p>

### Wariant zadania
* zaimplementować blokowanie dostepu do plików w trybie *jeden pisarz albo wielu czytelników* - zob. [flock(2)](https://man7.org/linux/man-pages/man2/flock.2.html)
* zaimplementować po stronie serwera możliwość udostępniania wielu niezależnych systemów plików

## Interpretacja treści i przyjęte założenia
<p align="justify">
Proponowane rozwiązanie to protokół stanowy, w którym udostępnienie zdalnego systemu plików użytkownikowi zostanie wykonane w ramach zestawionej sesji. Udostępniane przez serwer systemy plików są na serwerze oddzielnymi katalogami. Wszytskie operacje plikowe będą wykonywane przez system operacyjny Linux - wykorzystamy gotowe systemy plików. W konsekwencji planujemy nałożyć minimalną logikę na implementowane funkcje plikowe i w większości wypadków wysyłać bezposrodnie przez nie zwrócone wartości.
</p>

## Opis funkcjonalny
__Biblioteka kliencka__  
<p align="justify">
Biblioteka udostępnia programiście klasę NFSConnection, do stworzenia której programista musi podać nazwę użytkownika i hasło na systemie serwera oraz nazwę wybranego systemu plików udostępnianego przez serwer. Obiekt tej klasy enkapsuluje w swojej implementacji nawiązanie połączenia z serwerem, autoryzację użytkownika oraz wybranie systemu plików, i udostępnia następujące metody:
</p>

- `int open(char *path, int oflag, int mode)`
- `int close(int fd)`
- `ssize_t read(int fd, void *buf, size_t count)`
- `ssize_t write(int fd, const void *buf, size_t count)`
- `off_t lseek(int fd, off_t offset, int whence)`
- `int fstat(int fd, struct stat *statbuf)`
- `int unlink(const char *path)`
- `int flock(int fd, int operation)`

<p align="justify">
Metody te wysyłają do serwera rządania wykoniania danych operacji za pomocą podanego deskryptora. Klient nie przechowuje żadnych informacji poza utrzymywaniem połączenia z serwerem. To serwer przechowuje mapowanie między deskryptorami przekazanymi klientowi, a faktycznymi deskryptorami udostępnianych plików.
</p>

__Serwer__  
Program uruchomiony na maszynie przechowującej udostępniane systemy plików.
- Serwer wspiera autoryzację i prawa dostępu z użyciem użytkowników na systemie, na którym uruchomiony jest serwer. W konsekwencji program serwera musi działać z prawami root'a.
- Serwer w odpowiedzi na `open(...)` otwiera lokalnie plik i tworzy deskryptor własny, który wysyła klientowi, przechowując na okres otwarcia pliku mapowanie z deskryptora własnego na deskryptor lokalny.
- Komendy klienta korzystające z deskryptora wysłanego przez serwer powodują wykonanie odpowiadających funkcji systemowych na lokalnych deskryptorach serwera.
- Funkcje systemowe na plikach są wykonywane z prawami użytkownika którego reprezentuje klient, za pomocą funkcji systemowej `seteuid()`.

__Implementacja blokowania dostepu do plików w trybie *jeden pisarz albo wielu czytelników*__  
<p align="justify">
Pchamy chamsko flock przez sieć //TODO -> trzeba to ładnie napisać 
</p>

__Implementować po stronie serwera możliwość udostępniania wielu niezależnych systemów plików__  
<p align="justify">
Udostępnienie wielu systemów plików jest realizowane częściowo przez protokół, częściowo przez organizację systemu plików na maszynie serwera.

Każdy możliwy do udostępnienia system plików jest niezależnym folderem na maszynie serwera. Folder ten będzie stanowił *root folder* udostępnianego systemu plików. System plików przyjmuje nazwę katalogu, w którym jest realizowany.

Zestawiając połączenie klient-serwer (z użyciem protokołu TCP) klient wysyła dane do autoryzacji i rządany system plików. W przypadku braku uprawnień użytkownika do katalogu systemu plików, bądź błędnej nazwy systemu plików, połączenie zostanie zakończone. Zakończenie połączenia poprzedzi wysłanie przez serwer stosownego komunikatu o błądzie - np. *podany system plików nie istnieje*.
</p>

## Analiza stosowanych protokołów komunikacyjncyh i rozwiązań

## Planowany podział na moduły

* NFSConnection - moduł realizujący bibliotekę kliencą. Realizuje logikę połączenia i komunikacji z serwerem za pomocą modułu NFSCommunication.

* NFSCommunication - moduł odpowiedzialny za zestawianie połączeń TCP i przesyłanie pojedyńczych komunikatów. Wykorzystywany zarówno do implementacji klienta jak i serwera. Zawiera w sobie definicje wszystkich możliwych do przesyłania między stronami połączenia struktur danych reprezentujących komunikaty.

* NFSServer - moduł realizujący program serwera. Przyjmuje połączenia z NFSConnection i dla każdego tworzy NFSServerWorker. Realizuje zestawienie połączenia z NFSConnection za pomocą modułu NFSCommunication.

* NFSServerWorker - moduł realizujący połączenie z NFSConnection. Pracuje w kontekście jednego systemu plików. Realizuje logikę komunikacji z NFSConnection za pomocą modułu NFSCommunication.

* Aplikacja kliencka - programy pokazujące działanie biblioteki z wykorzystaniem NFSConnection.


## Szczegóły implementacji i używane biblioteki
Język implementacji: C++20  
Narzędzie budowania: CMake  
Formater kodu: clang-format - format własny

Użyte biblioteki:
- XDR
- systemowa biblioteka sockets