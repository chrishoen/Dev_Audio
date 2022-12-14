#pragma once

/*==============================================================================
==============================================================================*/

//******************************************************************************
//******************************************************************************
//******************************************************************************

#include <string>
#include <ctime>
#include <time.h>

//******************************************************************************
//******************************************************************************
//******************************************************************************
// This class implements a recorder state variable.

class RecorderState
{
public:
   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Constants.

   // State variable values.
   static const int cState_Stopped    = 0;
   static const int cState_Recording  = 1;
   static const int cState_Paused     = 2;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members:

   // State variable.
   int mSX;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.

   // Constructor.
   RecorderState();
   void reset();

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.

   // Set state variables.
   void setStopped();
   void setRecording();
   void setPaused();

   // Test state variables.
   bool isStopped();
   bool isRecording();
   bool isPaused();

   // Return the state as a string.
   const char* asString();
};

