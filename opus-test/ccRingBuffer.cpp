/*==============================================================================
Description:
==============================================================================*/

//******************************************************************************
//******************************************************************************
//******************************************************************************

#include "stdafx.h"
#include <limits.h>

#include "ccRingBuffer.h"

using namespace std;

namespace CC
{

//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
// No constructor. 

void RingBufferState::initialize(int aNumElements, size_t aElementSize, int aReadGap)
{
   mNumElements = aNumElements;
   mElementSize = aElementSize;
   mReadGap = aReadGap;
   mNextWriteIndex.store(0,std::memory_order_release);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
// Constructor.

RingBufferWriter::RingBufferWriter()
{
   mRB = 0;
   mNumElements = 0;
   mElementSize = 0;
   mReadGap = 0;
   mElementArrayMemory = 0;
   mTestFlag = false;
   resetVars();
}

void RingBufferWriter::initialize(RingBufferState* aRingBufferState, void* aElementArrayMemory)
{
   resetVars();
   mRB = aRingBufferState;
   mNumElements = mRB->mNumElements;
   mElementSize = mRB->mElementSize;
   mReadGap = mRB->mReadGap;
   mElementArrayMemory = aElementArrayMemory;
}

void RingBufferWriter::resetVars()
{
   resetTest();
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Helpers.
// 
// Return a pointer to an element, based on an index modulo the number
// of elements.
void* RingBufferWriter::elementAt(long long aIndex)
{
   aIndex %= mNumElements;
   return (void*)((char*)mElementArrayMemory + mElementSize * aIndex);
}

// Return the index of the next element to write to.
long long RingBufferWriter::getNextWriteIndex()
{
   // Return the index of the next element to write to.
   return mRB->mNextWriteIndex.load(std::memory_order_relaxed);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Write an element to the array at the write index, copying it from
// the function argument. Increment the write index state variable so 
// that it contains the index of the next element to write to.

void RingBufferWriter::doWrite(void* aElement)
{
   // Get the index of the next element to write to.
   long long tWriteIndex = mRB->mNextWriteIndex.load(std::memory_order_relaxed);

   // Get the address of the next element to write to.
   void* tPtr = elementAt(tWriteIndex);

   // Copy the element into the array.
   memcpy(tPtr, aElement, mElementSize);

   // Internal test function that can be used by inheritors to perform
   // ring buffer performance tests.
   if (mTestFlag)
   {
      doTest(tWriteIndex, tPtr);
   }

   // Increment the write index to the next element to write to.
   std::atomic_thread_fence(std::memory_order_release);
   mRB->mNextWriteIndex.fetch_add(1, std::memory_order_relaxed);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Write an array of elements to the array at the write index. Increment
// the write index accordingly.

void RingBufferWriter::doWriteArray(void* aElement, int aNumElements)
{
   char* tPtr = (char*)aElement;
   for (int i = 0; i < aNumElements; i++)
   {
      doWrite((void*)tPtr);
      tPtr += mRB->mElementSize;
   }
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Return a pointer to the next element to write to, which is the element
// at the write index. Do not increment the  write index state. The caller
// can then execute its own write operation.

void* RingBufferWriter::startWrite()
{
   // Get the index of the next element to write to.
   long long tWriteIndex = mRB->mNextWriteIndex.load(std::memory_order_relaxed);

   // Get the address of the next element to write to.
   void* tPtr = elementAt(tWriteIndex);

   // Return the address of the next element to write to.
   return tPtr;
}

// Increment the write index state variable after a started write is
// finished so that it contains the index of the last element written to.
void RingBufferWriter::finishWrite()
{
   if (mTestFlag)
   {
      // Get the index of the next element to write to.
      long long tWriteIndex = mRB->mNextWriteIndex.load(std::memory_order_relaxed);

      // Get the address of the next element to write to.
      void* tPtr = elementAt(tWriteIndex);

      // Internal test function that can be used by inheritors to perform
      // ring buffer consistency tests. This is called with the index of
      // the write and the element that was written.
      doTest(tWriteIndex, tPtr);
   }

   // Increment the write index to the next element to write to.
   std::atomic_thread_fence(std::memory_order_release);
   mRB->mNextWriteIndex.fetch_add(1, std::memory_order_relaxed);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
// Constructor.

RingBufferReader::RingBufferReader()
{
   mRB = 0;
   mNumElements = 0;
   mElementSize = 0;
   mReadGap = 0;
   mElementArrayMemory = 0;
   mTestFlag = false;
   resetVars();
}

void RingBufferReader::initialize(RingBufferState* aRingBufferState, void* aElementArrayMemory)
{
   resetVars();
   mRB = aRingBufferState;
   mNumElements = mRB->mNumElements;
   mElementSize = mRB->mElementSize;
   mReadGap = mRB->mReadGap;
   mElementArrayMemory = aElementArrayMemory;
}

void RingBufferReader::resetVars()
{
   mFirstFlag = true;
   mRestartAtMax = true;
   mLastReadIndex = 0;
   mNotReadyCount1 = 0;
   mNotReadyCount2 = 0;
   mNotReadyCount3 = 0;
   mErrorCount = 0;
   mDropCount = 0;
   mMaxDeltaRead = 0;
   mOverwriteCount = 0;
   mNotReadyFlag = false;
   mOverwriteFlag = false;
   mReadCount = 0;
   resetTest();
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Return a pointer to an element, based on an index modulo the number
// of elements.

void* RingBufferReader::elementAt(long long aIndex)
{
   aIndex %= mNumElements;
   return (void*)((char*)mElementArrayMemory + mElementSize * aIndex);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Return the number of elements that are available to be read.

int RingBufferReader::available()
{
   long long tMaxReadIndex = mRB->mNextWriteIndex.load(std::memory_order_acquire) - 1 - mReadGap;
   long long tDiff = tMaxReadIndex - mLastReadIndex;
   if (tDiff > mNumElements - 1) tDiff = mNumElements - 1;
   return (int)tDiff;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Restart read operations.

// Restart read operations. This sets the first flag true so that
// the next read will start at the last element that was written,
// which is the max available.
void RingBufferReader::doRestartAtMax()
{
   mFirstFlag = true;
   mRestartAtMax = true;
}

// Restart read operations. This sets the first flag true so that
// the next read will start at the minimum available.
void RingBufferReader::doRestartAtMin()
{
   mFirstFlag = true;
   mRestartAtMax = false;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Read an element from the array, copying it to the function argument.
// Return true if successful.
// 
// Here's an example of a buffer with NumElements = 8 and ReadGap = 3
// A reader can read any one element of 124 .. 127. The writer can go
// back and modify any one element of 128 .. 130.
//
// 122 2
// 123 3  zzzz
// 124 4  xxxx  NextWriteIndex - (NumElements - 1) = MinReadIndex
// 125 5  xxxx
// 126 6  xxxx
// 127 7  xxxx  NextWriteIndex - ReadGap - 1 = MaxReadIndex
// 128 0  yyyy  NextWriteIndex - ReadGap
// 129 1  yyyy
// 130 2  yyyy  NextWriteIndex - 1
// 131 3  zzzz  NextWriteIndex is the next element to write to

bool RingBufferReader::doRead(void* aElement)
{
   // Do this first.
   mNotReadyFlag = false;
   mOverwriteFlag = false;

   // Local variables.
   long long tMinReadIndex = 0;
   long long tMaxReadIndex = 0;
   long long tNextReadIndex = 0;

   // Get the initial write index. This might change asynchronously during
   // the read. The write index is the index of the next element that the 
   // writer will write to.
   long long tNextWriteIndex = mRB->mNextWriteIndex.load(std::memory_order_acquire);

   // Test for invalid data. This means that the writer has not yet
   // written any elements or is resetting the buffer.
   if (tNextWriteIndex == 0)
   {
      // The writer is not ready.
      mFirstFlag = true;
      mNotReadyCount1++;
      mNotReadyFlag = true;
      return false;
   }

   // Calculate the indices of the limits of available elements.
   tMinReadIndex = tNextWriteIndex - (mNumElements - 1);
   tMaxReadIndex = tNextWriteIndex - 1 - mReadGap;

   // If the max available element is negative then no elements are
   // available yet, so exit. This can happen with a nonzero read gap.
   if (tMaxReadIndex < 0)
   {
      // Set the read for the behind the maximum available element.
      mLastReadIndex = tMaxReadIndex - 1;
      mNotReadyCount2++;
      mNotReadyFlag = true;
      return false;
   }

   // Test for the first read.
   if (mFirstFlag)
   {
      mFirstFlag = false;
      mReadCount = 0;
      // Set the read for the maximum available element.
      if (mRestartAtMax)
      {
         mLastReadIndex = tMaxReadIndex - 1;
         tNextReadIndex = tMaxReadIndex;
      }
      // Set the read for the minimum available element.
      else
      {
         mLastReadIndex = tMinReadIndex - 1;
         tNextReadIndex = tMinReadIndex;
      }
   }
   else
   {
      // If the maximum available element has already been read then
      // no elements are available, so exit.
      if (mLastReadIndex == tMaxReadIndex)
      {
         // There's nothing to read.
         mNotReadyCount3++;
         mNotReadyFlag = true;
         return false;
      }

      // If the last element read is behind the minimum available element, 
      // then read from the minimum available element.
      if (mLastReadIndex < tMinReadIndex)
      {
         tNextReadIndex = tMinReadIndex;
      }
      // Else read from the next one.
      else
      {
         tNextReadIndex = mLastReadIndex + 1;
      }
   }

   // This should never happen.
   if (tNextReadIndex < 0)
   {
      mErrorCount++;
      mNotReadyFlag = true;
      return false;
   }

   // Get the address of the next element to read.
   void* tPtr = elementAt(tNextReadIndex);

   // Copy that element into the argument element.
   memcpy(aElement, tPtr, mElementSize);

   // If, during the read, the ring buffer was written to asynchronously
   // by the writer, then test if the read was possibly or actually
   // overwritten. If it was then drop the read.

   // Get the final write index. If a write occurred during the read then
   // this will be different than the write index at the beginning of the
   // read. If the read element was less than the final min available
   // element then the read was or could have been overwritten, so drop it.
   tNextWriteIndex = mRB->mNextWriteIndex.load(std::memory_order_acquire);
   tMinReadIndex = tNextWriteIndex - (mNumElements - 1);
   if (tNextReadIndex < tMinReadIndex)
   {
      mOverwriteCount++;
      mOverwriteFlag = true;
      return false;
   }

   // Internal test function that can be used by inheritors to perform
   // ring buffer consistency tests. This is called with the index of
   // the read and the element that was read.
   if (mTestFlag)
   {
      doTest(tNextReadIndex, aElement);
   }

   // Increment the drop count. If none were dropped then the read index
   // should be the index of the last succesful read plus one.
   if (mLastReadIndex > 0)
   {
      int tDeltaRead = (int)(tNextReadIndex - mLastReadIndex);
      if (tDeltaRead > mMaxDeltaRead) mMaxDeltaRead = tDeltaRead;
      mDropCount += tDeltaRead - 1;
   }

   // Save the index for the last successful read. This is used to 
   // undo this read operation.
   mSaveLastReadIndex = mLastReadIndex;

   // Store the new index for the last successful read.
   mLastReadIndex = tNextReadIndex;

   // Success. 
   mReadCount++;
   return true;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Undo the last read operation. This sets the last read index to 
// the saved last read index.

void RingBufferReader::doUndoLastRead()
{
   mLastReadIndex = mSaveLastReadIndex;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Read an element from the array, copying it to the function argument.
// Return true if successful.

bool RingBufferReader::doReadArray(void* aElement, int aNumElements)
{
   char* tPtr = (char*)aElement;
   for (int i = 0; i < aNumElements; i++)
   {
      bool tPass = doRead((void*)tPtr);
      tPtr += mRB->mElementSize;
      if (!tPass) return false;
   }
   return true;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
}//namespace