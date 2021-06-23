/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"

#if defined(__cplusplus) && !defined(SIMU)
extern "C"
{
#endif
#include "usb_dcd_int.h"
#include "usb_bsp.h"
#if defined(__cplusplus) && !defined(SIMU)
}
#endif

extern "C"
{
  __attribute__((naked)) __attribute__((used)) void HardFault_HandlerAsm(void)
  {

    __asm(".syntax unified\n"
          "MOVS R0, #4 \n"
          "MOV R1, LR \n"
          "TST R0, R1 \n"
          "BEQ _MSP \n"
          "MRS R0, PSP \n"
          "B HardFault_HandlerC \n"
          "_MSP: \n"
          "MRS R0, MSP \n"
          "B HardFault_HandlerC \n"
          ".syntax divided\n");
  }

  __attribute__((used)) void HardFault_HandlerC(unsigned long *hardfault_args)
  {
    /*
  volatile unsigned long stacked_r0 ;
  volatile unsigned long stacked_r1 ;
  volatile unsigned long stacked_r2 ;
  volatile unsigned long stacked_r3 ;
  volatile unsigned long stacked_r12 ;
  volatile unsigned long stacked_lr ;
  volatile unsigned long stacked_pc ;
  volatile unsigned long stacked_psr ;
  volatile unsigned long _CFSR ;
  volatile unsigned long _HFSR ;
  volatile unsigned long _DFSR ;
  volatile unsigned long _AFSR ;
  volatile unsigned long _BFAR ;
  volatile unsigned long _MMAR ;

  stacked_r0 = ((unsigned long)hardfault_args[0]) ;
  stacked_r1 = ((unsigned long)hardfault_args[1]) ;
  stacked_r2 = ((unsigned long)hardfault_args[2]) ;
  stacked_r3 = ((unsigned long)hardfault_args[3]) ;
  stacked_r12 = ((unsigned long)hardfault_args[4]) ;
  stacked_lr = ((unsigned long)hardfault_args[5]) ;
  stacked_pc = ((unsigned long)hardfault_args[6]) ;
  stacked_psr = ((unsigned long)hardfault_args[7]) ;

  // Configurable Fault Status Register
  // Consists of MMSR, BFSR and UFSR
  _CFSR = (*((volatile unsigned long *)(0xE000ED28))) ;

  // Hard Fault Status Register
  _HFSR = (*((volatile unsigned long *)(0xE000ED2C))) ;

  // Debug Fault Status Register
  _DFSR = (*((volatile unsigned long *)(0xE000ED30))) ;

  // Auxiliary Fault Status Register
  _AFSR = (*((volatile unsigned long *)(0xE000ED3C))) ;

  // Read the Fault Address Registers. These may not contain valid values.
  // Check BFARVALID/MMARVALID to see if they are valid values
  // MemManage Fault Address Register
  _MMAR = (*((volatile unsigned long *)(0xE000ED34))) ;
  // Bus Fault Address Register
  _BFAR = (*((volatile unsigned long *)(0xE000ED38))) ;
*/
    __asm("BKPT #0\n"); // Break into the debugger
  }
}
//audio
void audioConsumeCurrentBuffer()
{
}
void audioInit()
{
}

void referenceSystemAudioFiles()
{
}

void setSampleRate(uint32_t frequency)
{
}

void watchdogInit(unsigned int duration)
{
  IWDG->KR = 0x5555;    // Unlock registers
  IWDG->PR = 3;         // Divide by 32 => 1kHz clock
  IWDG->KR = 0x5555;    // Unlock registers
  IWDG->RLR = duration; // 1.5 seconds nominal
  IWDG->KR = 0xAAAA;    // reload
  IWDG->KR = 0xCCCC;    // start
}

// Starts TIMER at 2MHz
void init2MhzTimer()
{
  TIMER_2MHz_TIMER->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 2000000 - 1; // 0.5 uS, 2 MHz
  TIMER_2MHz_TIMER->ARR = 65535;
  TIMER_2MHz_TIMER->CR2 = 0;
  TIMER_2MHz_TIMER->CR1 = TIM_CR1_CEN;
}

// Starts TIMER at 200Hz (5ms)
void init5msTimer()
{
  INTERRUPT_xMS_TIMER->ARR = 4999;                                              // 5mS in uS
  INTERRUPT_xMS_TIMER->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 1000000 - 1; // 1uS
  INTERRUPT_xMS_TIMER->CCER = 0;
  INTERRUPT_xMS_TIMER->CCMR1 = 0;
  INTERRUPT_xMS_TIMER->EGR = 0;
  INTERRUPT_xMS_TIMER->CR1 = 5;
  INTERRUPT_xMS_TIMER->DIER |= 1;

  NVIC_EnableIRQ(INTERRUPT_xMS_IRQn);
  NVIC_SetPriority(INTERRUPT_xMS_IRQn, 7);
}

