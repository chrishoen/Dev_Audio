/*==============================================================================
Description:
==============================================================================*/

//******************************************************************************
//******************************************************************************
//******************************************************************************
#include "stdafx.h"

#define  _SOMESTATE_CPP_
#include "someState.h"

namespace Some
{

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Constructor

State::State()
{
   reset();
}

void State::reset()
{
   mSX1 = cState_None;
   mSX2 = cState_None;
   mGX = cState_None;
   mLX = cState_None;

   mGCodeTxCount1 = 0;
   mGCodeRxCount = 0;
   mSliceCount = 0;
   mGCodeTxTotal1 = 0;
   mSliceTotal = 0;

   mGCodeTxCount2 = 0;
   mGCodeRxCount2 = 0;
   mGCodeTxTotal2 = 0;

   memset(&mCurrentTime, 0, sizeof(mCurrentTime));
   memset(&mStartTime1, 0, sizeof(mStartTime1));
   memset(&mStartTime2, 0, sizeof(mStartTime2));
   memset(&mRunTime1, 0, sizeof(mRunTime1));
   memset(&mRunTime2, 0, sizeof(mRunTime2));

   mAStopEnableFlag = false;
   mAStopEnable = false;

   mAStopBeginFlag = false;
   mAStopEndFlag = false;
   mAStopParmFlag = false;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Return the state as a string.

const char* State::asString_StateSX1()
{
   switch (mSX1)
   {
   case cState_None:       return "None";
   case cState_Starting:   return "Starting";
   case cState_Running:    return "Running";
   case cState_Suspending: return "Suspending";
   case cState_Suspended:  return "Suspended";
   case cState_Aborting:   return "Aborting";
   case cState_Aborted:    return "Aborted";
   case cState_Done:       return "Done";
   case cState_Error:      return "Error";
   default: return "UNKNOWN";
   }
}

const char* State::asString_StateSX2()
{
   switch (mSX2)
   {
   case cState_None:       return "None";
   case cState_Starting:   return "Starting";
   case cState_Running:    return "Running";
   case cState_Aborting:   return "Aborting";
   case cState_Aborted:    return "Aborted";
   case cState_Done:       return "Done";
   case cState_Error:      return "Error";
   default: return "UNKNOWN";
   }
}

const char* State::asString_StateGX()
{
   switch (mGX)
   {
   case cState_None:       return "None";
   case cState_Running:    return "Running";
   default: return "UNKNOWN";
   }
}

const char* State::asString_StateLX()
{
   switch (mLX)
   {
   case cState_None:       return "None";
   case cState_Running:    return "Running";
   default: return "UNKNOWN";
   }
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Set state variables.

void State::setStateSX1_None()
{ 
   mSX1 = cState_None;
}
void State::setStateSX1_Starting()
{ 
   mSX1 = cState_Starting;
}
void State::setStateSX1_Running()
{ 
   mSX1 = cState_Running;
}
void State::setStateSX1_Suspending()
{ 
   mSX1 = cState_Suspending;
}
void State::setStateSX1_Suspended()  
{ 
   mSX1 = cState_Suspended;
}
void State::setStateSX1_Aborting()
{ 
   mSX1 = cState_Aborting;
}
void State::setStateSX1_Aborted()
{ 
   mSX1 = cState_Aborted;
   mGX = cState_None;
   mLX = cState_None;
}
void State::setStateSX1_Done()
{ 
   mSX1 = cState_Done;
   mGX = cState_None;
   mLX = cState_None;
}
void State::setStateSX1_Error()
{ 
   mSX1 = cState_Error;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Set state variables.

void State::setStateSX2_None()
{
   mSX2 = cState_None;
}
void State::setStateSX2_Starting()
{
   mSX2 = cState_Starting;
}
void State::setStateSX2_Running()
{
   mSX2 = cState_Running;
}
void State::setStateSX2_Aborting()
{
   mSX2 = cState_Aborting;
}
void State::setStateSX2_Aborted()
{
   mSX2 = cState_Aborted;
   mGX = cState_None;
   mLX = cState_None;
}
void State::setStateSX2_Done()
{
   mSX2 = cState_Done;
   mGX = cState_None;
   mLX = cState_None;
}
void State::setStateSX2_Error()
{
   mSX2 = cState_Error;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Set state variables.

void State::setStateGX_None()
{
   mGX = cState_None;
}
void State::setStateGX_Running()
{
   mGX = cState_Running;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Set state variables.

void State::setStateLX_None()
{
   mLX = cState_None;
}
void State::setStateLX_Running()
{
   mLX = cState_Running;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Get state variables.

bool State::isStateSX1_None()       { return mSX1 == cState_None; }
bool State::isStateSX1_Starting()   { return mSX1 == cState_Starting; }
bool State::isStateSX1_Running()    { return mSX1 == cState_Running; }
bool State::isStateSX1_Suspending() { return mSX1 == cState_Suspending; }
bool State::isStateSX1_Suspended()  { return mSX1 == cState_Suspended; }
bool State::isStateSX1_Aborting()   { return mSX1 == cState_Aborting; }
bool State::isStateSX1_Aborted()    { return mSX1 == cState_Aborted; }
bool State::isStateSX1_Done()       { return mSX1 == cState_Done; }
bool State::isStateSX1_Error()      { return mSX1 == cState_Error; }

bool State::isStateSX1_Busy()
{
   switch (mSX1)
   {
   case cState_None:       return false;
   case cState_Starting:   return true;
   case cState_Running:    return true;
   case cState_Suspending: return true;
   case cState_Suspended:  return false;
   case cState_Aborting:   return true;
   case cState_Aborted:    return false;
   case cState_Done:       return false;
   case cState_Error:      return false;
   default: return "UNKNOWN";
   }
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Get state variables.

bool State::isStateSX2_None()       { return mSX2 == cState_None; }
bool State::isStateSX2_Starting()   { return mSX2 == cState_Starting; }
bool State::isStateSX2_Running()    { return mSX2 == cState_Running; }
bool State::isStateSX2_Aborting()   { return mSX2 == cState_Aborting; }
bool State::isStateSX2_Aborted()    { return mSX2 == cState_Aborted; }
bool State::isStateSX2_Done()       { return mSX2 == cState_Done; }
bool State::isStateSX2_Error()      { return mSX2 == cState_Error; }

bool State::isStateSX2_Busy()
{
   switch (mSX2)
   {
   case cState_None:       return false;
   case cState_Starting:   return true;
   case cState_Running:    return true;
   case cState_Aborting:   return true;
   case cState_Aborted:    return false;
   case cState_Done:       return false;
   case cState_Error:      return false;
   default: return "UNKNOWN";
   }
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Get state variables.

bool State::isStateGX_None()       { return mGX == cState_None; }
bool State::isStateGX_Running()    { return mGX == cState_Running; }

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Get state variables.

bool State::isStateLX_None()       { return mLX == cState_None; }
bool State::isStateLX_Running()    { return mLX == cState_Running; }

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Update the script run time.

void State::initializeTime1()
{
   // Initialize counts.
   mGCodeTxCount1 = 0;
   mGCodeRxCount = 0;
   mSliceCount = 0;

   // Set the current start time.
   timespec_get(&mStartTime1, TIME_UTC);

   // Reset the run time.
   memset(&mRunTime1, 0, sizeof(mRunTime1));
}

void State::initializeTime2()
{
   // Initialize counts.
   mGCodeTxCount2 = 0;
   mGCodeRxCount2 = 0;

   // Set the current start time.
   timespec_get(&mStartTime2, TIME_UTC);

   // Reset the run time.
   memset(&mRunTime2, 0, sizeof(mRunTime1));
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Update the run times.

void my_timespec_diff(struct timespec *start, struct timespec *stop, struct timespec *result)
{
   if ((stop->tv_nsec - start->tv_nsec) < 0) {
      result->tv_sec = stop->tv_sec - start->tv_sec - 1;
      result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
   }
   else {
      result->tv_sec = stop->tv_sec - start->tv_sec;
      result->tv_nsec = stop->tv_nsec - start->tv_nsec;
   }
}

void State::updateTime()
{
   // Get the current time.
   timespec_get(&mCurrentTime, TIME_UTC);

   if (isStateSX1_Running() || isStateSX1_Suspended())
   {
      // Calculate the run time as the difference between current and start. 
      my_timespec_diff(&mStartTime1, &mCurrentTime, &mRunTime1);
   }

   if (isStateSX2_Running())
   {
      // Calculate the run time as the difference between current and start. 
      my_timespec_diff(&mStartTime2, &mCurrentTime, &mRunTime2);
   }
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Return the script run time as a string.

char* State::asString_RunTime1(char* aBuffer)
{
   // Calculate the script run time.
   int tTotalSec  = (int)mRunTime1.tv_sec;
   int tTotalMin  = tTotalSec / 60;
   int tTotalHour = tTotalSec / 3600;
   int tModSec    = tTotalSec % 60;
   int tModMin    = tTotalMin % 60;

   // Convert to a string.
   sprintf(aBuffer, "%02d:%02d:%02d", tTotalHour, tModMin, tModSec);
   return aBuffer;
}

char* State::asString_RunTime2(char* aBuffer)
{
   // Calculate the script run time.
   int tTotalSec = (int)mRunTime2.tv_sec;
   int tTotalMin = tTotalSec / 60;
   int tTotalHour = tTotalSec / 3600;
   int tModSec = tTotalSec % 60;
   int tModMin = tTotalMin % 60;

   // Convert to a string.
   sprintf(aBuffer, "%02d:%02d:%02d", tTotalHour, tModMin, tModSec);
   return aBuffer;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Return progress indicators. Return -1 if not valid.

double State::getProgress1()
{
   // Calculate progress.
   double tProgress = 0.0;
   if ((isStateSX1_Running() || isStateSX1_Suspended()) && gState.mGCodeTxTotal1 > 0)
   {
      tProgress = 100.0 * (double)mGCodeTxCount1 / (double)mGCodeTxTotal1;
   }
   return tProgress;
}

double State::getProgress2()
{
   // Calculate progress.
   double tProgress = 0.0;
   if (isStateSX2_Running() && gState.mGCodeTxTotal2 > 0)
   {
      tProgress = 100.0 * (double)mGCodeTxCount2 / (double)mGCodeTxTotal2;
   }
   return tProgress;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// 

void State::setAStopCode(const char* aString)
{
   if (strcmp(aString, "AStopEnable") == 0)
   {
      mAStopEnable = true;
      mAStopEnableFlag = true;
   }
   else if (strcmp(aString, "AStopDisable") == 0)
   {
      mAStopEnable = false;
      mAStopEnableFlag = true;
   }
   else if (strcmp(aString, "AStopBegin") == 0)
   {
      mAStopBeginFlag = true;
   }
   else if (strcmp(aString, "AStopEnd") == 0)
   {
      mAStopEndFlag = true;
   }
   else if (strcmp(aString, "AStopParm") == 0)
   {
      mAStopParmFlag = true;
   }
}



//******************************************************************************
//******************************************************************************
//******************************************************************************
}//namespace
