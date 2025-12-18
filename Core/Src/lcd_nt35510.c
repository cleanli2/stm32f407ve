//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它商业用途
//测试硬件：单片机STM32F407VET6,STM32F407VxT6最小系统开发板,主频168MHZ，晶振8MHZ
//QDtech-TFT液晶驱动 for STM32 FSMC
//Chan@ShenZhen QDtech co.,LTD
//公司网站:www.qdtft.com
//wiki技术资料网站：http://www.lcdwiki.com
//我司提供技术支持，任何技术问题欢迎随时交流学习
//固话(传真) :+86 0755-21077707 
//手机: (销售)18823372746 （技术)15989313508
//邮箱:(销售/订单) sales@qdtft.com  (售后/技术服务)service@qdtft.com
//QQ:(售前咨询)3002706772 (技术支持)3002778157
//技术交流QQ群:778679828
//创建日期:2020/06/29
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 深圳市全动电子技术有限公司 2018-2028
//All rights reserved
/************************************************************************************
//此模块可以直接插入STM32F407VxT6最小系统开发板TFTLCD插槽或者通过排线接入TFTLCD插座
//STM32连接引脚是指TFTLCD插槽或者插座引脚内部连接的STM32引脚
//=================================电源接线=======================================//
//     LCD模块                    STM32连接引脚
//      VDD           --->           DC5V/3.3V          //电源
//      GND           --->             GND              //电源地
//=============================液晶屏数据线接线===================================//
//本模块默认数据总线类型为16位并口总线(8位模式请使用DB0~DB7)
//     LCD模块                    STM32连接引脚
//      DB0           --->            PD14        -|   
//      DB1           --->            PD15         |  
//      DB2           --->            PD0          | 
//      DB3           --->            PD1          | 
//      DB4           --->            PE7          |
//      DB5           --->            PE8          |
//      DB6           --->            PE9          |
//      DB7           --->            PE10         |===>液晶屏16位并口数据信号
//      DB8           --->            PE11         |
//      DB9           --->            PE12         |
//      DB10          --->            PE13         |
//      DB11          --->            PE14         |
//      DB12          --->            PE15         |
//      DB13          --->            PD8          |
//      DB14          --->            PD9          |
//      DB15          --->            PD10        -|
//=============================液晶屏控制线接线===================================//
//     LCD模块 				            STM32连接引脚 
//      WR            --->            PD5             //液晶屏写数据控制信号
//      RD            --->            PD4             //液晶屏读数据控制信号
//      RS            --->            PD11            //液晶屏数据/命令控制信号
//      RST           --->          复位引脚（默认）  //液晶屏复位控制信号（也可选择PD13）
//      CS            --->            PD7             //液晶屏片选控制信号
//      BL            --->            PB15            //液晶屏背光控制信号
//===============================触摸屏触接线=====================================//
//如果模块不带触摸功能或者带有触摸功能，但是不需要触摸功能，则不需要进行触摸屏接线
//	   LCD模块                    STM32连接引脚 
//      PEN           --->            PB1             //触摸屏触摸中断信号
//      MISO          --->            PB2             //触摸屏SPI总线读信号
//      MOSI          --->            PC4             //触摸屏SPI总线写信号
//      T_CS          --->            PC13            //触摸屏片选控制信号
//      CLK           --->            PB0             //触摸屏SPI总线时钟信号
*************************************************************************************/	
 /* @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, QD electronic SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*************************************************************************************/		
#include "lcd_nt35510.h"
#include "lprintf.h"
#include "stm32f4xx_hal.h"

	   
void lcd_clr_window(u16 color, u16 xs, u16 ys, u16 xe, u16 ye);
//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;

//画笔颜色,背景颜色
u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;  
u16 DeviceCode;	 

#define US_CT 80
void delay_us(int t)
{
    while(t--){
        volatile int tn=US_CT;
        while(tn--);
    }
}
void delay_ms(int t)
{
    while(t--){
        delay_us(1000);
    }
}


u16 LCD_read(void)
{
	vu16 data;  //防止被优化
	data=LCD->LCD_RAM;	
	return data;
}

/*****************************************************************************
 * @name       :void LCD_WR_REG(u16 data)
 * @date       :2018-08-09 
 * @function   :Write an 16-bit command to the LCD screen
 * @parameters :data:Command value to be written
 * @retvalue   :None
******************************************************************************/
void LCD_WR_REG(vu16 data)
{ 
	data=data;  //使用-O2优化的时候,必须插入的延时
	#if LCD_USE8BIT_MODEL	
	 LCD->LCD_REG = data>>8;
	 LCD->LCD_REG = data;   
#else
	 LCD->LCD_REG = data;  
#endif	
}

/*****************************************************************************
 * @name       :void LCD_WR_DATA(u16 data)
 * @date       :2018-08-09 
 * @function   :Write an 16-bit data to the LCD screen
 * @parameters :data:data value to be written
 * @retvalue   :None
******************************************************************************/
void LCD_WR_DATA(vu16 data)
{
	data=data;			//使用-O2优化的时候,必须插入的延时
#if LCD_USE8BIT_MODEL
	LCD->LCD_RAM=0x00;
	LCD->LCD_RAM=data;
#else
	LCD->LCD_RAM=data;
#endif
}

