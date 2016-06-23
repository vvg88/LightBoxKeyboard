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
  
  /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/    
  /* Enable HSE */    
  //RCC->CR |= ((uint32_t)RCC_CR_HSEON);
 
  /* Wait till HSE is ready and if Time out is reached exit */
//  do
//  {
//    HSEStatus = RCC->CR & RCC_CR_HSERDY;
//    StartUpCounter++;  
//  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

//  if ((RCC->CR & RCC_CR_HSERDY) != RESET)
//  {
//    HSEStatus = (uint32_t)0x01;
//  }
//  else
//  {
//    HSEStatus = (uint32_t)0x00;
//  }  

//  if (HSEStatus == (uint32_t)0x01)
//  {
//#if !defined STM32F10X_LD_VL && !defined STM32F10X_MD_VL && !defined STM32F10X_HD_VL 
//    /* Enable Prefetch Buffer */
//    FLASH->ACR |= FLASH_ACR_PRFTBE;

//    /* Flash 0 wait state */
//    FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
//    FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_0;    
//#endif
 
    /* HCLK = SYSCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;
      
    /* PCLK2 = HCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;
    
    /* PCLK1 = HCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1;
    
//#ifdef STM32F10X_CL
//    /* Configure PLLs ------------------------------------------------------*/
//    /* PLL configuration: PLLCLK = PREDIV1 * 6 = 24 MHz */ 
//    RCC->CFGR &= (uint32_t)~(RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL);
//    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLSRC_PREDIV1 | 
//                            RCC_CFGR_PLLMULL6); 

//    /* PLL2 configuration: PLL2CLK = (HSE / 5) * 8 = 40 MHz */
//    /* PREDIV1 configuration: PREDIV1CLK = PLL2 / 10 = 4 MHz */       
//    RCC->CFGR2 &= (uint32_t)~(RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL |
//                              RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC);
//    RCC->CFGR2 |= (uint32_t)(RCC_CFGR2_PREDIV2_DIV5 | RCC_CFGR2_PLL2MUL8 |
//                             RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV10);
//  
//    /* Enable PLL2 */
//    RCC->CR |= RCC_CR_PLL2ON;
//    /* Wait till PLL2 is ready */
//    while((RCC->CR & RCC_CR_PLL2RDY) == 0)
//    {
//    }   
//#elif defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
//    /*  PLL configuration:  = (HSE / 2) * 6 = 24 MHz */
//    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
//    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLXTPRE_PREDIV1_Div2 | RCC_CFGR_PLLMULL6);
//#else    
    /*  PLL configuration:  = (HSI / 2) * 6 = 24 MHz */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC /*| RCC_CFGR_PLLXTPRE*/ | RCC_CFGR_PLLMULL));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSI_Div2 /*| RCC_CFGR_PLLXTPRE_HSE_Div2*/ | RCC_CFGR_PLLMULL6);
//#endif /* STM32F10X_CL */

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
  //}
};

/**
  * @brief  Инициализация тактирования необходимых периферийных модулей.
  * @param  None
  * @retval None
  */
void RCC_Init(void)
{
	/* Установка тактирования порта А, порта В и С, таймеров 1 и 8, АЦП1, АЦП2, АЦП3 */
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1 /*RCC_APB2Periph_TIM8 | RCC_APB2Periph_TIM1 |*/ 
												 /*| RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | RCC_APB2Periph_SPI1 | RCC_APB2Periph_ADC3*/, ENABLE);
	/* Установить частоту тактирования АЦП 12 МГц */
	//RCC_ADCCLKConfig(RCC_PCLK2_Div4);
	
	/* Установить частоту APB1 6 МГц */
	//RCC_PCLK1Config(RCC_HCLK_Div8);
	
	/* Установка тактирования таймера 6 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6 /*RCC_APB1Periph_DAC |  | RCC_APB1Periph_USART2 | RCC_APB1Periph_TIM5*/, ENABLE);
	/* Разрешить тактирование DMA1 */
 	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
	/* Выбор источника сигнала на линии MCO */
	//RCC_MCOConfig(RCC_MCO_SYSCLK);
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
	
	GpioInitStruct.GPIO_Pin = GPIO_Pin_All & (~GPIO_Pin_15);
	GPIO_Init(GPIOC, &GpioInitStruct);
	
	/* Initialise pins as inputs for detection hardware version */
	GpioInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOD, &GpioInitStruct);
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

// Инициализация микроконтроллера
void DeviceInit(void)
{
	RCC_Init();
	GPIOinit();
	USARTInit();
	TIMsInit();
	NVICinit();
};
