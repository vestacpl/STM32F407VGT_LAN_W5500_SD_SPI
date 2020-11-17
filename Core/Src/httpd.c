/***	INCLUDE	**************************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "fatfs.h"
#include "httpd.h"
#include "W5500.h"

/*** VARIABLES	***********************************************************************************************************************************************************************************/
/*****************************************************************************************************************************************************************************************************/
uint32_t data_size[8];								//размер данных для передачи
uint16_t last_data_part_size[8];			//размер последней части данных для передачи
uint16_t cnt_data_part[8];					//общее количество частей данных для передачи
uint16_t cnt_rem_data_part[8];	  //количество оставшихся частей данных для передачи
uint32_t total_count_bytes[8]; 		//количество переданных байтов документа
uint8_t http_doc[8];									//вариант документа для передачи
uint8_t prt_tp[8];											// тип протокола документа
char fname[20][8];													//имя файла (документа)

volatile uint16_t tcp_size_wnd = 2048;
const char http_header[] = { "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"};
const char jpg_header[] = {"HTTP/1.0 200 OK\r\nServer: nginx\r\nContent-Type: image/jpeg\r\nConnection: close\r\n\r\n"};
const char icon_header[] = { "HTTP/1.1 200 OK\r\nContent-Type: image/x-icon\r\n\r\n"};
const char error_header[] = {"HTTP/1.0 404 File not found\r\nServer: nginx\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"};
char *header;
const uint8_t e404_htm[] = {
					0x3c,0x68,0x74,0x6d,0x6c,0x3e,0x0a,0x20,0x20,0x3c,0x68,0x65,0x61,0x64,0x3e,0x0a,
					0x20,0x20,0x20,0x20,0x3c,0x74,0x69,0x74,0x6c,0x65,0x3e,0x34,0x30,0x34,0x20,0x4e,
					0x6f,0x74,0x20,0x46,0x6f,0x75,0x6e,0x64,0x3c,0x2f,0x74,0x69,0x74,0x6c,0x65,0x3e,
					0x0a,0x20,0x20,0x3c,0x2f,0x68,0x65,0x61,0x64,0x3e,0x0a,0x3c,0x62,0x6f,0x64,0x79,
					0x3e,0x0a,0x3c,0x68,0x31,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x74,0x65,0x78,
					0x74,0x2d,0x61,0x6c,0x69,0x67,0x6e,0x3a,0x20,0x63,0x65,0x6e,0x74,0x65,0x72,0x3b,
					0x22,0x3e,0x34,0x30,0x34,0x20,0x45,0x72,0x72,0x6f,0x72,0x20,0x46,0x69,0x6c,0x65,
					0x20,0x4e,0x6f,0x74,0x20,0x46,0x6f,0x75,0x6e,0x64,0x3c,0x2f,0x68,0x31,0x3e,0x0a,
					0x3c,0x68,0x32,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x74,0x65,0x78,0x74,0x2d,
					0x61,0x6c,0x69,0x67,0x6e,0x3a,0x20,0x63,0x65,0x6e,0x74,0x65,0x72,0x3b,0x22,0x3e,
					0x20,0x54,0x68,0x65,0x20,0x70,0x61,0x67,0x65,0x20,0x79,0x6f,0x75,0x20,0x61,0x72,
					0x65,0x20,0x6c,0x6f,0x6f,0x6b,0x69,0x6e,0x67,0x20,0x66,0x6f,0x72,0x20,0x6d,0x69,
					0x67,0x68,0x74,0x20,0x68,0x61,0x76,0x65,0x20,0x62,0x65,0x65,0x6e,0x20,0x72,0x65,
					0x6d,0x6f,0x76,0x65,0x64,0x2c,0x20,0x3c,0x62,0x72,0x20,0x2f,0x3e,0x68,0x61,0x64,
					0x20,0x69,0x74,0x73,0x20,0x6e,0x61,0x6d,0x65,0x20,0x63,0x68,0x61,0x6e,0x67,0x65,
					0x64,0x2c,0x20,0x6f,0x72,0x20,0x69,0x73,0x20,0x74,0x65,0x6d,0x70,0x6f,0x72,0x61,
					0x72,0x69,0x6c,0x79,0x20,0x75,0x6e,0x61,0x76,0x61,0x69,0x6c,0x61,0x62,0x6c,0x65,
					0x2e,0x3c,0x2f,0x68,0x32,0x3e,0x0a,0x3c,0x2f,0x62,0x6f,0x64,0x79,0x3e,0x3c,0x2f,
					0x68,0x74,0x6d,0x6c,0x3e};

extern uint8_t sect[515];
extern char str1[60];
extern char tmpbuf[128];
extern FIL MyFile;
extern FRESULT result;
extern uint32_t bytesread;
extern UART_HandleTypeDef huart2;

