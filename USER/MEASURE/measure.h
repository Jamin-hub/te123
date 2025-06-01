#ifndef __MEASURE_H__
#define __MEASURE_H__
#include "stm32f4xx.h"
#include "math.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#define COLS 2     //通道数
#define ROWS 10    //每个通道数据
#define SAMPLE_LEN 1024    //rms缓冲数据


typedef struct 
{
	float inputbuf[SAMPLE_LEN*2]; //fft输入
	float outputbuf[SAMPLE_LEN];  //fft输出
	float harmonic[20];
	float thd;
	uint8_t flag;  //计数完成标志
	uint32_t index;

}FFT_DATA;


typedef struct _AC_RMS  //定义有效值结构体
{
	float Vin_rms;   //输入交流电压有效值   
	float Iin_rms;  //输入交流电流有效值
	float Percentage;
}rms_type;

float get_acv_in(void);
float get_aci_in(void);
void calculate_rms(void);
int fft_getpeak(float *inputx,float *input,float *output,uint16_t inlen,uint8_t x,uint8_t N,float y);
float compute_thd(float *output, int outlen);

#endif
