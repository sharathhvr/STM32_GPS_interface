/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f4xx.h"


/*
 * Macros
 */
#define NMEA_GPRMC_SENTENCE_SIZE    70


void getLatitude(char* char_minutes,char* char_seconds,char* char_degrees,char* NMEA_Sent);
void getLongitude(char* char_minutes,char* char_seconds,char* char_degrees,char* NMEA_Sent);
static void prvSetupHardware(void);
static void prvSetupUART1(void);
static void prvSetupUART2(void);
static void prvSetupLED_GPIO(void);
static void printmsg(char *msg);
static void gpsCommand(char *msg);

/*
 * Global variables
 */

int lat_minutes=0;
int lat_degrees=0;
float lat_seconds=0;

int long_minutes=0;
int long_degrees=0;
float long_seconds=0;


int main(void)
{


	//1.Reset the RCC clock configuration
		//HSI ON, PLL OFF, HSE OFF , system clock= 16MHz, cpu_clock=16MHz
		RCC_DeInit();

		//2.Update the sys clock, make sys clock point to HSI
		SystemCoreClockUpdate();

		//call hardware setup
		prvSetupHardware();

		//gps setup
		gpsCommand("$PGCMD,33,0*6D\r\n");// turn off antenna data nuesance
		gpsCommand("$PMTK220,1000*1F\r\n");// set update frequency to 1Hz
	    gpsCommand("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"); // RMC NMEA Sentence




		for(;;);

}


static void prvSetupHardware(void)
{

	//setup UART2
	prvSetupUART2();
	//setup UART1
	prvSetupUART1();
	//setup led gpio
	prvSetupLED_GPIO();

}

/*
 *   BASIC FUNCTIONS
 */

static void prvSetupLED_GPIO(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef LED_GPIO;
	LED_GPIO.GPIO_Mode=GPIO_Mode_OUT;
	LED_GPIO.GPIO_OType=GPIO_OType_PP;
	LED_GPIO.GPIO_Pin=GPIO_Pin_5;
	LED_GPIO.GPIO_PuPd=GPIO_PuPd_NOPULL;
	LED_GPIO.GPIO_Speed=GPIO_Low_Speed;

	GPIO_Init(GPIOA,&LED_GPIO);

}


static void printmsg(char *msg)
{
	for(int i=0;i< strlen(msg);i++)
	{
		//wait untill DR empty
		while(!USART_GetFlagStatus(USART2,USART_FLAG_TXE));
		USART_SendData(USART2,msg[i]);
	}
}


static void prvSetupUART2(void)
{
		//1.Enable UART2 peripheral clock and GPIO portA
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);

			//USART pins PA2-Rx ,PA3-Tx

		//2.Alternate function configuration of MCU pins to behave as USART2 RX and TX

			// lets first initialize GPIO pins
			//populate GPIO init structure and then pass
			GPIO_InitTypeDef gpio_uart_pins;

			//GPIO PA2 and PA3
			memset(&gpio_uart_pins,0,sizeof(gpio_uart_pins));//setting all struct member elements to 0
			gpio_uart_pins.GPIO_Pin=GPIO_Pin_2 | GPIO_Pin_3;
			gpio_uart_pins.GPIO_Mode=GPIO_Mode_AF;
			gpio_uart_pins.GPIO_PuPd= GPIO_PuPd_UP; // USART initial state is UP
			GPIO_Init(GPIOA,&gpio_uart_pins);

			//AF mode setting for pins
			GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);// now PA2 is Rx
			GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);// now PA3 is Tx

		//3.UART peripheral configuration
			USART_InitTypeDef usart_peri_init;
			memset(&usart_peri_init,0,sizeof(usart_peri_init));//setting all struct member elements to 0
			usart_peri_init.USART_BaudRate=9600;
			usart_peri_init.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
			usart_peri_init.USART_Mode=  USART_Mode_Tx | USART_Mode_Rx;
			usart_peri_init.USART_Parity=USART_Parity_No;
			usart_peri_init.USART_StopBits=USART_StopBits_1;
			usart_peri_init.USART_WordLength=USART_WordLength_8b;
			USART_Init(USART2,&usart_peri_init);

		//4.Enable UART2 byte reception interrupt(there are many other interrupts)
			USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);

		//Lets set NVIC priority of USART2
			NVIC_SetPriority(USART2_IRQn,5);

		//enable the USART2 IRQ in the NVIC
			NVIC_EnableIRQ(USART2_IRQn);

		//5.Enable USART2 peripheral
			USART_Cmd(USART2,ENABLE);

}


