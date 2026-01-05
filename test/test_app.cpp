/**
 * @file test_app.cpp
 * @brief Unit tests for App class
 */

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "App.h"
#include "CanQueue.h"
#include "can_types.h"
#include "VoltageByte.h"
#include <CanMessage374.h>

// Mock BatteryModel for testing
class MockBatteryModel : public BatteryModel
{
public:
    MockBatteryModel(float capacity) : BatteryModel(capacity) {}

    void update(VoltageByte cellMinVoltage, float packCurrent, uint32_t deltaTMs) override
    {
        mock().actualCall("update").onObject(this).withParameter("cellMinVoltage", cellMinVoltage.get()).withParameter("packCurrent", packCurrent).withParameter("deltaTMs", deltaTMs);
        // Also call the real implementation if needed
        BatteryModel::update(cellMinVoltage, packCurrent, deltaTMs);
    }
};

void InitializeBatteryModel(MockBatteryModel* model)
{
    mock().expectNCalls(25, "update").onObject(model).withParameter("cellMinVoltage", VoltageByte::fromVoltage(4.0f).get()).withParameter("packCurrent", 1.0f).withParameter("deltaTMs", 100);
    // Send multiple updates to initialize battery model
    for (int i = 0; i < 25; i++)
    {
        model -> update(VoltageByte::fromVoltage(4.0f), 1.0f, 100); //100ms of 4.0 volts, 1A current
    }

}

TEST_GROUP(App_CanMsgReceived)
{
    CanQueue<QUEUE_CAPACITY> *txQueue;
    App *app;
    MockBatteryModel *batteryModel;

    void setup()
    {
        batteryModel = new MockBatteryModel(BATTERY_PACK_AH_CAPACITY);
        txQueue = new CanQueue<QUEUE_CAPACITY>();
        app = new App(txQueue, batteryModel);
    }

    void teardown()
    {
        delete app;
        delete txQueue;
        delete batteryModel;
        mock().clear();
    }
};

TEST(App_CanMsgReceived, MessageAddedToTxQueue_Channel0To1)
{
    // Create a CAN frame received on channel 0
    CAN_FRAME rxFrame;
    rxFrame.ID = 0x123;
    rxFrame.dlc = 8;
    rxFrame.ide = 0;
    rxFrame.rtr = 0;
    rxFrame.rx_channel = 0;
    rxFrame.data[0] = 0x11;
    rxFrame.data[1] = 0x22;
    rxFrame.data[2] = 0x33;
    rxFrame.data[3] = 0x44;
    rxFrame.data[4] = 0x55;
    rxFrame.data[5] = 0x66;
    rxFrame.data[6] = 0x77;
    rxFrame.data[7] = 0x88;

    // Verify queue is empty
    CHECK(txQueue->isEmpty());

    // Call canMsgReceived
    app->canMsgReceived(rxFrame);

    // Verify message was added to tx queue
    CHECK_FALSE(txQueue->isEmpty());
    LONGS_EQUAL(1, txQueue->length());

    // Get the message from the queue
    CAN_FRAME txFrame;
    CHECK(txQueue->pop(&txFrame));

    // Verify all fields are the same except tx_channel
    LONGS_EQUAL(rxFrame.ID, txFrame.ID);
    LONGS_EQUAL(rxFrame.dlc, txFrame.dlc);
    LONGS_EQUAL(rxFrame.ide, txFrame.ide);
    LONGS_EQUAL(rxFrame.rtr, txFrame.rtr);

    // Verify tx_channel is opposite of rx_channel
    LONGS_EQUAL(1, txFrame.tx_channel);

    // Verify data is unchanged
    for (int i = 0; i < 8; i++)
    {
        LONGS_EQUAL(rxFrame.data[i], txFrame.data[i]);
    }
}