/*****************************************************************************
 * @name       :u16 LCD_RD_DATA(void)
 * @date       :2018-11-13 
 * @function   :Read an 16-bit value from the LCD screen
 * @parameters :None
 * @retvalue   :read value
******************************************************************************/
u16 LCD_RD_DATA(void)
{
	return LCD_read();
}

/*****************************************************************************
 * @name       :void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue)
 * @date       :2018-08-09 
 * @function   :Write data into registers
 * @parameters :LCD_Reg:Register address
                LCD_RegValue:Data to be written
 * @retvalue   :None
******************************************************************************/
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue)
{	
#if LCD_USE8BIT_MODEL
	LCD->LCD_REG = LCD_Reg>>8;
	LCD->LCD_REG = LCD_Reg;
	LCD->LCD_RAM = 0x00;
	LCD->LCD_RAM = LCD_RegValue;
#else
	LCD->LCD_REG = LCD_Reg;		//写入要写的寄存器序号
	LCD->LCD_RAM = LCD_RegValue;//写入数据	     		
#endif	
}	   

/*****************************************************************************
 * @name       :void LCD_ReadReg(u16 LCD_Reg,u16 *Rval,int n)
 * @date       :2018-11-13 
 * @function   :read value from specially registers
 * @parameters :LCD_Reg:Register address
 * @retvalue   :read value
******************************************************************************/
void LCD_ReadReg(u16 LCD_Reg,u16 *Rval,int n)
{
	LCD_WR_REG(LCD_Reg); 
	while(n--)
	{	
#if LCD_USE8BIT_MODEL
		*(Rval++) = ((LCD_RD_DATA()<<8)|(LCD_RD_DATA()&0xFF));
#else	
		*(Rval++) = LCD_RD_DATA();
#endif
		delay_us(100);
	}
}

/*****************************************************************************
 * @name       :void LCD_WriteRAM_Prepare(void)
 * @date       :2018-08-09 
 * @function   :Write GRAM
 * @parameters :None
 * @retvalue   :None
******************************************************************************/	 
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);
}	 

/*****************************************************************************
 * @name       :void LCD_ReadRAM_Prepare(void)
 * @date       :2018-11-13 
 * @function   :Read GRAM
 * @parameters :None
 * @retvalue   :None
******************************************************************************/	 
void LCD_ReadRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.rramcmd);
}

/*****************************************************************************
 * @name       :void Lcd_WriteData_16Bit(u16 Data)
 * @date       :2018-08-09 
 * @function   :Write an 16-bit command to the LCD screen
 * @parameters :Data:Data to be written
 * @retvalue   :None
******************************************************************************/	 
void Lcd_WriteData_16Bit(u16 Data)
{	
#if LCD_USE8BIT_MODEL	
	 LCD->LCD_RAM = Data>>8;
	 LCD->LCD_RAM = Data;    //写十六位颜色值
#else
	 LCD->LCD_RAM = Data;  //写十六位颜色值
#endif
}

u16 Color_To_565(u8 r, u8 g, u8 b)
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}

/*****************************************************************************
 * @name       :u16 Lcd_ReadData_16Bit(void)
 * @date       :2018-11-13 
 * @function   :Read an 16-bit value from the LCD screen
 * @parameters :None
 * @retvalue   :read value
******************************************************************************/	
u16 Lcd_ReadData_16Bit(void)
{
	u16 r,g,b;
	//dummy data
	r = LCD_RD_DATA();
	delay_us(1);//延时1us
	//8bit:red data	
	//16bit:red and green data
	r = LCD_RD_DATA();
	delay_us(1);//延时1us	
	//8bit:green data
	//16bit:blue data
	g = LCD_RD_DATA();
#if LCD_USE8BIT_MODEL
	delay_us(1);//延时1us	
	//8bit:blue data
	b = LCD_RD_DATA();
#else
	b = g>>8;
	g = r&0xFF; 
	r = r>>8;
#endif
	return Color_To_565(r, g, b);
}

/*****************************************************************************
 * @name       :void LCD_DrawPoint(u16 x,u16 y)
 * @date       :2018-08-09 
 * @function   :Write a pixel data at a specified location
 * @parameters :x:the x coordinate of the pixel
                y:the y coordinate of the pixel
 * @retvalue   :None
******************************************************************************/	
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);//设置光标位置 
	Lcd_WriteData_16Bit(POINT_COLOR); 
}

/*****************************************************************************
 * @name       :u16 LCD_ReadPoint(u16 x,u16 y)
 * @date       :2018-11-13 
 * @function   :Read a pixel color value at a specified location
 * @parameters :x:the x coordinate of the pixel
                y:the y coordinate of the pixel
 * @retvalue   :the read color value
******************************************************************************/	
u16 LCD_ReadPoint(u16 x,u16 y)
{
	u16 color;
	if(x>=lcddev.width||y>=lcddev.height)
	{
		return 0;	//超过了范围,直接返回	
	}
	LCD_SetCursor(x,y);//设置光标位置 
	LCD_ReadRAM_Prepare();
	color = Lcd_ReadData_16Bit();
	return color;
}

