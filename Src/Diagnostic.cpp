#include "Diagnostic.h"
#include <TemperatureValue.h>
#include "version.h"

Diagnostic::Diagnostic()
    : m_canMessage374Override()
{
}

void Diagnostic::processCommand(const CAN_FRAME& frame)
{
    if (frame.dlc < 1) {
        return; // Invalid frame
    }

    uint8_t function = frame.data[0];

    switch (function) {
        case Diagnostic::FUNCTION_SET_TEMPERATURE_OVERRIDE:
            processSetTemperatureOverride(frame);
            break;
        
        default:
            // Unknown function, ignore
            break;
    }
}

bool Diagnostic::applyOverrides(CanMessage374& msg)
{
    return m_canMessage374Override.applyOverride(msg);
}

void Diagnostic::processSetTemperatureOverride(const CAN_FRAME& frame)
{
    if (frame.dlc < 3) {
        return; // Need at least 3 bytes: function, minTemp, maxTemp
    }

    auto constexpr frames_per_10_seconds = (10000 / CanMessage374::RECURRENCE_MS);

    // data[1]: Min temperature in usual CAN format
    TemperatureValue tempMin(frame.data[1]);

    // data[2]: Max temperature in usual CAN format
    TemperatureValue tempMax(frame.data[2]);

    // Set the override
    m_canMessage374Override.setOverride(tempMin, tempMax, frames_per_10_seconds);
}

void Diagnostic::sendDiagnosticData(CanQueue<QUEUE_CAPACITY>* txQueue, uint32_t seconds)
{
    CAN_FRAME frame = {};
    frame.ID = Diagnostic::HEARTBEAT_MESSAGE_ID; // Diagnostic message ID
    frame.dlc = 8;
    frame.ide = 0; // Standard ID
    frame.rtr = 0; // Data frame
    frame.tx_channel = 0; // Use channel 0 for transmission

     // Bytes 0-1 = major/minor version of the software (e.g., 1.0)
     frame.data[0] = ProjectVersion::MAJOR;
     frame.data[1] = ProjectVersion::MINOR;
     // Byte 2 = reserved
     frame.data[2] = m_canMessage374Override.getOriginalMinTemp().toCanByte();
     // Byte 3 = reserved
     frame.data[3] = m_canMessage374Override.getOriginalMaxTemp().toCanByte();
     // Bytes 4-7 = uptime in seconds (uint32_t)
     frame.data[4] = (seconds >> 24) & 0xFF;
     frame.data[5] = (seconds >> 16) & 0xFF;
     frame.data[6] = (seconds >> 8) & 0xFF;
     frame.data[7] = seconds & 0xFF;

    // Push the diagnostic message to the TxQueue
    txQueue->push(frame);

    frame.tx_channel = 1; // And send on channel 1 as well
    txQueue->push(frame);
}