#include "gt911.h"
#include "gpio.h"
#include "lcd.h"
#include "stdio.h"


/**
 * @brief ��GT911�Ĵ���д������
 * ͨ��I2Cͨ�ţ���GT911��ָ���Ĵ���д��ָ�����ȵ����ݡ�
 * @param _usRegAddr �Ĵ�����ַ
 * @param _pRegBuf ���ݻ�����ָ��
 * @param _ucLen ���ݳ���
 */
void GT911_WriteReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	HAL_I2C_Mem_Write(&GT911_I2C, GT911_IIC_WADDR, _usRegAddr, I2C_MEMADD_SIZE_16BIT, _pRegBuf, _ucLen, 0xff);
}

/**
 * @brief ��ȡGT911�Ĵ���
 * ͨ��I2C�ӿڶ�ȡGT911ָ���Ĵ�����ֵ��������ȡ�������ݱ��浽�������С�
 * @param _usRegAddr �Ĵ�����ַ
 * @param _pRegBuf �洢��ȡ���ݵĻ�����ָ��
 * @param _ucLen ��ȡ���ݵĳ���
 */
void GT911_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	HAL_I2C_Mem_Read(&GT911_I2C, GT911_IIC_RADDR, _usRegAddr, I2C_MEMADD_SIZE_16BIT, _pRegBuf, _ucLen, 0xff);
}


//==================================================================
//Name:						GT911_reset
//Author:					
//Date:						2017-07-06
//Function:				Rest GT911, just reset without sync
//Input:					void
//Output:					void
//option:
//==================================================================
void GT911_reset(void)
{
	HAL_GPIO_WritePin(T_RST_GPIO_Port,T_RST_Pin,GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(T_RST_GPIO_Port,T_RST_Pin,GPIO_PIN_SET);
	HAL_Delay(100);
}

//==================================================================
//Name:						Touch_Init
//Author:					
//Date:						2017-07-06
//Function:				GT911 init, including GPIO init, sync, and version check
//Input:					unsigned int addr				//address of register
//								unsigned char *value		//pointer of data output
//Output:					void
//option:
//==================================================================
uint8_t GT911_Init(void)
{
	uint8_t touchIC_ID[4];	
	GT911_reset();
	GT911_ReadReg(GT911_ID_ADDR,touchIC_ID,4);
	if( touchIC_ID[0] == '9' )
	{
		printf("Touch ID: %s \r\n",touchIC_ID);
		//GT9xx_send_config();
		return 1;
	}
	else
	{
		printf("Touch Error\r\n");
		return 0;
	}
}

uint8_t Touch_Get_Count(void)
{
	uint8_t count[1] = {0};
	GT911_ReadReg (GT911_READ_ADDR,count,1);	//read touch data
	return (count[0]&0x0f);
}

const uint16_t TPX[] = {0x8150,0x8158,0x8160,0x8168,0x8170}; //���������������ݵ�ַ��1~5��

//==================================================================
//Name:						Touch_Get_Data
//Author:					
//Date:						2017-07-06
//Function:				Get GT911 data, such as point and coordinate
//Input:					void
//Output:					void
//option:
//==================================================================
uint8_t GT911_Scan(uint8_t mode)
{
	uint8_t buf[4];
	uint8_t i=0;
	uint8_t res=0;
	uint8_t temp;
	uint8_t tempsta;
 	static uint8_t t=0;//���Ʋ�ѯ���,�Ӷ�����CPUռ����   
	t++;
	if((t%10)==0||t<10)//����ʱ,ÿ����10��CTP_Scan�����ż��1��,�Ӷ���ʡCPUʹ����
	{ 
 		GT911_ReadReg(GT911_READ_ADDR, &mode,1);
		if(mode&0X80&&((mode&0XF)<6))
		{
			temp=0;	
			GT911_WriteReg (GT911_READ_ADDR,&temp,1);
		}		
		if((mode&0XF)&&((mode&0XF)<6))
		{
			temp=0XFF<<(mode&0XF);		//����ĸ���ת��Ϊ1��λ��,ƥ��tp_dev.sta���� 
			tempsta=tp_dev.sta;			//���浱ǰ��tp_dev.staֵ
			tp_dev.sta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES; 
			tp_dev.x[4]=tp_dev.x[0];	//���津��0������
			tp_dev.y[4]=tp_dev.y[0];
			for(i=0;i<5;i++)
			{
				if(tp_dev.sta&(1<<i))	//������Ч?
				{
					GT911_ReadReg(TPX[i],buf,4);	//��ȡXY����ֵ
					if(DFT_SCAN_DIR==U2D_L2R)//����
					{
						tp_dev.y[i]=320-((uint16_t)buf[1]<<8)-buf[0];
						tp_dev.x[i]=(((uint16_t)buf[3]<<8)+buf[2]);
					}
					else if(DFT_SCAN_DIR==R2L_U2D)
					{
						tp_dev.x[i]=((uint16_t)buf[1]<<8)+buf[0];
						tp_dev.y[i]=((uint16_t)buf[3]<<8)+buf[2];
					} 
					else if(DFT_SCAN_DIR==L2R_D2U)
					{
						tp_dev.x[i]=320-(((uint16_t)buf[1]<<8)+buf[0]);
						tp_dev.y[i]=480-(((uint16_t)buf[3]<<8)+buf[2]);
					} 
					else 
					{
						tp_dev.y[i]=((uint16_t)buf[1]<<8)+buf[0];
						tp_dev.x[i]=480-(((uint16_t)buf[3]<<8)+buf[2]);
					}				
				    // printf("x[%d]:%d,y[%d]:%d\r\n",i,tp_dev.x[i],i,tp_dev.y[i]);
				}			
			} 
			res=1;
			if(tp_dev.x[0]>lcddev.width||tp_dev.y[0]>lcddev.height)//�Ƿ�����(���곬����)
			{ 
				if((mode&0XF)>1)		//��������������,�򸴵ڶ�����������ݵ���һ������.
				{
					tp_dev.x[0]=tp_dev.x[1];
					tp_dev.y[0]=tp_dev.y[1];
					t=0;				//����һ��,��������������10��,�Ӷ����������
				}else					//�Ƿ�����,����Դ˴�����(��ԭԭ����)  
				{
					tp_dev.x[0]=tp_dev.x[4];
					tp_dev.y[0]=tp_dev.y[4];
					mode=0X80;		
					tp_dev.sta=tempsta;	//�ָ�tp_dev.sta
				}
			}else t=0;					//����һ��,��������������10��,�Ӷ����������
		}
	}
	if((mode&0X8F)==0X80)//�޴����㰴��
	{ 
		if(tp_dev.sta&TP_PRES_DOWN)	//֮ǰ�Ǳ����µ�
		{
			tp_dev.sta&=~(1<<7);	//��ǰ����ɿ�
		}else						//֮ǰ��û�б�����
		{ 
			tp_dev.x[0]=0xffff;
			tp_dev.y[0]=0xffff;
			tp_dev.sta&=0XE0;	//�������Ч���	
		}	 
	} 	
	if(t>240)t=10;//���´�10��ʼ����
	return res;
}