TEST(App_CanMsgReceived, MessageAddedToTxQueue_Channel1To0)
{
    // Create a CAN frame received on channel 1
    CAN_FRAME rxFrame;
    rxFrame.ID = 0x456;
    rxFrame.dlc = 5;
    rxFrame.ide = 0;
    rxFrame.rtr = 0;
    rxFrame.rx_channel = 1;
    rxFrame.data[0] = 0xAA;
    rxFrame.data[1] = 0xBB;
    rxFrame.data[2] = 0xCC;
    rxFrame.data[3] = 0xDD;
    rxFrame.data[4] = 0xEE;

    // Verify queue is empty
    CHECK(txQueue->isEmpty());

    // Call canMsgReceived
    app->canMsgReceived(rxFrame);

    // Verify message was added to tx queue
    CHECK_FALSE(txQueue->isEmpty());
    LONGS_EQUAL(1, txQueue->length());

    // Get the message from the queue
    CAN_FRAME txFrame;
    CHECK(txQueue->pop(&txFrame));

    // Verify all fields are the same except tx_channel
    LONGS_EQUAL(rxFrame.ID, txFrame.ID);
    LONGS_EQUAL(rxFrame.dlc, txFrame.dlc);
    LONGS_EQUAL(rxFrame.ide, txFrame.ide);
    LONGS_EQUAL(rxFrame.rtr, txFrame.rtr);

    // Verify tx_channel is opposite of rx_channel
    LONGS_EQUAL(0, txFrame.tx_channel);

    // Verify data is unchanged (only first dlc bytes are relevant)
    for (int i = 0; i < rxFrame.dlc; i++)
    {
        LONGS_EQUAL(rxFrame.data[i], txFrame.data[i]);
    }
}

TEST(App_CanMsgReceived, Message373_not_modified)
{
    // Create a CAN frame received on channel 1
    CAN_FRAME rxFrame;
    rxFrame.ID = 0x373;
    rxFrame.dlc = 8;
    rxFrame.ide = 0;
    rxFrame.rtr = 0;
    rxFrame.rx_channel = 1;
    rxFrame.data[0] = 0xAA;
    rxFrame.data[1] = 0xBB;
    rxFrame.data[2] = 0xCC;
    rxFrame.data[3] = 0xDD;
    rxFrame.data[4] = 0xEE;
    rxFrame.data[5] = 0xFF;
    rxFrame.data[6] = 0x11;
    rxFrame.data[7] = 0x22;

    // Verify queue is empty
    CHECK(txQueue->isEmpty());

    // Expect the battery model update to be called (0x373 triggers it)
    mock().ignoreOtherCalls();

    // Call canMsgReceived
    app->canMsgReceived(rxFrame);

    // Verify message was added to tx queue
    CHECK_FALSE(txQueue->isEmpty());
    LONGS_EQUAL(1, txQueue->length());

    // Get the message from the queue
    CAN_FRAME txFrame;
    CHECK(txQueue->pop(&txFrame));

    // Verify all fields are the same except tx_channel
    LONGS_EQUAL(rxFrame.ID, txFrame.ID);
    LONGS_EQUAL(rxFrame.dlc, txFrame.dlc);
    LONGS_EQUAL(rxFrame.ide, txFrame.ide);
    LONGS_EQUAL(rxFrame.rtr, txFrame.rtr);

    // Verify tx_channel is opposite of rx_channel
    LONGS_EQUAL(0, txFrame.tx_channel);

    // Verify data is unchanged (only first dlc bytes are relevant)
    for (int i = 0; i < rxFrame.dlc; i++)
    {
        LONGS_EQUAL(rxFrame.data[i], txFrame.data[i]);
    }
}

