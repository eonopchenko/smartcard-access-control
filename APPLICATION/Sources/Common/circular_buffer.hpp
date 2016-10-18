/**
* @file circular_buffer.hpp
* @brief Circular buffer header
*/
#ifndef __CIRCULAR_BUFFER_HPP
#define __CIRCULAR_BUFFER_HPP

#include <stdint.h>


/**
* @brief Circular buffer class
*/
class CircularBuffer
{
	public:
		void Reset();									// Buffer cleaning
		uint32_t Put(const void* data, uint32_t count);	// Put data to the buffer
		uint32_t Get(void* buffer, uint32_t count);		// Byte array extraction
		uint32_t Copy(void* buffer, uint32_t count);	// Copy data from buffer
		void Delete(uint32_t count);					// Delete data from buffer
		bool PutChar(uint8_t byte);						// Put char to the buffer
		bool GetChar(void* buffer);						// Char extraction
		
		/**
		* @brief Check, whether the buffer is empty
		* @return true, if the buffer is empty
		*/
		bool IsEmpty()
		{
			return !ByteCount;
		};
		
		/**
		* @brief Get bytes count
		* @return bytes count
		*/
		uint16_t GetByteCount()
		{
			return ByteCount;
		};
		
		/// Constructor
		CircularBuffer(uint32_t size);
		
	private:
		uint8_t* Buffer;	///< Buffer
		uint8_t* Head;		///< Buffer head
		uint8_t* Tail;		///< Buffer tail
		uint32_t Size;		///< Buffer size
		uint32_t ByteCount;	///< Bytes count
};

#endif /* __CIRCULAR_BUFFER_HPP */

