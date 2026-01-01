/**
 * @file App.cpp
 * @brief Implementation of the App class
 * 
 * This provides the default implementation of the App class methods.
 */

#include "App.h"
#include "CanQueue.h"

/**
 * @brief Process received CAN messages
 */
void App::canMsgReceived(const CAN_FRAME& frame) {
        // Send messages back on the opposite channel
        CAN_FRAME response = frame;
        response.tx_channel = frame.rx_channel ? 0 : 1; // Send back on opposite channel
        
        // Push the response to the TxQueue
        m_txQueue->push(response);
        
        // Add your custom message processing logic here
        // For example:
        // - Parse message data
        // - Update internal state
        // - Generate responses
        // - Store values for later processing
    }
    
/**
 * @brief Handle periodic time ticks
 */
void App::timeTickMs(uint32_t ms) {
    m_ticks += ms;

    m_one_second -= ms;
    if (m_one_second <= 0) {
        m_one_second += 1000;
        m_seconds++;
        sendHeartbeat();
    }        
    
    // Add your periodic tasks here:
    // - Check timeouts
    // - Update state machines
    // - Send periodic messages
    // - Monitor system health
}

/**
 * @brief Send a heartbeat CAN message
 */
void App::sendHeartbeat() {
    CAN_FRAME heartbeat;
    heartbeat.ID = 0x720;  // Example heartbeat ID
    heartbeat.dlc = 8;
    heartbeat.ide = 0;
    heartbeat.rtr = 0;
    heartbeat.tx_channel = 0;
    
    heartbeat.data[0] = (m_seconds >> 24) & 0xFF;
    heartbeat.data[1] = (m_seconds >> 16) & 0xFF;
    heartbeat.data[2] = (m_seconds >> 8) & 0xFF;
    heartbeat.data[3] = m_seconds & 0xFF;
    heartbeat.data[4] = 0; // Reserved
    heartbeat.data[5] = 0; // Reserved
    heartbeat.data[6] = 0; // Reserved
    heartbeat.data[7] = 0; // Reserved
    
    // Queue for transmission
    //m_txQueue->push(heartbeat);

    heartbeat.tx_channel = 1;
    heartbeat.ID = 0x721;  // Example heartbeat ID for second channel
    m_txQueue->push(heartbeat);
}