TEST(App_CanMsgReceived, Message373_updates_model)
{
    CAN_FRAME frame;
    frame.ID = 0x373;
    frame.dlc = 8;
    frame.ide = 0; // Standard ID
    frame.rtr = 0; // Data frame
    frame.rx_channel = 1;

    // D0: Max voltage = 4.1V -> (4.1 * 100) - 210 = 200
    frame.data[0] = 200;

    // D1: Min voltage = 4.1V -> (4.1 * 100) - 210 = 200
    frame.data[1] = 200;

    // D2-D3: Current = 10A -> (10 * 100) + 32700 = 33700 = 0x83C4
    uint16_t current_raw = 33700;
    frame.data[2] = (current_raw >> 8) & 0xFF; // High byte = 0x83
    frame.data[3] = current_raw & 0xFF;        // Low byte = 0xC4

    // D4-D5: Pack voltage = 360.8V -> (360.8 * 10) = 3608 = 0x0E18
    uint16_t voltage_raw = 3608;
    frame.data[4] = (voltage_raw >> 8) & 0xFF; // High byte = 0x0E
    frame.data[5] = voltage_raw & 0xFF;        // Low byte = 0x18

    // D6-D7: Unused (can be set to 0)
    frame.data[6] = 0;
    frame.data[7] = 0;

    // Verify queue is empty
    CHECK(txQueue->isEmpty());

    // Call canMsgReceived
    VoltageByte expectedVoltage = VoltageByte::fromVoltage(4.10f);
    mock().expectOneCall("update").onObject(batteryModel).withParameter("cellMinVoltage", expectedVoltage.get()).withParameter("packCurrent", 10.0f).withParameter("deltaTMs", 10);
    app->canMsgReceived(frame);
    mock().checkExpectations();

    // Verify message was added to tx queue
    CHECK_FALSE(txQueue->isEmpty());
    LONGS_EQUAL(1, txQueue->length());

    // Get the message from the queue
    CAN_FRAME txFrame;
    CHECK(txQueue->pop(&txFrame));

    // Verify all fields are the same except tx_channel
    LONGS_EQUAL(frame.ID, txFrame.ID);
    LONGS_EQUAL(frame.dlc, txFrame.dlc);
    LONGS_EQUAL(frame.ide, txFrame.ide);
    LONGS_EQUAL(frame.rtr, txFrame.rtr);

    // Verify tx_channel is opposite of rx_channel
    LONGS_EQUAL(0, txFrame.tx_channel);

    // Verify data is unchanged (only first dlc bytes are relevant)
    for (int i = 0; i < frame.dlc; i++)
    {
        LONGS_EQUAL(frame.data[i], txFrame.data[i]);
    }
}

TEST(App_CanMsgReceived, Message374_is_replaced)
{
    CAN_FRAME frame;
    frame.ID = 0x374;
    frame.dlc = 8;
    frame.ide = 0; // Standard ID
    frame.rtr = 0; // Data frame
    frame.rx_channel = 0;
    CanMessage374 rxMsg(&frame);

    rxMsg.setSoC1(75.5f);
    rxMsg.setSoC2(73.0f);
    rxMsg.setCellMaxTemperature(28.0f);
    rxMsg.setCellMinTemperature(25.0f);
    rxMsg.setBatteryCapacity(58.0f);

    // Verify queue is empty
    CHECK(txQueue->isEmpty());
    //ensure the battery model is initialized
    InitializeBatteryModel(batteryModel);

    // Call canMsgReceived
    CAN_FRAME frameCopy = frame; // Make a copy to pass
    app->canMsgReceived(frameCopy);
    mock().checkExpectations();

    // Verify message was added to tx queue
    CHECK_FALSE(txQueue->isEmpty());
    LONGS_EQUAL(1, txQueue->length());

    // Get the message from the queue
    CAN_FRAME txFrame;
    CHECK(txQueue->pop(&txFrame));

    // Verify all fields are the same except tx_channel
    LONGS_EQUAL(frame.ID, txFrame.ID);
    LONGS_EQUAL(frame.dlc, txFrame.dlc);
    LONGS_EQUAL(frame.ide, txFrame.ide);
    LONGS_EQUAL(frame.rtr, txFrame.rtr);

    // Verify tx_channel is opposite of rx_channel
    LONGS_EQUAL(1, txFrame.tx_channel);

    // Verify data is changed
    CanMessage374 txMsg(&txFrame);
    CHECK(txMsg.getSoC1() != rxMsg.getSoC1());
    CHECK(txMsg.getSoC2() != rxMsg.getSoC2());
    CHECK(txMsg.getBatteryCapacity() != rxMsg.getBatteryCapacity());

    // temperatures should be the same
    DOUBLES_EQUAL(rxMsg.getCellMaxTemperature(), txMsg.getCellMaxTemperature(), 0.01);
    DOUBLES_EQUAL(rxMsg.getCellMinTemperature(), txMsg.getCellMinTemperature(), 0.01);
}

