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
#include <stdio.h>

uint8_t serial2Mode = 0;

void uartSetup(unsigned int baudrate, bool dma)
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHBPeriphClockCmd(SERIAL_RCC_AHB1Periph, ENABLE);
  RCC_APB2PeriphClockCmd(SERIAL_RCC_APB2Periph, ENABLE);

  GPIO_PinAFConfig(SERIAL_GPIO, SERIAL_GPIO_PinSource_RX, SERIAL_GPIO_AF);
  GPIO_PinAFConfig(SERIAL_GPIO, SERIAL_GPIO_PinSource_TX, SERIAL_GPIO_AF);

  GPIO_InitStructure.GPIO_Pin = SERIAL_GPIO_PIN_TX | SERIAL_GPIO_PIN_RX;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(SERIAL_GPIO, &GPIO_InitStructure);

  USART_InitStructure.USART_BaudRate = baudrate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(SERIAL_USART, &USART_InitStructure);
  USART_Cmd(SERIAL_USART, ENABLE);
}

void serial2Init(unsigned int mode, unsigned int protocol)
{
  serial2Stop();

  serial2Mode = mode;
  switch (mode)
  {
  case UART_MODE_TELEMETRY_MIRROR:
#if !defined(STM32F0)
    uartSetup(FRSKY_SPORT_BAUDRATE, false);
#endif
    break;
#if defined(DEBUG) || defined(CLI)
  case UART_MODE_DEBUG:
    uartSetup(DEBUG_BAUDRATE, false);
    break;
#endif
  case UART_MODE_TELEMETRY:
#if !defined(STM32F0)
    if (protocol == PROTOCOL_FRSKY_D_SECONDARY)
    {
      uartSetup(FRSKY_D_BAUDRATE, true);
    }
#endif
    break;
  case UART_MODE_LUA:
    uartSetup(DEBUG_BAUDRATE, false);
  }
}

void serial2Putc(char c)
{
#if !defined(SIMU)
  while (USART_GetFlagStatus(SERIAL_USART, USART_FLAG_TXE) == RESET);
  USART_SendData(SERIAL_USART, c);
#endif
}

void serial2SbusInit()
{
  // uartSetup(SBUS_BAUDRATE, true);
  // SERIAL_USART->CR1 |= USART_CR1_M | USART_CR1_PCE ;
}

void serial2Stop()
{
  USART_DeInit(SERIAL_USART);
}

uint8_t serial2TracesEnabled()
{
#if defined(DEBUG)
  return (serial2Mode == UART_MODE_DEBUG);
#else
  return false;
#endif
}

// extern "C" void SERIAL_USART_IRQHandler(void)
// {
//   DEBUG_INTERRUPT(INT_SER2);
//   // Send
//   if (USART_GetITStatus(SERIAL_USART, USART_IT_TXE) != RESET) {
//     uint8_t txchar;
//     if (serial2TxFifo.pop(txchar)) {
//       /* Write one byte to the transmit data register */
//       USART_SendData(SERIAL_USART, txchar);
//     }
//     else {
//       USART_ITConfig(SERIAL_USART, USART_IT_TXE, DISABLE);
//     }
//   }

// #if defined(CLI)
//   if (!(getSelectedUsbMode() == USB_SERIAL_MODE)) {
//     // Receive
//     uint32_t status = SERIAL_USART->SR;
//     while (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS)) {
//       uint8_t data = SERIAL_USART->DR;
//       if (!(status & USART_FLAG_ERRORS)) {
//         switch (serial2Mode) {
//           case UART_MODE_DEBUG:
//             cliRxFifo.push(data);
//             break;
//         }
//       }
//       status = SERIAL_USART->SR;
//     }
//   }
// #endif
// }
