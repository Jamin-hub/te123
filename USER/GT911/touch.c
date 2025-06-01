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

 	 			    					  
//��һ�����(2*2�ĵ�)		   
//x,y:����
//color:��ɫ
void TP_Draw_Big_Point(uint16_t x,uint16_t y,uint16_t color)
{	    
	LCD_DrawPoint(x,y,color);//���ĵ� 
	LCD_DrawPoint(x+1,y,color);
	LCD_DrawPoint(x,y+1,color);
	LCD_DrawPoint(x+1,y+1,color);	 	  	
}


//��������ʼ��  		    
uint8_t TP_Init(void)
{
  	while(!GT911_Init()){
		HAL_Delay(500);
	}

	tp_dev.scan=GT911_Scan;	//ɨ�躯��ָ��GT911������ɨ��
	tp_dev.touchtype|=0X80;	//������  
	return 0;
}