TEST(App_CanMsgReceived, Message374_data_matches_battery_model)
{
    mock().ignoreOtherCalls();
    // Set battery model to known values by sending 0x373 messages first
    CAN_FRAME frame373;
    frame373.ID = 0x373;
    frame373.dlc = 8;
    frame373.ide = 0; // Standard ID
    frame373.rtr = 0; // Data frame
    frame373.rx_channel = 0;
    // D0: Max voltage = 4.0V -> (4.0 * 100) - 210 = 190
    frame373.data[0] = 190;
    // D1: Min voltage = 4.0V -> (4.0 * 100) - 210 = 190
    frame373.data[1] = 190;
    // D2-D3: Current = 0A -> (0 * 100) + 32700 = 32700 = 0x7F34
    uint16_t current_raw = 32700;
    frame373.data[2] = (current_raw >> 8) & 0xFF; // High byte
    frame373.data[3] = current_raw & 0xFF;        // Low byte
    // D4-D5: Pack voltage = 360.0V -> (360.0 * 10) = 3600 = 0x0E10
    uint16_t voltage_raw = 3600;
    frame373.data[4] = (voltage_raw >> 8) & 0xFF; // High byte
    frame373.data[5] = voltage_raw & 0xFF;        // Low byte
    // D6-D7: Unused
    frame373.data[6] = 0;
    frame373.data[7] = 0;
    // Send multiple 0x373 messages to initialize battery model
    for (int i = 0; i < 25; i++)
    {
        app->canMsgReceived(frame373);
        CHECK(txQueue->pop(nullptr)); // Discard tx message
    }

    CAN_FRAME frame;
    frame.ID = 0x374;
    frame.dlc = 8;
    frame.ide = 0; // Standard ID
    frame.rtr = 0; // Data frame
    frame.rx_channel = 0;
    CanMessage374 rxMsg(&frame);

    rxMsg.setSoC1(75.5f);
    rxMsg.setSoC2(73.0f);
    rxMsg.setCellMaxTemperature(22.0f);
    rxMsg.setCellMinTemperature(23.0f);
    rxMsg.setBatteryCapacity(58.0f);

    // Verify queue is empty
    CHECK(txQueue->isEmpty());

    // Call canMsgReceived
    CAN_FRAME frameCopy = frame; // Make a copy to pass
    app->canMsgReceived(frameCopy);
    mock().checkExpectations();

    // Verify message was added to tx queue
    CHECK_FALSE(txQueue->isEmpty());
    LONGS_EQUAL(1, txQueue->length());

    // Get the message from the queue
    CAN_FRAME txFrame;
    CHECK(txQueue->pop(&txFrame));

    // Verify all fields are the same except tx_channel
    LONGS_EQUAL(frame.ID, txFrame.ID);
    LONGS_EQUAL(frame.dlc, txFrame.dlc);
    LONGS_EQUAL(frame.ide, txFrame.ide);
    LONGS_EQUAL(frame.rtr, txFrame.rtr);

    // Verify tx_channel is opposite of rx_channel
    LONGS_EQUAL(1, txFrame.tx_channel);

    // Verify data is changed
    CanMessage374 txMsg(&txFrame);
    DOUBLES_EQUAL(batteryModel->getSoC1(), txMsg.getSoC1(), 0.05);
    DOUBLES_EQUAL(batteryModel->getSoC2(), txMsg.getSoC2(), 0.05);
    DOUBLES_EQUAL(batteryModel->getCapacity(), txMsg.getBatteryCapacity(), 0.01);
    // temperatures should be the same
    DOUBLES_EQUAL(rxMsg.getCellMaxTemperature(), txMsg.getCellMaxTemperature(), 0.01);
    DOUBLES_EQUAL(rxMsg.getCellMinTemperature(), txMsg.getCellMinTemperature(), 0.01);
}

