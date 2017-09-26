#include "stm32f10x.h"

/**
  * @brief  Sets System clock frequency to 24MHz and configure HCLK, PCLK2 
  *         and PCLK1 prescalers.
  * @note   This function should be used only after reset.
  * @param  None
  * @retval None
  */
void SetSysClockHsiTo24(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;
   
  /* HCLK = SYSCLK */
  RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;
    
  /* PCLK2 = HCLK */
  RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;
  
  /* PCLK1 = HCLK */
  RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1;
  

  /*  PLL configuration:  = (HSI / 2) * 6 = 24 MHz */
  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC /*| RCC_CFGR_PLLXTPRE*/ | RCC_CFGR_PLLMULL));
  RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSI_Div2 /*| RCC_CFGR_PLLXTPRE_HSE_Div2*/ | RCC_CFGR_PLLMULL6);

  /* Enable PLL */
  RCC->CR |= RCC_CR_PLLON;

  /* Wait till PLL is ready */
  while((RCC->CR & RCC_CR_PLLRDY) == 0)
  {
  }

  /* Select PLL as system clock source */
  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
  RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;    

  /* Wait till PLL is used as system clock source */
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08)
  {
  }
};

/**
  * @brief  Инициализация тактирования необходимых периферийных модулей.
  * @param  None
  * @retval None
  */
void RCC_Init(void)
{
	/* Установка тактирования порта А, порта В и С, таймеров 1 и 8, АЦП1, АЦП2, АЦП3 */
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);

	/* Установка тактирования таймера 6 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6 /*RCC_APB1Periph_DAC |  | RCC_APB1Periph_USART2 | RCC_APB1Periph_TIM5*/, ENABLE);
	
#ifdef DEBUG
	/* Выбор источника сигнала на линии MCO */
	RCC_MCOConfig(RCC_MCO_SYSCLK);
#endif
};

/**
  * @brief  Инициализация портов ввода/вывода (GPIO).
  * @param  None
  * @retval None
  */
void GPIOinit(void)
{
	GPIO_InitTypeDef GpioInitStruct;
	
	/* Configure USART1 Rx as input floating */
	GpioInitStruct.GPIO_Pin = GPIO_Pin_10;
	GpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GpioInitStruct);
	
	/* Configure USART1 Tx as alternate function push-pull (PA9) */
	GpioInitStruct.GPIO_Pin = GPIO_Pin_9;
	GpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GpioInitStruct);
	
	/* Initialise pins as inputs. Buttons are connected to them */
	GpioInitStruct.GPIO_Pin = GPIO_Pin_All & (~(GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15));
	GpioInitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GpioInitStruct);
	
	GpioInitStruct.GPIO_Pin = GPIO_Pin_All & (~(GPIO_Pin_2 | GPIO_Pin_3));
	GPIO_Init(GPIOB, &GpioInitStruct);
	
	GpioInitStruct.GPIO_Pin = GPIO_Pin_All & (~GPIO_Pin_12);
	GPIO_Init(GPIOC, &GpioInitStruct);
	
	/* Initialise pins as inputs for detection hardware version */
	GpioInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOD, &GpioInitStruct);
	
#ifdef DEBUG
	/* Вывод системной тактовой частоты PA8 */
	GpioInitStruct.GPIO_Pin = GPIO_Pin_8;
	GpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GpioInitStruct);
	
	GpioInitStruct.GPIO_Pin = GPIO_Pin_2;
	GpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GpioInitStruct);
	GPIOD->BSRR = GPIO_BSRR_BR2;
#endif
};

/**
  * @brief  Инициализация главного USART (USART1 используется для связи с главным контроллером).
  * @param  None
  * @retval None
  */
void USARTInit(void)
{
	USART_InitTypeDef UsartInitStruct;
	
	/* Initialisation of USART init-structure. Use defalt settings exept baud rate, length of word and ParityBit */
	USART_StructInit(&UsartInitStruct);
	UsartInitStruct.USART_BaudRate = 25000;
	UsartInitStruct.USART_WordLength = USART_WordLength_9b;
	UsartInitStruct.USART_Parity = USART_Parity_No;
	
	/* Initialisation of USARTx */
	USART_Init(USART1, &UsartInitStruct);
	
	/* Enabling of USARTx */
	USART_Cmd(USART1, ENABLE);
};

/**
  * @brief  Инициализация таймеров
  * @param  None
  * @retval None
  */
void TIMsInit(void)
{
	/* Базовая структура инициализации */
	TIM_TimeBaseInitTypeDef  TimeBaseStruct;
	
	/***************** Инициализация таймера TIM6 *******************/
	// This timer is used for sampling a period of scanning buttons and encoders
	
	TimeBaseStruct.TIM_Period = 500;
  TimeBaseStruct.TIM_Prescaler = 24 - 1;
  TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TimeBaseStruct.TIM_RepetitionCounter = 0;
	/* Инициализация таймера */
  TIM_TimeBaseInit(TIM6, &TimeBaseStruct);
		
	/***************** Инициализация таймера TIM6 *******************/
};

/**
  * @brief  Инициализация прерываний
  * @param  None
  * @retval None
  */
void NVICinit(void)
{
	NVIC_InitTypeDef NVICinitStruct;
	
	// Разрешить прерывание от TIM6
	NVICinitStruct.NVIC_IRQChannel = TIM6_IRQn;
	NVICinitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVICinitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVICinitStruct);
};

void IwdgInitAndStart(void)
{
	RCC_LSICmd(ENABLE); // Enable LSI
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);	// wait until LSI sets a stable frequency
	
	// Set prascaler and reload value so as to get 1.5 s interval
	// 1 / (40000 Hz / 16) * 3750(0xEA6) = 1.5 s
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);		// Enable write to prescaler and reload register
	IWDG_SetPrescaler(IWDG_Prescaler_16);
	IWDG_SetReload(0xEA6);
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
	
	IWDG_Enable();	// Enable watchdog
}

// Инициализация микроконтроллера
void DeviceInit(void)
{
	IwdgInitAndStart();
	RCC_Init();
	GPIOinit();
	USARTInit();
	TIMsInit();
	NVICinit();
};
