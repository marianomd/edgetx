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

#if defined(SIMU)
// not needed
#else
const int8_t ana_direction[NUM_ANALOGS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};
const uint8_t ana_mapping[NUM_ANALOGS] =  {3, 2, 1, 0, 6, 7, 4, 5, 8, 9, 10};
                                          
#endif

#if NUM_PWMANALOGS > 0
#define FIRST_ANALOG_ADC (ANALOGS_PWM_ENABLED() ? NUM_PWMANALOGS : 0)
#define NUM_ANALOGS_ADC (ANALOGS_PWM_ENABLED() ? (NUM_ANALOGS - NUM_PWMANALOGS) : NUM_ANALOGS)
#elif defined(PCBX9E)
#define FIRST_ANALOG_ADC 0
#define NUM_ANALOGS_ADC 10
#define NUM_ANALOGS_ADC_EXT (NUM_ANALOGS - 10)
#else
#define FIRST_ANALOG_ADC 0
#define NUM_ANALOGS_ADC NUM_ANALOGS
#endif

uint16_t adcValues[NUM_ANALOGS] __DMA;

#define ADC_DMA_CHANNEL DMA1_Channel1
#define ADC_DMA_TC_FLAG DMA1_FLAG_TC1

static void adc_dma_arm(void)
{
  ADC_StartOfConversion(ADC1);
}

void adcInit()
{
  // -- init rcc --
  // ADC CLOCK = 24 / 4 = 6MHz
  RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div2);

  // enable ADC clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  // enable dma clock
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  // periph clock enable for port
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);

  // init gpio
  GPIO_InitTypeDef gpio_init;
  GPIO_StructInit(&gpio_init);

  // set up analog inputs ADC0...ADC7(PA0...PA7)
  gpio_init.GPIO_Pin = 0b11111111;
  gpio_init.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(GPIOA, &gpio_init);

  // set up analog inputs ADC8, ADC9(PB0, PB1)
  gpio_init.GPIO_Pin = 0b11;
  gpio_init.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(GPIOB, &gpio_init);

  // battery voltage is on PC0(ADC10)
  gpio_init.GPIO_Pin = 0b1;
  gpio_init.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(GPIOC, &gpio_init);

  // init mode
  ADC_InitTypeDef adc_init;
  ADC_StructInit(&adc_init);

  // ADC configuration
  adc_init.ADC_ContinuousConvMode = ENABLE; // ! select continuous conversion mode
  adc_init.ADC_ExternalTrigConv = 0;
  adc_init.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // select no ext triggering
  adc_init.ADC_DataAlign = ADC_DataAlign_Right;                      // r 12-bit data alignment in ADC reg
  adc_init.ADC_Resolution = ADC_Resolution_12b;
  adc_init.ADC_ScanDirection = ADC_ScanDirection_Upward;

  // load structure values to control and status registers
  ADC_Init(ADC1, &adc_init);

  // configure each channel
  ADC_ChannelConfig(ADC1, ADC_Channel_0, ADC_SampleTime_41_5Cycles);
  ADC_ChannelConfig(ADC1, ADC_Channel_1, ADC_SampleTime_41_5Cycles);
  ADC_ChannelConfig(ADC1, ADC_Channel_2, ADC_SampleTime_41_5Cycles);
  ADC_ChannelConfig(ADC1, ADC_Channel_3, ADC_SampleTime_41_5Cycles);
  ADC_ChannelConfig(ADC1, ADC_Channel_4, ADC_SampleTime_41_5Cycles);
  ADC_ChannelConfig(ADC1, ADC_Channel_5, ADC_SampleTime_41_5Cycles);
  ADC_ChannelConfig(ADC1, ADC_Channel_6, ADC_SampleTime_41_5Cycles);
  ADC_ChannelConfig(ADC1, ADC_Channel_7, ADC_SampleTime_41_5Cycles);
  ADC_ChannelConfig(ADC1, ADC_Channel_8, ADC_SampleTime_41_5Cycles);
  ADC_ChannelConfig(ADC1, ADC_Channel_9, ADC_SampleTime_41_5Cycles);
  ADC_ChannelConfig(ADC1, ADC_Channel_10, ADC_SampleTime_41_5Cycles);

  // enable ADC
  ADC_Cmd(ADC1, ENABLE);

  // enable DMA for ADC
  ADC_DMACmd(ADC1, ENABLE);

  // -- init dma --

  DMA_InitTypeDef dma_init;
  DMA_StructInit(&dma_init);

  // reset DMA1 channe1 to default values
  DMA_DeInit(ADC_DMA_CHANNEL);

  // set up dma to convert 2 adc channels to two mem locations:
  // channel will be used for memory to memory transfer
  dma_init.DMA_M2M = DMA_M2M_Disable;
  // setting normal mode(non circular)
  dma_init.DMA_Mode = DMA_Mode_Circular;
  // medium priority
  dma_init.DMA_Priority = DMA_Priority_High;
  // source and destination 16bit
  dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  // automatic memory destination increment enable.
  dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
  // source address increment disable
  dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  // Location assigned to peripheral register will be source
  dma_init.DMA_DIR = DMA_DIR_PeripheralSRC;
  // chunk of data to be transfered
  dma_init.DMA_BufferSize = NUM_ANALOGS;
  // source and destination start addresses
  dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  dma_init.DMA_MemoryBaseAddr = (uint32_t)adcValues;
  // send values to DMA registers
  DMA_Init(ADC_DMA_CHANNEL, &dma_init);

  // enable the DMA1 - Channel1
  DMA_Cmd(ADC_DMA_CHANNEL, ENABLE);

  // start conversion:
  adc_dma_arm();
}

void adcRead()
{
  // adc dma finished?
  if (DMA_GetITStatus(ADC_DMA_TC_FLAG))
  {

#if NUM_PWMANALOGS > 0
    if (ANALOGS_PWM_ENABLED())
    {
      analogPwmRead(adcValues);
    }
#endif
    // fine, arm DMA again:
    adc_dma_arm();
  }
}

// TODO
void adcStop()
{
}

#if !defined(SIMU)
uint16_t getAnalogValue(uint8_t index)
{
  if (IS_POT(index) && !IS_POT_SLIDER_AVAILABLE(index))
  {
    // Use fixed analog value for non-existing and/or non-connected pots.
    // Non-connected analog inputs will slightly follow the adjacent connected analog inputs,
    // which produces ghost readings on these inputs.
    return 0;
  }
  index = ana_mapping[index];
  if (ana_direction[index] < 0)
    return 4095 - adcValues[index];
  else
    return adcValues[index];
}
#endif // #if !defined(SIMU)
