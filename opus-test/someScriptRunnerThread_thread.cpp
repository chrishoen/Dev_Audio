/*==============================================================================
==============================================================================*/

//******************************************************************************
//******************************************************************************
//******************************************************************************
#include "stdafx.h"

#include "cmnPriorities.h"
#include "someState.h"

#define  _SOMESCRIPTRUNNERTHREAD_CPP_
#include "someScriptRunnerThread.h"

namespace Some
{

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Constructor.

ScriptRunnerThread::ScriptRunnerThread()
{
   using namespace std::placeholders;

   // Set base class thread variables.
   BaseClass::mShortThread->setThreadName("ScriptShort");
   BaseClass::mShortThread->setThreadPriority(Cmn::gPriorities.mScriptShort);

   BaseClass::mLongThread->setThreadName("ScriptLong");
   BaseClass::mLongThread->setThreadPriority(Cmn::gPriorities.mScriptLong);

   // Set base class call pointers.
   BaseClass::mShortThread->mThreadInitCallPointer = 
      std::bind(&ScriptRunnerThread::threadInitFunction, this);
   BaseClass::mShortThread->mThreadExitCallPointer = 
      std::bind(&ScriptRunnerThread::threadExitFunction, this);
   BaseClass::mShortThread->mThreadExecuteOnTimerCallPointer = 
      std::bind(&ScriptRunnerThread::executeOnTimer, this, _1);

   // Bind qcalls.
   mAbortScriptQCall.bind  (this->mShortThread, this, &ScriptRunnerThread::executeAbortScript);

   // Bind qcalls.
   mRunScriptQCall.bind    (this->mLongThread, this, &ScriptRunnerThread::executeRunScript);
}

ScriptRunnerThread::~ScriptRunnerThread()
{
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Show thread info for this thread and for child threads.

void ScriptRunnerThread::showThreadInfo()
{
   BaseClass::showThreadInfo();
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Thread init function. This is called by the base class immedidately 
// after the thread starts running. It creates and launches the 
// child SerialStringThread.

void ScriptRunnerThread::threadInitFunction()
{
   Trc::write(11, 0, "ScriptRunnerThread::threadInitFunction done");
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Thread exit function. This is called by the base class immedidately
// before the thread is terminated. It shuts down the child SerialStringThread.

void ScriptRunnerThread::threadExitFunction()
{
   Prn::print(0, "ScriptRunnerThread::threadExitFunction END");
   Trc::write(11, 0, "ScriptRunnerThread::threadExitFunction done");
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Thread shutdown function. This calls the base class shutdownThread
// function to terminate the thread. This executes in the context of
// the calling thread.

void ScriptRunnerThread::shutdownThreads()
{
   Trc::write(11, 0, "ScriptRunnerThread::shutdownThread");
   Prn::print(0, "ScriptRunnerThread::shutdownThread BEGIN");

   // Abort the long thread.
   BaseClass::mNotify.abort();

   // Shutdown the two threads.
   BaseClass::shutdownThreads();

   Prn::print(0, "ScriptRunnerThread::shutdownThread END");
   Trc::write(11, 0, "ScriptRunnerThread::shutdownThread done");
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Execute periodically. This is called by the base class timer.

void ScriptRunnerThread::executeOnTimer(int aTimerCount)
{
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// This is bound to the qcall. It notifies the long thread to abort
// any running script.

void ScriptRunnerThread::executeAbortScript()
{
   // Notify the the long thread to abort any running script.
   BaseClass::mNotify.abort();
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
}//namespace