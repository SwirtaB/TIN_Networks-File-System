# TIN - NFS - Dokumentacja końcowa

## Skład zespołu
* Świrta Bartosz - lider
* Wojno Maciej
* Woźniak Szymon 
* Hebda Jakub 

Data przekazania dokumentacji końcowej: 25.01.2022

## Temat projektu
<p align="justify">
Napisać program obsługujący uproszczoną wersję protokołu NFS (Network File System). Należy zaimplementować: serwer, bibliotekę kliencką (jako plik .a lub .so) oraz testowe programy klienckie realizujące funkcje operujące na zdalnych plikach (zlokalizowanych na serwerze sieciowym). Biblioteka kliencka ma udostępniać zdalne odpowiedniki niektórych funkcji systemowych na plikach (open, close, read, write, lseek, unlink, fstat, flock).
</p>

### Wariant zadania
* zaimplementować blokowanie dostepu do plików w trybie *jeden pisarz albo wielu czytelników* - zob. [flock(2)](https://man7.org/linux/man-pages/man2/flock.2.html)
* zaimplementować po stronie serwera możliwość udostępniania wielu niezależnych systemów plików

## Interpretacja treści i przyjęte założenia
<p align="justify">
Proponowane rozwiązanie to protokół stanowy, w którym udostępnianie zdalnego systemu plików użytkownikowi zostanie wykonane w ramach zestawionej sesji. Udostępniane przez serwer systemy plików są na serwerze oddzielnymi katalogami. Wszytskie operacje plikowe będą wykonywane przez system operacyjny Linux - wykorzystamy gotowe systemy plików. W konsekwencji planujemy nałożyć minimalną logikę na implementowane funkcje plikowe i w większości wypadków wysyłać w odpowiedzi bezpośrodnio zwrócone przez nie wartości.  
Przyjmujemy następujące założenia:  

- Komunikacja między klientem a serwerem odbywa się z wykorzystaniem protokołu TCP
- Klient łącząc się z serwerem uzyskuje dostęp do wybranego systemu plików
- Klient w ramach utworzonego połączenia może wykonywać operacje plikowe
- Klient może zestawić wiele niezależnych połaczeń z serwerem
- Serwer wykonuje operacje na plikach wykorzystując funkcje systemowe
- Serwer autoryzuje użytkownika za pomocą mechanizmu użytkowników systemowych
- Implementacja nakłada ograniczenie na maksymalny rozmiar pliku wynikający z możliwość zaalokowania bufora o maksymalnym rozmiarze int64. W praktyce jest to ~8EB co jest połową teoretycznego maksymalnego rozmiaru pliku na maszynie 64bit.
</p>

## Opis funkcjonalny
### Ujęcie ogólne  
Klientowi zostają udostępnione poniższe funkcjonalności:
- możliwość połączenia się ze zdalnym systemem plików
- dokonywanie standardowych operacji plikowych:
    - odczyt pliku
    - zapis pliku
    - edycja pliku
    - usunięcie pliku  

__Biblioteka kliencka__  
<p align="justify">
Biblioteka udostępnia programiście klasę NFSConnection, do stworzenia której programista musi podać nazwę użytkownika i hasło na systemie serwera oraz nazwę wybranego systemu plików udostępnianego przez serwer. Obiekt tej klasy enkapsuluje w swojej implementacji nawiązanie połączenia z serwerem, autoryzację użytkownika oraz wybranie systemu plików, i udostępnia następujące metody:
</p>

- `ConnectReturn connect(const std::string &hostName, const std::string &username, const std::string &password, const std::string &filesystemName)`
- `int open(char *path, int oflag, int mode)`
- `int close(int fd)`
- `ssize_t read(int fd, void *buf, size_t count)`
- `ssize_t write(int fd, const void *buf, size_t count)`
- `off_t lseek(int fd, off_t offset, int whence)`
- `int fstat(int fd, struct stat *statbuf)`
- `int unlink(const char *path)`
- `int flock(int fd, int operation)`

<p align="justify">
Metody te wysyłają do serwera rządania wykoniania danych operacji za pomocą podanego logicznego deskryptora. NFSConnection nie przechowuje żadnych informacji poza utrzymywaniem połączenia z serwerem (przydzielone gniazdo) i kodem błędów. Użytkownik biblioteki klienckiej zarządza otrzymanym od serwera deskryptorem pliku i za jego pomocą wykonuje na otwartych plikach operacje.
</p>

__Serwer__  
Program uruchomiony na maszynie przechowującej udostępniane systemy plików.
- Serwer wspiera autoryzację i prawa dostępu z użyciem użytkowników na systemie, na którym uruchomiony jest serwer. W konsekwencji program serwera musi działać z prawami root'a.
- Serwer w odpowiedzi na `open(...)` otwiera lokalnie plik i tworzy logiczny deskryptor, który wysyła klientowi, przechowując na okres otwarcia pliku mapowanie z deskryptora logicznego na deskryptor lokalny.
- Komendy klienta korzystające z deskryptora wysłanego przez serwer powodują wykonanie odpowiadających funkcji systemowych na lokalnych deskryptorach serwera.
- Funkcje systemowe na plikach są wykonywane z prawami użytkownika którego reprezentuje klient, za pomocą funkcji systemowej `seteuid()`.

__Implementacja blokowania dostepu do plików w trybie *jeden pisarz albo wielu czytelników*__  
<p align="justify">
Implementacja zostanie wykonana z użyciem systemowego polecenia flock(2). Ze względu na fakt, że protokół nie obsługuje bezpośrednio systemu plików postanowiliśmy wykorzystać systemową implementację tej metody, a za pomocą protokołu będziemy przekazywać jedynie atrybuty funkcji i wartości przez nią zwracane.
</p>

__Implementacja po stronie serwera możliwość udostępniania wielu niezależnych systemów plików__  
<p align="justify">
Udostępnienie wielu systemów plików jest realizowane częściowo przez protokół, częściowo przez organizację systemu plików na maszynie serwera.

Każdy możliwy do udostępnienia system plików jest niezależnym folderem na maszynie serwera. Folder ten będzie stanowił *root folder* udostępnianego systemu plików. System plików przyjmuje nazwę katalogu, w którym jest realizowany.

Zestawiając połączenie klient-serwer (z użyciem protokołu TCP) klient wysyła dane do autoryzacji i rządany system plików. W przypadku braku uprawnień użytkownika do katalogu systemu plików, bądź błędnej nazwy systemu plików, połączenie zostanie zakończone. Zakończenie połączenia poprzedzi wysłanie przez serwer stosownego komunikatu o błądzie - np. *podany system plików nie istnieje*.
</p>

## Protokół komunikacyjny

Komunikacja między klientem a serwerem odbywa się poprzez krótkie, synchroniczne konwersacje inicjowane przez klienta. Pierwszą konwersacją po zestawieniu sesji TCP zawsze jest autoryzacja użytkownika, która zestawia połączenie naszego protokołu. Konwersacja kończy się otrzymaniem komunikatu, po którym nadawca nie spodziewa się odpowiedzi.

### Przykładowa sesja połączenia
![session example](./reports/figures/session_example.png)

### Struktury reprezentujące pojedyńcze komunikaty:
Klienta:
```
struct CMSGConnectInfoUsername {
    uint8_t code; // CONNECT_INFO_USERNAME
    char username[];
    uint64_t username_size;
}
```
```
struct CMSGConnectInfoPassword {
    uint8_t code; // CONNECT_INFO_PASSWORD
    char password[];
    uint64_t password_size;
}
```
```
struct CMSGConnectInfoFSName {
    uint8_t code; // CONNECT_INFO_FSNAME
    char fsname[];
    uint64_t fsname_size;
}
```
```
struct CMSGRequestOpen {
    uint8_t code; // REQUEST_OPEN
    uint64_t oflag;
    uint64_t mode;
    char path[];
    uint64_t path_size;
}
```
```
struct CMSGRequestClose {
    uint8_t code; // REQUEST_CLOSE
    int64_t fd;
}
```
```
struct CMSGRequestRead{
    uint8_t code; // REQUEST_READ
    int64_t fd;
    uint64_t size;
}
```
```
struct CMSGRequestWrite{
    uint8_t code; // REQUEST_WRITE
    int64_t fd;
    char data[];
    uint64_t size;
}
```
```
struct CMSGRequestLseek{
    uint8_t code; // REQUEST_LSEEK
    int64_t fd;
    int64_t offset;
    int64_t whence;
}
```
```
struct CMSGRequestFstat{
    uint8_t code; // REQUEST_FSTAT
    int64_t fd;
}
```
```
struct CMSGRequestUnlink{
    uint8_t code; // REQUEST_UNLINK
    char path[];
    uint64_t path_size;
}
```
```
struct CMSGRequestFlock{
    uint8_t code; // REQUEST_FLOCK
    int64_t fd;
    int64_t operation;
}
```
```
struct CMSGDisconnect{
    uint8_t code; // DISCONNECT
}
```
Serwera:

```
struct SMSGProvideUsername {
    uint8_t code; // PROVIDE_USERNAME
}
```
```
struct SMSGProvidePassword {
    uint8_t code; // PROVIDE_PASSWORD
}
```
```
struct SMSGProvideFSName {
    uint8_t code; // PROVIDE_FSNAME
}
```
```
struct SMSGAuthorizationOk {
    uint8_t code; // AUTHORIZATION_OK
}
```
```
struct SMSGAuthorizationFailed {
    uint8_t code; // AUTHORIZATION_FAILED
}
```
```
struct SMSGResultOpen {
    uint8_t code; // RESULT_OPEN
    int64_t fd;
    int64_t errno;
}
```
```
struct SMSGResultClose {
    uint8_t code; // RESULT_CLOSE
    int64_t result;
    int64_t errno;
}
```
```
struct SMSGResultRead {
    uint8_t code; // RESULT_READ
    int64_t errno;
    char data[];
    uint64_t size;
}
```
```
struct SMSGResultWrite {
    uint8_t code; // RESULT_WRITE
    int64_t result;
    int64_t errno;
}
```
```
struct SMSGResultLseek {
    uint8_t code; // RESULT_LSEEK
    int64_t offset;
    int64_t errno;
}
```
```
struct SMSGResultFstat {
    uint8_t code; // RESULT_FSTAT
    int64_t result;
    int64_t errno;
    struct stat statbuf;
}
```
```
struct SMSGResultUnlink {
    uint8_t code; // RESULT_UNLINK
    int64_t result;
    int64_t errno;
}
```
```
struct SMSGResultFlock {
    uint8_t code; // RESULT_FLOCK
    int64_t result;
    int64_t errno;
}
```

Przed komuniatem przesyłany jest integer 64-bitowy zawierający rozmiar przesyłanego dalej komunikatu. Pierwszy bajt każdego rodzaju komunikatu jest nagłówkiem identyfikującym jednoznacznie typ odbieranego komunikatu.

### Spodziewane odpowiedzi na komunikaty
`CMSGConnectInfoUsername -> SMSGProvidePassword`  
`CMSGConnectInfoPassword -> SMSGProvideFSName | SMSGAuthorizationFailed`  
`CMSGConnectInfoFSName -> SMSGProvidePassword | SMSGAuthorizationFailed | SMSGAuthorizationOk`  
`CMSGRequestOpen -> SMSGResultOpen`  
`CMSGRequestClose -> SMSGResultClose`  
`CMSGRequestRead -> SMSGResultRead`  
`CMSGRequestWrite -> SMSGResultWrite`  
`CMSGRequestLseek -> SMSGResultLseek`  
`CMSGRequestFstat -> SMSGResultFstat`  
`CMSGRequestUnlink -> SMSGResultUnlink`  
`CMSGRequestFlock -> SMSGResultFlock`  
  
`SMSGProvideUsername -> CMSGConnectInfo`  
`SMSGProvidePassword -> CMSGConnectInfo`  
`SMSGProvideFSName -> CMSGConnectInfo`  
  
Na pozostałe komunikaty nie spodziewamy się odpowiedzi. W przypadku otrzymania przez klienta błędnej odpowiedzi od serwera, dana funkcja kończy się z błędem, sesja protokołu zostaje utrzymana. Interfejs biblioteki klienckiej pozwala na tylko poprawną komunikację z serwerem.

## Analiza protokołu

Wybraliśmy protokół TCP ponieważ:
- połączenie ma charakter sesyjny/konwersacyjny, z serwerem udostępniającym usługę
- zapewnia dostarczenie wszystkich przesyłanych danych
- ułatwia przesyłanie dużych ciągów danych (istotne z punktu widzenia przesyłania plików)
- zapewnia abstrakcje strumieniowości przesyłanych danych

<p align="justify">
Operacje plikowe są wykonywane po stronie serwera, bez ingerencji protokołu. Naturalnym wydało się więc użycie systemowego mechanizmu użytkowników do zarządzania prawami dostępu. Dodatkowo postanowiliśmy wykorzystać to jako mechanizm autoryzacji.
</p>


<p align="justify">
Główną wadą tego rozwiązania jest przesyłanie żywym tekstem hasła przez niezabezpieczone połączenie. Tak samo problemem jest przesyłanie niezaszyfrowanych danych odczytywanych/zapisywanych do plików na serwerze. Moglibyśmy rozwiązać te problemy zestawiając między klientem a serwerem zaszyfrowane połączenie TLS, w taki sam sposób jak robi to SSH.
</p>

## Podział na moduły

* NFSConnection - moduł realizujący bibliotekę kliencą. Realizuje logikę połączenia i komunikacji z serwerem za pomocą modułu NFSCommunication.

* NFSCommunication - moduł odpowiedzialny za zestawianie połączeń TCP i przesyłanie pojedyńczych komunikatów. Wykorzystywany zarówno do implementacji klienta jak i serwera. Zawiera w sobie definicje wszystkich możliwych do przesyłania między stronami połączenia struktur danych reprezentujących komunikaty.

* NFSServer - moduł realizujący program serwera. Przyjmuje połączenia z NFSConnection i dla każdego tworzy NFSServerWorker. Realizuje zestawienie połączenia z NFSConnection za pomocą modułu NFSCommunication.

* NFSServerWorker - moduł realizujący połączenie z NFSConnection. Pracuje w kontekście jednego systemu plików. Realizuje logikę komunikacji z NFSConnection za pomocą modułu NFSCommunication.

* Aplikacja kliencka - programy pokazujące działanie biblioteki z wykorzystaniem NFSConnection.

## Szczegóły implementacji i używane biblioteki
Język implementacji: __C++17__  
Kompilator: __clang 13__  
Narzędzie budowania: __CMake__  
Formater kodu: __clang-format__ - format własny

Użyte biblioteki:  
- systemowa biblioteka sockets do połączeń TCP  
- libcrypt oraz linuxowe shadow.h do weryfikacji hasła użytkownika klienta  

## Zawartość repozytorium
Projekt składa się dwóch części: biblioteki implementującej protokół i serwer, oraz przykładowych programów w tym aplikacji klienckiej.

Część pierwsza znajduje się w folderach [include](./include) oraz [src](./src), zawierają one odpowiednio pliki nagłówkowe oraz pliki źródłowe.

Druga część projektu znajduje się w folderze [examples](./examples). Znajduje się tam kilka prostych przykładów użycia biblioteki, przykładowa aplikacja kliencka oraz testy akceptacyjne.

## Budowanie projektu
W celu zbudowania biblioteki należy wywołać w terminalu:
```
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build .
cd ..
```
Biblioteka zostanie zbudowana w folderze **lib** jako biblioteka współdzielona *.so*.

W celu zbudowania przykładów należy ustawić flagę **BUILD_EXAMPLES** w **CMakeLists.txt** na wartość **True**.
```
set(BUILD_EXAMPLES True)
```
Następnie wywołać w terminalu:
```
cmake --build ./cmake-build
```
Programy wykonywalne zostną zbudowane w folderze **bin**.


## Opis interfejsu użytkownika aplikacji klienckiej
### Informacje ogólne
<p align="justify">
Aplikacja kliencka jest prostą aplikacją konsolową, która pozwala na wywoływanie poleceń udostępnianych przez protokół. Interfejs aplikacji został podzielony na dwa widoki. Pierwszy pozwala na zestawienie połączenia z serwerem (Menu do zestawiania połaczenia). Drugi widok służy do wykonywania operacji plikowych poprzez wywoływanie poleceń (Menu główne). Obydwa widoki wyświetlają listę dostępnych komend wraz z ich argumentami, a następnie czekają na polecenie od użytkownika.
</p>

#### Format poleceń wpisywanych przez użytkownika
<p align="justify">
Jako identyfikator polecenia użytkownik może podać numer lub nazwę wybranej komendy z wyświetlonej listy.
</p>

```
identyfikator_polecenia  argument_1  argument_2  ...  argument_n
```

### Opis dostępnych widoków aplikacji
#### Menu do zestawiania połączenia
<p align="justify">
Widok ten udostępnia użytkownikowi jedynie dwa polecenia:
</p>

- `connect <hostName> <username> <password> <filesystemName>` - Polecenia pozwala na połączenie się do systemu plików o nazwie "filesystemName" znadującego się na serwerze o adresie "hostname" jako użykownik o nazwie "username" i haśle "password"
- `exit ` - Kończy działanie programu

![Connect menu](./reports/figures/user_app_1.png "Menu nawiązywania połączenia")

<p align="justify">
Po nawiązaniu poprawnego połączenia z serwerem aplikacja przenosi użytkownika do menu głównego.
</p>

#### Menu główne
<p align="justify">
Menu główne pozwala użytkownikowi na wywołanie poniższych poleceń: 
</p>

- `open <path> <oflag> <mode>` - pozwala na otwarcie pliku/folderu znajdującego się na serwerze we wskazywanym przez ścieżkę (path) miejscu. Przy wywołaniu użytkownik musi również określić wartości liczbowe flagi (oflag) oraz trybu (mode). Polecenie tworzy deskryptor pliku, który jest wykorzystywany przez inne polecenia.
- `download file <file> <target>` - pozwala na skopiowanie i pobranie pliku (file) z serwera, a następnie zapisanie go we wskazanym miejscu (target).
- `send file <file> <target>` - pozwala na przesłanie na serwer pliku (file), a następnie zapisanie go we wskazanym miejscu (target) na serwerze.
- `off_t lseek(int fd, off_t offset, int whence)` - służy do ustawienia wartości offset dla pliku przypisanego do aktualnego deskryptora pliku.
- `int fstat(int fd, struct stat *statbuf)` - pozwala na wyświetlenie statystyk aktualnego deskryptora pliku.
- `unlink <path>` - służy do usuwania wskazanego przez ścieżkę (path) pliku z serwera. 
- `flock <operation>` - pozwala na wywołanie systemowej funkcji flock dla wybranej operacji (operation).
- `close` - służy do zamknięcia aktualnego deskryptora pliku.
- `exit ` - Kończy działanie programu.

![Main menu](./reports/figures/user_app_2.png "Menu główne")

## Serwer
### Implementacja
<p align="justify">
Uruchomiony serwer w pierwszej kolejności weryfikuje czy został uruchomiony z prawami root'a. Jeśli nie, kończy pracę z błędem. Następnie serwer wczytuje plik konfiguracyjny, zgodnie z opisem poniżej. Po wczytaniu konfiguracji serwer zaczyna nasłuchiwać na przychodzące połączenia TCP na ustawionym porcie, i dla każdego połączenia tworzy oddzielny proces obsługujący połączenie z klientem.
Proces obsługujący połączenie autoryzuje klienta i rozpoczyna proces realizacji rządań klienta. Przechowuje deskryptory otwartych plików i realizuje lokanie systemowe odpowiedniki funkcji klienckich. Przed wykonaniem każdej operacji woła <b>seteuid(uid)</b> ustawiając swoje efektywne id na id użytkownika w imieniu którego wykonuje operację. Po rozłączeniu z klientem zamyka wszystkie pliki pozostawione jako otwarte przez klienta.

Serwer korzysta z modułu [logger](./include/Logging.hpp), który na wyjście <b>stderr</b> wypisuje informacje o wykonywanych operacjach i błędach wraz ze znacznikiem czasowym. Taka implementacja loggera pozwala na proste przekierowanie jego wyników np. do pliku zgodnie z upodobaniem adnimistratora serwera.
</p>

### Kofiguracja
<p align="justify">
Serwer do działania potrzebuje pliku konfiguracyjnego, definiującego udostępniane systemy plików. Podczas uruchamiania serwera można opcjonalnie podać ścieżkę do pliku z konfiguracją. W przeciwnym razie będzie on szukał pliku konfiguracyjnego pod ścieżką <b>/etc/tinnfs.conf</b>.
</p>

W pliku konfiguracyjnym mogą znajdować się dwa rodzaje ustawień: 
- `port <port>` - ustawienie portu na jakim serwer nasłuchuje na połączenia. Podanie portu w pliku konfiguracyjnym jest opcjonalne. Jeśli nie zostanie on podany, serwer użyje domyślnego portu - 46879.
- `filesystem <name> <path>` - definicja systemu plików. Obowiązkowe jest podanie w pliku konfiguracyjnym przynajmniej jednego systemu plików.

Przykładowy plik konfiguracyjny: 
```
port 12345
filesystem share /opt/nfsshare
filesystem root /
```

## Kluczowe rozwiązania
<p align="justify">
Protokół zaprojektowaliśmy tak, by możliwie wiele funkcji było wykonywanych przez system na którym pracuje serwer, w większości opakowuje on funkcje systemowe tak by były one poprawnie przesłane przez sieć.  
</p>

<p align="justify">
Klasa NFSConnection w wygodny sposób dokonuje abstrakcji sesji protokołu, dając dostęp do niezależnego logowania, wykonywania operacji i pobierania informacji o błędach. Pozwala to na korzystanie z wielu oddzielnych sesji w ramach jednego pragramu korzystającego z naszej bilioteki.
</p>

<p align="justify">
Serwer w celu autoryzacji i egezkwowania poziomów dostępu korzysta z mechanizmu użytkowników systemowych, wymaga to by był on uruchomiony z prawami root'a. Funkcje systemowe na plikach są wykonywane z prawami użytkownika którego reprezentuje klient, za pomocą funkcji systemowej <b>seteuid()</b>, co sprawia, że poziom dostępu jest silnie wspierany przez mechanizmy systemowe.
</p>

### Autoryzacja z wykorzystaniem systemowego mechanizmu użytkowników
Wykorzystanie tego sposobu autoryzacji wpisuje się w nasze podejście by skorzystać z możliwie wielu sprawdzonych mechanizmów, co do których działania nie mamy żadnych wątpliwości. Użytkownicy systemowi pozwalają na:
- Proste zarządzanie użytkownikami i ich prawami dostępu przez administrację serwera
- Zapewnienie poprawnej autoryzacji użytkoników
- Natywne wsparcie systemowych poziomów dostępów, w tym grup użytkowników
- Bezpieczne przechowywanie danych autoryzujących użytkowników

### Protokół jako *"sieciowy wrapper"* funkcji systemowych
Ograniczenie logiki protokołu do minimum pozwoliło na wytworzenie lekkiego rozwiązania. Interfejs funkcji, ich zachowanie i wartości zwracane są identyczne ze standardową implementacją w systemach Linux, co ułatwia korzystanie z naszej biblioteki. Sprawia to też, że zachowanie protokołu jest nieskomplikowane i przewidywalne, a on sam powinnien działać stabilnie.

## Analiza zagrożeń
W proponowanym protokole i implementacji dostrzegamy cztery główne zagrożenia:

### Brak szyfrowanych połączeń
<p align="justify">
Wszystkie wiadomości przekazywane w protokole nie są w żaden sposób szyfrowane, co sprawia, że są podatne na ataki typu man in the middle. Jest to poważna luka, jednakże implementacja poprawnego szyfrowania wbudowanego w protokół nie jest prosta i wykracza znacznie poza zakres projektu. Można ją wyeliminować wykorzystując mechanizm IPsec, lub dostarczając zewnętrzne szyfrowanie, tj. dokonać tunelowania naszego protokołu w ramach zaszyfrowanego połączenia realizowanego przez inne narzędzie. Oba proponowane rozwiązania eliminują problem i nie wpływają na działanie naszego protokołu.
</p>

### Model zaufania klient-serwer
Protokół zakłada bardzo prosty, wręcz naiwny model zaufania.  

__Po stronie serwera__:
 * każdy klient, który zna jego adres jest zaufany i protokół może działać 
 * późniejsza autoryzacja mówi jedynie serwerowi czy klient faktycznie ma dostęp do żądanych zasobów
 * nie da się stwierdzić, czy maszyna z której komunikuje się klient jest zaufana, czy może jest to osoba niepowołana, która weszła w posiadanie danych uwierzytelniających

__Po stronie klienta__:
 * brak możliwości weryfikacji czy serwer do którego się łączymy jest tym do którego chcemy się połaczyć, a nie jest innym serwerem podszywającym się pod niego.

Powyższe problemy są możliwe do wyeliminowania z użyciem zewnętrznego mechanizmu. Dostarczenie rozwiązań proponowanych w [punkcie wyżej](#Brak-szyfrowanych-połączeń) pozwoli na skorzystanie z wielu mechanizmów uwierzytelniania obu stron.

### Serwer uruchomiony z prawami root'a
<p align="justify">
Serwer, aby mógł wykonywać operację w imieniu dowolnego użytkownika systemu na którym się znajduje, wymaga bycia uruchomionym z prawami root'a. Wszystkie operacje są jednak wykonywane jako użytkownik, który je wywołuje poprzez użycie mechanizmu <b>seteuid</b>. Zakładając więc brak błędów w programie pozwalających pominąć wywołanie <b>seteuid</b> oraz poprawne i bezpieczne działanie mechanizmu autoryzacji, nie wprowadza to do systemu żadnych zagrożeń poza tym, na co pozwalają uprawnienia danego użytkownika systemowego.
</p>

### Konieczność poprawnej konfiguracji systemu
<p align="justify">
Protokół nie gwarantuje bezpośrednio ograniczenia dostępu klientów jedynie do wydzielonego folderu, w którym serwer realizuje zdalny system plików. By ograniczyć dostęp konieczna jest odpowiednia konfiguracja systemu na którym uruchomiony jest serwer, a konkretnie przyznanie użytkownikom dostępu jedynie do folderów
reprezentujących zdalne systemy plików. W połączeniu z implementacją serwera opartą o mechanizm <b>seteuid</b> daje to gwarancję, że klient nie wyjdzie poza dostępny mu obszar.
</p>

## Testowanie
<p align="justify">
Poprawność działania systemu próbujemy weryfikować testem akceptacyjnym. Test polega na uruchomieniu serwera w określonym stanie startowym z dostępem użytkownika testowego. Następnie uruchamiane są klienty testowe, które wykonują na serwerze określone operacje i sprawdzają, czy efekt ich działania jest taki jak spodziewany. Jeżeli nie zostaną wykryte odstępstwa zakładamy, że implementacja działa poprawnie.
</p>

### Etapy testu:
- tworzenie losowego pliku, zapisanie go na serwerze, odczytanie go z serwera i porównanie ze stworzonym
- wykonanie lseek na koniec zapisanego pliku, sprawdzenie czy przesunął się o rozmiar stworzonego pliku
- pobranie fstat stworzonego pliku z serwera i porównanie z wynikiem fstat wykonanego lokalnie na tym samym pliku
- otwarcie testowego folderu w celu pobrania fstat, porównanie z wynikiem fstat wykonanego lokalnie na tym samym folderze
- unlink na stworzonym losowym pliku i lokalne sprawdzenie czy został usunięty
- otwarcie pliku przez dwóch klientów jednocześnie i sprawdzenie, że flock pozwala na zsynchronizowanie zapisów tak, aby zapis drugiego klienta wykonał się po długim, sztucznie opóźnionym zapisie pierwszego

### Wyniki testu:
<p align="justify">
Test pozwolił nam na znalezienie błędu w implementacji unlink po stronie serwera, który wysyłał w odpowiedzi komunikat z kodem odpowiedzi na inną operację.
Po wyeliminowaniu powższego błędu, test zakończył się z wynikiem pozytywnym.
</p>
