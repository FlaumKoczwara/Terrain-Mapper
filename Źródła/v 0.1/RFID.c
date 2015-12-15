


#include "MKL46Z4.h"
#include "SPI.h"
#include "RFID.h"
#include "UART.h"
#include "RFID_registers.h"

/**
 *		\brief Tablica struktur.
 *	Tablica struktur tworząca bazę danych kart RFID.
 */
T_card MIFARE_CARD_DATABASE[CARD_ARRAY_SIZE];  

/**
 *		\brief Funkcja inicjalizująca piny GPIO, interfejs SPI oraz moduł RC522.
 *
 *		\warning Musi zostać wywołana przed pętlą nieskończoną w funkcji main()
 */
void RFID_init(void)
{
	uint8_t temp=0;
	// właczenie zegara do portów 
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK
			|  SIM_SCGC5_PORTB_MASK
			|  SIM_SCGC5_PORTC_MASK
			|  SIM_SCGC5_PORTD_MASK
			|  SIM_SCGC5_PORTE_MASK;
	SPI_init();		// inicjalizacja SPI

	/* USTAWIENIE PORTU ODPOWIEDZIALNEGO ZA RESET SPRZĘTOWY UKŁADU MFRC522 */
	RESET_PORTx->PCR[RESET_NUMBER] |= PORT_PCR_MUX(1);
	RESET_PTx->PDDR |= (1<<RESET_NUMBER);
	RESET_PTx->PSOR |= (1<<RESET_NUMBER);		

	RC522_write(CommandReg,SoftReset_CMD);		// reset programowy, przywraca domyślne wartości wszystkim rejestrom

	RC522_write(TModeReg, 0x8D);					// TModeReg -> konfiguruje zewnętrzny timer => timer startuje od razu po zakończeniu transmisji, 0xD00 -> tak ustawione są 4 starsze bity preskalera
	RC522_write(TPrescalerReg, 0x3E);			// definiuje ustawienie 8 młodszych bitów preskalera 0x03E
	RC522_write(TReloadReg_1, 30);				// wartość rejestru przeładowania starszy bajt
	RC522_write(TReloadReg_2, 0);					// wartość rejstru przeładowania młodszy bajt
	RC522_write(TxASKReg, 0x40);					// rejestr kontroluje ustawienia modulacji transmisji => 100% używamy modulacji ASK niezależnie od ustawień rejestru ModGsPReg
	RC522_write(ModeReg, 0x3D);	          // rejestr od generalnych ustawień transmisji => tramiter stratuje tylko jest pole elektryczne,MFIN jest aktywny w stanie wysokim,usatwienie wartości startowej dla CRC


	// sprawdzenie włączenia anteny jeśli jest wyłączona to ją właczamy
	temp = RC522_read(TxControlReg);
	if(!(temp&0x03))
	{
		RC522_write(TxControlReg,temp|0x03);
	}
}

/**
 *		\brief Funkcja odczytuje jednen bajt z rejestru RC522.
 *
 *   \param uint8_t reg  - adres rejestru z którego odczytujemy dane
 * 	\returns Dane odczytane z RC522 
 */
uint8_t RC522_read(uint8_t reg)
{
	uint8_t receive_data=0;
	SS_LOW;
	SPI_transmit(0x80 | ((reg<<1)&0x7E) );
	receive_data=SPI_transmit(0x00);
	SS_HIGH;
	return receive_data;	
}

/**
 *		\brief Funkcja zapisuje jednen bajt do danego rejestru RC522.
 *
 *   \param uint8_t reg  - adres rejestru do którego zapisujemy dane 
 *   \param uint8_t data - dane do zapisu 
 * 	\returns Dane odczytane z RC522 
 */
void RC522_write(uint8_t reg,uint8_t data)
{
	SS_LOW;
	SPI_transmit( (reg<<1)&0x7E );
	SPI_transmit(data);
	SS_HIGH;
}


/**
 *	\brief Funkcja inicjuje komunikacje z kartą znajdującą się w zasięgu anteny czytnika.
 *
 * \param uint8_t *tag_type - wskaźnik do tablicy do której ma zostać napisany typ karty którą odczytujemy
 *	\return Zwraca status powodzenia komunikacji 
 *		
 * Funkcja wysyła rządanie nawiązania kounikacji do kart które znajdują się w polu magnetycznym czytnika. 
 * Karta odsyła nam jako informację zwrotną jakim jest typem
 *  
 */