void stop5msTimer(void)
{
  INTERRUPT_xMS_TIMER->CR1 = 0; // stop timer
  NVIC_DisableIRQ(INTERRUPT_xMS_IRQn);
}

void interrupt5ms()
{
  static uint32_t pre_scale; // Used to get 10 Hz counter
  if (++pre_scale >= 2)
  {
    pre_scale = 0;
    DEBUG_TIMER_START(debugTimerPer10ms);
    DEBUG_TIMER_SAMPLE(debugTimerPer10msPeriod);
    per10ms();
    DEBUG_TIMER_STOP(debugTimerPer10ms);
  }
}

#if !defined(SIMU)
extern "C" void INTERRUPT_xMS_IRQHandler()
{
  INTERRUPT_xMS_TIMER->SR &= ~TIM_SR_UIF;
  interrupt5ms();
  DEBUG_INTERRUPT(INT_5MS);
}
#endif

#if defined(PWR_PRESS_BUTTON) && !defined(SIMU)
#define PWR_PRESS_DURATION_MIN 100 // 1s
#define PWR_PRESS_DURATION_MAX 500 // 5s
#endif

void init_gpio()
{
  RCC_AHBPeriphClockCmd(I2C_RCC_AHB1Periph, ENABLE);
  RCC_APB1PeriphClockCmd(I2C_RCC_APB1Periph, ENABLE);

  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Pin = I2C_SCL_GPIO_PIN | I2C_SDA_GPIO_PIN;
  //gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_OType = GPIO_OType_OD;
  gpio_init.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(I2C_GPIO, &gpio_init);

  GPIO_PinAFConfig(I2C_GPIO, I2C_SCL_GPIO_PinSource, I2C_GPIO_AF);
  GPIO_PinAFConfig(I2C_GPIO, I2C_SDA_GPIO_PinSource, I2C_GPIO_AF);
}
void resetReason()
{
  TRACE("Reset reason:");
  if (RCC->CSR & RCC_CSR_LSION)
  {
    TRACE("Internal Low Speed oscillator enable");
  }
  if (RCC->CSR & RCC_CSR_LSIRDY)
  {
    TRACE("Internal Low Speed oscillator Ready");
  }
  if (RCC->CSR & RCC_CSR_V18PWRRSTF)
  {
    TRACE("V1.8 power domain reset flag");
  }
  if (RCC->CSR & RCC_CSR_RMVF)
  {
    TRACE("Remove reset flag");
  }
  if (RCC->CSR & RCC_CSR_OBLRSTF)
  {
    TRACE("OBL reset flag");
  }
  if (RCC->CSR & RCC_CSR_PINRSTF)
  {
    TRACE("PIN reset flag");
  }
  if (RCC->CSR & RCC_CSR_PORRSTF)
  {
    TRACE("POR/PDR reset flag");
  }
  if (RCC->CSR & RCC_CSR_SFTRSTF)
  {
    TRACE("Software Reset flag");
  }
  if (RCC->CSR & RCC_CSR_IWDGRSTF)
  {
    TRACE("Independent Watchdog reset flag");
  }
  if (RCC->CSR & RCC_CSR_WWDGRSTF)
  {
    TRACE("Window watchdog reset flag");
  }
  if (RCC->CSR & RCC_CSR_LPWRRSTF)
  {
    TRACE("Low-Power reset flag");
  }
}
void boardInit()
{
#if !defined(SIMU)
  RCC_AHBPeriphClockCmd(RCC_AHB1_LIST, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1_LIST, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2_LIST, ENABLE);
#if defined(DEBUG) && defined(SERIAL_GPIO)
  serial2Init(UART_MODE_DEBUG, 0); // default serial mode (None if DEBUG not defined)
  TRACE("---------------------- FlySky board started :) -------------------------");
#endif
  // Reset reason
  resetReason();
  //pwrInit();
  keysInit();
  adcInit();
  delaysInit();
  lcdInit(); // delaysInit() must be called before
  //define this deiver
  //audioInit();
  init2MhzTimer();
  init5msTimer();
  __enable_irq();
  backlightInit();
  backlightEnable(1);
  init_gpio();
  eepromInit();
  //storageEraseAll(false);
   ////usbInit();
  // TRACE("i2c test");
  // i2c_test();

  // while (1)
  // {
  // }

#if defined(DEBUG)
  DBGMCU_APB1PeriphConfig(DBGMCU_IWDG_STOP | DBGMCU_TIM1_STOP | DBGMCU_TIM2_STOP | DBGMCU_TIM3_STOP | DBGMCU_TIM6_STOP | DBGMCU_TIM14_STOP, ENABLE);
#endif

#if defined(PWR_PRESS_BUTTON)
  if (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
  {
    lcdClear();
#if defined(PCBX9E)
    lcdDrawBitmap(76, 2, bmp_lock, 0, 60);
#else
    lcdDrawFilledRect(LCD_W / 2 - 18, LCD_H / 2 - 3, 6, 6, SOLID, 0);
#endif
    lcdRefresh();
    lcdRefreshWait();

    tmr10ms_t start = get_tmr10ms();
    tmr10ms_t duration = 0;
    uint8_t pwr_on = 0;
    while (pwrPressed())
    {
      duration = get_tmr10ms() - start;
      if (duration < PWR_PRESS_DURATION_MIN)
      {
        unsigned index = duration / (PWR_PRESS_DURATION_MIN / 4);
        lcdClear();
#if defined(PCBX9E)
        lcdDrawBitmap(76, 2, bmp_startup, index * 60, 60);
#else
        for (uint8_t i = 0; i < 4; i++)
        {
          if (index >= i)
          {
            lcdDrawFilledRect(LCD_W / 2 - 18 + 10 * i, LCD_H / 2 - 3, 6, 6, SOLID, 0);
          }
        }
#endif
      }
      else if (duration >= PWR_PRESS_DURATION_MAX)
      {
        drawSleepBitmap();
        backlightDisable();
      }
      else
      {
        if (pwr_on != 1)
        {
          pwr_on = 1;
          pwrInit();
          backlightInit();
          haptic.play(15, 3, PLAY_NOW);
        }
      }
      lcdRefresh();
      lcdRefreshWait();
    }
    if (duration < PWR_PRESS_DURATION_MIN || duration >= PWR_PRESS_DURATION_MAX)
    {
      boardOff();
    }
  }
  else
  {
    pwrInit();
    backlightInit();
  }
#if defined(TOPLCD_GPIO)
  toplcdInit();
#endif
#else // defined(PWR_PRESS_BUTTON)
  backlightInit();
#endif

  if (HAS_SPORT_UPDATE_CONNECTOR())
  {
    sportUpdateInit();
  }
#endif // !defined(SIMU)
}

void boardOff()
{
#if defined(STATUS_LEDS)
  ledOff();
#endif

  BACKLIGHT_DISABLE();

#if defined(TOPLCD_GPIO)
  toplcdOff();
#endif

#if defined(PWR_PRESS_BUTTON)
  while (pwrPressed())
  {
    wdt_reset();
  }
#endif
  lcdOff();
  SysTick->CTRL = 0; // turn off systick
  pwrOff();
}

uint8_t currentTrainerMode = 0xff;

void checkTrainerSettings()
{
  uint8_t requiredTrainerMode = g_model.trainerMode;
  if (requiredTrainerMode != currentTrainerMode)
  {
    switch (currentTrainerMode)
    {
    case TRAINER_MODE_MASTER_TRAINER_JACK:
      stop_trainer_capture();
      break;
    case TRAINER_MODE_SLAVE:
      stop_trainer_ppm();
      break;
      /*
      case TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE:
        stop_cppm_on_heartbeat_capture() ;
        break;
      case TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE:
        stop_sbus_on_heartbeat_capture() ;
        break;*/
#if defined(TRAINER_BATTERY_COMPARTMENT)
    case TRAINER_MODE_MASTER_BATTERY_COMPARTMENT:
      //serial2Stop();
      break;
#endif
    }

    currentTrainerMode = requiredTrainerMode;
    switch (requiredTrainerMode)
    {
    case TRAINER_MODE_SLAVE:
      init_trainer_ppm();
      break;
      /*
      case TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE:
         init_cppm_on_heartbeat_capture();
         break;
      case TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE:
         init_sbus_on_heartbeat_capture();
         break;*/

#if defined(TRAINER_BATTERY_COMPARTMENT)
    case TRAINER_MODE_MASTER_BATTERY_COMPARTMENT:
      /*
        if (g_eeGeneral.serial2Mode == UART_MODE_SBUS_TRAINER) {
          serial2SbusInit();
          break;
      }*/
      // no break
#endif
    default:
      // master is default
      init_trainer_capture();
      break;
    }
  }
}

uint16_t getBatteryVoltage()
{
  uint32_t mv = (adcValues[TX_VOLTAGE] * (3300 * BATT_SCALE)) / (4095 * 51);
  return (uint16_t)(mv / 10) + 20;
}
