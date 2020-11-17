/***	INCLUDE	**************************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
#include <sd_spi.h>

/***	VARIABLES	************************************************************************************************************************************************************************************/
/*****************************************************************************************************************************************************************************************************/
char USER_Path[4];
FATFS SDFatFs;
FATFS *fs;
FIL MyFile;
FRESULT result;
uint32_t bytesread;
sd_info_ptr sdinfo;
FILINFO sFileInfo;
DIR sDirectory;
char aStringBuffer[60];
uint8_t aBuffer[512];

extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart2;

/***	FUNCTIONS	***********************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
uint8_t SD_SPI_Init(void)
{
	uint8_t cmd;
	int16_t cnt;
	uint8_t arr[4];
	sdinfo.type = 0;

	HAL_Delay(250);														// SD voltage stability delay

	HAL_SPI_Init(&hspi2);

	SD_DESELECT;
	for(cnt = 0; cnt<10; cnt++) // 80 pulse bit. Set SPI as SD card interface
		SPI_Release();

	SD_SELECT;
	if (SD_SPI_Cmd(CMD0, 0) == 1) // Enter Idle state
		{
			SPI_Release();
			if (SD_SPI_Cmd(CMD8, 0x1AA) == 1) // SDv2
				{
					for (cnt = 0; cnt < 4; cnt++)
						arr[cnt] = SPI_ReceiveByte();
					if (arr[2] == 0x01 && arr[3] == 0xAA) // The card can work at vdd range of 2.7-3.6V
						{
							for (cnt = 12000; (cnt && SD_SPI_Cmd(ACMD41, 1UL << 30)); cnt--)	{;}	 // Wait for leaving idle state (ACMD41 with HCS bit)
							if (cnt && SD_SPI_Cmd(CMD58, 0) == 0)
								{ // Check CCS bit in the OCR
									for (cnt = 0; cnt < 4; cnt++) 	arr[cnt] = SPI_ReceiveByte();
									sdinfo.type = (arr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; // SDv2 (HC or SC)
								}
						}
				}
			else		//SDv1 or MMCv3
				{
					if (SD_SPI_Cmd(ACMD41, 0) <= 1)
						{
							sdinfo.type = CT_SD1; cmd = ACMD41; // SDv1
						}
						else
						{
							sdinfo.type = CT_MMC; cmd = CMD1; // MMCv3
						}
					for (cnt = 25000; cnt && SD_SPI_Cmd(cmd, 0); cnt--) ; // Wait for leaving idle state
					if ( ! cnt || SD_SPI_Cmd(CMD16, 512) != 0) // Set R/W block length to 512
					sdinfo.type = 0;
				}
		}
	else
		{
		Error_Handler();
		}

	return 0;
}

/****************************************************************************************************************/
/****************************************************************************************************************/
uint8_t SPIx_WriteRead(uint8_t bt)
{
  uint8_t recv_bt = 0;
  if(HAL_SPI_TransmitReceive(&hspi2, (uint8_t*) &bt, (uint8_t*) &recv_bt, 1, 0x1000) != HAL_OK)
  {
  	SD_Error_Handler(SD_TransmitReceive_ERR);
  }
  return recv_bt;
}

/****************************************************************************************************************/
/****************************************************************************************************************/
void SPI_SendByte(uint8_t bt)
{
  SPIx_WriteRead(bt);
}

/****************************************************************************************************************/
/****************************************************************************************************************/
uint8_t SPI_ReceiveByte(void)
{
  uint8_t bt = SPIx_WriteRead(0xFF);
  return bt;
}

/****************************************************************************************************************/
/****************************************************************************************************************/
void SPI_Release(void)
{
  SPIx_WriteRead(0xFF);
}

/****************************************************************************************************************/
/****************************************************************************************************************/
uint8_t SD_SPI_WaitingForReadiness(void)
{
	uint8_t res;
	uint16_t cnt = 0;

	do {
				 res = SPI_ReceiveByte();
				 cnt++;
	} while ( (res != 0xFF) && (cnt < 0xFFFF) );

	if (cnt >= 0xFFFF) return ERROR;

	  return (res == 0xFF) ? OK: ERROR;
}

/****************************************************************************************************************/
/****************************************************************************************************************/
uint8_t SD_SPI_Cmd(uint8_t cmd, uint32_t argument)
{
  uint8_t bt, res;

	// ACMD is the command sequence of CMD55-CMD?
	if (cmd & 0x80)
	{
		cmd &= 0x7F;
		res = SD_SPI_Cmd(CMD55, 0);
		if (res > 1) return res;
	}

	// Select the card
	SD_DESELECT;
	SPI_ReceiveByte();
	SD_SELECT;
	SPI_ReceiveByte();

	// Send a command packet
	SPI_SendByte(cmd); // Start + Command index
	SPI_SendByte((uint8_t)(argument >> 24)); // Argument[31..24]
	SPI_SendByte((uint8_t)(argument >> 16)); // Argument[23..16]
	SPI_SendByte((uint8_t)(argument >> 8)); // Argument[15..8]
	SPI_SendByte((uint8_t)argument); // Argument[7..0]
	bt = 0x01; // Dummy CRC + Stop

	if (cmd == CMD0) {bt = 0x95;} // Valid CRC for CMD0(0)
	if (cmd == CMD8) {bt = 0x87;} // Valid CRC for CMD8(0x1AA)
	SPI_SendByte(bt);

  // Receive a command response
  bt = 10; // Wait for a valid response in timeout of 10 attempts
  do {
    		res = SPI_ReceiveByte();
  } while ((res & 0x80) && --bt);

  return res;

}

