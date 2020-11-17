/***	INCLUDE	**************************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "w5500.h"
#include "httpd.h"

/***	 VARIABLES	**********************************************************************************************************************************************************************************/
/*****************************************************************************************************************************************************************************************************/
uint8_t sect[515];
uint8_t ipaddr[4]=IP_ADDR;
uint8_t ipgate[4]=IP_GATE;
uint8_t ipmask[4]=IP_MASK;
uint16_t local_port = LOCAL_PORT;
uint8_t macaddr[6] = MAC_ADDR;
char str1[60]={0};
char tmpbuf[128];

extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi1;
extern uint8_t w5500_irq_flag;
extern uint8_t irq_prc;

uint8_t tst = 1;
uint32_t tst1 = 0;

/***	FUNCTIONS	***********************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
void W5500_Init(void)
{
	LED_Green_OFF;

  uint8_t res, opcode = {0};

  //Hard Reset
  HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
  while(1)
	{
		res = W5500_ReadReg(0x01, PHYCFGR);
		if((res & LINK) != 0)
				break;
		}

  // Soft Reset
	opcode = (BSB_COMMON<<3) | OM_FDM1;
	W5500_WriteReg(opcode, MR, 0x80);
	HAL_Delay(100);

	W5500_WriteReg(opcode, SHAR0,macaddr[0]);
	W5500_WriteReg(opcode, SHAR1,macaddr[1]);
	W5500_WriteReg(opcode, SHAR2,macaddr[2]);
	W5500_WriteReg(opcode, SHAR3,macaddr[3]);
	W5500_WriteReg(opcode, SHAR4,macaddr[4]);
	W5500_WriteReg(opcode, SHAR5,macaddr[5]);
	W5500_WriteReg(opcode, GWR0,ipgate[0]);
	W5500_WriteReg(opcode, GWR1,ipgate[1]);
	W5500_WriteReg(opcode, GWR2,ipgate[2]);
	W5500_WriteReg(opcode, GWR3,ipgate[3]);
	W5500_WriteReg(opcode, SUBR0,ipmask[0]);
	W5500_WriteReg(opcode, SUBR1,ipmask[1]);
	W5500_WriteReg(opcode, SUBR2,ipmask[2]);
	W5500_WriteReg(opcode, SUBR3,ipmask[3]);
	W5500_WriteReg(opcode, SIPR0,ipaddr[0]);
	W5500_WriteReg(opcode, SIPR1,ipaddr[1]);
	W5500_WriteReg(opcode, SIPR2,ipaddr[2]);
	W5500_WriteReg(opcode, SIPR3,ipaddr[3]);

	W5500_WriteReg(opcode, SIMR, S0_IMR);		//IRQ socket_0

	// Setting socket_0 port
	opcode = (BSB_S0<<3) | OM_FDM1;
	W5500_NOP();
	W5500_WriteReg(opcode, Sn_PORT0, local_port >> 8);
	W5500_WriteReg(opcode, Sn_PORT1, local_port);

	// Open socket
	while(SocketListen(0))
	{
		LED_Orange_ON;
	}

	LED_Orange_OFF;

	LED_Green_ON;
}

//**************************************************************************************************//
//**************************************************************************************************//
void W5500_WriteReg(uint8_t op, uint16_t addres, uint8_t data)
{
  uint8_t buf[4] = {addres >> 8, addres, op|(RWB_WRITE<<2), data};
  SS_SELECT();
  HAL_SPI_Transmit(&hspi1, buf, 4, 0xFFFFFFFF);
  SS_DESELECT();
}

//**************************************************************************************************//
//**************************************************************************************************//
void W5500_WriteBuf(data_sect_ptr *datasect, uint16_t len)
{
  SS_SELECT();
  HAL_SPI_Transmit(&hspi1, (uint8_t*) datasect, len, 0xFFFFFFFF);
  SS_DESELECT();
}

//**************************************************************************************************//
//**************************************************************************************************//
void W5500_WriteSockBuf(uint8_t sock_num, uint16_t point, uint8_t *buf, uint16_t len)
{
  data_sect_ptr *datasect = (void*)buf;
  datasect->opcode = (((sock_num<<2)|BSB_S0_TX)<<3)|(RWB_WRITE<<2)|OM_FDM0;
  datasect->addr = be16toword(point);
  W5500_WriteBuf(datasect,len+3);
}

//**************************************************//
//**************************************************//

//**************************************************************************************************//
//**************************************************************************************************//
uint8_t W5500_ReadReg(uint8_t op, uint16_t addres)
{
  uint8_t data;
  uint8_t wbuf[4] = {addres >> 8, addres, op, 0x0};
  uint8_t rbuf[4];
  SS_SELECT();
  HAL_SPI_TransmitReceive(&hspi1, wbuf, rbuf, 4, 0xFFFFFFFF);
  SS_DESELECT();
  data = rbuf[3];
  return data;
}

