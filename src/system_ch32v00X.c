/********************************** (C) COPYRIGHT
 ******************************** File Name          : system_ch32v00X.c Author
 *: WCH Version            : V1.0.0 Date               : 2024/11/04 Description
 *: CH32V00X Device Peripheral Access Layer System Source File.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include <ch32v00X.h>

__I uint8_t HBPrescTable[16] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};

/*********************************************************************
 * @fn      SystemInit
 *
 * @brief   Setup the microcontroller system Initialize the Embedded Flash
 * Interface, the PLL and update the SystemCoreClock variable.
 *
 * @return  none
 */
void SystemInit(void) {
  uint32_t tmp = 0;

  /* Flash 2 wait state */
  FLASH->ACTLR = (uint32_t)FLASH_ACTLR_LATENCY_2;
  RCC->CTLR |= (uint32_t)0x00000001;
  RCC->CFGR0 &= (uint32_t)0x68FF0000;

  tmp = RCC->CTLR;
  tmp &= (uint32_t)0xFED6FFFB;
  tmp |= (uint32_t)(1 << 20);
  RCC->CTLR = tmp;

  RCC->CTLR &= (uint32_t)0xFFFBFFFF;
  RCC->CFGR0 &= (uint32_t)0xFFFEFFFF;
  RCC->INTR = 0x009D0000;
}

/*********************************************************************
 * @fn      SystemCoreClockUpdate
 *
 * @brief   Update SystemCoreClock variable according to Clock Register Values.
 *
 * @return  none
 */
void SystemCoreClockUpdate(void) {
  uint32_t tmp = 0, pllsource = 0;

  tmp = RCC->CFGR0 & RCC_SWS;

  switch (tmp) {
  case 0x00:
    SystemCoreClock = HSI_VALUE;
    break;
  case 0x04:
    SystemCoreClock = HSE_VALUE;
    break;
  case 0x08:
    pllsource = RCC->CFGR0 & RCC_PLLSRC;
    if (pllsource == 0x00) {
      SystemCoreClock = HSI_VALUE * 2;
    } else {
      SystemCoreClock = HSE_VALUE * 2;
    }
    break;
  default:
    SystemCoreClock = HSI_VALUE;
    break;
  }

  tmp = HBPrescTable[((RCC->CFGR0 & RCC_HPRE) >> 4)];

  if (((RCC->CFGR0 & RCC_HPRE) >> 4) < 8) {
    SystemCoreClock /= tmp;
  } else {
    SystemCoreClock >>= tmp;
  }
}

/*********************************************************************
 * @fn      SetSysClockTo_8MHz_HSI
 *
 * @brief   Sets HSI as System clock source and configure HCLK, PCLK2 and PCLK1
 * prescalers.
 *
 * @return  none
 */
void SetSysClockTo_8MHz_HSI(void) {
  /* HCLK = SYSCLK = PB1 */
  RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV3;

  /* Flash 0 wait state */
  FLASH->ACTLR = (uint32_t)FLASH_ACTLR_LATENCY_0;
}

/*********************************************************************
 * @fn      SetSysClockTo_24MHZ_HSI
 *
 * @brief   Sets System clock frequency to 24MHz and configure HCLK, PCLK2 and
 * PCLK1 prescalers.
 *
 * @return  none
 */
void SetSysClockTo_24MHZ_HSI(void) {
  /* HCLK = SYSCLK = PB1 */
  RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;

  /* Flash 1 wait state */
  FLASH->ACTLR = (uint32_t)FLASH_ACTLR_LATENCY_1;
}

/*********************************************************************
 * @fn      SetSysClockTo_48MHZ_HSI
 *
 * @brief   Sets System clock frequency to 48MHz and configure HCLK, PCLK2 and
 * PCLK1 prescalers.
 *
 * @return  none
 */
void SetSysClockTo_48MHZ_HSI(void) {
  /* HCLK = SYSCLK = PB1 */
  RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;

  /* PLL configuration: PLLCLK = HSI * 2 = 48 MHz */
  RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC));
  RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Mul2);

  /* Enable PLL */
  RCC->CTLR |= RCC_PLLON;
  /* Wait till PLL is ready */
  while ((RCC->CTLR & RCC_PLLRDY) == 0) {
  }
  /* Select PLL as system clock source */
  RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
  RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
  /* Wait till PLL is used as system clock source */
  while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08) {
  }
  /* Flash 2 wait state */
  FLASH->ACTLR = (uint32_t)FLASH_ACTLR_LATENCY_2;
}

/*********************************************************************
 * @fn      SetSysClockTo_8MHz_HSE
 *
 * @brief   Sets System clock frequency to 8MHz and configure HCLK, PCLK2 and
 * PCLK1 prescalers.
 *
 * @return  none
 */