/****************************************************************************************************************/
/****************************************************************************************************************/
uint8_t SD_SPI_Read_Block(uint8_t *buff, uint32_t lba)
{
  uint8_t res = 0;
  uint16_t cnt = 0;

	res = SD_SPI_Cmd (CMD17, lba);
	if (res) return 5; //	Error

	SPI_Release();

  do{
				res=SPI_ReceiveByte();
				cnt++;
  } while ((res != 0xFE) && (cnt < 0xFFFF)); // Wait till mark(0xFE) is received
  if (cnt >= 0xFFFF) return 5;	 //	 Error

  for (cnt = 0; cnt<512; cnt++) buff[cnt]=SPI_ReceiveByte(); // Write data to the buffer
  SPI_Release(); // Skip CRC
  SPI_Release();

  return 0;
}

/****************************************************************************************************************/
/****************************************************************************************************************/
uint8_t SD_SPI_Write_Block (uint8_t *buff, uint32_t lba)
{
  uint8_t res;
  uint16_t cnt;

  res = SD_SPI_Cmd(CMD24, lba);

  if(res != 0x00) return 6; // Error

  SPI_Release();
  SPI_SendByte (0xFE); // Send transmission start mark
  for (cnt = 0; cnt<512; cnt++) SPI_SendByte(buff[cnt]); // Write data to the SD
  SPI_Release();  // Skip CRC
  SPI_Release();
  res = SPI_ReceiveByte();
  if((res & 0x05) != 0x05) return 6; // Error  (datasheet p. 111)

  cnt = 0;
  do {
				res=SPI_ReceiveByte();
				cnt++;
  } while ( (res != 0xFF)&&(cnt<0xFFFF) );		//Wait till BUSY mode is finished
  if (cnt>=0xFFFF) return 6;		// Error

  return 0;
}

/****************************************************************************************************************/
/****************************************************************************************************************/
uint8_t SD_SPI_MountLogicalDrive(void)
{
	if(f_mount(&SDFatFs, (TCHAR const*)USER_Path, 0))			SD_Error_Handler(SD_UNMOUNT)
			;

	return SD_OK;
}

/****************************************************************************************************************/
/****************************************************************************************************************/
uint8_t SD_SPI_ReadFile(TCHAR* filname)
{
			if(f_open(&MyFile, filname, FA_READ))		return SD_NO_FILE;
					;

			SD_SPI_ReadLongFile();
			f_close(&MyFile);;

	return SD_OK;
}

/****************************************************************************************************************/
/****************************************************************************************************************/
uint8_t SD_SPI_ReadLongFile(void)
{
  uint16_t tmp = 0;
  uint32_t cnt = 0;
  uint32_t fil_size = MyFile.obj.objsize;
  uint32_t bt_rd_cnt;

	do
	{
		if(fil_size < 512)
		{
			tmp = fil_size;
		}
		else
		{
			tmp = 512;
		}
		fil_size -= tmp;

		f_lseek(&MyFile, cnt);
		f_read(&MyFile, aBuffer, tmp, (UINT *)&bt_rd_cnt);

		cnt += tmp;
	}
	while(fil_size > 0);

  	return OK;
}

/****************************************************************************************************************/
/****************************************************************************************************************/
uint8_t SD_SPI_WriteFile(TCHAR* filname)
{
	FRESULT res;
	uint8_t tx_buf[] = "Added text!";
	uint32_t bt_wr_cnt;

	SD_SPI_MountLogicalDrive();

	if(f_open(&MyFile, filname, FA_CREATE_ALWAYS | FA_WRITE))			SD_Error_Handler(SD_NO_FILE);
		;

	res = f_write(&MyFile, tx_buf, sizeof(bt_wr_cnt), (void*)&bt_wr_cnt);

	if((bt_wr_cnt == 0) || (res))		SD_Error_Handler(SD_WriteFile_ERR)
			;

	f_close(&MyFile);

	return SD_OK;
}

/****************************************************************************************************************/
/****************************************************************************************************************/
//uint8_t SD_SPI_GetFileInfo(void)
//{
//	uint8_t res;
//	DWORD free_clusters, free_sectors;
//
//	SD_SPI_MountLogicalDrive();
//
//	res = f_opendir(&sDirectory, "/");		// "/" - directory name to open
//
//	if (res == FR_OK)
//	{
//		while(1)
//		{
//			res = f_readdir(&sDirectory, &sFileInfo);
//
//			if ((res == FR_OK) && (sFileInfo.fname[0]))
//			{
//				HAL_UART_Transmit(&huart2, (uint8_t*)sFileInfo.fname, strlen((char*)sFileInfo.fname), 0x1000);
//
//				if(sFileInfo.fattrib & AM_DIR)
//				{
//					HAL_UART_Transmit(&huart2, (uint8_t*)"  [DIR]", 7, 0x1000);
//				}
//			}
//			else break;
//
//			HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 0x1000);
//		}
//	}
//	f_closedir(&sDirectory);
//
//	f_getfree("/", &free_clusters, &fs);
//
//	return 0;
//}

/****************************************************************************************************************/
/****************************************************************************************************************/
uint8_t SD_Error_Handler(uint8_t res)
{
	switch(res)
		{
			case 1:
				LED_Red_ON;
						break;

			default:
				LED_Orange_ON;
						return 0;
		}

	while(1);

	return 0;

}

/****************************************************************************************************************/
/****************************************************************************************************************/