static void prvSetupUART1(void)
{
	//1.Enable UART1 peripheral clock and GPIO portA
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);

		//USART pins PA2-Rx ,PA3-Tx

		//2.Alternate function configuration of MCU pins to behave as USART2 RX and TX

			// lets first initialize GPIO pins
			//populate GPIO init structure and then pass
			GPIO_InitTypeDef gpio_uart_pins;

			//GPIO PA2 and PA3
			memset(&gpio_uart_pins,0,sizeof(gpio_uart_pins));//setting all struct member elements to 0
			gpio_uart_pins.GPIO_Pin=GPIO_Pin_9 | GPIO_Pin_10;
			gpio_uart_pins.GPIO_Mode=GPIO_Mode_AF;
			gpio_uart_pins.GPIO_PuPd= GPIO_PuPd_UP; // USART initial state is UP
			GPIO_Init(GPIOA,&gpio_uart_pins);

			//AF mode setting for pins
			GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);// now PA9 is tx
			GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);// now PA10 is rx

		//3.UART peripheral configuration
			USART_InitTypeDef usart_peri_init;
			memset(&usart_peri_init,0,sizeof(usart_peri_init));//setting all struct member elements to 0
			usart_peri_init.USART_BaudRate=9600 ;
			usart_peri_init.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
			usart_peri_init.USART_Mode=  USART_Mode_Tx | USART_Mode_Rx;
			usart_peri_init.USART_Parity=USART_Parity_No;
			usart_peri_init.USART_StopBits=USART_StopBits_1;
			usart_peri_init.USART_WordLength=USART_WordLength_8b;
			USART_Init(USART1,&usart_peri_init);

		//4.Enable UART2 byte reception interrupt(there are many other interrupts)
			USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);

		//Lets set NVIC priority of USART2
			NVIC_SetPriority(USART1_IRQn,5);

		//enable the USART2 IRQ in the NVIC
			NVIC_EnableIRQ(USART1_IRQn);

		//5.Enable USART2 peripheral
			USART_Cmd(USART1,ENABLE);

}


void USART1_IRQHandler(void)
{
	static int flag=0;
	static int i=0;
	static char char_lat_minutes[10];
	static char char_lat_seconds[10];
	static char char_lat_degrees[10];
	static char char_long_minutes[10];
	static char char_long_seconds[10];
	static char char_long_degrees[10];
	char test_print[100];
	uint16_t data_byte;
	char NMEA_Sent[NMEA_GPRMC_SENTENCE_SIZE];
	if( USART_GetFlagStatus(USART1,USART_FLAG_RXNE) )
	{
		//a data byte is received from the user
	   data_byte = USART_ReceiveData(USART1);
	   if(data_byte == 36)
		{
		   flag=1;
		}
	   //extract the NMEA sentence
	   if((i < NMEA_GPRMC_SENTENCE_SIZE-1 ) & (flag==1))
	   {
		   NMEA_Sent[i]=(char)data_byte;
		   i++;
	   }
	   else
	   {
		   //dummy print
		   printmsg("\r\n");

		   if( ((char)NMEA_Sent[3]) =='R') // check if its a GPRMC sentence
		   {
			   	   getLatitude(char_lat_minutes,char_lat_seconds,char_lat_degrees,NMEA_Sent);

				   lat_degrees=atoi(char_lat_degrees);
				   lat_minutes=atoi(char_lat_minutes);
				   lat_seconds=60*(atof(char_lat_seconds));

				   sprintf(test_print,"Latitude: %d degrees %d minutes %f seconds",lat_degrees,lat_minutes,lat_seconds);
				   printmsg(test_print);
				   printmsg("\r\n");

				   getLongitude(char_long_minutes,char_long_seconds,char_long_degrees,NMEA_Sent);

				   long_degrees=atoi(char_long_degrees);
				   long_minutes=atoi(char_long_minutes);
				   long_seconds=60*(atof(char_long_seconds));

				   sprintf(test_print,"Longitude: %d degrees %d minutes %f seconds",long_degrees,long_minutes,long_seconds);
				   printmsg(test_print);
				   printmsg("\r\n");

		   }
		   i=0;
		   flag=0;
	   }
	}
}


static void gpsCommand(char *msg)
{
	for(int i=0;i< strlen(msg);i++)
	{
		//wait untill DR empty
		while(!USART_GetFlagStatus(USART1,USART_FLAG_TXE));
		USART_SendData(USART1,msg[i]);
	}
}

void getLongitude(char* char_minutes,char* char_seconds,char* char_degrees,char* NMEA_Sent)
{
	//Extract minutes
	   int k=0;
	   for(int j= 35;j<37;j++)
	   {
		   char_minutes[k]=NMEA_Sent[j];
		   k++;
	   }
	// extract seconds
		k=0;
	   for(int j= 37;j<42;j++)
	   {
		   char_seconds[k]=NMEA_Sent[j];
		   k++;
	   }
	// extract degrees
		k=0;
	   for(int j= 32;j<35;j++)
	   {
		   char_degrees[k]=NMEA_Sent[j];
		   k++;
	   }

}


void getLatitude(char* char_minutes,char* char_seconds,char* char_degrees,char* NMEA_Sent)
{
//Extract minutes
   int k=0;
   for(int j= 22;j<24;j++)
   {
	   char_minutes[k]=NMEA_Sent[j];
	   k++;
   }
// extract seconds
	k=0;
   for(int j= 24;j<29;j++)
   {
	   char_seconds[k]=NMEA_Sent[j];
	   k++;
   }
// extract degrees
	k=0;
   for(int j= 20;j<22;j++)
   {
	   char_degrees[k]=NMEA_Sent[j];
	   k++;
   }
}