void SetSysClockTo_8MHz_HSE(void) {
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

  /* Close PA1-PA2 GPIO function */
  RCC->PB2PCENR |= RCC_AFIOEN;
  AFIO->PCFR1 |= (1 << 17);

  RCC->CTLR |= ((uint32_t)RCC_HSEON);

  /* Wait till HSE is ready and if Time out is reached exit */
  do {
    HSEStatus = RCC->CTLR & RCC_HSERDY;
    StartUpCounter++;
  } while ((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

  if ((RCC->CTLR & RCC_HSERDY) != RESET) {
    HSEStatus = (uint32_t)0x01;
  } else {
    HSEStatus = (uint32_t)0x00;
  }

  if (HSEStatus == (uint32_t)0x01) {
    /* HCLK = SYSCLK = PB1 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV3;

    /* Select HSE as system clock source */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_HSE;
    /* Wait till HSE is used as system clock source */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x04) {
    }
    /* Flash 0 wait state */
    FLASH->ACTLR = (uint32_t)FLASH_ACTLR_LATENCY_0;
  } else {
    /*
     * If HSE fails to start-up, the application will have wrong clock
     * configuration. User can add here some code to deal with this error
     */
    /* Open PA1-PA2 GPIO function */
    AFIO->PCFR1 &= ~(1 << 17);
    RCC->PB2PCENR &= ~RCC_AFIOEN;

    RCC->CTLR &= ((uint32_t)~RCC_HSEON);
  }
}

/*********************************************************************
 * @fn      SetSysClockTo_24MHz_HSE
 *
 * @brief   Sets System clock frequency to 24MHz and configure HCLK, PCLK2 and
 * PCLK1 prescalers.
 *
 * @return  none
 */
void SetSysClockTo_24MHz_HSE(void) {
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

  /* Close PA1-PA2 GPIO function */
  RCC->PB2PCENR |= RCC_AFIOEN;
  AFIO->PCFR1 |= (1 << 17);

  RCC->CTLR |= ((uint32_t)RCC_HSEON);

  /* Wait till HSE is ready and if Time out is reached exit */
  do {
    HSEStatus = RCC->CTLR & RCC_HSERDY;
    StartUpCounter++;
  } while ((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

  if ((RCC->CTLR & RCC_HSERDY) != RESET) {
    HSEStatus = (uint32_t)0x01;
  } else {
    HSEStatus = (uint32_t)0x00;
  }

  if (HSEStatus == (uint32_t)0x01) {
    /* HCLK = SYSCLK = PB1 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;

    /* Select HSE as system clock source */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_HSE;
    /* Wait till HSE is used as system clock source */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x04) {
    }
    /* Flash 1 wait state */
    FLASH->ACTLR = (uint32_t)FLASH_ACTLR_LATENCY_1;
  } else {
    /*
     * If HSE fails to start-up, the application will have wrong clock
     * configuration. User can add here some code to deal with this error
     */
    /* Open PA1-PA2 GPIO function */
    AFIO->PCFR1 &= ~(1 << 17);
    RCC->PB2PCENR &= ~RCC_AFIOEN;

    RCC->CTLR &= ((uint32_t)~RCC_HSEON);
  }
}

/*********************************************************************
 * @fn      SetSysClockTo_48MHz_HSE
 *
 * @brief   Sets System clock frequency to 48MHz and configure HCLK, PCLK2 and
 * PCLK1 prescalers.
 *
 * @return  none
 */
void SetSysClockTo_48MHz_HSE(void) {
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

  /* Close PA1-PA2 GPIO function */
  RCC->PB2PCENR |= RCC_AFIOEN;
  AFIO->PCFR1 |= (1 << 17);

  RCC->CTLR |= ((uint32_t)RCC_HSEON);

  /* Wait till HSE is ready and if Time out is reached exit */
  do {
    HSEStatus = RCC->CTLR & RCC_HSERDY;
    StartUpCounter++;
  } while ((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

  if ((RCC->CTLR & RCC_HSERDY) != RESET) {
    HSEStatus = (uint32_t)0x01;
  } else {
    HSEStatus = (uint32_t)0x00;
  }

  if (HSEStatus == (uint32_t)0x01) {
    /* HCLK = SYSCLK = PB1 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;

    /* PLL configuration: PLLCLK = HSE * 2 = 48 MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC));
    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE_Mul2);

    /* Enable PLL */
    RCC->CTLR |= RCC_PLLON;
    /* Wait till PLL is ready */
    while ((RCC->CTLR & RCC_PLLRDY) == 0) {
    }
    /* Select PLL as system clock source */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08) {
    }
    /* Flash 2 wait state */
    FLASH->ACTLR = (uint32_t)FLASH_ACTLR_LATENCY_2;
  } else {
    /*
     * If HSE fails to start-up, the application will have wrong clock
     * configuration. User can add here some code to deal with this error
     */
    /* Open PA1-PA2 GPIO function */
    AFIO->PCFR1 &= ~(1 << 17);
    RCC->PB2PCENR &= ~RCC_AFIOEN;

    RCC->CTLR &= ((uint32_t)~RCC_HSEON);
  }
}
