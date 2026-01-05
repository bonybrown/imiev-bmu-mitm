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
#include "version.h"
/**
 * @brief Process received CAN messages
 */
void App::canMsgReceived(const CAN_FRAME &frame)
{
    bool sendResponse = true;
    // copy the frame to modify if needed
    CAN_FRAME response = frame;

    // Update the battery model with data from message 0x373, received every 100ms
    if (frame.ID == 0x373)
    {
        CanMessage373 rxMsg(&frame);
        VoltageByte cellMin = rxMsg.getCellMinVoltage();
        float packCurrent = rxMsg.getPackCurrent();
        m_batteryModel->update(cellMin, packCurrent, CanMessage373::RECURRANCE_MS);
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
        // Only send response if battery model is initialized
        sendResponse = m_batteryModel->isInitialized();
    }

    // Send responses back on opposite channel they were received from
    response.tx_channel = frame.rx_channel ? 0 : 1;
    // Push the response to the TxQueue, if sending this frame
    if (sendResponse)
    {
        m_txQueue->push(response);
    }
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
    heartbeat.ID = 0x720; /// Heartbeat message ID, value chosen arbitrarily, but believed to be unused
    heartbeat.dlc = 8;
    heartbeat.ide = 0;
    heartbeat.rtr = 0;
    heartbeat.tx_channel = 0;

    // Hearbeat data for ID 0x720
    // Bytes 0-1 = major/minor version of the software (e.g., 1.0)
    heartbeat.data[0] = ProjectVersion::MAJOR;
    heartbeat.data[1] = ProjectVersion::MINOR;
    // Byte 2 = reserved
    heartbeat.data[2] = 0;
    // Byte 3 = reserved
    heartbeat.data[3] = 0;
    // Bytes 4-7 = uptime in seconds (uint32_t)
    heartbeat.data[4] = (m_seconds >> 24) & 0xFF;
    heartbeat.data[5] = (m_seconds >> 16) & 0xFF;
    heartbeat.data[6] = (m_seconds >> 8) & 0xFF;
    heartbeat.data[7] = m_seconds & 0xFF;

    // Queue for transmission on both channels
    m_txQueue->push(heartbeat);
    heartbeat.tx_channel = 1;
    m_txQueue->push(heartbeat);
}
