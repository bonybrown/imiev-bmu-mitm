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
#include <TemperatureValue.h>
/**
 * @brief Process received CAN messages
 */
void App::canMsgReceived(const CAN_FRAME &frame)
{
    bool forwardFrame = true;
    // copy the frame to modify if needed
    CAN_FRAME response = frame;

    // Update the battery model with data from message 0x373, received every 100ms
    if (frame.ID == CanMessage373::MESSAGE_ID)
    {
        CanMessage373 rxMsg(&frame);
        VoltageByte cellMin = rxMsg.getCellMinVoltage();
        float packCurrent = rxMsg.getPackCurrent();
        m_batteryModel->update(cellMin, packCurrent, CanMessage373::RECURRANCE_MS);
    }

    // Modify message 0x374 with updated SoC values
    else if (frame.ID == CanMessage374::MESSAGE_ID)
    {
        CanMessage374 rxMsg(&frame);
        // Apply any overrides first
        m_diagnostic.applyOverrides(rxMsg);
        // Modify some fields before sending back
        rxMsg.setBatteryCapacity(m_batteryModel->getCapacity());
        rxMsg.setSoC1(m_batteryModel->getSoC1());
        rxMsg.setSoC2(m_batteryModel->getSoC2());
        // Leave temperatures unchanged
        response = *(rxMsg.getFrame());
        // Only send response if battery model is initialized
        forwardFrame = m_batteryModel->isInitialized();
    }
    // Process diagnostic commands
    else if (frame.ID == Diagnostic::COMMAND_MESSAGE_ID) // Diagnostic command ID
    {
        forwardFrame = false; // Do not forward command frames
        m_diagnostic.processCommand(frame);
    }

    // Send responses back on opposite channel they were received from
    response.tx_channel = frame.rx_channel ? 0 : 1;
    // Push the response to the TxQueue, if sending this frame
    if (forwardFrame)
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
        m_diagnostic.sendDiagnosticData(m_txQueue, m_seconds);
    }

}
