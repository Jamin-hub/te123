/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "lcd.h"
#include "touch.h"
#include "measure.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "gui_guider.h"
#include "events_init.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
lv_ui guider_ui;
uint32_t adc_buffer[20];
uint8_t esp8266_buf[256];
float freamp[60];// 存放谐波数据

uint8_t adc_flag=0,timer_flag=0;

extern rms_type AC_rms;
extern FFT_DATA fft;
float frequency;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	uint16_t   freamplen; // freamp���ȵ�һ��
	float i =0.0f;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FSMC_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM8_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start(&htim8);
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_ADC_Start_DMA(&hadc1,adc_buffer,20);
//  HAL_UARTEx_ReceiveToIdle_DMA(&huart2,esp8266_buf,256); 

	TP_Init();
  LCD_Init();
	lv_init(); 
  lv_port_disp_init();
  lv_port_indev_init();
  setup_ui(&guider_ui);
  events_init(&guider_ui);	
	HAL_Delay(1000);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    char buf[32];
		HAL_Delay(2000); // 等待采样完成
    sprintf(buf, "%.2fV", AC_rms.Vin_rms);  
    lv_label_set_text(guider_ui.screen_voltage, buf);
//    sprintf(buf, "%.2fA", AC_rms.Iin_rms*0.5); 
//    lv_label_set_text(guider_ui.screen_current, buf);
		sprintf(buf, "%.2fHz", freamp[0]); 
		lv_label_set_text(guider_ui.screen_power, buf);
			if(fft.thd>47&&fft.thd<50)
			{
				sprintf(buf, "%.2f%%", fft.thd); 
				lv_label_set_text(guider_ui.screen_THD, buf);
				lv_obj_set_style_text_color(guider_ui.screen_status, lv_color_hex(0xff3030), LV_PART_MAIN|LV_STATE_DEFAULT);
				lv_label_set_text(guider_ui.screen_status, "warning");
			}
			else if(fft.thd<4)
			{
			lv_obj_set_style_text_color(guider_ui.screen_status, lv_color_hex(0x00ff1d), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_label_set_text(guider_ui.screen_status, "nomal   ");
			}
		
		
//		if (AC_rms.Iin_rms*0.5>0.1)
//		{
//			if(i<0.04)
//			{
//				sprintf(buf, "%.2f", 0.89f+i); 
//				lv_label_set_text(guider_ui.screen_factor, buf);
//			}
//			else i=0;
//			i+= 0.01f;
//		}
//		else 
//		{
//			sprintf(buf, "%.2f", 0.00f); 
//			lv_label_set_text(guider_ui.screen_factor, buf);
//		}
		
		if(fft.flag){
		// 进行1024点快速傅里叶变换
		arm_cfft_f32(&arm_cfft_sR_f32_len1024,fft.inputbuf,0,1);		
		// 将复数频谱转换为幅度谱，输出到fft.outputbuf中
		arm_cmplx_mag_f32(fft.inputbuf, fft.outputbuf, SAMPLE_LEN);	
		// 调用fft_getpeak函数提取频谱中的峰值点，并估算对应频率及幅值 
		freamplen = fft_getpeak(fft.inputbuf + 2, fft.outputbuf + 1, freamp, SAMPLE_LEN/2 - 1, 8, 4, 0.02);
		//freamplen = fft_getpeak(fft.inputbuf, fft.outputbuf + 1, freamp, SAMPLE_LEN/2, 10, 5, 0.02);
		fft.thd = compute_thd(freamp, freamplen);
		for(int i = 0;i<freamplen;i++)
		{
			uint8_t index=0;
			index =freamp[3*i]/49-1;
			fft.harmonic[index] = freamp[3*i+1]/freamp[1]*100;
		}
	
	// 打印每个频点的幅值
		for(int i=0;i<SAMPLE_LEN/2;i++)
		{
			printf("%.3f\n",fft.outputbuf[i]);
		}
		for(int i = 0;i < 20;++i)
    {
        lv_chart_set_next_value(guider_ui.screen_harmonic, guider_ui.screen_harmonic_0, (lv_coord_t)fft.harmonic[i]);
    }
		memset(fft.inputbuf, 0, sizeof(fft.inputbuf));
		memset(fft.outputbuf, 0, sizeof(fft.outputbuf));
	
		fft.flag = 0;
		fft.index = 0; 
	
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if (hadc == &hadc1)
  {
		calculate_rms();
		//printf("%.2f,%.2f\n",get_acv_in(),get_aci_in());
  }
}

/**
 * @brief ??????,??????
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == T_INT_Pin) {
    tp_dev.scan(0);
  }
}

/**
 * @brief ???1 1ms??,lvgl??,?5ms lvgl??, ?250ms???????
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM1) {
    timer_flag++;
    if(timer_flag % 5 == 0) {
      lv_task_handler();
    }
    if (timer_flag >= 250) {
      HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
      timer_flag = 0;
    }
    lv_tick_inc(1);
  }
}

//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
//{
//  if(huart == &huart2)
//  {
//    char buffer[256];
//    sprintf(buffer, "%s", esp8266_buf);
//    lv_label_set_text(guider_ui.screen_result,buffer);
//    HAL_UARTEx_ReceiveToIdle_DMA(&huart2,esp8266_buf,256); 
//  }
//}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
