
#ifndef INC_W5500_H_
#define INC_W5500_H_

/***	DEFINES	***************************************************************************************************************************************************************************************/
/*****************************************************************************************************************************************************************************************************/
#define LED_Green_ON HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET); 																//	LED_Green
#define LED_Green_OFF HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
#define LED_Orange_ON HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET); 															//	LED_Orange
#define LED_Orange_OFF HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
#define LED_Blue_ON HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET); 																	//	LED_Red
#define LED_Blue_OFF HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
#define LED_Red_ON HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); 																	//	LED_Red
#define LED_Red_OFF HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

#define LINK 0x01

#define be16toword(a) ((((a)>>8)&0xff)|(((a)<<8)&0xff00))
#define W5500_IRQ_MASK 0x80
#define W5500_IRQMARK_ISSET()       (w5500_irq_flag & W5500_IRQ_MASK)

#define Reset_Pin GPIO_PIN_1
#define Reset_GPIO_Port GPIOB

#define CS_GPIO_PORT GPIOB
#define CS_PIN GPIO_PIN_6
#define SS_SELECT() HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_RESET)
#define SS_DESELECT() HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_SET)

#define IP_ADDR {192,168,0,197}
#define IP_GATE {192,168,0,1}
#define IP_MASK {255,255,255,0}
#define LOCAL_PORT 80

#define MAC_ADDR {0x00,0x15,0x42,0xBF,0xF0,0x51}

#define BSB_COMMON 0x00
#define W5500_IRQ_MASK              0x80

#define COMMON_REG 0x00
#define SOCK_CMD_REG_MEM 0x01
#define SOCK_TX_BUF_MEM 0x02
#define SOCK_RX_BUF_MEM 0x03
#define SOCK_0 0x00
#define SOCK_1 0x01
#define SOCK_2 0x02
#define SOCK_3 0x03
#define SOCK_4 0x04
#define SOCK_5 0x05
#define SOCK_6 0x06
#define SOCK_7 0x07

#define BSB_S0 0x01
#define BSB_S0_TX 0x02
#define BSB_S0_RX 0x03

#define RWB_WRITE 1
#define RWB_READ 0

#define OM_FDM0 0x00		// WDM (variable length data mode)
#define OM_FDM1 0x01		// 1 byte length data mode
#define OM_FDM2 0x02		// 2 byte length data mode
#define OM_FDM3 0x03		// 4 byte length data mode

#define MR 0x0000//Mode Register

#define SHAR0 0x0009//Source Hardware Address Register MSB
#define SHAR1 0x000A
#define SHAR2 0x000B
#define SHAR3 0x000C
#define SHAR4 0x000D
#define SHAR5 0x000E// LSB
#define GWR0 0x0001//Gateway IP Address Register MSB
#define GWR1 0x0002
#define GWR2 0x0003
#define GWR3 0x0004// LSB
#define SUBR0 0x0005//Subnet Mask Register MSB
#define SUBR1 0x0006
#define SUBR2 0x0007
#define SUBR3 0x0008// LSB
#define SIPR0 0x000F//Source IP Address Register MSB
#define SIPR1 0x0010
#define SIPR2 0x0011
#define SIPR3 0x0012// LSB

#define PHYCFGR 0x002E

#define Sn_PORT0 0x0004 // Socket 0 Source Port Register MSB
#define Sn_PORT1 0x0005 // Socket 0 Source Port Register LSB

#define Sn_MR 0x0000 // Socket 0 Mode Register
#define Sn_CR 0x0001 // Socket 0 Command Register
#define Sn_SR 0x0003 // Socket 0 Status Register

#define Sn_CR       0x0001
    #define OPEN        0x01
    #define LISTEN      0x02
    #define CONNECT     0x04
    #define DISCON      0x08
    #define CLOSE       0x10
    #define SEND        0x20
    #define SEND_MAC    0x21
    #define SEND_KEEP   0x22
    #define RECV        0x40

#define Sn_SR       0x0003
    #define SOCK_CLOSED     0x00
    #define SOCK_INIT       0x13
    #define SOCK_LISTEN     0x14
    #define SOCK_ESTABLISHED    0x17
    #define SOCK_CLOSE_WAIT     0x1c
    #define SOCK_UDP        0x22
    #define SOCK_MACRAW     0x02

    #define SOCK_SYNSEND    0x15
    #define SOCK_SYNRECV    0x16
    #define SOCK_FIN_WAI    0x18
    #define SOCK_CLOSING    0x1a
    #define SOCK_TIME_WAIT  0x1b
    #define SOCK_LAST_ACK   0x1d

#define Sn_IR       0x0002
    #define IR_SEND_OK      0x10
    #define IR_TIMEOUT      0x08
    #define IR_RECV         0x04
    #define IR_DISCON       0x02
    #define IR_CON          0x01

