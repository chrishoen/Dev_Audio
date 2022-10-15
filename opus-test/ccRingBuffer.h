#pragma once

/*==============================================================================

Ring buffer of fixed size opaque objects.

It is single writer multiple reader thread safe.
It is lock free and non blocking
There are two versions, one for heap and one for shared memory.

This accomodates a single writer and multiple indepenent readers. The writer 
writes to the buffer with no knowledge of the readers and the readers can read
from the buffer independantly from each other. There is no locking of the
buffer. Each reader maintains its own state and manages its own logic for
keeping track of buffer indices and dropping of elements and testing for
overwrites.

The ring buffer is based on the idea of an infinite memory of contiguous
fixed size elements that is written to sequentially by a single writer. Writes
are indexed by according to a long long WriteIndex, which is effectively
infinite.

The actual memory is finite, the size of which is specified by NumElements
and ElementSize. Writes into the memory are performed using modulo arithmetic
with NumElements.

Writes are concepually executed as
   ElementArray[NextWriteIndex % NumElements] = NewElement
   ++NextWriteIndex %= NumElements

where NextWriteIndex is the index of the next array element to write to.

Readers can read from the memory as long as the reads are within the bounds
of the indices of a min and max available, where

   MinReadIndex = NextWriteIndex - (NumElements - 1)
   MaxReadIndex = NextWriteIndex - 1
where
   MaxReadIndex - MinReadIndex = NumElements - 1

Here's an example of a buffer with NumElements = 8 that is past the
initialization stage. It is full. The write index is in the first column
and the modulo of it is in the second column.

122 2
123 3  zzzz
124 4  xxxx  NextWriteIndex - (NumElements - 1) = MinReadIndex
125 5  xxxx
126 6  xxxx
127 7  xxxx
128 0  xxxx
129 1  xxxx
130 2  xxxx  NextWriteIndex - 1 = MaxReadIndex
131 3  zzzz  NextWriteIndex is the next element to write to

Only elements marked with xxxx can be safely read. They are on the closed
interval
[MinReadIndex .. MaxReadIndex] = 
[NextWriteIndex - (NumElements - 1) .. NextWriteIndex - 1]

A reader can read always safely read any one element of 124 .. 130.
During a read of 123, an asynchrounous write to 131 could occur and
the read would be overwritten. So, any read from less than 124 is
not allowed.

As a complication, consider a buffer where the writer writes an
element to the array that only contains partial data. After writing
a few more elements, the writer wants to go back and update a previous
element. A read gap is provided for this case.

Here's an example of a buffer with NumElements = 8 and ReadGap = 3
A reader can read any one element of 124 .. 127. The writer can go
back and modify any one element of 128 .. 130.

122 2
123 3  zzzz
124 4  xxxx  NextWriteIndex - (NumElements - 1) = MinReadIndex
125 5  xxxx
126 6  xxxx
127 7  xxxx  NextWriteIndex - ReadGap - 1  = MaxReadIndex
128 0  yyyy  NextWriteIndex - ReadGap
129 1  yyyy
130 2  yyyy  NextWriteIndex - 1
131 3  zzzz  NextWriteIndex is the next element to write to

A reader can read always safely read any one element of 124 .. 127.

Only elements marked with xxxx can be safely read on the closed interval
[WriteIndex - (NumElements - 1) .. WriteIndex - ReadGap - 1]

While writing an element marked zzzz, the writer can go back and modify
elements marked with yyyy on the closed interval
[WriteIndex - ReadGap .. WriteIndex]

ReadGap = 0 means no read gap. Then the reader can read any element of
124 .. 130 and the writer cannot modify any elements once they have been
written.

ALSO, DON'T FORGET:
The following rules are for shared memory, regions that are shared between
different processes (who therefore have different address spaces):

1) No constructors.
2) No pointers.
3) No dynamic memory, this means no std::vector, ...
4) No vtables, this means no virtual functions.
5) Be careful with your loads and stores.

=============================================================================*/

//******************************************************************************
//******************************************************************************
//******************************************************************************

#include <stddef.h>
#include <atomic>

//******************************************************************************
//******************************************************************************
//******************************************************************************

namespace CC
{

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Ring buffer. This contains variables that describe a ring buffer
// and its state. This class is shared memory safe.

class RingBufferState
{
public:

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members.

   // Number of elements in the ring buffer.
   long long mNumElements;

   // Size of each element in the ring buffer.
   size_t mElementSize;

   // Read gap.
   long long mReadGap;

   // The index of the next element to write to. If this is equal to
   // zero then no writes have occured and the ring buffer is empty.
   // This single variable encapsulates the state of the ring buffer. It
   // can only be written to by a single writer thread and it can be read
   // by multiple reader threads. Buffer memory is addressed via this index
   // modulo the number of elements. This variable is surrounded by padding
   // so that it is on a separate cache line.

   long long mPadding1[8];
   std::atomic<long long> mNextWriteIndex;
   long long mPadding2[8];

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.

