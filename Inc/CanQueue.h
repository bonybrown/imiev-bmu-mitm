/**
 * @file CanQueue.h
 * @brief Fixed-size queue for CAN_FRAME items
 * 
 * A simple circular buffer implementation for storing CAN frames.
 * Thread-safe operations require external synchronization.
 */

#ifndef CAN_QUEUE_H
#define CAN_QUEUE_H

#include <stdint.h>
#include "can_types.h"

/**
 * @class CanQueue
 * @brief Fixed-size circular queue for CAN frames
 * 
 * Provides a simple FIFO queue implementation for CAN_FRAME objects.
 * The queue is implemented as a circular buffer with a fixed capacity
 * determined at compile time via template parameter.
 * 
 * @tparam CAPACITY Maximum number of CAN frames the queue can hold
 */
template<uint16_t CAPACITY>
class CanQueue {
public:
    /**
     * @brief Default constructor
     * Initializes an empty queue
     */
    CanQueue() : head_(0), tail_(0), count_(0) {
    }
    
    /**
     * @brief Push a CAN frame onto the queue
     * @param frame The CAN frame to add
     * @return true if successful, false if queue is full
     */
    bool push(const CAN_FRAME& frame) {
        if (isFull()) {
            return false;
        }
        
        buffer_[head_] = frame;
        head_ = (head_ + 1) % CAPACITY;
        count_++;
        return true;
    }
    
    /**
     * @brief Pop a CAN frame from the queue
     * @param frame Pointer to store the popped frame
     * @return true if successful, false if queue is empty
     */
    bool pop(CAN_FRAME* frame) {
        if (isEmpty()) {
            return false;
        }
        
        if (frame != nullptr) {
            *frame = buffer_[tail_];
        }
        
        tail_ = (tail_ + 1) % CAPACITY;
        count_--;
        return true;
    }
    
    /**
     * @brief Peek at the front element without removing it
     * @param frame Pointer to store the peeked frame
     * @return true if successful, false if queue is empty
     */
    bool peek(CAN_FRAME* frame) const {
        if (isEmpty()) {
            return false;
        }
        
        if (frame != nullptr) {
            *frame = buffer_[tail_];
        }
        
        return true;
    }
    
    /**
     * @brief Check if queue is empty
     * @return true if queue contains no elements
     */
    bool isEmpty() const {
        return count_ == 0;
    }
    
    /**
     * @brief Check if queue is full
     * @return true if queue is at maximum capacity
     */
    bool isFull() const {
        return count_ >= CAPACITY;
    }
    
    /**
     * @brief Get number of elements in queue
     * @return Number of CAN frames currently in the queue
     */
    uint16_t length() const {
        return count_;
    }
    
    /**
     * @brief Get maximum capacity of queue
     * @return Maximum number of frames the queue can hold
     */
    uint16_t capacity() const {
        return CAPACITY;
    }
    
    /**
     * @brief Clear all elements from the queue
     */
    void clear() {
        head_ = 0;
        tail_ = 0;
        count_ = 0;
    }
    
    /**
     * @brief Get number of available slots
     * @return Number of frames that can still be added
     */
    uint16_t available() const {
        return CAPACITY - count_;
    }
    
private:
    CAN_FRAME buffer_[CAPACITY];  ///< Circular buffer storage
    uint16_t head_;                ///< Write position
    uint16_t tail_;                ///< Read position
    uint16_t count_;               ///< Number of items in queue
};

#endif // CAN_QUEUE_H
