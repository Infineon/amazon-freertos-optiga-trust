/*
 * Copyright (c) 2015-2020, Infineon Technologies AG
 * All rights reserved.                        
 *                                             
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *                                                                              
 * To improve the quality of the software, users are encouraged to share 
 * modifications, enhancements or bug fixes with Infineon Technologies AG 
 * at XMCSupport@infineon.com.
 *
 */

/**
 * @file UART.h
 * @date 16 Dec., 2019
 * @version 2.9
 *
 * @brief UART Driver for Infineon XMC devices
 *
 * History
 *
 * Version 1.0 Initial version<br>
 *
 * Version 2.4 Added Status Structure for handling the driver status<br>
 *
 * Version 2.5 Runtime variables defined as volatile<br>
 *
 * Version 2.6 Adapted to xmc1400 devices<br>
 *
 * Version 2.7 Fixed IRQ numbers for XMC1400
 *
 * Version 2.8 Fixed UART_INFO.mode variable type<br>
 *
 * Version 2.9 Added interrupt priority<br>
 *
 */

#include "Driver_USART.h"
#include "xmc_scu.h"
#include "xmc_gpio.h"
#include "xmc_uart.h"

// UART flags
#define UART_INITIALIZED       (1 << 0)
#define UART_POWERED           (1 << 1)
#define UART_CONFIGURED        (1 << 2)
#define UART_TX_ENABLED        (1 << 3)
#define UART_RX_ENABLED        (1 << 4)

#if (UC_SERIES == XMC14)
#define USIC0_0_IRQn 9U
#define UART0_ISR    IRQ9_Handler

#define USIC0_1_IRQn 10U
#define UART1_ISR    IRQ10_Handler

#define USIC1_0_IRQn 11U
#define UART2_ISR    IRQ11_Handler

#define USIC1_1_IRQn 12U
#define UART3_ISR    IRQ12_Handler

#else

#define UART0_ISR USIC0_0_IRQHandler
#define UART1_ISR USIC0_1_IRQHandler
#define UART2_ISR USIC1_0_IRQHandler
#define UART3_ISR USIC1_1_IRQHandler
#define UART4_ISR USIC2_0_IRQHandler
#define UART5_ISR USIC2_1_IRQHandler

#endif

// UART Transfer Information (Run-Time)
typedef struct UART_TRANSFER_INFO 
{
  uint32_t                rx_num;        // Total number of data to be received
  uint32_t                tx_num;        // Total number of data to be send
  uint8_t                 *rx_buf;       // Pointer to in data buffer
  uint8_t                 *tx_buf;       // Pointer to out data buffer
  uint32_t                rx_cnt;        // Number of data received
  uint32_t                tx_cnt;        // Number of data sent
} UART_TRANSFER_INFO_t ;

// UART Information (Run-Time)
typedef struct 
{
  ARM_USART_SignalEvent_t cb_event;           // Event callback
  ARM_USART_STATUS        status;             // Status flags
  UART_TRANSFER_INFO_t    xfer;               // Transfer information
  uint8_t                 flags;              // UART driver flags
  uint32_t                mode;               // UART driver flags
  volatile uint32_t       rx_fifo_pointer;    // FIFO rx pointer
  volatile uint32_t       tx_fifo_pointer;    // FIFO tx pointer
} UART_INFO;

// GPIO
typedef struct XMC_GPIO 
{
  XMC_GPIO_PORT_t *const port;
  const uint8_t pin;
} XMC_GPIO_t;


// UART Resources definitions
typedef struct 
{
  XMC_GPIO_t             pin_tx;                     // TX  Pin identifier
  XMC_GPIO_CONFIG_t      *pin_tx_config;             // TX  Pin configuration
  uint32_t               pin_tx_alternate_function;  // TX  pin alternate function              
  XMC_GPIO_t             pin_rx;                     // RX  Pin identifier
  XMC_GPIO_CONFIG_t      *pin_rx_config;             // RX  Pin configuration
  uint8_t                input;                      // Input for RX  Pin
  XMC_USIC_CH_t          *uart;                      // Pointer to UART peripheral
  IRQn_Type              irq_num;                    // UART TX IRQ Number
  uint32_t               irq_priority;               // UART IRQ priority
  uint32_t               tx_fifo_size_reg;           // FIFO tx size register
  uint32_t               tx_fifo_size_num;           // FIFO tx size register num
  uint32_t               rx_fifo_size_reg;           // FIFO rx size register 
  uint32_t               rx_fifo_size_num;           // FIFO rx size register num
  volatile UART_INFO     *info;                      // Run-Time Information
} const UART_RESOURCES;



