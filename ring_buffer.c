/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>
#include "ring_buffer.h"


bool RingBuffer_Init(RingBuffer *ringBuffer, char *dataBuffer, size_t dataBufferSize) 
{
	assert(ringBuffer);
	assert(dataBuffer);
	assert(dataBufferSize > 0);
	
	if ((ringBuffer) && (dataBuffer) && (dataBufferSize > 0)) {
	    
	  ringBuffer->buffer_start_addr = dataBuffer;
	  ringBuffer->buffer_capacity = dataBufferSize;
	  
	  ringBuffer->tail = 0;
	  ringBuffer->head = 0;
	  ringBuffer->count = 0;
	  
	  return true;
	  
	}
	
	return false;
}

bool RingBuffer_Clear(RingBuffer *ringBuffer)
{
	assert(ringBuffer);
	
	if (ringBuffer) {
		memset(ringBuffer->buffer_start_addr, 0, ringBuffer->buffer_capacity);
		ringBuffer->count = 0;
		ringBuffer->head = 0;
		ringBuffer->tail = 0;
		return true;
	}
	return false;
}

bool RingBuffer_IsEmpty(const RingBuffer *ringBuffer)
{
  assert(ringBuffer);	
	if(ringBuffer->count == 0)
		 { 
		     return true; 
		 }
	else
	    {
	        return false;
	    }
}

size_t RingBuffer_GetLen(const RingBuffer *ringBuffer)
{
	assert(ringBuffer);
	
	if (ringBuffer) {
		return ringBuffer->count;
	}
	return 0;
	
}

size_t RingBuffer_GetCapacity(const RingBuffer *ringBuffer)
{
	assert(ringBuffer);
	
	if (ringBuffer) {
		return ringBuffer->buffer_capacity;
	}
	return 0;	
}


bool RingBuffer_PutChar(RingBuffer *ringBuffer, char c)      // do not overwrite
{
	assert(ringBuffer);
	
	if (ringBuffer) {
	    
	    if(ringBuffer->count == ringBuffer->buffer_capacity)  // if full, do not overwrite
	        {
	            return false;
	        }
	    
		ringBuffer->buffer_start_addr[ringBuffer->head] = c;
		ringBuffer->head++;
		
		if(ringBuffer->head == ringBuffer->buffer_capacity)
		    {
		        ringBuffer->head = 0;
		        ringBuffer->count++;       // do it one more time
		    }
		else
		    {
		        ringBuffer->count++;
		    }
		    
		
		
		return true;
	}
	return false;
}

bool RingBuffer_GetChar(RingBuffer *ringBuffer, char *c)
{
	assert(ringBuffer);
	assert(c);
	
  if ((ringBuffer) && (c)) {
      
        if(ringBuffer->count == 0)
            {
                return false;
            }
        else
        {
		*c = ringBuffer->buffer_start_addr[ringBuffer->tail];
		ringBuffer->tail++;
		ringBuffer->count--;
		
		if(ringBuffer->tail == ringBuffer->buffer_capacity)
		    {
		        ringBuffer->tail = 0;
		    }
		
		return true;
        }
	}
	return false;
}
