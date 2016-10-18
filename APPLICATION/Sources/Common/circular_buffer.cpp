/**
* @file circular_buffer.cpp
* @brief Circular buffer implementation
*/
#include "circular_buffer.hpp"
#include <string.h>


/**
* @brief Constructor
* @param size - buffer size
*/
CircularBuffer::CircularBuffer(uint32_t size)
{
    Buffer = new uint8_t[size];
    Size = size;
    Head = Buffer;
    Tail = Buffer;
    ByteCount = 0;
};


/**
* @brief Buffer cleaning
*/
void CircularBuffer::Reset()
{
    Head = Buffer;
    Tail = Buffer;
    ByteCount = 0;
};


/**
* @brief Put data to the buffer
* @param data - source data pointer
* @param count - bytes count
* @return added bytes count
*/
uint32_t CircularBuffer::Put(const void* data, uint32_t count)
{
    /// Limit data size
    uint32_t copyCount = Size - ByteCount;
    if(copyCount > count)
    {
        copyCount = count;
    }
    else
    {
        count = copyCount;
    }

    /// Copy data to the buffer
    uint8_t* bufferEnd = Buffer + Size;
    for(const uint8_t* src = (const uint8_t* )data;  copyCount;  copyCount--)
    {
        *Tail++ = *src++;
        if(Tail == bufferEnd)
        {
            Tail = Buffer;
        }
    }
    
    /// Increment counter
    ByteCount += count;

    /// Return bytes count
    return count;
};


/**
* @brief Put char to the buffer
* @param chr - source char
*/
bool CircularBuffer::PutChar(uint8_t chr)
{
	/// Add char, increment tail pointer
    *Tail++ = chr;
    if(Tail == (Buffer + Size))
    {
        Tail = Buffer;
    }

    /// Increment bytes counter
    ByteCount++;

    /// Return result
    return true;
};


/**
* @buffer Byte array extraction
* @param buffer - destination buffer pointer
* @param count - bytes count
* @return extracted bytes count
*/
uint32_t CircularBuffer::Get(void* buffer, uint32_t count)
{
    /// Limit data size
    uint32_t extractCount = ByteCount;
    if(extractCount > count)
    {
        extractCount = count;
    }
    else
    {
        count = extractCount;
    }
    
    /// Copy data to the buffer
    uint8_t* bufferEnd = Buffer + Size;
    for(uint8_t* dst = (uint8_t* )buffer;  extractCount;  extractCount--)
    {
        *dst = *Head++;
        if(Head == bufferEnd)
        {
            Head = Buffer;
        }
    }
	
	/// Decrement bytes counter
    ByteCount -= count;
    
    /// Return bytes count
    return count;
};


/**
* @brief Char extraction
* @param buffer - destination buffer pointer
* @return true, whether operation is successfull
*/
bool CircularBuffer::GetChar(void* buffer)
{
    if(ByteCount)
    {
        *(uint8_t* )buffer = *Head++;
        if(Head == (Buffer + Size))
        {
            Head = Buffer;
        }
        ByteCount--;
        return true;
    }
    
    return false;
};


/**
* @brief Copy data from buffer
* @param buffer - destination buffer pointer
* @param count - bytes count
* @return copied bytes count
*/
uint32_t CircularBuffer::Copy(void* buffer, uint32_t count)
{
    /// Limit data size
    uint32_t copyCount = ByteCount;
    if(copyCount > count)
    {
        copyCount = count;
    }
    else
    {
        count = copyCount;
    }

    /// Copy data
    uint8_t* bufferEnd = Buffer + Size;
    uint8_t* localHead = Head;
    for(uint8_t* dst = (uint8_t* )buffer;  copyCount;  copyCount--)
    {
        *dst++ = *localHead++;
        if(localHead == bufferEnd)
        {
            localHead = Buffer;
        }
    }
    
    /// Return bytes count
    return count;
};


/**
* @brief Delete data from buffer
* @param count - bytes count
*/
void CircularBuffer::Delete(uint32_t count)
{
    /// Limit data size
    if(count > ByteCount)
    {
        count = ByteCount;
    }
    
    /// Shift head pointer
    uint8_t* bufferEnd = Buffer + Size;
    Head += count;
	if(Head >= bufferEnd)
	{
        Head = Buffer + ((uint32_t)Head - (uint32_t)bufferEnd);
    }

    /// Decrement bytes counter
    ByteCount -= count;
};
