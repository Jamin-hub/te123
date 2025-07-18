#include "touch.h"
#include "lcd.h"
#include "stdlib.h"
#include "math.h"
#include "stdio.h"

_m_tp_dev tp_dev=
{
	TP_Init,
	GT911_Scan,
	0,
	0, 
	0,
	0,
};					

 	 			    					  
//画一个大点(2*2的点)		   
//x,y:坐标
//color:颜色
void TP_Draw_Big_Point(uint16_t x,uint16_t y,uint16_t color)
{	    
	LCD_DrawPoint(x,y,color);//中心点 
	LCD_DrawPoint(x+1,y,color);
	LCD_DrawPoint(x,y+1,color);
	LCD_DrawPoint(x+1,y+1,color);	 	  	
}


//触摸屏初始化  		    
uint8_t TP_Init(void)
{
  	while(!GT911_Init()){
		HAL_Delay(500);
	}

	tp_dev.scan=GT911_Scan;	//扫描函数指向GT911触摸屏扫描
	tp_dev.touchtype|=0X80;	//电容屏  
	return 0;
}