#define SIMR    0x0018
    #define S7_IMR      0x80
    #define S6_IMR      0x40
    #define S5_IMR      0x20
    #define S4_IMR      0x10
    #define S3_IMR      0x08
    #define S2_IMR      0x04
    #define S1_IMR      0x02
    #define S0_IMR      0x01

#define SIR     0x0017
    #define S7_INT      0x80
    #define S6_INT      0x40
    #define S5_INT      0x20
    #define S4_INT      0x10
    #define S3_INT      0x08
    #define S2_INT      0x04
    #define S1_INT      0x02
    #define S0_INT      0x01

#define W5500_SOCKET_DISCONN        0x01
#define W5500_SOCKET_CONN           0x02
#define W5500_SOCKET_SNDOK          0x04
#define W5500_SOCKET_RECVOK         0x08
#define W5500_SOCKET_TIMEOUT        0x10

#define w5500_connect()             (w5500_irq_flag & W5500_SOCKET_CONN)
#define w5500_disconnect()          (w5500_irq_flag & W5500_SOCKET_DISCONN)
#define w5500_sendok()              (w5500_irq_flag & W5500_SOCKET_SNDOK)
#define w5500_recvok()              (w5500_irq_flag & W5500_SOCKET_RECVOK)
#define w5500_timeout()             (w5500_irq_flag & W5500_SOCKET_TIMEOUT)

//Socket mode
#define Mode_CLOSED 0x00
#define Mode_TCP 0x01
#define Mode_UDP 0x02
#define Mode_MACRAV 0x04

//Socket states
#define SOCK_CLOSED 0x00
#define SOCK_INIT 0x13
#define SOCK_LISTEN 0x14
#define SOCK_ESTABLISHED 0x17

#define Sn_MSSR0 0x0012
#define Sn_MSSR1 0x0013
#define Sn_TX_FSR0 0x0020
#define Sn_TX_FSR1 0x0021
#define Sn_TX_RD0 0x0022
#define Sn_TX_RD1 0x0023
#define Sn_TX_WR0 0x0024
#define Sn_TX_WR1 0x0025
#define Sn_RX_RSR0 0x0026
#define Sn_RX_RSR1 0x0027
#define Sn_RX_RD0 0x0028
#define Sn_RX_RD1 0x0029

//	Статусы передачи данных
#define DATA_COMPLETED 0 //передача данных закончена
#define DATA_ONE 1 //передаём единственный пакет
#define DATA_FIRST 2 //передаём первый пакет
#define DATA_MIDDLE 3 //передаём средний пакет
#define DATA_LAST 4 //передаём последний пакет
#define DATA_END 5 //закрываем соединение после передачи данных

#define PRT_TCP_HTTP 1

/***	TYPEDEF 	**************************************************************************************************************************************************************************************/
/*****************************************************************************************************************************************************************************************************/
typedef struct {
  volatile uint16_t addr;
  volatile uint8_t opcode;
  uint8_t data[];
} data_sect_ptr;		// Пакет переменной длины

/***	FUNCTION PROTOTYPES 	*******************************************************************************************************************************************************************/
/*****************************************************************************************************************************************************************************************************/
void W5500_ReadBuf(data_sect_ptr *datasect, uint16_t len);
void W5500_Init(void);
void W5500_GetHttpRequest(uint8_t sock_num);
void W5500_WriteSockBuf(uint8_t sock_num, uint16_t point, uint8_t *buf, uint16_t len);
void W5500_WriteReg(uint8_t op, uint16_t addres, uint8_t data);
uint8_t W5500_ReadReg(uint8_t op, uint16_t addres);
uint8_t W5500_ReadSockBufByte(uint8_t sock_num, uint16_t point);
uint8_t W5500_ReadSockBufByte(uint8_t sock_num, uint16_t point);
void W5500_ReadSockBuf(uint8_t sock_num, uint16_t point, uint8_t *buf, uint16_t len);
void W5500_IRQ_CALLBACK(uint8_t sock_num);
void W5500_Irq_Process(uint8_t sock_num);
void W5500_NOP(void);

void SocketClose(uint8_t sock);
void SocketClosedWait(uint8_t sock_num);
void SocketOpen(uint8_t sock_num, uint16_t mode);
void SocketInitWait(uint8_t sock_num);
uint8_t SocketListen(uint8_t sock_num);
void SocketListenWait(uint8_t sock_num);
void SocketSetWritePointer(uint8_t sock_num, uint16_t point);
void SocketResv(uint8_t sock_num);
void SocketSend(uint8_t sock_num);

uint8_t SocketGetStatus(uint8_t sock_num);
uint16_t SocketGetReadPointer(uint8_t sock_num);
uint16_t SocketGetWritePointer(uint8_t sock_num);
uint16_t SocketGetSizeRX(uint8_t sock_num);
uint16_t SocketGetReadPointer(uint8_t sock_num);
void SocketReset(uint8_t sock);

#endif /* INC_W5500_H_ */