/***	FUNCTIONS	***********************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
void Http_Request(uint8_t sock_num)
{
	uint8_t rx_bt = 0;
	uint16_t point, i = {0};
  char *ptr;
  int ch = '.';

  point = SocketGetReadPointer(sock_num);
  while (rx_bt != (uint8_t)'/')
  {
    rx_bt = W5500_ReadSockBufByte(sock_num, point+i);
    i++;
  }

	point += i;
	rx_bt = W5500_ReadSockBufByte(sock_num, point);

	if(rx_bt == (uint8_t)' ' )
	{
		strcpy(fname[sock_num], "index.htm");
		http_doc[sock_num] = EXISTING_HTML;
	}
	else
	{
		i=0;
		while (1)
		{
			tmpbuf[i] = W5500_ReadSockBufByte(sock_num, point+i);
			if(tmpbuf[i] == (uint8_t)' ')
				break;
			i++;
		}
		tmpbuf[i] = 0; //закончим строку
		strcpy(fname[sock_num], tmpbuf);
	}

	result = f_open(&MyFile, fname[sock_num], FA_READ);

	if (result == FR_OK)
	{
	  ptr = strchr(fname[sock_num], ch);
	  ptr++;
	  if (strncmp(ptr,"jpg", 3) == 0)
	  {
	    http_doc[sock_num] = EXISTING_JPG;
	    data_size[sock_num] = strlen(jpg_header);
	  }
	  if (strncmp(ptr,"ico", 3) == 0)
	  {
	    http_doc[sock_num] = EXISTING_ICO;
	    data_size[sock_num] = strlen(icon_header);
	  }
	  else
	  {
	    http_doc[sock_num] = EXISTING_HTML;
	    data_size[sock_num] = strlen(http_header);
	  }
	 data_size[sock_num] += MyFile.obj.objsize;
	}
	else
	{
	  http_doc[sock_num] = E404_HTML;
	  data_size[sock_num] = strlen(error_header);
	  data_size[sock_num] += sizeof(e404_htm);
	}

	cnt_rem_data_part[sock_num] = data_size[sock_num] / tcp_size_wnd + 1;
	last_data_part_size[sock_num] = data_size[sock_num] % tcp_size_wnd;

	if(last_data_part_size[sock_num] == 0)
	{
	  last_data_part_size[sock_num] = tcp_size_wnd;
	  cnt_rem_data_part[sock_num]--;
	}

	cnt_data_part[sock_num] = cnt_rem_data_part[sock_num];

	if (cnt_rem_data_part[sock_num] == 1) {
	  Http_Send_One(sock_num);
	}
	else if (cnt_rem_data_part[sock_num] > 1) {
	  Http_Send_First(sock_num);
	}

}

//**************************************************************************************************//
//**************************************************************************************************//
void Http_Send_One(uint8_t sock_num)
{
	uint8_t num_sect=0;
  uint16_t i, data_len, header_len, end_point, len_sect = {0};

	if ((http_doc[sock_num] == EXISTING_HTML)||\
			(http_doc[sock_num] == EXISTING_JPG)||\
			(http_doc[sock_num] == EXISTING_ICO))
	{
		switch(http_doc[sock_num])
		{
			case EXISTING_HTML:
				header = (void*)http_header;
				break;
			case EXISTING_ICO:
				header = (void*)icon_header;
				break;
			case EXISTING_JPG:
				header = (void*)jpg_header;
				break;
		}

		header_len = strlen(header);
		data_len = (uint16_t)MyFile.obj.objsize;
		end_point = SocketGetWritePointer(sock_num);
		end_point += (header_len+data_len);

		SocketSetWritePointer(sock_num, end_point);
		end_point = SocketGetWritePointer(sock_num);
		memcpy(sect+3, header, header_len);
		W5500_WriteSockBuf(sock_num, end_point, (uint8_t*)sect, header_len);

		end_point += header_len;
		num_sect = data_len / 512;

		for(i=0; i <= num_sect; i++)
		{
			if(i < (num_sect-1)) len_sect = 512;
			else len_sect = data_len;
			result = f_lseek(&MyFile, i * 512);
			result = f_read(&MyFile, sect+3, len_sect, (UINT *)&bytesread);
			W5500_WriteSockBuf(sock_num, end_point, (uint8_t*)sect, len_sect);
			end_point += len_sect;
			data_len -= len_sect;
		}
	}
	else
	{
		header_len = strlen(error_header);
		data_len = sizeof(e404_htm);
		end_point = SocketGetWritePointer(sock_num);
		end_point += (header_len+data_len);
		SocketSetWritePointer(sock_num, end_point);
		end_point = SocketGetWritePointer(sock_num);
		memcpy(sect+3, error_header, header_len);
		W5500_WriteSockBuf(sock_num, end_point, (uint8_t*)sect, header_len);
		end_point+=header_len;
		memcpy(sect+3, e404_htm, data_len);
		W5500_WriteSockBuf(sock_num, end_point, (uint8_t*)sect, data_len);
		end_point+=data_len;
	}
	SocketResv(sock_num);
	SocketSend(sock_num);
	f_close(&MyFile);
	SocketReset(sock_num);
}