//**************************************************************************************************//
//**************************************************************************************************//
void W5500_ReadBuf(data_sect_ptr *datasect, uint16_t len)		//	Чтение данных переменной длины из определенного буфера (2)
{
  SS_SELECT();
  HAL_SPI_Transmit(&hspi1, (uint8_t*) datasect, 3, 0xFFFFFFFF);
  HAL_SPI_Receive(&hspi1, (uint8_t*) datasect, len, 0xFFFFFFFF);
  SS_DESELECT();
}

//**************************************************************************************************//
//**************************************************************************************************//
void W5500_ReadSockBuf(uint8_t sock_num, uint16_t point, uint8_t *buf, uint16_t len)		//	Чтение данных переменной длины из определенного буфера (1)
{
  data_sect_ptr *datasect = (void*)buf;
  datasect->opcode = (((sock_num<<2)|BSB_S0_RX)<<3)|OM_FDM0;
  datasect->addr = be16toword(point);
  W5500_ReadBuf(datasect,len);
}

//**************************************************************************************************//
//**************************************************************************************************//
uint8_t W5500_ReadSockBufByte(uint8_t sock_num, uint16_t point)		//	Чтение одного байта из определенного буфера
{
  uint8_t opcode, bt;
  opcode = (((sock_num<<2)|BSB_S0_RX)<<3)|OM_FDM1;

  W5500_NOP();
  bt = W5500_ReadReg(opcode, point);
  return bt;
}

//**************************************************//
//**************************************************//

//**************************************************************************************************//
//**************************************************************************************************//
void W5500_Irq_Process(uint8_t sock_num)
{
	uint8_t res, ind = {0};
	uint8_t opcode = (((sock_num<<2)|SOCK_CMD_REG_MEM)<<3)|OM_FDM1;

	W5500_NOP();
	res = W5500_ReadReg(0x01, SIR);

	if(res & S0_INT) {
		W5500_NOP();
		ind = W5500_ReadReg(opcode, Sn_IR);
		W5500_WriteReg(opcode, Sn_IR, ind);
		w5500_irq_flag &= W5500_IRQ_MASK;

		 if(ind & IR_CON) {
			 w5500_irq_flag |= W5500_SOCKET_CONN;
			 LED_Blue_ON;
		 }
		        //断开连接
			if(ind & IR_DISCON) {
				w5500_irq_flag |= W5500_SOCKET_DISCONN;
			}
			//发送成功
			if(ind & IR_SEND_OK) {
				w5500_irq_flag |= W5500_SOCKET_SNDOK;
			}
			//接收成功
			if(ind & IR_RECV) {
				w5500_irq_flag |= W5500_SOCKET_RECVOK;
			}
			//超时
			if(ind & IR_TIMEOUT) {
				w5500_irq_flag |= W5500_SOCKET_TIMEOUT;
			}

			W5500_IRQ_CALLBACK(sock_num);
	}
}

//**************************************************************************************************//
//**************************************************************************************************//
void W5500_IRQ_CALLBACK(uint8_t sock_num)
{
	if(w5500_connect()) {
		LED_Blue_ON;
		irq_prc = 1;
	}

	if(w5500_disconnect()) {
		LED_Red_ON;
		SocketReset(sock_num);
			irq_prc = 1;
	}

	if(w5500_sendok()) {
		// LED_Blue_ON;
		irq_prc = 1;
	}

	if(w5500_recvok()) {
//		LED_Orange_ON;
		W5500_GetHttpRequest(sock_num);
		irq_prc = 1;
	}

	if(w5500_timeout()) {
		SocketReset(sock_num);
		irq_prc = 1;
	}

}

//**************************************************************************************************//
//**************************************************************************************************//
void W5500_GetHttpRequest(uint8_t sock_num)
{
  uint16_t point = 0;

	point = SocketGetReadPointer(sock_num);
	W5500_ReadSockBuf(sock_num, point, (uint8_t*)tmpbuf, 5);
	if (strncmp(tmpbuf,"GET /", 5) == 0)
	{
		Http_Request(sock_num);
	}
}

//**************************************************************************************************//
//**************************************************************************************************//
void W5500_NOP(void)
{
	uint8_t res = 0;
	res = W5500_ReadReg(0x01, 0x0000);
	(void)res;
}

//**************************************************//
//**************************************************//

