/**
*	\brief Biblioteka do obsługi czujnika RC522
* \author Maciej Flaum Tomasz Koczwara
*
*
*/
#ifndef _RFID_REGISTERS_H_
#define _RFID_REGISTERS_H_
/* KOMENDY STERUJĄCE KARTAMI MIFARE */
# define PICC_REQIDL          0x26              // find the antenna area does not enter hibernation */
# define PICC_REQALL          0x52               // find all the cards antenna area */
# define PICC_ANTICOLL        0x93              //  anti-collision*/
# define PICC_SElECTTAG       0x93               //  election card*/
# define PICC_AUTHENT1A       0x60               // authentication key A*/
# define PICC_AUTHENT1B       0x61               // authentication key B*/
# define PICC_READ            0x30               //  Read Block*/
# define PICC_WRITE           0xA0               //  write block*/
# define PICC_DECREMENT       0xC0               //debit*/
# define PICC_INCREMENT       0xC1               // recharge*/
# define PICC_RESTORE         0xC2               // transfer block data to the buffer*/
# define PICC_TRANSFER        0xB0               // save the data in the buffer*/
# define PICC_HALT            0x50               //  Sleep*/

/* KOMENDY STERUJĄCE RC522 */
#define Idle_CMD 				0x00
#define Mem_CMD					0x01
#define GenerateRandomId_CMD	0x02
#define CalcCRC_CMD				0x03
#define Transmit_CMD			0x04
#define NoCmdChange_CMD			0x07
#define Receive_CMD				0x08
#define Transceive_CMD			0x0C
#define Reserved_CMD			0x0D
#define MFAuthent_CMD			0x0E
#define SoftReset_CMD			0x0F

/* MAPA REJESTRÓW RC522 */
// Command and Status
#define Page0_Reserved_1 	0x00
#define CommandReg				0x01
#define ComIEnReg					0x02
#define DivIEnReg					0x03
#define ComIrqReg					0x04
#define DivIrqReg					0x05
#define ErrorReg					0x06
#define Status1Reg				0x07
#define Status2Reg				0x08
#define FIFODataReg				0x09
#define FIFOLevelReg			0x0A
#define WaterLevelReg			0x0B
#define ControlReg				0x0C
#define BitFramingReg			0x0D
#define CollReg						0x0E
#define Page0_Reserved_2	0x0F

// Command
#define Page1_Reserved_1	0x10
#define ModeReg						0x11
#define TxModeReg					0x12
#define RxModeReg					0x13
#define TxControlReg			0x14
#define TxASKReg					0x15
#define TxSelReg					0x16
#define RxSelReg					0x17
#define RxThresholdReg		0x18
#define	DemodReg					0x19
#define Page1_Reserved_2	0x1A
#define Page1_Reserved_3	0x1B
#define MfTxReg						0x1C
#define MfRxReg						0x1D
#define Page1_Reserved_4	0x1E
#define SerialSpeedReg		0x1F

// CFG
#define Page2_Reserved_1	0x20
#define CRCResultReg_1		0x21
#define CRCResultReg_2		0x22
#define Page2_Reserved_2	0x23
#define ModWidthReg				0x24
#define Page2_Reserved_3	0x25
#define RFCfgReg					0x26
#define GsNReg						0x27
#define CWGsPReg					0x28
#define ModGsPReg					0x29
#define TModeReg					0x2A
#define TPrescalerReg			0x2B
#define TReloadReg_1			0x2C
#define TReloadReg_2			0x2D
#define TCounterValReg_1	0x2E
#define TCounterValReg_2 	0x2F

// TestRegister
#define Page3_Reserved_1 	0x30
#define TestSel1Reg				0x31
#define TestSel2Reg				0x32
#define TestPinEnReg			0x33
#define TestPinValueReg		0x34
#define TestBusReg				0x35
#define AutoTestReg				0x36
#define VersionReg				0x37
#define AnalogTestReg			0x38
#define TestDAC1Reg				0x39
#define TestDAC2Reg				0x3A
#define TestADCReg				0x3B
#define Page3_Reserved_2 	0x3C
#define Page3_Reserved_3	0x3D
#define Page3_Reserved_4	0x3E
#define Page3_Reserved_5	0x3F

#endif