//**************************************************************************************************//
//**************************************************************************************************//
void Http_Send_First(uint8_t sock_num)
{
  uint8_t num_sect = 0;
  uint16_t i, data_len, header_len, end_point, len_sect = {0};

	switch(http_doc[sock_num])
	{
		case EXISTING_HTML:
			header = (void*)http_header;
			break;
		case EXISTING_ICO:
			header = (void*)icon_header;
			break;
		case EXISTING_JPG:
			header = (void*)jpg_header;
			break;
	}

	header_len = strlen(header);
	data_len = (tcp_size_wnd - header_len);
	end_point = SocketGetWritePointer(sock_num);
	end_point += (header_len + data_len);

	SocketSetWritePointer(sock_num, end_point);
	end_point = SocketGetWritePointer(sock_num);

	memcpy(sect+3, header, header_len);
	W5500_WriteSockBuf(sock_num, end_point, (uint8_t*)sect, header_len);
	end_point += header_len;
	num_sect = data_len / 512;

	for(i=0; i <= num_sect; i++)
	{
		if(i < (num_sect-1)) len_sect = 512;
		else len_sect = data_len;
		result = f_lseek(&MyFile, i*512);
		result = f_read(&MyFile, sect+3, len_sect, (UINT *)&bytesread);
		W5500_WriteSockBuf(sock_num, end_point, (uint8_t*)sect, len_sect);
		end_point += len_sect;
		data_len -= len_sect;
	}

	SocketResv(sock_num);
	SocketSend(sock_num);

	cnt_rem_data_part[sock_num]--;

	if(cnt_rem_data_part[sock_num] > 1) {
		Http_Send_Middle(sock_num);
	}
	else {
		Http_Send_Last(sock_num);
	}

 total_count_bytes[sock_num] = (uint32_t)(tcp_size_wnd - header_len);
}

//**************************************************************************************************//
//**************************************************************************************************//
void Http_Send_Middle(uint8_t sock_num)
{
	uint8_t num_sect=0;
  uint16_t i, data_len, end_point, len_sect = {0};

	data_len = tcp_size_wnd;
	end_point = SocketGetWritePointer(sock_num);
	end_point += data_len;
	SocketSetWritePointer(sock_num, end_point);
	end_point = SocketGetWritePointer(sock_num);

	num_sect = data_len / 512;
	if((data_len % 512) == 0)
		num_sect--;

	for(i = 0; i <= num_sect; i++)
	{
		if(i < (num_sect-1))
			len_sect = 512;
		else len_sect = data_len;
		result = f_lseek(&MyFile,(DWORD)(i*512) + total_count_bytes[sock_num]);
		result = f_read(&MyFile,sect+3,len_sect,(UINT *)&bytesread);
		W5500_WriteSockBuf(sock_num, end_point, (uint8_t*)sect, len_sect);
		end_point += len_sect;
		data_len -= len_sect;
	}

	SocketResv(sock_num);
	SocketSend(sock_num);

	cnt_rem_data_part[sock_num]--;

	if(cnt_rem_data_part[sock_num] > 1) {
		Http_Send_Middle(sock_num);
	}
	else {
		Http_Send_Last(sock_num);
	}
	total_count_bytes[sock_num] += (uint32_t)tcp_size_wnd;
}

//**************************************************************************************************//
//**************************************************************************************************//
void Http_Send_Last(uint8_t sock_num)
{
	uint8_t num_sect=0;
  uint16_t i, data_len, end_point, len_sect =0;

	data_len = last_data_part_size[sock_num];
	end_point = SocketGetWritePointer(sock_num);
	end_point+=data_len;
	SocketSetWritePointer(sock_num, end_point);
	end_point = SocketGetWritePointer(sock_num);

	num_sect = data_len / 512;
	if((data_len % 512) == 0)
		num_sect--;

	for(i = 0; i <= num_sect; i++)
	{
		if(i < (num_sect-1))
			len_sect = 512;
		else len_sect = data_len;
		result = f_lseek(&MyFile, (DWORD)(i*512) + total_count_bytes[sock_num]);
		result = f_read(&MyFile,sect+3, len_sect, (UINT *)&bytesread);
		W5500_WriteSockBuf(sock_num, end_point, (uint8_t*)sect, len_sect);
		end_point += len_sect;
		data_len -= len_sect;
	}

	f_close(&MyFile);

	SocketResv(sock_num);
	SocketSend(sock_num);

	SocketReset(sock_num);
}