//**************************************************************************************************//
//**************************************************************************************************//
uint8_t SocketListen(uint8_t sock_num)
{
	uint8_t res = 0;
  uint8_t opcode = (((sock_num<<2)|SOCK_CMD_REG_MEM)<<3)|OM_FDM1;

  SocketOpen(sock_num, Mode_TCP);
  W5500_NOP();
  res = W5500_ReadReg(opcode, Sn_SR);
  if(res != SOCK_INIT){
  	SocketClose(sock_num);
  	return 1;
  }

  W5500_WriteReg(opcode, Sn_CR, LISTEN);
  W5500_NOP();
  res = W5500_ReadReg(opcode, Sn_SR);
  if(res != SOCK_LISTEN){
    	SocketClose(sock_num);
    	return 1;
    }

 return 0;
}

//**************************************************************************************************//
//**************************************************************************************************//
void SocketResv(uint8_t sock_num)
{
  uint8_t opcode;
  opcode = (((sock_num<<2)|BSB_S0)<<3)|OM_FDM1;

  W5500_NOP();
  W5500_WriteReg(opcode, Sn_CR, 0x40);
}

//**************************************************************************************************//
//**************************************************************************************************//
void SocketSend(uint8_t sock_num)
{
  uint8_t opcode;
  opcode = (((sock_num<<2)|BSB_S0)<<3)|OM_FDM1;

  W5500_NOP();
  W5500_WriteReg(opcode, Sn_CR, 0x20);
}

//**************************************************************************************************//
//**************************************************************************************************//
uint8_t SocketGetStatus(uint8_t sock_num)
{
  uint8_t dt;
  uint8_t opcode=0;
  opcode = (((sock_num<<2) | BSB_S0) << 3) | OM_FDM1;

  W5500_NOP();
  dt = W5500_ReadReg(opcode, Sn_SR);
  return dt;
}

//**************************************************************************************************//
//**************************************************************************************************//
uint16_t SocketGetSizeRX(uint8_t sock_num)
{
  uint16_t len;
  uint8_t opcode=0;
  opcode = (((sock_num<<2)|BSB_S0)<<3)|OM_FDM1;

  W5500_NOP();
  len = (W5500_ReadReg(opcode,Sn_RX_RSR0)<<8 | W5500_ReadReg(opcode,Sn_RX_RSR1));
  return len;
}

//**************************************************************************************************//
//**************************************************************************************************//
uint16_t SocketGetReadPointer(uint8_t sock_num)

{
  uint16_t point;
  uint8_t opcode;
  opcode = (((sock_num<<2)|BSB_S0)<<3)|OM_FDM1;

  W5500_NOP();
  point = (W5500_ReadReg(opcode,Sn_RX_RD0)<<8 | W5500_ReadReg(opcode,Sn_RX_RD1));
  return point;
}

//**************************************************************************************************//
//**************************************************************************************************//
uint16_t SocketGetWritePointer(uint8_t sock_num)
{
  uint16_t point;
  uint8_t opcode;
  opcode = (((sock_num<<2)|BSB_S0)<<3)|OM_FDM1;

  W5500_NOP();
  point = (W5500_ReadReg(opcode,Sn_TX_WR0)<<8 | W5500_ReadReg(opcode,Sn_TX_WR1));
  return point;
}

//**************************************************************************************************//
//**************************************************************************************************//
void SocketSetWritePointer(uint8_t sock_num, uint16_t point)
{
  uint8_t opcode;
  opcode = (((sock_num<<2)|BSB_S0)<<3)|OM_FDM1;

  W5500_NOP();
  W5500_WriteReg(opcode, Sn_TX_WR0, point>>8);
  W5500_WriteReg(opcode, Sn_TX_WR1, (uint8_t)point);
}

//**************************************************************************************************//
//**************************************************************************************************//
void SocketOpen(uint8_t sock_num, uint16_t mode)		// Открытие сокета
{
  uint8_t opcode=0;
  opcode = (((sock_num<<2)|BSB_S0)<<3)|OM_FDM1;

  W5500_NOP();
  W5500_WriteReg(opcode, Sn_MR, mode);
  W5500_NOP();
  W5500_WriteReg(opcode, Sn_CR, 0x01);
}

//**************************************************************************************************//
//**************************************************************************************************//
void SocketClose(uint8_t sock)
{
	uint8_t opcode = (sock<<5) | (SOCK_CMD_REG_MEM<<3) | OM_FDM1;

	W5500_NOP();
	W5500_WriteReg(opcode, Sn_CR, CLOSE);
}

//**************************************************************************************************//
//**************************************************************************************************//
void SocketReset(uint8_t sock)
{
	uint8_t opcode = (sock<<5) | (SOCK_CMD_REG_MEM<<3) | OM_FDM1;

	SocketClose(sock);
	W5500_NOP();
	W5500_WriteReg(opcode, Sn_PORT0, local_port >> 8);
	W5500_WriteReg(opcode, Sn_PORT1, local_port);
	while(SocketListen(sock)){
		LED_Orange_ON;
	}
	LED_Orange_OFF;
}










