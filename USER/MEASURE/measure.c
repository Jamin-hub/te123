#include "measure.h"
#include "adc.h"

FFT_DATA fft = {
	.inputbuf = {0},       // 全部初始化为0
	.outputbuf = {0},
	.harmonic = {0}, // 同上
	.thd = 0,
	.flag = 0,			   // 存满标志
	.index = 0,			   // FFT采样点计数器
};         

float AC_Vin_verf=2.47f; //基准电压
float AC_Vin_ratio=660.50f; //放大倍数

extern uint32_t adc_buffer[20]; //单次采样
uint32_t *adc_result=adc_buffer;

rms_type AC_rms={0,0};
float AC_sample_mat[COLS][SAMPLE_LEN]={0};

float get_acv_in(void)
{
	float temp = 0;
	uint32_t sum = 0;
	int i;
	for(i=1;i<20;i+=2)
	{
		sum+=adc_result[i];
	}
	temp =(double)sum*(3.3f-0.0f)/4095.0f/ROWS;//计算10个采样点的平均值，并转换为实际电压值
	return temp-1;//AC_Vin_verf)*AC_Vin_ratio;
}

float get_aci_in(void)
{
	float temp=0;
	uint32_t sum=0;
	for(int i=0;i <20;i+=2)
	{
		sum+=adc_result[i];
	}
	temp =(double)sum*(3.3f-0.0f)/4095.0f/ROWS;
	return temp;//-AC_Vin_verf)*AC_Vin_ratio;
	
}

void calculate_rms(void) 
{
	static uint32_t count=0;
	uint32_t temp=count % SAMPLE_LEN/2;
	count++;
	AC_sample_mat[0][temp]= get_acv_in(); 
	AC_sample_mat[1][temp]= get_aci_in(); 
	//存入 FFT 输入缓冲
	if (fft.index < SAMPLE_LEN && fft.flag==0) {
		//float32_t window = 0.5f - 0.5f * arm_cos_f32((2.0f * PI * fft.index) / (SAMPLE_LEN - 1));
		fft.inputbuf[2*fft.index] = AC_sample_mat[0][temp]; 
		fft.inputbuf[2*fft.index+1]=0;//虚部全部为0
		fft.index++;
	}
	else if (fft.index >= SAMPLE_LEN && fft.flag==0) {
		fft.flag = 1;  // 设置标志，准备做FFT处理
		fft.index = 0;
	}
	if(SAMPLE_LEN/2-1==temp)
	{
		count=0;
    arm_rms_f32(AC_sample_mat[0], SAMPLE_LEN/2, &AC_rms.Vin_rms);
		arm_rms_f32(AC_sample_mat[1], SAMPLE_LEN/2, &AC_rms.Iin_rms);
	}

}


int fft_getpeak(float *inputx,float *input,float *output,uint16_t inlen,uint8_t x,uint8_t N,float y) //  intlen 输入数组长度，x寻找长度
{                                                                           
	int i,i2;
	uint32_t idex;  //不同于上一个函数中的，因为他们在不同的函数中被定义
	float datas;
	float sum;
	int outlen=0;
	for(i=0;i<inlen-x;i+=x)
	{
		arm_max_f32(input+i,x,&datas,&idex);   
		if( (input[i+idex]>=input[i+idex+1])&&(input[i+idex]>=input[i+idex-1])&&( (2*datas)/SAMPLE_LEN )>y)   
		   {
			   sum=0;   
			   for(i2=i+idex-N;i2<i+idex+N;i2++)   
			   {
				   sum+=input[i2];          
			   }        
			   if(1.5*sum/(2*N)<datas)       
			   {                                                                                             
				    output[3*outlen+2] = atan2(inputx[2*(i+idex+1)+1],inputx[2*(i+idex+1)])*180/3.1415926f;	//计算相位		   
				    output[3*outlen+1] = 1.0*(2*datas)/SAMPLE_LEN;   //计算幅度
					  output[3*outlen] = (1.0*2500*(i+idex+1)+1.0*2500*(i+idex+2))/2/SAMPLE_LEN;//计算频率
					  outlen++;				   
			   }                                                                                               
         else continue;			   
		   }
			
		else continue;
	}
	return outlen;
}

float compute_thd(float *output, int outlen) {
    float fundamental_amplitude = 0.0f;
    float sum_harmonic_power = 0.0f;

    // 第一个峰值是基波
    if (outlen > 0) {
        fundamental_amplitude = output[1];  // output[3*k+1] = 幅度
    }

    // 累加所有谐波的平方（从第二个峰值开始）
    for (int i = 1; i < outlen; i++) {
        float harmonic_amplitude = output[3 * i + 1];
        sum_harmonic_power += harmonic_amplitude * harmonic_amplitude;
    }

    // 计算THD
    float thd = (sqrtf(sum_harmonic_power) / fundamental_amplitude) * 100.0f;
    return thd;
}

