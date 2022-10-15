#pragma once

/*==============================================================================
==============================================================================*/

//******************************************************************************
//******************************************************************************
//******************************************************************************

#include <string>
#include <ctime>
#include <time.h>

namespace Some
{

//******************************************************************************
//******************************************************************************
//******************************************************************************
// This class implements a general purpose shared storage object. It contains
// status counters and flags.

class State
{
public:
   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Constants.

   // State variable values.
   static const int cState_None       = -1;
   static const int cState_Starting   = 1;
   static const int cState_Running    = 2;
   static const int cState_Suspending = 3;
   static const int cState_Suspended  = 4;
   static const int cState_Aborting   = 5;
   static const int cState_Aborted    = 6;
   static const int cState_Done       = 7;
   static const int cState_Error      = 8;

   // None.
   static const int cNone = 0;

   // CMark types.
   static const int cAStop_Disable = 0;
   static const int cAStop_Enable = 1;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members:

   // Program state variables. File mode1 is zip.
   int mSX1;    // script seq thread run state.File mode1 is zip.
   int mSX2;    // script seq thread run state.File mode2 is gcode.
   int mGX;     // gcode thread state.
   int mLX;     // lcd graphics state.

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members:

   // Feynman gcode transmit and receive counts. File mode1 is zip.
   int mGCodeTxCount1;
   int mGCodeRxCount;

   // Slice count.
   int mSliceCount;

   // Max counts in script.
   int mGCodeTxTotal1;
   int mSliceTotal;

   // Feynman gcode transmit and receive counts. File mode2 is gcode.
   int mGCodeTxCount2;
   int mGCodeRxCount2;

   // Max counts in script.
   int mGCodeTxTotal2;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members:

   // Current time.
   struct timespec mCurrentTime;

   // Start time. This is the time that the current script started.
   struct timespec mStartTime1;
   struct timespec mStartTime2;

   // Run time. This is the time that the current script has been running.
   struct timespec mRunTime1;
   struct timespec mRunTime2;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members:

   // Auto stop for G38.3. Enable
   bool mAStopEnableFlag;
   bool mAStopEnable;

   // Auto stop begin, end print.
   bool mAStopBeginFlag;
   bool mAStopEndFlag;

   // Auto stop parm request flag.
   bool mAStopParmFlag;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods:

   // Constructor.
   State();
   void reset();

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.

   // Return the state as a string.
   const char* asString_StateSX1();
   const char* asString_StateSX2();
   const char* asString_StateGX();
   const char* asString_StateLX();

   // Set state variables.
   void setStateSX1_None();
   void setStateSX1_Starting();
   void setStateSX1_Running();
   void setStateSX1_Suspending();
   void setStateSX1_Suspended();
   void setStateSX1_Aborting();
   void setStateSX1_Aborted();
   void setStateSX1_Done();
   void setStateSX1_Error();

   void setStateSX2_None();
   void setStateSX2_Starting();
   void setStateSX2_Running();
   void setStateSX2_Aborting();
   void setStateSX2_Aborted();
   void setStateSX2_Done();
   void setStateSX2_Error();

   // Set state variables.
   void setStateGX_None();
   void setStateGX_Running();

   // Set state variables.
   void setStateLX_None();
   void setStateLX_Running();

   // Get state variables.
   bool isStateSX1_None();
   bool isStateSX1_Starting();
   bool isStateSX1_Running();
   bool isStateSX1_Suspending();
   bool isStateSX1_Suspended();
   bool isStateSX1_Aborting();
   bool isStateSX1_Aborted();
   bool isStateSX1_Done();
   bool isStateSX1_Error();
   bool isStateSX1_Busy();

   // Get state variables.
   bool isStateSX2_None();
   bool isStateSX2_Starting();
   bool isStateSX2_Running();
   bool isStateSX2_Aborting();
   bool isStateSX2_Aborted();
   bool isStateSX2_Done();
   bool isStateSX2_Error();
   bool isStateSX2_Busy();

   // Get state variables.
   bool isStateGX_None();
   bool isStateGX_Running();

   // Get state variables.
   bool isStateLX_None();
   bool isStateLX_Running();

   // Return progress indicators. Return -1 if not valid.
   double getProgress1();
   double getProgress2();

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.

   // Update the script run time.
   void initializeTime1();
   void initializeTime2();
   void updateTime();

   // Return the script run time as a string.
   char* asString_RunTime1(char* aBuffer);
   char* asString_RunTime2(char* aBuffer);

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // CMarker.

   // Convert from string.
   void setAStopCode(const char* aString);

};

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Global instance

#ifdef _SOMESTATE_CPP_
          State gState;
#else
   extern State gState;
#endif

//******************************************************************************
//******************************************************************************
//******************************************************************************
}//namespace