TEST(App_CanMsgReceived, RealWorldData)
{
    // data from real-world capture of message 0x373 on channel 1
    CAN_FRAME rxFrame;
    rxFrame.ID = 0x373;
    rxFrame.dlc = 8;
    rxFrame.ide = 0;
    rxFrame.rtr = 0;
    rxFrame.rx_channel = 1;
    rxFrame.data[0] = 0xc8;
    rxFrame.data[1] = 0xc7;
    rxFrame.data[2] = 0x7f;
    rxFrame.data[3] = 0xb9;
    rxFrame.data[4] = 0x0e;
    rxFrame.data[5] = 0x14;
    rxFrame.data[6] = 0x00;
    rxFrame.data[7] = 0x06;

    // Verify queue is empty
    CHECK(txQueue->isEmpty());

    // Expect the battery model update to be called (0x373 triggers it)
    mock().ignoreOtherCalls();

    // Call canMsgReceived
    for(int i = 0; i < 100; i++){
        app->canMsgReceived(rxFrame);
        txQueue->pop(nullptr); // discard tx message
    }
    CHECK(batteryModel->isInitialized());
    LONGS_EQUAL(98, batteryModel->getSoC1());
    LONGS_EQUAL(98, batteryModel->getSoC2());

    CAN_FRAME msg374;
    msg374.ID = 0x374;
    msg374.dlc = 8;
    msg374.ide = 0;
    msg374.rtr = 0;
    msg374.rx_channel = 1;
    msg374.data[0] = 0x01;
    msg374.data[1] = 0x02;
    msg374.data[2] = 0x03;
    msg374.data[3] = 0x04;
    msg374.data[4] = 0x05;
    msg374.data[5] = 0x06;
    msg374.data[6] = 0x07;
    msg374.data[7] = 0x08;

    // Call canMsgReceived
    app->canMsgReceived(msg374);
    
    // Verify message was added to tx queue
    CHECK_FALSE(txQueue->isEmpty());
    LONGS_EQUAL(1, txQueue->length());

    // Get the message from the queue
    CAN_FRAME txFrame;
    CHECK(txQueue->pop(&txFrame));

    // Verify all fields are the same except tx_channel
    LONGS_EQUAL(msg374.ID, txFrame.ID);
    LONGS_EQUAL(msg374.dlc, txFrame.dlc);
    LONGS_EQUAL(msg374.ide, txFrame.ide);
    LONGS_EQUAL(msg374.rtr, txFrame.rtr);

    // Verify tx_channel is opposite of rx_channel
    LONGS_EQUAL(0, txFrame.tx_channel);

    CHECK(msg374.data[0] == 0xd0 ); // soc1 = 98
    CHECK(msg374.data[1] == 0xd0 ); // soc2 = 98
    CHECK(msg374.data[2] == 0x03 ); // data unchanged
    CHECK(msg374.data[3] == 0x04 ); // data unchanged
    CHECK(msg374.data[4] == 0x05 ); // max temp unchanged
    CHECK(msg374.data[5] == 0x06 ); // min temp unchanged
    CHECK(msg374.data[6] == 0xba ); // new capacity
    CHECK(msg374.data[7] == 0x08 ); // data unchanged

}

TEST(App_CanMsgReceived, Message374IsNotFowardedUnlessModelInitialized)
{
    CAN_FRAME frame;
    frame.ID = 0x374;
    frame.dlc = 8;
    frame.ide = 0; // Standard ID
    frame.rtr = 0; // Data frame
    frame.rx_channel = 0;
    CanMessage374 rxMsg(&frame);

    // Verify queue is empty
    CHECK(txQueue->isEmpty());
    // Verify model is not initialized
    CHECK_FALSE(batteryModel->isInitialized());

    // Expect no calls to battery model update
    mock().expectNoCall("update");

    // Call canMsgReceived
    app->canMsgReceived(frame);
    mock().checkExpectations();

    // Verify model is still not initialized
    CHECK_FALSE(batteryModel->isInitialized());

    // Verify message was NOT added to tx queue
    CHECK(txQueue->isEmpty());
}
