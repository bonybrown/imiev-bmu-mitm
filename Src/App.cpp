/**
 * @file App.cpp
 * @brief Implementation of the App class
 *
 * This provides the default implementation of the App class methods.
 */

#include "App.h"
#include "CanQueue.h"
#include <CanMessage373.h>
#include <stdio.h>
#include <CanMessage374.h>
/**
 * @brief Process received CAN messages
 */
void App::canMsgReceived(const CAN_FRAME &frame)
{
    // copy the frame to modify if needed
    CAN_FRAME response = frame;

    // Update the battery model with data from message 0x373, received every 100ms
    if (frame.ID == 0x373)
    {
        CanMessage373 rxMsg(&frame);
        VoltageByte cellMin = rxMsg.getCellMinVoltage();
        float packCurrent = rxMsg.getPackCurrent();
        m_batteryModel->update(cellMin, packCurrent, 100); // deltaTMs = 100ms
    }

    // Modify message 0x374 with updated SoC values
    else if (frame.ID == 0x374)
    {
        CanMessage374 rxMsg(&frame);
        // Modify some fields before sending back
        rxMsg.setBatteryCapacity(m_batteryModel->getCapacity());
        rxMsg.setSoC1(m_batteryModel->getSoC1());
        rxMsg.setSoC2(m_batteryModel->getSoC2());
        // Leave temperatures unchanged
        response = *(rxMsg.getFrame());
    }

    // Send responses back on opposite channel they were received from
    response.tx_channel = frame.rx_channel ? 0 : 1;
    // Push the response to the TxQueue
    m_txQueue->push(response);
}

/**
 * @brief Handle periodic time ticks
 */
void App::timeTickMs(uint32_t ms)
{
    m_ticks += ms;

    m_one_second -= ms;
    if (m_one_second <= 0)
    {
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
void App::sendHeartbeat()
{
    CAN_FRAME heartbeat;
    heartbeat.ID = 0x720; // Example heartbeat ID
    heartbeat.dlc = 8;
    heartbeat.ide = 0;
    heartbeat.rtr = 0;
    heartbeat.tx_channel = 0;

    heartbeat.data[0] = (m_seconds >> 24) & 0xFF;
    heartbeat.data[1] = (m_seconds >> 16) & 0xFF;
    heartbeat.data[2] = (m_seconds >> 8) & 0xFF;
    heartbeat.data[3] = m_seconds & 0xFF;
    heartbeat.data[4] = m_batteryModel->isInitialized(); // Reserved
    heartbeat.data[5] = m_batteryModel->getRemainingAh1(); // Reserved
    heartbeat.data[6] = m_batteryModel->getRemainingAh2(); // Reserved
    heartbeat.data[7] = m_batteryModel->getValidDataCounter(); // Reserved

    // Queue for transmission
    m_txQueue->push(heartbeat);

    heartbeat.tx_channel = 1;
    heartbeat.ID = 0x721; // Example heartbeat ID for second channel
    m_txQueue->push(heartbeat);
}