uint8_t	RFID_request(uint8_t *tag_type)
{
	uint8_t  status=0;		// zmienna pomocnicza 
	uint32_t backBits=0;	// liczba otrzymanych z karty danych 

	RC522_write(BitFramingReg, 0x07); //TxLastBists = BitFramingReg[2..0], bit framing pokazuje jakiej długości mamy ramkę. Komenda reqall jest 7 bitowa zatem ustawiamy 0x07

	tag_type[0] = PICC_REQALL;						// przekazujemy do wysłania komendę typu request 
	status = RC522_to_card(Transceive_CMD, tag_type, 1, tag_type, &backBits);
	// Transcive CMD to komenda RC522 mówiąca że ma przesłać zawartość bufora FIFO
	if ((status != CARD_FOUND) || (backBits != 0x10))  // odpowiedź na zapytanie REQ ma 16bitów długości (0x10) zatem jeśli otrzymaliśmy inną długość ( w FIFO RC522) znalazło się więcej bajtów to znaczy że wystąpił błąd i nie oczytaliśmy poprawnie danych
	{
		status = ERROR;
	}

	return status;
}

/**
 * \brief Funkcja realizująca komunikację kartą MIFARE poprzez RC522.
 *
 *	\param uint8_t cmd - komenda jaką ma zrealizować RC522
 *	\param uint8_t *send_data - wskaźnik do danych które chcemy wysłać do karty (mogą to być komendy obsługiwane przez kartę np PIC_REQALL
 *  \param uint8_t send_data_lenght - ile danych wysyłamy do karty
 *  \param uint8_t *back_data - wskaźnik do tablicy/zmiennej w której zapisujemy dane odebrane z karty (UID, zapisane wcześniej dane)
 *  \param uint32_t *back_data_lenght - ilość danych jaka znajduje się w buforze FIFO, i którą musimy odebrać
 *	\returns Zwraca status powodzenia wykonanej operacji 
 *
 * Realizuje podstawową komunikację z kartą typu MIFARE. Możliwe jest wysyłanie komend i danych do karty RFID. 
 * Funkcja umożliwia również odbiór danych z karty.
 */
