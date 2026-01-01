# Main Program Architecture with App Class

This directory contains the main program structure that uses an object-oriented approach with the `App` class.

## Architecture Overview

### Key Components

1. **main.cpp** - Main program
   - Initializes hardware
   - Creates App instance
   - Main loop that:
     - Processes RX CAN frames from RxQueue → passes to App
     - Processes TX CAN frames from TxQueue → sends to CAN bus
     - Calls App time tick periodically (up to every ms)

2. **App.h / App.cpp** - App class implementation
   - Constructor takes TxQueue pointer: `App(CanQueue<QUEUE_CAPACITY>* txQueue)`
   - `canMsgReceived(frame)` - Called for each received CAN frame
   - `timeTickMs(ms)` - Called for periodic tasks, up to once per millisecond.
   - App can queue outgoing messages by pushing onto the `m_txQueue` member.

3. **RxQueue** - CanQueue<QUEUE_CAPACITY> for received CAN frames
   - Populated by CAN interrupt handlers
   - Consumed by main loop → passed to App
   - Access in main loop protected by IRQ disable/enable fence

4. **TxQueue** - CanQueue<QUEUE_CAPACITY> for frames to transmit
   - Populated by App implementation
   - Consumed by main loop → sent via CAN
   - TxQueue never accessed by interrupts

5. **can_callbacks.cpp** - CAN interrupt handlers
   - Push received frames directly to RxQueue

## How to Use

The App class is already instantiated as a global variable in main.cpp. You can modify the App class implementation directly in App.cpp to customize the application behavior.

### Accessing the TxQueue

The App class has direct access to the TxQueue via the `m_txQueue` member:

```cpp
void App::canMsgReceived(const CAN_FRAME& frame) {
    // Queue a message for transmission
    CAN_FRAME response;
    response.ID = 0x123;
    response.dlc = 8;
    response.tx_channel = 0;  // 0 = CAN1, 1 = CAN2. Or refer to incoming frame.rx_channel
    // ... fill frame data
    m_txQueue->push(response);
}
```

## Message Flow

CAN messages are passed via separate TX and RX queues. Each queue holds up to 64 `CAN_FRAME` entries.
(configurable in can_types.h).
Each `CAN_FRAME` in the queue has a `rx_channel` or `tx_channel`
member that determines which CAN channel the message was received
from or will be sent to.

### Receive Path
```
CAN Hardware → CAN ISR
[interrupt]
  HAL_CAN_RxFIFO[12]MsgPendingCallback[12]() → HAL_CAN_GetRxMessage()
  → RxQueue.push()
[main]
  main loop → RxQueue.pop() → app->canMsgReceived()
```

### Transmit Path
```
[main]
app logic → TxQueue.push() → main loop → TxQueue.pop() 
  → HAL_CAN_AddTxMessage() → CAN Hardware
```

### Time Tick
```
[main]
main loop (when at least 1ms has passed) → app->timeTickMs(elapsed_milliseconds)
```

