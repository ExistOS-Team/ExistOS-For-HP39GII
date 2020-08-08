/*
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <drivers\ddi_icoll.h>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
//! \brief TBD
#define PRIORITY_DAC_ISR		(IcollPriority_t)ICOLL_PRIORITY_LEVEL_2
//! \brief TBD
#define PRIORITY_DAC_ERROR		(IcollPriority_t)ICOLL_PRIORITY_LEVEL_1
//! \brief TBD
#define PRIORITY_ADC_DMA_ISR        (IcollPriority_t)ICOLL_PRIORITY_LEVEL_2
//! \brief TBD
#define PRIORITY_ADC_ERROR_ISR      (IcollPriority_t)ICOLL_PRIORITY_LEVEL_2
//! \brief TBD
#define PRIORITY_HP_SHORT_ISR       (IcollPriority_t)ICOLL_PRIORITY_LEVEL_2
//! \brief Video synchronization interrupt for the display driver
#define PRIORITY_VIDEO_ISR          (IcollPriority_t)ICOLL_PRIORITY_LEVEL_3

//! \brief Priority of the DCP ISR.  This does not change the priority of the
//! individual channels or the color space converter
#define PRIORITY_DCP_ISR		(IcollPriority_t)ICOLL_PRIORITY_LEVEL_2

//! \brief TBD
#define PRIORITY_LCDIF_ISR		(IcollPriority_t)ICOLL_PRIORITY_LEVEL_3

//! \brief TBD
#define DAC_ICOLL_FUNCTION_TYPE     (IcollFunctionType_t)IRQ_HANDLER_DEFERRED
//! \brief TBD
#define ADC_ICOLL_FUNCTION_TYPE     (IcollFunctionType_t)IRQ_HANDLER_DEFERRED
//! \brief TBD
#define LCDIF_ICOLL_FUNCTION_TYPE   (IcollFunctionType_t)IRQ_HANDLER_DEFERRED
//! \brief TBD
#define VIDEO_ICOLL_FUNCTION_TYPE   (IcollFunctionType_t)IRQ_HANDLER_DEFERRED

#endif // _INTERRUPTS_H

////////////////////////////////////////////////////////////////////////////////
// End of file
////////////////////////////////////////////////////////////////////////////////
//! @}
