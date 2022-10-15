#pragma once

#include "risCmdLineExec.h"

//******************************************************************************
// specific command line executive
//******************************************************************************

class CmdLineExec : public Ris::BaseCmdLineExec
{
public:

   CmdLineExec();

   void reset() override;
   void execute(Ris::CmdLineCmd* aCmd) override;
   void special(int aSpecial) override;

   void executeRun(Ris::CmdLineCmd* aCmd);
   void executeSin1(Ris::CmdLineCmd* aCmd);
   void executePlay1(Ris::CmdLineCmd* aCmd);

   void executeRec1(Ris::CmdLineCmd* aCmd);
   void executeRec2(Ris::CmdLineCmd* aCmd);
   void executeRec3(Ris::CmdLineCmd* aCmd);
   void executePause(Ris::CmdLineCmd* aCmd);
   void executeResume(Ris::CmdLineCmd* aCmd);
   void executeStop(Ris::CmdLineCmd* aCmd);
   void executeShow(Ris::CmdLineCmd* aCmd);

   void executeGo1(Ris::CmdLineCmd* aCmd);
   void executeGo2(Ris::CmdLineCmd* aCmd);
   void executeGo3(Ris::CmdLineCmd* aCmd);
   void executeGo4(Ris::CmdLineCmd* aCmd);
   void executeGo5(Ris::CmdLineCmd* aCmd);

   void executeParms (Ris::CmdLineCmd* aCmd);
};

//******************************************************************************

