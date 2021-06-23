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

void extmoduleSendNextFrame();

void extmoduleStop()
{
  TRACE("extmoduleStop");
}

void extmoduleNoneStart()
{
  TRACE("extmoduleNoneStart");
}

void extmodulePpmStart()
{
    TRACE("extmodulePpmStart");
}

void extmodulePxxStart()
{
    TRACE("extmodulePxxStart");
}


void extmoduleCrossfireStart()
{
  TRACE("extmoduleCrossfireStart");
}

void extmoduleSendNextFrame()
{
 TRACE("extmoduleSendNextFrame");
}

extern "C" void EXTMODULE_DMA_IRQHandler()
{
  
}

extern "C" void EXTMODULE_TIMER_CC_IRQHandler()
{
  
}
