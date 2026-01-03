# Main Program Architecture

## Architecture Overview

### Key Components

1. **main.cpp** - Main program
   - Initializes hardware
   - Creates App instance (with BatteryModel instance)
   - Main loop that:
     - Processes RX CAN frames from RxQueue → passes to App via `canMsgReceived()` method.
     - Processes TX CAN frames from TxQueue → sends to CAN bus
     - Calls App time tick periodically (up to every ms) via `timeTickMs()` method

2. **App.h / App.cpp** - App class implementation
   - Constructor takes TxQueue pointer: `App(CanQueue<QUEUE_CAPACITY>* txQueue)` and
   `BatteryModel` instance pointer.
   - `canMsgReceived(frame)` - Called for each received CAN frame
   - `timeTickMs(ms)` - Called for periodic tasks, up to once per millisecond.
   - App can queue outgoing messages by pushing onto the `m_txQueue` member.

3. **can_callbacks.cpp** - CAN interrupt handlers
   - Push received frames directly to RxQueue

4. **CanQueue.h** - CanQueue<QUEUE_CAPACITY>, a queue of `CAN_FRAME` elements for received/transmitted CAN frames.
Each `CAN_FRAME` element holds the CAN bus ID it was recieved
or will be transmitted on.
    - **RxQueue** - frames recieved on either CAN bus
        - Populated by CAN interrupt handlers
        - Consumed by main loop → passed to App
        - Access in main loop protected by IRQ disable/enable fence
    - **TxQueue** -  frames to transmit
        - Populated by App implementation
        - Consumed by main loop → sent via CAN
        - TxQueue never accessed by interrupts

5. **BatteryModel.cpp/BatteryModel.h** - implements the battery pack charge/discharge model.
    - `update` method recieves latest lowest-cell voltage and
    charge/discharge current.
    - has accessor methods to get calculated state-of-charge values.


## How to Use

### Accessing the TxQueue

The App class has direct access to the TxQueue via the `m_txQueue` member:

```cpp
void App::canMsgReceived(const CAN_FRAME& frame) {
    // Queue a message for transmission
    CAN_FRAME response;
    response.ID = 0x123;
    response.dlc = 8;
    response.tx_channel = 0;    // 0 = CAN1, 1 = CAN2. 
                                // Or refer to incoming frame.rx_channel member
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