uint8_t RC522_to_card(uint8_t cmd, uint8_t *send_data, uint8_t send_data_len, uint8_t *back_data, uint32_t *back_data_len)
{
	uint8_t status = ERROR;					// zmienna statusowa, jej zawartość jest zwracana po zakończeniu funckji 
	uint8_t irqEn = 0x00;						// póki co nie wiem co to wgl jest :(
	uint8_t waitIRq = 0x00;         // póki co nie wiem co to wgl jest :(
	uint8_t lastBits;
	uint8_t n;
	uint8_t	tmp;
	uint32_t i;

	switch (cmd)
	{
	case MFAuthent_CMD:		//Certification cards close
		irqEn = 0x12;
		waitIRq = 0x10;   
		break;

	case Transceive_CMD:	  //Transmit FIFO data
		irqEn = 0x77;
		waitIRq = 0x30;
		break;

	default:
		break;
	}

	/* PRZYGOTWANIE RC522 na komunikacja z kartą zbliżeniową */
	n=RC522_read(ComIrqReg);             // odczytuje stan wymuszeń przerwań
	RC522_write(ComIrqReg,n&(~0x80));    // czyścimy wszystkie przerwania 
	n=RC522_read(FIFOLevelReg);					 // odczytujemy poziom zapełnienia rejestru FIFO 
	RC522_write(FIFOLevelReg,n|0x80);    // kasujemy rejestr FIFO
	RC522_write(CommandReg, Idle_CMD);   // NO action; przerywa aktualne operacje przetwarzane przez RC522

	// Kopiujemy do rejestru FIFO RC522 dane do wysyłki ( mogą to też być komendy sterujące kartą )
	for (i=0; i<send_data_len; i++)
	{
		RC522_write(FIFODataReg, send_data[i]);
	}

	//Execute the cmd
	RC522_write(CommandReg, cmd);
	if (cmd == Transceive_CMD)
	{
		n=RC522_read(BitFramingReg);
		RC522_write(BitFramingReg,n|0x80);
	}

	// oczekiwanie na zakończenie wysyłania danych
	// gdyby się nie udało odczytać karty i timer wbudowany w RC522 nie zakończył transmisji
	// jest to dodatkowa pętla oczekująca, gdy zawiodą inne zabezpieczenia
	// pętla zakończy działanie prgoramu i wyświetli błąd
	// zegar zrywający transmisję przekręca się co 25ms. Startuje on zaraz gdy PCD zakończy transmisję.
	i = 2000;	
	do
	{
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = RC522_read(ComIrqReg);			// czytamy zawartość rejestru przerwań
		i--;														// teraz n trzyma wartość rejestru przerwań po zakończeniu transmisji, nie wolno nadpisać tej zmiennej!!!
	}
	while ((i!=0) && !(n&0x01) && !(n&waitIRq));  
	// przerywamy pętlę gdy:
	// a) doliczyliśmy do 0      => równoznaczne z błędem transmisji 
	// b) timer doliczył do zera => równoznaczne z timeout'em transmisji
	// c) zawartość rejestru pokrywa się z oczekiwaniami => transmija mogła się zakończyć dobrze , sprawdzamy czy tak się stało 


	tmp=RC522_read(BitFramingReg);
	RC522_write(BitFramingReg,tmp&(~0x80));  // wyłączamy zezwolenie na transmisję 

	/* SPRAWDZAMY STAN PO ZAKOŃCZENIU TRANSMISJI */
	if (i != 0)		// jeśli trasnmisja zakończyła się przed obiegiem pętli
	{
		// sprawdzamy czy w rejestrze pojawiły się jakieś błędy
		if(!(RC522_read(ErrorReg) & 0x1B))	//BufferOvfl Collerr CRCErr ProtecolErr
		{

			status = CARD_FOUND;   	// jeśli nie ma błędów to karta znaleziona poprawnie
			if (n & irqEn & 0x01)   // jeśli po zakończeniu transmisji wiemy że timer doliczył do końca to karta nie została znaleziona, zwracamy błąd
			{
				status = CARD_NOT_FOUND;			//??
			}

			if (cmd == Transceive_CMD)  // jeśli przesyłaliśmy dane 
			{
				n = RC522_read(FIFOLevelReg);							// przypisujemy zmiennej n ilość danych odebranych z karty 
				lastBits = RC522_read(ControlReg) & 0x07; // sprawdzamy ile bitów w ostatnim odebranym bajcie jest do odczytania przez nas 
				if (lastBits)
				{
					*back_data_len = (n-1)*8 + lastBits;
					// mówi nam ile bitów jest poprawnych 

				}
				else
				{
					*back_data_len = n*8; // jeżeli lasBits == 0 to cały baj jest ok
					// długość danych do odczytu to: ilość bajtów*8 bitów
				}
				if (n == 0) // jeśli FIFO pokazuje nam że odczytaliśmy 0 bajtów to tak naprawdę mamy jeden bajt odbrany
				{
					n = 1;		// odczytaliśmy 1 bajt
				}
				if (n > MAX_LEN)
				{
					n = MAX_LEN;
					// możemy odczytać maksymalnie tyle bajtów jak duży mamy bufor 
				}

				// pętla odczytująca wszystkie odbrane dane 
				for (i=0; i<n; i++)
				{
					back_data[i] = RC522_read(FIFODataReg);
				}
			}
		}
		else // jeśli transmija została zakończona przez pętlę zabezpieczającą to wiemy że na pewno mamy błąd w transmisji 
		{
			status = ERROR;
		}

	}

	return status;
}

/**
 *		\brief Funkcja odczytująca unikalny numer seryjny karty.
 *
 *		\param uint8_t *serial_out - wskaźnik do tablicy o wymiarze minimum 5, w której zapisujemy wynik pomiarów
 *	  \returns Zwraca status powodzenia wykonanej operacji 
 *		UID - Unique ID ( unikalny numer seryjny )
 *		\warning Przed wywołaniem funkcji karta musi zostać aktywowana wywołaniem funkcji RFID_request(...)
 *						 
 */
