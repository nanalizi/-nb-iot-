#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "math.h"			
#include "stdio.h"
#include "stm32f10x_flash.h"
#include "stdlib.h"
#include "string.h"
#include "wdg.h"
#include "timer.h"
#include "stm32f10x_tim.h"
#include "bc26.h"	 
#include "adc.h"
#include "math.h"
#include "ds18b20.h"

extern char  RxBuffer[100],RxCounter;//串口2 BC26

GPIO_InitTypeDef  GPIO_InitStructure; 
unsigned char AD_CHANNEL=0;
unsigned long PH_num=0,PU_V=0;
float PH_Value=0;
u8 ph_temp=0,tu_temp=0;
u16 ph_result=0,tu_result=0;
float TDS=0.0,TDS_voltage;
float TDS_value=0.0,voltage_value;
float temp_data=250.0;
float compensationCoefficient=1.0;//温度校准系数
float compensationVolatge;
float kValue=1.67;
float TEMP_Value=0.0;
float TU=0.0;
float TU_value=0.0;
float TU_calibration=0.0;
float K_Value=1691.5;


// ADC1转换的电压值通过MDA方式传到SRAM
extern __IO uint16_t ADC_ConvertedValue[4];

// 用于保存转换计算后的电压值 	 
float ADC_ConvertedValueLocal[4];  



 /***
LED
***/
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable the GPIO  Clock */					 		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC| RCC_APB2Periph_AFIO,ENABLE);
	
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);		//屏蔽所有作为JTAG口的GPIO口
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);		//屏蔽PB口上IO口JTAG功能

//LED 引脚	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /*I/O 方向 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; /*I/O 输出速度*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA , GPIO_Pin_8);       //初始状态，关闭LED
	GPIO_SetBits(GPIOA , GPIO_Pin_11);       //初始状态，关闭LED
	GPIO_SetBits(GPIOA , GPIO_Pin_12);       //初始状态，关闭LED
}
/***
此功能实现往阿里云发数据，用户需要更改三元素与推送主题一致。保持和自己的设备一样
***/
void OPEN_BC26(void)
{
   char *strx;
 
    printf("AT\r\n"); 
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    printf("AT\r\n"); 
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
	IWDG_Feed();//喂狗
   if(strx==NULL)
	{
        PWRKEY=1;//拉低
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);	
        PWRKEY=0;//拉高正常开机
        IWDG_Feed();//喂狗
	}
    printf("AT\r\n"); 
    delay_ms(300);
    IWDG_Feed();//喂狗
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    printf("ATE0&W\r\n"); //关闭回显
    delay_ms(300); 
    LED=0;
    IWDG_Feed();//喂狗
    printf("AT+QMTDISC=0\r\n");//关闭连接
    delay_ms(300);
    printf("AT+QMTCLOSE=0\r\n");
    delay_ms(300); 
}


/***
浑浊度
***/
u8 TU_Value_Conversion()
{
		TU=ADC_ConvertedValueLocal[2]/0.66;
	  TU_calibration=-0.0192*(temp_data/10-25.0)+TU;      
	  TU_value=-865.68*TU_calibration + K_Value;
	
		if(TU_value<=0){TU_value=0;}
		if(TU_value>=3000){TU_value=3000;}
		
		printf("TU val: %f\n",TU_value);
		return TU_value;
}

/***
温度
***/
u8 TEMP_Value_Conversion()
{
	  TEMP_Value=DS18B20_Get_Temp();
	
		printf("temp val: %f\n",TEMP_Value);
		return TEMP_Value;
}



/***
PH
***/
u8 PH_Value_Conversion()
{
		u16 adcx;
		float PH;
		float tmpetValue;
		adcx=Get_Adc_Average(4,10);
		tmpetValue = (float)(DS18B20_Get_Temp())/10;
		  
   if( tmpetValue > 42 )  adcx += 5;
   else if(tmpetValue > 28){
     adcx += 5*(tmpetValue - 28)/14;
   }
		
		PH=(float)adcx*(3.3/4096);
		PH = -5.7541*PH+16.654;   //输出电压范围0~3V3
		if(PH<=0.0){PH=0.0;}
		if(PH>=14.0){PH=14.0;}
		//printf("adc4 val: %d\r\t",adcx);
		printf("PH val: %f\n",PH);
		
		delay_ms(250);	 //延时300ms
		PCout(13)=	!PCout(13);
		
		return PH;
}

/***************************************************************************
 * 描  述 : MAIN函数
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************/
 
 int main(void)
 {	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	delay_init();	    	 //延时函数初始化	  	
	GPIO_Configuration();
	uart_init(115200);//串口1初始化，可连接PC进行打印模块返回数据    
	uart2_init(115200);//对接BC26串口初始化
	IWDG_Init(7,625);    //8S一次
	ADCx_Init(); 
	 
	while( DS18B20_Init() ){
		printf("未检测到温度传感器" );
		delay_ms(900);
	}
		OPEN_BC26();//对BC26开机  
    BC26_Init();//对设备初始化
    MQTT_Init();
	while(1)
	{		
		u8 temp,PH,TU;
		int i;
		for(i=0;i<4;i++)
		{
		ADC_ConvertedValueLocal[i]=(float)ADC_ConvertedValue[i]/4096*3.3;
		printf("ADC_ConvertedValue[%d]=%4.3f V\r\n",i,ADC_ConvertedValueLocal[i]);
		}
		temp = TEMP_Value_Conversion();
		PH = PH_Value_Conversion();
		TU = TU_Value_Conversion();

   	aliyunMQTT_PUBdata(temp,PH,TU);
		delay_ms(500);
//		PCout(13)=	!PCout(13);	
    IWDG_Feed();//喂狗
	}
 }