/*****************************************************************************
 * @name       :void LCD_Clear(u16 Color)
 * @date       :2018-08-09 
 * @function   :Full screen filled LCD screen
 * @parameters :color:Filled color
 * @retvalue   :None
******************************************************************************/	
void LCD_Clear(u16 Color)
{
  u32 lcd_clear_ms=HAL_GetTick();
#if 0
	u32 total_point=lcddev.width*lcddev.height;
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);   
	for(i=0;i<total_point;i++)
	{ 
#if LCD_USE8BIT_MODEL	
	 LCD->LCD_RAM = Color>>8;
	 LCD->LCD_RAM = Color;    //写十六位颜色值
#else
		LCD->LCD_RAM = Color;
#endif
	}
#endif
	lcd_clr_window(Color, 0,0,lcddev.width-1,lcddev.height-1);
    lcd_clear_ms=HAL_GetTick()-lcd_clear_ms;
    prt_dec(lcd_clear_ms);
} 

/*****************************************************************************
 * @name       :void LCD_GPIOInit(void)
 * @date       :2018-08-09 
 * @function   :Initialization LCD screen GPIO
 * @parameters :None
 * @retvalue   :None
******************************************************************************/	
void LCD_GPIOInit(void)
{
#if 0
	GPIO_InitTypeDef  GPIO_InitStructure;
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 
	FSMC_NORSRAMTimingInitTypeDef  writeTiming;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE, ENABLE);//使能PD,PE,PF,PG时钟  
  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);//使能FSMC时钟  
	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;//PB15 推挽输出,控制背光
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化 //PB15 推挽输出,控制背光
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;//PD7 推挽输出,复位控制
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化 
	
  GPIO_InitStructure.GPIO_Pin = (3<<0)|(3<<4)|(7<<8)|(3<<14);//PD0,1,4,5,8,9,10,14,15 AF OUT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化  
	
  GPIO_InitStructure.GPIO_Pin = (0X1FF<<7);//PE7~15,AF OUT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化  

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_11;//PF12,FSMC_A6
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化  


  GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);//PD0,AF12
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);//PD1,AF12
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC); 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC); 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);//PD15,AF12
 
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);//PE7,AF12
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);//PE15,AF12
 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource7,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource11,GPIO_AF_FSMC);


  readWriteTiming.FSMC_AddressSetupTime = 0XF;	 //地址建立时间（ADDSET）为16个HCLK 1/168M=6ns*16=96ns	
  readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（ADDHLD）模式A未用到	
  readWriteTiming.FSMC_DataSetupTime = 60;			//数据保存时间为60个HCLK	=6*60=360ns
  readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
  readWriteTiming.FSMC_CLKDivision = 0x00;
  readWriteTiming.FSMC_DataLatency = 0x00;
  readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
    

	writeTiming.FSMC_AddressSetupTime =9;	      //地址建立时间（ADDSET）为9个HCLK =54ns 
  writeTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A		
  writeTiming.FSMC_DataSetupTime = 8;		 //数据保存时间为6ns*9个HCLK=54ns
  writeTiming.FSMC_BusTurnAroundDuration = 0x00;
  writeTiming.FSMC_CLKDivision = 0x00;
  writeTiming.FSMC_DataLatency = 0x00;
  writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 

 
  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;//  这里我们使用NE4 ，也就对应BTCR[6],[7]。
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; // 不复用数据地址
  FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
#if LCD_USE8BIT_MODEL 	
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;//存储器数据宽度为8bit
#else
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16bit   
#endif  
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//  存储器写使能
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // 读写使用不同的时序
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //读写时序
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //写时序

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //初始化FSMC配置

  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  // 使能BANK1 
	
	//重新配置写时序控制寄存器的时序使WR时序为最快   	 							    
	FSMC_Bank1E->BWTR[0]&=~(0XF<<0);//地址建立时间(ADDSET)清零 	 
	FSMC_Bank1E->BWTR[0]&=~(0XF<<8);//数据保存时间清零
	FSMC_Bank1E->BWTR[0]|=3<<0;		//地址建立时间(ADDSET)为4个HCLK =24ns  	 
	FSMC_Bank1E->BWTR[0]|=2<<8; 	//数据保存时间(DATAST)为6ns*4个HCLK=24ns
#endif
}

/*****************************************************************************
 * @name       :void LCD_RESET(void)
 * @date       :2018-08-09 
 * @function   :Reset LCD screen
 * @parameters :None
 * @retvalue   :None
******************************************************************************/	
void LCD_RESET(void)
{
#if 0
	LCD_RST=0;
	delay_ms(100);	
	LCD_RST=1;
	delay_ms(50);
#endif
}

