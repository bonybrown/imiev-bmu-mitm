/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.cpp
 * @brief          : Main program with App class architecture
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

#include "main.h"
#include "can.h"
#include "iwdg.h"
#include "gpio.h"
#include <stdint.h>
#include "can_types.h"
#include "CanQueue.h"
#include "App.h"
#include "utility.h"
#include <stm32f1xx_hal_rcc_ex.h>

// CAN Queue instances
static CanQueue<QUEUE_CAPACITY> g_rxQueue;
static CanQueue<QUEUE_CAPACITY> g_TxQueue;
// Create App
BatteryModel g_batteryModel(BATTERY_PACK_AH_CAPACITY);
App g_app(&g_TxQueue, &g_batteryModel);
uint32_t g_lastTickTime = 0;

// Function prototypes
void SystemClock_Config(void);
void InitializeHardware(void);
void ProcessCanRx(void);
void ProcessCanTx(void);
void ProcessTick(void);

// CAN callbacks (defined in can_callbacks.cpp)
extern "C"
{
    void HAL_CAN_RxFIFO0MsgPendingCallback1(CAN_HandleTypeDef *canChan);
    void HAL_CAN_RxFIFO1MsgPendingCallback1(CAN_HandleTypeDef *canChan);
    void HAL_CAN_RxFIFO0MsgPendingCallback2(CAN_HandleTypeDef *canChan);
    void HAL_CAN_RxFIFO1MsgPendingCallback2(CAN_HandleTypeDef *canChan);
}

/**
 * @brief Main program entry point
 */
int main(void)
{
    // Initialize hardware
    InitializeHardware();

    g_lastTickTime = HAL_GetTick();

    // Main loop
    while (1)
    {
        // Process received CAN frames
        ProcessCanRx();

        // Process transmit CAN frames
        ProcessCanTx();

        // Process time tick
        ProcessTick();

        // Optional: Refresh watchdog
        // HAL_IWDG_Refresh(&hiwdg);
    }
}

/**
 * @brief Initialize all hardware peripherals
 */
void InitializeHardware(void)
{
    // Reset of all peripherals, Initializes the Flash interface and the Systick
    HAL_Init();

    // Configure the system clock
    SystemClock_Config();

    // Initialize peripherals
    MX_GPIO_Init();
    MX_CAN1_Init();
    MX_CAN2_Init();

    // Optional: Initialize watchdog
    // MX_IWDG_Init();

    // Register CAN callbacks - using callbacks for RxQueue integration
    HAL_CAN_RegisterCallback(&hcan1, HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID, HAL_CAN_RxFIFO0MsgPendingCallback1);
    HAL_CAN_RegisterCallback(&hcan1, HAL_CAN_RX_FIFO1_MSG_PENDING_CB_ID, HAL_CAN_RxFIFO1MsgPendingCallback1);
    HAL_CAN_RegisterCallback(&hcan2, HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID, HAL_CAN_RxFIFO0MsgPendingCallback2);
    HAL_CAN_RegisterCallback(&hcan2, HAL_CAN_RX_FIFO1_MSG_PENDING_CB_ID, HAL_CAN_RxFIFO1MsgPendingCallback2);

    // Activate CAN notifications
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING);
    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING);

    // Add CAN filters
    AddCANFilters(&hcan1);
    AddCANFilters(&hcan2);
}

/**
 * @brief Safely pop a CAN frame from the given queue with interrupts disabled
 * @param queue The CAN queue to pop from
 * @param frame Pointer to store the popped CAN frame
 * @return true if a frame was popped, false if the queue was empty
 */
inline bool SafePopCanQueue(CanQueue<QUEUE_CAPACITY> &queue, CAN_FRAME *frame)
{
    __disable_irq();
    bool result = queue.pop(frame);
    __enable_irq();
    return result;
}

/**
 * @brief Process received CAN frames from RxQueue
 */
void ProcessCanRx(void)
{
    CAN_FRAME frame;

    // Process all available frames in RxQueue
    while (SafePopCanQueue(g_rxQueue, &frame))
    {
        // Pass frame to App for processing
        g_app.canMsgReceived(frame);
    }
}

/**
 * @brief Process CAN frames to transmit from TxQueue
 */
void ProcessCanTx(void)
{
    CAN_FRAME frame;

    // Transmit all available frames in TxQueue
    while (g_TxQueue.peek(&frame))
    {
        CAN_HandleTypeDef *canChan = &hcan1;
        // Get CAN channel from frame
        if (frame.tx_channel > 0)
        {
            canChan = &hcan2;
        }

        // Check if there are free TX mailboxes
        if (HAL_CAN_GetTxMailboxesFreeLevel(canChan) > 0)
        {
            // Prepare CAN header
            CAN_TxHeaderTypeDef header;
            header.IDE = frame.ide;
            if (frame.ide)
            {
                header.ExtId = frame.ID;
            }
            else
            {
                header.StdId = frame.ID;
            }

            header.DLC = frame.dlc;
            header.RTR = frame.rtr;

            // Transmit the frame
            uint32_t mailbox;
            if (HAL_CAN_AddTxMessage(canChan, &header, frame.data, &mailbox) == HAL_OK)
            {
                // Successfully queued for transmission, remove from TxQueue
                g_TxQueue.pop(&frame);
            }
            else
            {
                // Failed to add to mailbox, try again later
                // exit the loop
                break;
            }
        }
        else
        {
            // No free mailboxes, try again later
            // exit the loop
            break;
        }
    }
}

/**
 * @brief Process periodic time tick for App
 */
void ProcessTick(void)
{
    // Call time tick handler every 1ms
    uint32_t currentTime = HAL_GetTick();
    if ((currentTime != g_lastTickTime))
    {
        uint32_t diff = CalculateTickDifference(currentTime, g_lastTickTime);
        g_lastTickTime = currentTime;
        g_app.timeTickMs(diff);
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV5;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_PLL2;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL2_ON;
    RCC_OscInitStruct.PLL2.PLL2MUL = RCC_PLL2_MUL8;
    RCC_OscInitStruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure the Systick interrupt time
     */
    __HAL_RCC_PLLI2S_ENABLE();
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    HAL_NVIC_SystemReset();
}

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Get access to RxQueue for CAN interrupt handlers
     * @return Pointer to RxQueue
     */
    CanQueue<QUEUE_CAPACITY> *GetRxQueue(void)
    {
        return &g_rxQueue;
    }

#ifdef __cplusplus
}
#endif
