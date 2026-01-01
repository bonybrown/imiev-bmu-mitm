/**
 * @file can_callbacks.cpp
 * @brief CAN interrupt callbacks for main program
 * 
 * These callbacks push received CAN frames directly to the RxQueue
 */

#include "can.h"
#include "CanQueue.h"

// Implementation of GetRxQueue to provide access to RxQueue
// is in main.cpp
extern "C" {
    CanQueue<QUEUE_CAPACITY>* GetRxQueue(void);
}
/**
 * @brief Common implementation for CAN RX message handling
 * @param channel CAN channel index (0 for CAN1, 1 for CAN2)
 * @param canChan CAN handle
 * @param fifo FIFO number (CAN_RX_FIFO0 or CAN_RX_FIFO1)
 */
static void HandleCanRxMessage(uint8_t channel, CAN_HandleTypeDef *canChan, uint32_t fifo)
{
    CAN_RxHeaderTypeDef rxHeader;
    CAN_FRAME frame;
    
    if (HAL_CAN_GetRxMessage(canChan, fifo, &rxHeader, frame.data) == HAL_OK)
    {
        frame.ID = rxHeader.StdId;
        frame.dlc = rxHeader.DLC;
        frame.ide = rxHeader.IDE;
        frame.rtr = rxHeader.RTR;
        frame.rx_channel = channel;
        
        CanQueue<QUEUE_CAPACITY>* rxQueue = GetRxQueue();
        if (rxQueue != nullptr) {
            rxQueue->push(frame);
        }
    }
}

extern "C" {

/**
 * @brief CAN1 FIFO0 message pending callback
 */
void HAL_CAN_RxFIFO0MsgPendingCallback1(CAN_HandleTypeDef *canChan)
{
    HandleCanRxMessage(0, canChan, CAN_RX_FIFO0);
}

/**
 * @brief CAN1 FIFO1 message pending callback
 */
void HAL_CAN_RxFIFO1MsgPendingCallback1(CAN_HandleTypeDef *canChan)
{
    HandleCanRxMessage(0, canChan, CAN_RX_FIFO1);
}

/**
 * @brief CAN2 FIFO0 message pending callback
 */
void HAL_CAN_RxFIFO0MsgPendingCallback2(CAN_HandleTypeDef *canChan)
{
    HandleCanRxMessage(1, canChan, CAN_RX_FIFO0);
}

/**
 * @brief CAN2 FIFO1 message pending callback
 */
void HAL_CAN_RxFIFO1MsgPendingCallback2(CAN_HandleTypeDef *canChan)
{
    HandleCanRxMessage(1, canChan, CAN_RX_FIFO1);
}

} // extern "C"