/*****************************************************************************
 * @name       :void LCD_Init(void)
 * @date       :2018-08-09 
 * @function   :Initialization LCD screen
 * @parameters :None
 * @retvalue   :None
******************************************************************************/	 	 
void LCD_Init(void)
{  
	#if LCD_USE8BIT_MODEL	
	u8 init_flag=0;
	#endif
	LCD_GPIOInit();//LCD GPIO初始化	
	#if LCD_USE8BIT_MODEL		
Init:
	#endif
	delay_ms(50); //必须加延时，不然reset后屏底部会出现横条纹
	//LCD_RESET();    //如果不使用开发板复位引脚，则调用此复位函数
//************* NT35510初始化**********//	
	LCD_WR_REG(0xF000); LCD_WR_DATA(0x55);
		LCD_WR_REG(0xF001); LCD_WR_DATA(0xAA);
		LCD_WR_REG(0xF002); LCD_WR_DATA(0x52);
		LCD_WR_REG(0xF003); LCD_WR_DATA(0x08);
		LCD_WR_REG(0xF004); LCD_WR_DATA(0x01);
		//# AVDD: manual); LCD_WR_DATA(
		LCD_WR_REG(0xB600); LCD_WR_DATA(0x34);
		LCD_WR_REG(0xB601); LCD_WR_DATA(0x34);
		LCD_WR_REG(0xB602); LCD_WR_DATA(0x34);

		LCD_WR_REG(0xB000); LCD_WR_DATA(0x0D);//09
		LCD_WR_REG(0xB001); LCD_WR_DATA(0x0D);
		LCD_WR_REG(0xB002); LCD_WR_DATA(0x0D);
		//# AVEE: manual); LCD_WR_DATA( -6V
		LCD_WR_REG(0xB700); LCD_WR_DATA(0x24);
		LCD_WR_REG(0xB701); LCD_WR_DATA(0x24);
		LCD_WR_REG(0xB702); LCD_WR_DATA(0x24);

		LCD_WR_REG(0xB100); LCD_WR_DATA(0x0D);
		LCD_WR_REG(0xB101); LCD_WR_DATA(0x0D);
		LCD_WR_REG(0xB102); LCD_WR_DATA(0x0D);
		//#Power Control for
		//VCL
		LCD_WR_REG(0xB800); LCD_WR_DATA(0x24);
		LCD_WR_REG(0xB801); LCD_WR_DATA(0x24);
		LCD_WR_REG(0xB802); LCD_WR_DATA(0x24);

		LCD_WR_REG(0xB200); LCD_WR_DATA(0x00);

		//# VGH: Clamp Enable); LCD_WR_DATA(
		LCD_WR_REG(0xB900); LCD_WR_DATA(0x24);
		LCD_WR_REG(0xB901); LCD_WR_DATA(0x24);
		LCD_WR_REG(0xB902); LCD_WR_DATA(0x24);

		LCD_WR_REG(0xB300); LCD_WR_DATA(0x05);
		LCD_WR_REG(0xB301); LCD_WR_DATA(0x05);
		LCD_WR_REG(0xB302); LCD_WR_DATA(0x05);

		///LCD_WR_REG(0xBF00); LCD_WR_DATA(0x01);

		//# VGL(LVGL):
		LCD_WR_REG(0xBA00); LCD_WR_DATA(0x34);
		LCD_WR_REG(0xBA01); LCD_WR_DATA(0x34);
		LCD_WR_REG(0xBA02); LCD_WR_DATA(0x34);
		//# VGL_REG(VGLO)
		LCD_WR_REG(0xB500); LCD_WR_DATA(0x0B);
		LCD_WR_REG(0xB501); LCD_WR_DATA(0x0B);
		LCD_WR_REG(0xB502); LCD_WR_DATA(0x0B);
		//# VGMP/VGSP:
		LCD_WR_REG(0xBC00); LCD_WR_DATA(0X00);
		LCD_WR_REG(0xBC01); LCD_WR_DATA(0xA3);
		LCD_WR_REG(0xBC02); LCD_WR_DATA(0X00);
		//# VGMN/VGSN
		LCD_WR_REG(0xBD00); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xBD01); LCD_WR_DATA(0xA3);
		LCD_WR_REG(0xBD02); LCD_WR_DATA(0x00);
		//# VCOM=-0.1
		LCD_WR_REG(0xBE00); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xBE01); LCD_WR_DATA(0x63);//4f
			//  VCOMH+0x01;
		//#R+
		LCD_WR_REG(0xD100); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD101); LCD_WR_DATA(0x37);
		LCD_WR_REG(0xD102); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD103); LCD_WR_DATA(0x52);
		LCD_WR_REG(0xD104); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD105); LCD_WR_DATA(0x7B);
		LCD_WR_REG(0xD106); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD107); LCD_WR_DATA(0x99);
		LCD_WR_REG(0xD108); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD109); LCD_WR_DATA(0xB1);
		LCD_WR_REG(0xD10A); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD10B); LCD_WR_DATA(0xD2);
		LCD_WR_REG(0xD10C); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD10D); LCD_WR_DATA(0xF6);
		LCD_WR_REG(0xD10E); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD10F); LCD_WR_DATA(0x27);
		LCD_WR_REG(0xD110); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD111); LCD_WR_DATA(0x4E);
		LCD_WR_REG(0xD112); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD113); LCD_WR_DATA(0x8C);
		LCD_WR_REG(0xD114); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD115); LCD_WR_DATA(0xBE);
		LCD_WR_REG(0xD116); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD117); LCD_WR_DATA(0x0B);
		LCD_WR_REG(0xD118); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD119); LCD_WR_DATA(0x48);
		LCD_WR_REG(0xD11A); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD11B); LCD_WR_DATA(0x4A);
		LCD_WR_REG(0xD11C); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD11D); LCD_WR_DATA(0x7E);
		LCD_WR_REG(0xD11E); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD11F); LCD_WR_DATA(0xBC);
		LCD_WR_REG(0xD120); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD121); LCD_WR_DATA(0xE1);
		LCD_WR_REG(0xD122); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD123); LCD_WR_DATA(0x10);
		LCD_WR_REG(0xD124); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD125); LCD_WR_DATA(0x31);
		LCD_WR_REG(0xD126); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD127); LCD_WR_DATA(0x5A);
		LCD_WR_REG(0xD128); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD129); LCD_WR_DATA(0x73);
		LCD_WR_REG(0xD12A); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD12B); LCD_WR_DATA(0x94);
		LCD_WR_REG(0xD12C); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD12D); LCD_WR_DATA(0x9F);
		LCD_WR_REG(0xD12E); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD12F); LCD_WR_DATA(0xB3);
		LCD_WR_REG(0xD130); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD131); LCD_WR_DATA(0xB9);
		LCD_WR_REG(0xD132); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD133); LCD_WR_DATA(0xC1);
		//#G+
		LCD_WR_REG(0xD200); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD201); LCD_WR_DATA(0x37);
		LCD_WR_REG(0xD202); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD203); LCD_WR_DATA(0x52);
		LCD_WR_REG(0xD204); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD205); LCD_WR_DATA(0x7B);
		LCD_WR_REG(0xD206); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD207); LCD_WR_DATA(0x99);
		LCD_WR_REG(0xD208); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD209); LCD_WR_DATA(0xB1);
		LCD_WR_REG(0xD20A); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD20B); LCD_WR_DATA(0xD2);
		LCD_WR_REG(0xD20C); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD20D); LCD_WR_DATA(0xF6);
		LCD_WR_REG(0xD20E); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD20F); LCD_WR_DATA(0x27);
		LCD_WR_REG(0xD210); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD211); LCD_WR_DATA(0x4E);
		LCD_WR_REG(0xD212); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD213); LCD_WR_DATA(0x8C);
		LCD_WR_REG(0xD214); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD215); LCD_WR_DATA(0xBE);
		LCD_WR_REG(0xD216); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD217); LCD_WR_DATA(0x0B);
		LCD_WR_REG(0xD218); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD219); LCD_WR_DATA(0x48);
		LCD_WR_REG(0xD21A); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD21B); LCD_WR_DATA(0x4A);
		LCD_WR_REG(0xD21C); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD21D); LCD_WR_DATA(0x7E);
		LCD_WR_REG(0xD21E); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD21F); LCD_WR_DATA(0xBC);
		LCD_WR_REG(0xD220); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD221); LCD_WR_DATA(0xE1);
		LCD_WR_REG(0xD222); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD223); LCD_WR_DATA(0x10);
		LCD_WR_REG(0xD224); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD225); LCD_WR_DATA(0x31);
		LCD_WR_REG(0xD226); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD227); LCD_WR_DATA(0x5A);
		LCD_WR_REG(0xD228); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD229); LCD_WR_DATA(0x73);
		LCD_WR_REG(0xD22A); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD22B); LCD_WR_DATA(0x94);
		LCD_WR_REG(0xD22C); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD22D); LCD_WR_DATA(0x9F);
		LCD_WR_REG(0xD22E); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD22F); LCD_WR_DATA(0xB3);
		LCD_WR_REG(0xD230); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD231); LCD_WR_DATA(0xB9);
		LCD_WR_REG(0xD232); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD233); LCD_WR_DATA(0xC1);
		//#B+
		LCD_WR_REG(0xD300); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD301); LCD_WR_DATA(0x37);
		LCD_WR_REG(0xD302); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD303); LCD_WR_DATA(0x52);
		LCD_WR_REG(0xD304); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD305); LCD_WR_DATA(0x7B);
		LCD_WR_REG(0xD306); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD307); LCD_WR_DATA(0x99);
		LCD_WR_REG(0xD308); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD309); LCD_WR_DATA(0xB1);
		LCD_WR_REG(0xD30A); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD30B); LCD_WR_DATA(0xD2);
		LCD_WR_REG(0xD30C); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD30D); LCD_WR_DATA(0xF6);
		LCD_WR_REG(0xD30E); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD30F); LCD_WR_DATA(0x27);
		LCD_WR_REG(0xD310); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD311); LCD_WR_DATA(0x4E);
		LCD_WR_REG(0xD312); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD313); LCD_WR_DATA(0x8C);
		LCD_WR_REG(0xD314); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD315); LCD_WR_DATA(0xBE);
		LCD_WR_REG(0xD316); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD317); LCD_WR_DATA(0x0B);
		LCD_WR_REG(0xD318); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD319); LCD_WR_DATA(0x48);
		LCD_WR_REG(0xD31A); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD31B); LCD_WR_DATA(0x4A);
		LCD_WR_REG(0xD31C); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD31D); LCD_WR_DATA(0x7E);
		LCD_WR_REG(0xD31E); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD31F); LCD_WR_DATA(0xBC);
		LCD_WR_REG(0xD320); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD321); LCD_WR_DATA(0xE1);
		LCD_WR_REG(0xD322); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD323); LCD_WR_DATA(0x10);
		LCD_WR_REG(0xD324); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD325); LCD_WR_DATA(0x31);
		LCD_WR_REG(0xD326); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD327); LCD_WR_DATA(0x5A);
		LCD_WR_REG(0xD328); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD329); LCD_WR_DATA(0x73);
		LCD_WR_REG(0xD32A); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD32B); LCD_WR_DATA(0x94);
		LCD_WR_REG(0xD32C); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD32D); LCD_WR_DATA(0x9F);
		LCD_WR_REG(0xD32E); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD32F); LCD_WR_DATA(0xB3);
		LCD_WR_REG(0xD330); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD331); LCD_WR_DATA(0xB9);
		LCD_WR_REG(0xD332); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD333); LCD_WR_DATA(0xC1);

		//#R-///////////////////////////////////////////
		LCD_WR_REG(0xD400); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD401); LCD_WR_DATA(0x37);
		LCD_WR_REG(0xD402); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD403); LCD_WR_DATA(0x52);
		LCD_WR_REG(0xD404); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD405); LCD_WR_DATA(0x7B);
		LCD_WR_REG(0xD406); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD407); LCD_WR_DATA(0x99);
		LCD_WR_REG(0xD408); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD409); LCD_WR_DATA(0xB1);
		LCD_WR_REG(0xD40A); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD40B); LCD_WR_DATA(0xD2);
		LCD_WR_REG(0xD40C); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD40D); LCD_WR_DATA(0xF6);
		LCD_WR_REG(0xD40E); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD40F); LCD_WR_DATA(0x27);
		LCD_WR_REG(0xD410); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD411); LCD_WR_DATA(0x4E);
		LCD_WR_REG(0xD412); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD413); LCD_WR_DATA(0x8C);
		LCD_WR_REG(0xD414); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD415); LCD_WR_DATA(0xBE);
		LCD_WR_REG(0xD416); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD417); LCD_WR_DATA(0x0B);
		LCD_WR_REG(0xD418); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD419); LCD_WR_DATA(0x48);
		LCD_WR_REG(0xD41A); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD41B); LCD_WR_DATA(0x4A);
		LCD_WR_REG(0xD41C); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD41D); LCD_WR_DATA(0x7E);
		LCD_WR_REG(0xD41E); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD41F); LCD_WR_DATA(0xBC);
		LCD_WR_REG(0xD420); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD421); LCD_WR_DATA(0xE1);
		LCD_WR_REG(0xD422); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD423); LCD_WR_DATA(0x10);
		LCD_WR_REG(0xD424); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD425); LCD_WR_DATA(0x31);
		LCD_WR_REG(0xD426); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD427); LCD_WR_DATA(0x5A);
		LCD_WR_REG(0xD428); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD429); LCD_WR_DATA(0x73);
		LCD_WR_REG(0xD42A); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD42B); LCD_WR_DATA(0x94);
		LCD_WR_REG(0xD42C); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD42D); LCD_WR_DATA(0x9F);
		LCD_WR_REG(0xD42E); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD42F); LCD_WR_DATA(0xB3);
		LCD_WR_REG(0xD430); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD431); LCD_WR_DATA(0xB9);
		LCD_WR_REG(0xD432); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD433); LCD_WR_DATA(0xC1);

		//#G-//////////////////////////////////////////////
		LCD_WR_REG(0xD500); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD501); LCD_WR_DATA(0x37);
		LCD_WR_REG(0xD502); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD503); LCD_WR_DATA(0x52);
		LCD_WR_REG(0xD504); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD505); LCD_WR_DATA(0x7B);
		LCD_WR_REG(0xD506); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD507); LCD_WR_DATA(0x99);
		LCD_WR_REG(0xD508); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD509); LCD_WR_DATA(0xB1);
		LCD_WR_REG(0xD50A); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD50B); LCD_WR_DATA(0xD2);
		LCD_WR_REG(0xD50C); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD50D); LCD_WR_DATA(0xF6);
		LCD_WR_REG(0xD50E); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD50F); LCD_WR_DATA(0x27);
		LCD_WR_REG(0xD510); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD511); LCD_WR_DATA(0x4E);
		LCD_WR_REG(0xD512); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD513); LCD_WR_DATA(0x8C);
		LCD_WR_REG(0xD514); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD515); LCD_WR_DATA(0xBE);
		LCD_WR_REG(0xD516); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD517); LCD_WR_DATA(0x0B);
		LCD_WR_REG(0xD518); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD519); LCD_WR_DATA(0x48);
		LCD_WR_REG(0xD51A); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD51B); LCD_WR_DATA(0x4A);
		LCD_WR_REG(0xD51C); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD51D); LCD_WR_DATA(0x7E);
		LCD_WR_REG(0xD51E); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD51F); LCD_WR_DATA(0xBC);
		LCD_WR_REG(0xD520); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD521); LCD_WR_DATA(0xE1);
		LCD_WR_REG(0xD522); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD523); LCD_WR_DATA(0x10);
		LCD_WR_REG(0xD524); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD525); LCD_WR_DATA(0x31);
		LCD_WR_REG(0xD526); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD527); LCD_WR_DATA(0x5A);
		LCD_WR_REG(0xD528); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD529); LCD_WR_DATA(0x73);
		LCD_WR_REG(0xD52A); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD52B); LCD_WR_DATA(0x94);
		LCD_WR_REG(0xD52C); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD52D); LCD_WR_DATA(0x9F);
		LCD_WR_REG(0xD52E); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD52F); LCD_WR_DATA(0xB3);
		LCD_WR_REG(0xD530); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD531); LCD_WR_DATA(0xB9);
		LCD_WR_REG(0xD532); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD533); LCD_WR_DATA(0xC1);
		//#B-///////////////////////////////
		LCD_WR_REG(0xD600); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD601); LCD_WR_DATA(0x37);
		LCD_WR_REG(0xD602); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD603); LCD_WR_DATA(0x52);
		LCD_WR_REG(0xD604); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD605); LCD_WR_DATA(0x7B);
		LCD_WR_REG(0xD606); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD607); LCD_WR_DATA(0x99);
		LCD_WR_REG(0xD608); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD609); LCD_WR_DATA(0xB1);
		LCD_WR_REG(0xD60A); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD60B); LCD_WR_DATA(0xD2);
		LCD_WR_REG(0xD60C); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xD60D); LCD_WR_DATA(0xF6);
		LCD_WR_REG(0xD60E); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD60F); LCD_WR_DATA(0x27);
		LCD_WR_REG(0xD610); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD611); LCD_WR_DATA(0x4E);
		LCD_WR_REG(0xD612); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD613); LCD_WR_DATA(0x8C);
		LCD_WR_REG(0xD614); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xD615); LCD_WR_DATA(0xBE);
		LCD_WR_REG(0xD616); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD617); LCD_WR_DATA(0x0B);
		LCD_WR_REG(0xD618); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD619); LCD_WR_DATA(0x48);
		LCD_WR_REG(0xD61A); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD61B); LCD_WR_DATA(0x4A);
		LCD_WR_REG(0xD61C); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD61D); LCD_WR_DATA(0x7E);
		LCD_WR_REG(0xD61E); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD61F); LCD_WR_DATA(0xBC);
		LCD_WR_REG(0xD620); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xD621); LCD_WR_DATA(0xE1);
		LCD_WR_REG(0xD622); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD623); LCD_WR_DATA(0x10);
		LCD_WR_REG(0xD624); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD625); LCD_WR_DATA(0x31);
		LCD_WR_REG(0xD626); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD627); LCD_WR_DATA(0x5A);
		LCD_WR_REG(0xD628); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD629); LCD_WR_DATA(0x73);
		LCD_WR_REG(0xD62A); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD62B); LCD_WR_DATA(0x94);
		LCD_WR_REG(0xD62C); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD62D); LCD_WR_DATA(0x9F);
		LCD_WR_REG(0xD62E); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD62F); LCD_WR_DATA(0xB3);
		LCD_WR_REG(0xD630); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD631); LCD_WR_DATA(0xB9);
		LCD_WR_REG(0xD632); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xD633); LCD_WR_DATA(0xC1);



		//#Enable Page0
		LCD_WR_REG(0xF000); LCD_WR_DATA(0x55);
		LCD_WR_REG(0xF001); LCD_WR_DATA(0xAA);
		LCD_WR_REG(0xF002); LCD_WR_DATA(0x52);
		LCD_WR_REG(0xF003); LCD_WR_DATA(0x08);
		LCD_WR_REG(0xF004); LCD_WR_DATA(0x00);
		//# RGB I/F Setting
		LCD_WR_REG(0xB000); LCD_WR_DATA(0x08);
		LCD_WR_REG(0xB001); LCD_WR_DATA(0x05);
		LCD_WR_REG(0xB002); LCD_WR_DATA(0x02);
		LCD_WR_REG(0xB003); LCD_WR_DATA(0x05);
		LCD_WR_REG(0xB004); LCD_WR_DATA(0x02);
		//## SDT:
		LCD_WR_REG(0xB600); LCD_WR_DATA(0x08);
		LCD_WR_REG(0xB500); LCD_WR_DATA(0x50);//0x6b ???? 480x854       0x50 ???? 480x800

		//## Gate EQ:
		LCD_WR_REG(0xB700); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xB701); LCD_WR_DATA(0x00);

		//## Source EQ:
		LCD_WR_REG(0xB800); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xB801); LCD_WR_DATA(0x05);
		LCD_WR_REG(0xB802); LCD_WR_DATA(0x05);
		LCD_WR_REG(0xB803); LCD_WR_DATA(0x05);

		//# Inversion: Column inversion (NVT)
		LCD_WR_REG(0xBC00); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xBC01); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xBC02); LCD_WR_DATA(0x00);

		//# BOE's Setting(default)
		LCD_WR_REG(0xCC00); LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCC01); LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC02); LCD_WR_DATA(0x00);

		//# Display Timing:
		LCD_WR_REG(0xBD00); LCD_WR_DATA(0x01);
		LCD_WR_REG(0xBD01); LCD_WR_DATA(0x84);
		LCD_WR_REG(0xBD02); LCD_WR_DATA(0x07);
		LCD_WR_REG(0xBD03); LCD_WR_DATA(0x31);
		LCD_WR_REG(0xBD04); LCD_WR_DATA(0x00);

		LCD_WR_REG(0xBA00); LCD_WR_DATA(0x01);

		LCD_WR_REG(0xFF00); LCD_WR_DATA(0xAA);
		LCD_WR_REG(0xFF01); LCD_WR_DATA(0x55);
		LCD_WR_REG(0xFF02); LCD_WR_DATA(0x25);
		LCD_WR_REG(0xFF03); LCD_WR_DATA(0x01);

		LCD_WR_REG(0x3500); LCD_WR_DATA(0x00);
		LCD_WR_REG(0x3600); LCD_WR_DATA(0x00);
		LCD_WR_REG(0x3a00); LCD_WR_DATA(0x55);  ////55=16?/////66=18?
		LCD_WR_REG(0x1100);
		delay_ms(120); 
		LCD_WR_REG(0x2900 ); 
		LCD_WR_REG(0x2c00);
	#if LCD_USE8BIT_MODEL	 //8位模式下，LCD初始化两遍，防止reset或者断电重启后，显示异常
	if(init_flag==0)
	{
		init_flag=1;
		goto Init;
	}
	#endif
  LCD_direction(1);//设置LCD显示方向

	LCD_LED_ON;//点亮背光	 
	LCD_Clear(WHITE);//清全屏白色
}
 
