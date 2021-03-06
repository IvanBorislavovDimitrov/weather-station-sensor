#include "spi_hard.h"


SPI_InitTypeDef  SPI_InitStructure;


void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1, ENABLE );	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

 	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; 
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	
	SPI_InitStructure.SPI_CRCPolynomial = 7;	
	SPI_Init(SPI1, &SPI_InitStructure); 
 
	SPI_Cmd(SPI1, ENABLE); 
	
	SPI1_ReadWriteByte(0xff);	 
}   


//SPI set Speed Function
//SpeedSet:
//SPI_BaudRatePrescaler_2   2 Frequency division   (SPI 36M@sys 72M)
//SPI_BaudRatePrescaler_8   8 Frequency division   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_16  16 Frequency division  (SPI 4.5M@sys 72M)
//SPI_BaudRatePrescaler_256 256 Frequency division (SPI 281.25K@sys 72M)
void SPI1_SetSpeed(u8 SpeedSet)
{
	SPI_InitStructure.SPI_BaudRatePrescaler = SpeedSet ;
  	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1,ENABLE);
} 

u8 SPI1_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) 
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(SPI1, TxData); 
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	return SPI_I2S_ReceiveData(SPI1); 				    
}


void SPI_CS_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6);
	
	SPI1_Init();
	
	SPI1_SetSpeed(SPI_BaudRatePrescaler_4);
		
}



void SPI_BME280_CS_Low(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);
}


void SPI_BME280_CS_High(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_6);
}


int8_t user_spi_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	int8_t rslt = 0;
	SPI_BME280_CS_High();
	SPI_BME280_CS_Low();
	
	SPI1_ReadWriteByte(reg_addr);
	for(int i = 0; i < len; i++)
	{
		*reg_data = SPI1_ReadWriteByte(0xFF);
		reg_data++;
	}
	
	SPI_BME280_CS_High();
	
	return rslt;
}


int8_t user_spi_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	int8_t rslt = 0;
	SPI_BME280_CS_High();
	SPI_BME280_CS_Low();
	
	SPI1_ReadWriteByte(reg_addr);
	for(int i = 0; i < len; i++)
	{
		SPI1_ReadWriteByte(*reg_data);
		reg_data++;
	}
	
	SPI_BME280_CS_High();
	
	return rslt;
}
