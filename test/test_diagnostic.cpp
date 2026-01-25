#include "CanQueue.h"
#include "version.h"
#include "CppUTest/TestHarness.h"
#include "Diagnostic.h"
#include "CanMessage374.h"
#include "can_types.h"

TEST_GROUP(Diagnostic)
{
    Diagnostic* diag;
    void setup() { diag = new Diagnostic(); }
    void teardown() { delete diag; }
};

TEST(Diagnostic, CanBeConstructed)
{
    CHECK(diag != nullptr);
}

TEST(Diagnostic, ApplyOverridesReturnsFalseByDefault)
{
    CAN_FRAME frame = {};
    frame.ID = 0x374;
    frame.dlc = 8;
    CanMessage374 msg(&frame);
    msg.setCellMinTemperature(TemperatureValue(15.0f));
    msg.setCellMaxTemperature(TemperatureValue(18.0f));
    CHECK_FALSE(diag->applyOverrides(msg));
}

TEST(Diagnostic, ProcessCommandWithInvalidFrameDoesNothing)
{
    CAN_FRAME frame = {};
    frame.dlc = 0; // Invalid
    diag->processCommand(frame); // Should not crash or throw
    // No assertion needed, just check for no crash
}

TEST(Diagnostic, ProcessSetTemperatureOverrideActivatesOverride)
{
    CAN_FRAME frame = {};
    frame.dlc = 3;
    frame.data[0] = Diagnostic::FUNCTION_SET_TEMPERATURE_OVERRIDE;
    frame.data[1] = TemperatureValue(30.0f).toCanByte();
    frame.data[2] = TemperatureValue(32.0f).toCanByte();
    diag->processCommand(frame);
    // Should now be active
    frame.ID = 0x374;
    frame.dlc = 8;
    CanMessage374 msg(&frame);
    msg.setCellMinTemperature(TemperatureValue(15.0f));
    msg.setCellMaxTemperature(TemperatureValue(18.0f));
    CHECK_TRUE(diag->applyOverrides(msg));
    DOUBLES_EQUAL(30.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(32.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
}

TEST(Diagnostic, SendDiagnosticDataProducesCorrectCANMessage)
{
    CanQueue<QUEUE_CAPACITY> txQueue;
    uint32_t seconds = 0x12345678;
    diag->sendDiagnosticData(&txQueue, seconds);

    // Should have two messages (one for each channel)
    CHECK_FALSE(txQueue.isEmpty());
    CAN_FRAME msg;
    CHECK_TRUE(txQueue.pop(&msg));
    CHECK_EQUAL(0x720, msg.ID);
    CHECK_EQUAL(8, msg.dlc);
    CHECK_EQUAL(0, msg.tx_channel);
    CHECK_EQUAL(ProjectVersion::MAJOR, msg.data[0]);
    CHECK_EQUAL(ProjectVersion::MINOR, msg.data[1]);
    CHECK_EQUAL(50, msg.data[2]);
    CHECK_EQUAL(50, msg.data[3]);
    uint32_t sent_seconds = (msg.data[4] << 24) | (msg.data[5] << 16) | (msg.data[6] << 8) | msg.data[7];
    CHECK_EQUAL(seconds, sent_seconds);

    // Second message, channel 1
    CHECK_FALSE(txQueue.isEmpty());
    CAN_FRAME msg2;
    CHECK_TRUE(txQueue.pop(&msg2));
    CHECK_EQUAL(0x720, msg2.ID);
    CHECK_EQUAL(8, msg2.dlc);
    CHECK_EQUAL(1, msg2.tx_channel);
    CHECK_EQUAL(ProjectVersion::MAJOR, msg2.data[0]);
    CHECK_EQUAL(ProjectVersion::MINOR, msg2.data[1]);
    CHECK_EQUAL(50, msg2.data[2]);
    CHECK_EQUAL(50, msg2.data[3]);
    uint32_t sent_seconds2 = (msg2.data[4] << 24) | (msg2.data[5] << 16) | (msg2.data[6] << 8) | msg2.data[7];
    CHECK_EQUAL(seconds, sent_seconds2);

    // Queue should now be empty
    CHECK_TRUE(txQueue.isEmpty());
}