/*****************************************************************************
 * @name       :void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
 * @date       :2018-08-09 
 * @function   :Setting LCD display window
 * @parameters :xStar:the bebinning x coordinate of the LCD display window
								yStar:the bebinning y coordinate of the LCD display window
								xEnd:the endning x coordinate of the LCD display window
								yEnd:the endning y coordinate of the LCD display window
 * @retvalue   :None
******************************************************************************/ 
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
{	
	LCD_WR_REG(lcddev.setxcmd);LCD_WR_DATA(xStar>>8);  
	LCD_WR_REG(lcddev.setxcmd+1);LCD_WR_DATA(xStar&0XFF);	  
	LCD_WR_REG(lcddev.setxcmd+2);LCD_WR_DATA(xEnd>>8);   
	LCD_WR_REG(lcddev.setxcmd+3);LCD_WR_DATA(xEnd&0XFF);   
	LCD_WR_REG(lcddev.setycmd);LCD_WR_DATA(yStar>>8);   
	LCD_WR_REG(lcddev.setycmd+1);LCD_WR_DATA(yStar&0XFF);  
	LCD_WR_REG(lcddev.setycmd+2);LCD_WR_DATA(yEnd>>8);   
	LCD_WR_REG(lcddev.setycmd+3);LCD_WR_DATA(yEnd&0XFF); 

	LCD_WriteRAM_Prepare();	//开始写入GRAM			
}   