uint8_t RFID_get_card_serial(uint8_t *serial_out)
{
	uint8_t status;           // trzyma aktualny status 
	uint8_t i;							  // zmienna pomocnicza
	uint8_t serNumCheck=0;		// zmienna służąca do sprawdzania poprawności odbioru serial number
	uint32_t unLen;           // zmienna pomocnicza

	RC522_write(BitFramingReg, 0x00);		//nTxLastBists = BitFramingReg[2..0]
	// ustawienie nadawania ramki danych do karty złożonej z 8 bitów

	serial_out[0] = PICC_ANTICOLL;			// ta komenda karty zwraca nam jej numer seryjny 
	serial_out[1] = 0x20;					
	status = RC522_to_card(Transceive_CMD, serial_out, 2, serial_out, &unLen);

	if (status == CARD_FOUND)
	{
		//Check card serial number
		// jeżeli po 4 krotnym xorowaniu wyjdzie nam to samo to znaczy się że odberany serial number z karty jest poprawny
		for (i=0; i<4; i++)
		{
			serNumCheck ^= serial_out[i];
		}
		if (serNumCheck != serial_out[i])
		{
			status = ERROR;
		}
	}
	return status;
}

/** 
 *	\brief Dodaje kod zapisany w karcie do bazy danych.
 *	
 *	\return Zwraca status powodzenia operacji
 *
 *	Jeśli karty nie ma w bazie danych jest ona do niej dodawana na wolnej pozycji 
 *  Jeśli karta przyłożona do czujnika jest już zapisana w bazie danych.
 *  
 *
 */
uint8_t RFID_add_card_to_database(void)
{
//	uint8_t status; 				// chwilowo nie używane
	uint8_t byte;						// zmienna pomocnicza do sprawdzania czy udało się poprawnie odczytać karte
	uint8_t i; 							// dla petli 
	uint8_t x; 							// dla petli
	uint8_t good_byte=0;	  // uzywana do sprawdzenia czy dana karta juz znajduje sie w bazie danych 
	enum st {init,search_card,check_database,add_card};		// typ wyliczeniowy do maszyny stanu
	static uint8_t temp_array[5];				// trzyma odczytany serial number z karty
	static uint8_t state;								// maszyna stanów

	switch(state)
	{
	case init:
		// wyświetlamy informacje żeby przyłożyć kartę do czytnika
		UART_send(UART2,"Czekam na karte\n\r");
		state=search_card;	
		break;
	case search_card:
		// szukamy kart w polu magnetycznym czujnika
		byte=RFID_request(temp_array);
		// gdy znaleźliźmy karte
		if(byte==CARD_FOUND)
		{
			RFID_get_card_serial(temp_array);		// odczytujemy jej numer seryjny
			state=check_database;								// w nastepnym kroku bedziemy sprawdzac karte

			// wyswietlamy informacje o znalezionej karcie 
			UART_send(UART2,"Znaleziono karte o nr seryjnym: ");		
			for(i=0;i<5;i++) UART_send_int(UART2,temp_array[i]);		
			UART_send(UART2,"\n\r");				
		}
		break;

	case check_database:
		// sprawdzamy czy dana karta znajduje sie juz w bazie danych 
		for(i=0;i<CARD_ARRAY_SIZE;i++)
		{
			for(x=0;x<5;x++)
			{

				if(temp_array[x]==MIFARE_CARD_DATABASE[i].serial_number[x]) {good_byte++;}
				// jezeli wartosc zapisana w tablicy jest taka sama jak odczytana z karty 
				// to inkrementujemy zmienna good byte
			}
			// jezeli 5 bytów jest takich samych oznacza że jest to ta sama karta zatem nie zapisujemy jej w database tylko szumay nowej karty do zapisania 
			if(good_byte==5) 
			{
				state=init;
				UART_send(UART2,"Ta karta jest juz w bazie\n\r");
				break;}		// jesli znaleziono w bazie zapisana karte w ktorej sa takie same 
		}
		if (state==check_database) state=add_card;  // jeśli karta nie znajduje się w bazie to idziemy ja zapisać ( dodać do database ) w innym przypadku szukamy nowej karty


		break;

	case add_card:
		// w tym stanie szukamy wolnej pozycji w bazie 
		for(i=0;i<CARD_ARRAY_SIZE;i++)
		{
			// gdy znajdziemy wolną strukturę
			if(MIFARE_CARD_DATABASE[i].busy_flag==FREE)
			{
				UART_send(UART2,"Zapisano na pozycji ");
				UART_send_int(UART2,i);
				UART_send(UART2,"\n\r");

				MIFARE_CARD_DATABASE[i].busy_flag=BUSY;   // ta struktura jest już zajeta
				for(x=0;x<5;x++)
				{
					// kopiujemy serial number do struktury
					MIFARE_CARD_DATABASE[i].serial_number[x]=temp_array[x];
					// czyścimy stary serial number
					temp_array[x]=0;
				}

				state=init;		// szukamy nowej karty
				break;
			}
		}
		break;
	}
	return 0;
}

