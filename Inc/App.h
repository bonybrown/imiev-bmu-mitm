/**
 * @file App.h
 * @brief Application interface class for CAN message processing
 * 
 * This header defines the App interface that must be implemented
 * by concrete application classes. The App receives CAN messages
 * and periodic time ticks from the main loop.
 */

#ifndef APP_H
#define APP_H

#include <stdint.h>
#include "can_types.h"
#include "BatteryModel.h"
#include "Diagnostic.h"

// Forward declaration
template<uint16_t CAPACITY> class CanQueue;

const float BATTERY_PACK_AH_CAPACITY = 93.0f; // Battery capacity in amp-hours

/**
 * @class App
 * @brief Application logic
 * 
 * Implement this interface to create custom CAN message processing
 * applications. The main loop will:
 * - Call canMsgReceived() for each incoming CAN frame
 * - Call timeTickMs() periodically for time-based processing
 */
class App {
public:
    App(CanQueue<QUEUE_CAPACITY>* txQueue, BatteryModel*batteryModel) :
     m_txQueue(txQueue),
     m_ticks(0), 
     m_one_second(1000),
     m_seconds(0),
     m_batteryModel(batteryModel),
     m_diagnostic() {} 
    /**
     * @brief Called when a CAN message is received
     * @param frame The received CAN frame
     * 
     * This method is called from the main loop whenever a frame
     * is available in the RxQueue. Process the message and optionally
     * queue responses to the TxQueue.
     */
    void canMsgReceived(const CAN_FRAME& frame);
    
    /**
     * @brief Called periodically for time-based processing
     * @param ms Time elapsed since last call in milliseconds
     * 
     * This method is called from the main loop, potentially every ms.
     * Use it for periodic tasks, timeouts, and state management.
     */
    void timeTickMs(uint32_t ms);
protected:
    CanQueue<QUEUE_CAPACITY>* m_txQueue; ///< Pointer to the TxQueue for sending messages
    uint32_t m_ticks;         ///< Internal tick counter
    int32_t m_one_second;    ///< Counter for one second intervals
    uint32_t m_seconds;      ///< Elapsed seconds counter
    BatteryModel* m_batteryModel; ///< Pointer to the BatteryModel instance
    Diagnostic m_diagnostic; ///< Diagnostic handler
};


#endif // APP_H
