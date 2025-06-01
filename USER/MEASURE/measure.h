#ifndef __MEASURE_H__
#define __MEASURE_H__
#include "stm32f4xx.h"
#include "math.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#define COLS 2     //ͨ����
#define ROWS 10    //ÿ��ͨ������
#define SAMPLE_LEN 1024    //rms��������


typedef struct 
{
	float inputbuf[SAMPLE_LEN*2]; //fft����
	float outputbuf[SAMPLE_LEN];  //fft���
	float harmonic[20];
	float thd;
	uint8_t flag;  //������ɱ�־
	uint32_t index;

}FFT_DATA;


typedef struct _AC_RMS  //������Чֵ�ṹ��
{
	float Vin_rms;   //���뽻����ѹ��Чֵ   
	float Iin_rms;  //���뽻��������Чֵ
	float Percentage;
}rms_type;

float get_acv_in(void);
float get_aci_in(void);
void calculate_rms(void);
int fft_getpeak(float *inputx,float *input,float *output,uint16_t inlen,uint8_t x,uint8_t N,float y);
float compute_thd(float *output, int outlen);

#endif