/**
 *	\brief Pokazuje zawartość bazy danych.
 *
 *	\return narazie to nic nie zwraca :D
 *
 */
uint8_t RFID_show_database(void)
{
	uint8_t i,x;  // dla petli

	UART_send(UART2,"\n\r");	
	UART_send(UART2,"\n\r");	

	for(i=0;i<CARD_ARRAY_SIZE;i++)
	{
		if(MIFARE_CARD_DATABASE[i].busy_flag==FREE) 
		{
			UART_send_int(UART2,i);
			UART_send(UART2,". Slot wolny\n\r");
		}
		else 
		{
			UART_send_int(UART2,i);
			UART_send(UART2,". Numer seryjny: ");
			for(x=0;x<5;x++) { UART_send_int(UART2,MIFARE_CARD_DATABASE[i].serial_number[x]); }
			UART_send(UART2,"\n\r");			
		}				
	}	
	return 0;
}

/**
 *	\brief Usuwa z bazy danych kartę przyłożoną do czytnika
 *
 *
 *
 */
uint8_t RFID_delete_card_from_database(void)
{

	uint8_t i,x;	// dla petli 
	enum st {init,search_card,delete_card};
	uint8_t byte;
	static uint8_t temp_array[5];
	static uint8_t state;
	uint8_t good_byte;

	switch(state)
	{
	case init:
		UART_send(UART2,"Przyloz karte ktora chcesz usunac z bazy danych\n\r");
		state=search_card;	
		break;
	case search_card:
		// szukamy kart w polu magnetycznym czujnika
		byte=RFID_request(temp_array);
		// gdy znaleźliźmy karte
		if(byte==CARD_FOUND)
		{
			RFID_get_card_serial(temp_array);		// odczytujemy jej numer seryjny
			state=delete_card;								// w nastepnym kroku bedziemy sprawdzac karte

			// wyswietlamy informacje o znalezionej karcie 
			UART_send(UART2,"Znaleziono karte o nr seryjnym: ");		
			for(i=0;i<5;i++) UART_send_int(UART2,temp_array[i]);		
			UART_send(UART2,"\n\r");				
		}
		break;

	case delete_card:

		// sprawdzamy czy dana karta znajduje sie juz w bazie danych 
		for(i=0;i<CARD_ARRAY_SIZE;i++)
		{
			for(x=0;x<5;x++)
			{

				if(temp_array[x]==MIFARE_CARD_DATABASE[i].serial_number[x]) {good_byte++;}
				// jezeli wartosc zapisana w tablicy jest taka sama jak odczytana z karty 
				// to inkrementujemy zmienna good byte
			}

			// jezeli 5 bytów jest takich samych oznacza że jest to ta sama karta zatem usuwamy jej w database 
			if(good_byte==5) 
			{
				state=init;
				good_byte=0;
				UART_send(UART2,"Karta zostala usunieta! \n\r");
				// zeby usunac wypelniamy uzyte pola zerami 
				// a dana komorke ustawiamy na wartosc free
				MIFARE_CARD_DATABASE[i].busy_flag=FREE;   // ta struktura jest już wolna

				for(x=0;x<5;x++)
				{
					// kopiujemy serial number do struktury
					MIFARE_CARD_DATABASE[i].serial_number[x]=0;
					// czyścimy stary serial number
					temp_array[x]=0;
				}
				state=init;
				break;
			}		
			else {state=init;}
		}
		break;
	}
	return 0;
}