   // No constructor.
   virtual ~RingBufferState() {}
   void initialize(int aNumElements, size_t aElementSize, int aReadGap);
};

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Ring buffer writer class. There should only be one instance of this
// class for the single writer. This attaches to a ring buffer and is 
// used to write to it. It is not shared memory safe.

class RingBufferWriter
{
public:

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members.

   // The ring buffer.
   RingBufferState* mRB;

   // The ring buffer element array memory.
   void* mElementArrayMemory;

   // Number of elements in the ring buffer.
   long long mNumElements;

   // Size of each element in the ring buffer.
   size_t mElementSize;

   // Read gap.
   long long mReadGap;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members.

   // If true then call an inheritor supplied test function after a
   // write operation.
   bool mTestFlag;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.

   // Constructor.
   RingBufferWriter();
   virtual ~RingBufferWriter() {};
   void resetVars();
   virtual void resetTest() {}
   void initialize(RingBufferState* aRingBufferState, void* aElementArrayMemory);

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods. Helpers.

   // Return a pointer to an element, based on an index modulo the number
   // of elements.
   void* elementAt(long long aIndex);

   // Return the index of the next element to write to.
   long long getNextWriteIndex();

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.
   
   // Write an element to the array at the write index, copying it from
   // the function argument. Increment the write index state variable so 
   // that it contains the index of the next element to write to.
   void doWrite(void* aElement);

   // Write an array of source elements to the array at the write index.
   // Increment the write index accordingly.
   void doWriteArray(void* aElementSourceArray, int aNumElements);

   // Return a pointer to the next element to write to, which is the element
   // at the write index. Do not increment the  write index state. The caller
   // can then execute its own write operation.
   void* startWrite();

   // Increment the write index state variable after a started write is
   // finished so that it contains the index of the last element written to.
   void finishWrite();

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.
   
   // Internal test function that can be used by inheritors to perform
   // ring buffer consistency tests. This is called after the write, but
   // before incrementing the write index. It is passed the the write
   // index and the element that was written.
   virtual void doTest(long long aIndex, void* aElement) {}
};

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Ring buffer reader class. There can be multiple instances of this
// class for multiple readers. This attaches to a ring buffer and is 
// used to read from it. It is not shared memory safe.

class RingBufferReader
{
public:

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members.

   // The ring buffer.
   RingBufferState* mRB;

   // The ring buffer element array memory.
   void* mElementArrayMemory;

   // Number of elements in the ring buffer.
   long long mNumElements;

   // Size of each element in the ring buffer.
   size_t mElementSize;

   // Read gap.
   long long mReadGap;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members.

   // The index of the last successful read.
   long long mLastReadIndex;

   // Saved index of the last successful read. This is used to 
   // undo a read.
   long long mSaveLastReadIndex;

   // If true then this is the first read.
   bool mFirstFlag;

   // If true then restart reads at the max available,
   // else restart reads at the min available.
   bool mRestartAtMax;

   // If true then call an inheritor supplied test function after a
   // read operation.
   bool mTestFlag;

   // The number of reads that had no elements available to read.
   int mNotReadyCount1;
   int mNotReadyCount2;
   int mNotReadyCount3;

   // The number of elements that were dropped.
   int mDropCount;

   // The number of overwrites that occurred.
   int mOverwriteCount;

   // The max number of difference between two consecutive read operations.
   int mMaxDeltaRead;

   // Internal debug error count;
   int mErrorCount;

   // If true then the last read operation resulted in not ready condition.
   bool mNotReadyFlag;

   // If true then the last read operation resulted in an overwrite condition.
   bool mOverwriteFlag;

   // The number of successful reads.
   int mReadCount;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.

   // Constructor.
   RingBufferReader();
   virtual ~RingBufferReader() {};
   void resetVars();
   virtual void resetTest() {}
   void initialize(RingBufferState* aRingBuffer, void* aElementArrayMemory);

   // Return a pointer to an element, based on an index modulo the number
   // of elements.
   void* elementAt(long long aIndex);

   // Return the number of elements that are available to be read.
   int available();

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.

   // Restart read operations. This sets the first flag true so that
   // the next read will start at the last element that was written,
   // which is the max available.
   virtual void doRestartAtMax();

   // Restart read operations. This sets the first flag true so that
   // the next read will start at the minimum available.
   virtual void doRestartAtMin();

   // Read an element from the array, copying it to the function argument.
   // Return true if successful.
   bool doRead(void* aElement);

   // Read a number of elements from the array, copying them to the
   // function argument destination array. Return the number of elements
   // that were copied.
   int doReadArray(void* aElementDestinArray, int aNumElements);

   // Undo the last read operation. This sets the last read index to 
   // the saved last read index.
   void doUndoLastRead();

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.

   // Internal test function that can be used by inheritors to perform
   // ring buffer consistency tests. This is called after the read, but
   // before incrementing the read index. It is passed the the read
   // index and the element that was read.
   virtual void doTest(long long aIndex, void* aElement) {}
};

//******************************************************************************
//******************************************************************************
//******************************************************************************
}//namespace