void color16_lcd(u16 color, u32 n);
void lcd_clr_window(u16 color, u16 xs, u16 ys, u16 xe, u16 ye)
{
	LCD_SetWindows(xs,ys,xe,ye);   
    color16_lcd(color, ((unsigned int)xe-xs+1)*(ye-ys+1));
#if 0
	for(u32 i=0;i<total_point;i++)
	{ 
		LCD->LCD_RAM = color;
	}
#endif
}
/*****************************************************************************
 * @name       :void LCD_SetCursor(u16 Xpos, u16 Ypos)
 * @date       :2018-08-09 
 * @function   :Set coordinate value
 * @parameters :Xpos:the  x coordinate of the pixel
								Ypos:the  y coordinate of the pixel
 * @retvalue   :None
******************************************************************************/ 
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	  	    			
	LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);	
} 

/*****************************************************************************
 * @name       :void LCD_direction(u8 direction)
 * @date       :2018-08-09 
 * @function   :Setting the display direction of LCD screen
 * @parameters :direction:0-0 degree
                          1-90 degree
													2-180 degree
													3-270 degree
 * @retvalue   :None
******************************************************************************/ 
void LCD_direction(u8 direction)
{ 
	lcddev.setxcmd=0x2A00;
	lcddev.setycmd=0x2B00;
	lcddev.wramcmd=0x2C00;
	lcddev.rramcmd=0x2E00;
	switch(direction){		  
		case 0:						 	 		
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;		
			LCD_WriteReg(0x3600,0x00);
		break;
		case 1:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
			LCD_WriteReg(0x3600,(1<<5)|(1<<6));
		break;
		case 2:						 	 		
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;	
			LCD_WriteReg(0x3600,(1<<7)|(1<<6));
		break;
		case 3:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
			LCD_WriteReg(0x3600,(1<<7)|(1<<5));
		break;	
		default:break;
	}		
}	 

/*****************************************************************************
 * @name       :u16 LCD_Read_ID(void)
 * @date       :2018-11-13 
 * @function   :Read ID
 * @parameters :None
 * @retvalue   :ID value
******************************************************************************/ 
u16 LCD_Read_ID(void)
{
	u16 val;
	LCD_WR_REG(0xF000);
	LCD_WR_DATA(0x55);
	LCD_WR_REG(0xF001);
	LCD_WR_DATA(0xAA);
	LCD_WR_REG(0xF002);
	LCD_WR_DATA(0x52);
	LCD_WR_REG(0xF003);
	LCD_WR_DATA(0x08);
	LCD_WR_REG(0xF004);
	LCD_WR_DATA(0x01);

    LCD_ReadReg(0xC500,&lcddev.id,1);//读回0X55
	lcddev.id<<=8;
	LCD_ReadReg(0xC501,&val,1);//读回0X10	
	lcddev.id |= val;
	return lcddev.id;
}
