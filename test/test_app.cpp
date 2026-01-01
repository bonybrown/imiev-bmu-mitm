/**
 * @file test_app.cpp
 * @brief Unit tests for App class
 */

#include "CppUTest/TestHarness.h"
#include "App.h"
#include "CanQueue.h"
#include "can_types.h"

TEST_GROUP(App_CanMsgReceived)
{
    CanQueue<QUEUE_CAPACITY>* txQueue;
    App* app;
    
    void setup() {
        txQueue = new CanQueue<QUEUE_CAPACITY>();
        app = new App(txQueue);
    }
    
    void teardown() {
        delete app;
        delete txQueue;
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
    for (int i = 0; i < 8; i++) {
        LONGS_EQUAL(rxFrame.data[i], txFrame.data[i]);
    }
}

TEST(App_CanMsgReceived, MessageAddedToTxQueue_Channel1To0)
{
    // Create a CAN frame received on channel 1
    CAN_FRAME rxFrame;
    rxFrame.ID = 0x456;
    rxFrame.dlc = 5;
    rxFrame.ide = 1;
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
    for (int i = 0; i < rxFrame.dlc; i++) {
        LONGS_EQUAL(rxFrame.data[i], txFrame.data[i]);
    }
}

TEST(App_CanMsgReceived, Message373_not_modified)
{
    // Create a CAN frame received on channel 1
    CAN_FRAME rxFrame;
    rxFrame.ID = 0x373;
    rxFrame.dlc = 8;
    rxFrame.ide = 1;
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
    for (int i = 0; i < rxFrame.dlc; i++) {
        LONGS_EQUAL(rxFrame.data[i], txFrame.data[i]);
    }
}
