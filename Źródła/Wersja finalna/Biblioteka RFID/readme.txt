Opis biblioteki RFID - moduł RC522

1. Przygotowanie modułu:
Moduł trzeba podpiąć odpowiednio do procesora. Biblioteka wykorzystuje interfejs SPI0 zatem jesteśmy ograniczeni do pinów procesora które są do niego podłączone.
Pin reset może zostać podłączony do dowolnego wolnego pinu procesora.
Piny do których podpinamy moduł możemy zmienić w pliku RFID.h. Domyślnie są to:
MISO PA17
MOSI PA16
SCK  PA15
CS   PA14
RESET PB9

Należy pamiętać iż moduł jest zasilany z napięcia 3.3[V]. Podanie napięcia wyższego może spowodować uszkodzenie czytnika.

2. Inicjalizacja biblioteki 
Na początku musimy dodać do naszego projektu następujące pliki:
RFID.c
RFID.h
SPI.c
SPI.h,
oraz zainkludować w pliku main.c plik nagłówkowy RFID.h.

W funkcji main() przed pętlą nieskończoną ( while(1) ) wywołujemy funkcję RFID_init().
Konfiguruje ona interfejs SPI do pracy z modułem oraz dokonuje inicjalizacji układu scalonego czytnika kart.

3. Praca z biblioteką
W chwili obecnej jedyną formą komunikacji z kartami zbliżeniowymi MIFARE jest odczyt numeru seryjnego karty przyłożonej do czytnika.
Numer ten składa się z 5 bajtów. Aby więc przechowywać numer potrzebujemy tablicę piecio elementową:
uint8_t odczytany_numer[5].

4. Przykłady pracy z niektórymi funkcjami bibliotecznymi 

uint8_t RFID_get_serial_number(uint8_t *serial_number)

pseudokod C:
uint8_t odczytany_numer[5]; // tablica w której zapiszemy odczytany numer
if(RFID_get_serial_number(odczytany_numer)) { /* tutaj możemy np wyświetlić odczytany numer */ }
