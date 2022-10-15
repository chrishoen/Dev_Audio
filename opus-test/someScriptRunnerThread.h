#pragma once

/*==============================================================================
Script runner prototype thread class.
==============================================================================*/

//******************************************************************************
//******************************************************************************
//******************************************************************************

#include <string>
#include "risThreadsTwoThread.h"
#include "risThreadsSynch.h"
#include "risCmdLineScript.h"

//******************************************************************************
//******************************************************************************
//******************************************************************************

namespace Some
{

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Script runner prototype thread class.
// 
// This is a script runner two thread. It's purpose is to run a script text
// file that contains instructions to send request strings to a responder
// program that sends back response strings. For each command instruction,
// a request string is formulated and send to the responder. It then
// receives and processes the response strings.
// 
// It inherits from BaseTwoThread to obtain a two thread functionality.
//
// The long thread executes a run script qcall that provides the execution
// context for the request/response string processing sequence. It reads
// a command instruction from the script file and executes it to send
// a request and wait for a notication of the response.
//
// This creates a child serial string thread that establishes and manages
// a serial connection, receives strings and passes them to the parent via
// a qcall callback, and allows for the transmission of strings. The child
// thread also notifies the parent thread of serial connection
// establishment/disestablishment via a qcall callback.

// The short thread executes a receive string qcall that is invoked by the
// child serial string thread. It notifies the long thread
// when responses are received.
//

class  ScriptRunnerThread : public Ris::Threads::BaseTwoThread
{
public:
   typedef Ris::Threads::BaseTwoThread BaseClass;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Constants:

   // Wait timeouts.
   static const int cInfiniteTimeout = -1;
   static const int cRxStringTimeout = 1000;
   static const int cScriptThrottle = 500;
   static const int cDelay1 = 100;

   // Notification codes.
   static const int cRxStringNotifyCode = 11;
   static const int cFlushRxStringNotifyCode = 17;

   // Loop exit status codes.
   static const int cLoopExitNormal = 0;
   static const int cLoopExitSuspended = 1;
   static const int cLoopExitAborted = 2;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members.

   // Run script exit code.
   int mLoopExitCode;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members.

   // Command line script file reader.
   Ris::CmdLineScript mScript;

   // Command line command for script reader.
   Ris::CmdLineCmd mCmd;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Members.

   // If true the execute periodically.
   bool mTPFlag;

   // Metrics.
   int  mStatusCount1;
   int  mStatusCount2;

   // Metrics.
   int mReadCount;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods.

   // Constructor.
   ScriptRunnerThread();
  ~ScriptRunnerThread();

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods. Thread base class overloads.

   // Thread init function. This is called by the base class immedidately 
   // after the thread starts running. It creates and launches the 
   // child SerialStringThread.
   void threadInitFunction() override;

   // Thread exit function. This is called by the base class immedidately
   // before the thread is terminated. It shuts down the child SerialStringThread.
   void threadExitFunction() override;

   // Thread shutdown function. This calls the base class shutdownThread
   // function to terminate the thread. This executes in the context of
   // the calling thread.
   void shutdownThreads() override;

   // Execute periodically. This is called by the base class timer.
   void executeOnTimer(int aTimerCount) override;

   // Show thread info for this thread and for child threads.
   void showThreadInfo() override;

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods. qcalls.

   // Abort script qcall. It is invoked by the control thread to execute an
   // abort script in the context of this thread.
   Ris::Threads::QCall0 mAbortScriptQCall;

   // This is bound to the qcall. It notifies the long thread to abort
   // any running script.
   void executeAbortScript();

   //***************************************************************************
   //***************************************************************************
   //***************************************************************************
   // Methods. qcalls.

   // Run script qcall. It is invoked by the command line executive.
   Ris::Threads::QCall0 mRunScriptQCall;

   // Run script function. This is bound to the qcall. This runs a
   // script file.
   // 
   // This is used for running script files.
   void executeRunScript();

   // Execute a command line command from the script file. It calls one of
   // the following specific command execution functions.
   void execute(Ris::CmdLineCmd* aCmd);

   // Execute specific commands.
   void executeRecord(Ris::CmdLineCmd* aCmd);
   void executePause(Ris::CmdLineCmd* aCmd);
   void executeResume(Ris::CmdLineCmd* aCmd);
   void executeStop(Ris::CmdLineCmd* aCmd);
   void executeWait(Ris::CmdLineCmd* aCmd);
};

//******************************************************************************
//******************************************************************************
//******************************************************************************
// Global singular instance.

#ifdef _SOMESCRIPTRUNNERTHREAD_CPP_
         ScriptRunnerThread* gScriptRunnerThread = 0;
#else
extern   ScriptRunnerThread* gScriptRunnerThread;
#endif

//******************************************************************************
//******************************************************************************
//******************************************************************************
}//namespace
