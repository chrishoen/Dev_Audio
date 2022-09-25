#include "stdafx.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "string.h"

#include "risProgramTime.h"
#include "risBitUtils.h"
#include "my_functions.h"
#include "CmdLineExec.h"

//******************************************************************************
//******************************************************************************
//******************************************************************************

CmdLineExec::CmdLineExec()
{
}

void CmdLineExec::reset()
{
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
// This class is the program command line executive. It processes user
// command line inputs and executes them. It inherits from the command line
// command executive base class, which provides an interface for executing
// command line commands. It provides an override execute function that is
// called by a console executive when it receives a console command line input.
// The execute function then executes the command.

void CmdLineExec::execute(Ris::CmdLineCmd* aCmd)
{
   if(aCmd->isCmd("RESET"))   reset();
   if (aCmd->isCmd("GO1"))    executeGo1(aCmd);
   if (aCmd->isCmd("GO2"))    executeGo2(aCmd);
   if (aCmd->isCmd("GO3"))    executeGo3(aCmd);
   if (aCmd->isCmd("GO4"))    executeGo4(aCmd);
   if (aCmd->isCmd("GO5"))    executeGo5(aCmd);
   if (aCmd->isCmd("GO6"))    executeGo6(aCmd);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executeGo1(Ris::CmdLineCmd* aCmd)
{
   Prn::print(0, "start");
   int tRet = system(
      "sudo setcap 'cap_sys_nice=eip cap_sys_resource=eip cap_fowner=eip cap_ipc_lock=eip cap_ipc_owner=eip' /opt/prime/bin/captest");
   Prn::print(0, "finish %d", tRet);
}


//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executeGo2(Ris::CmdLineCmd* aCmd)
{
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executeGo3(Ris::CmdLineCmd* aCmd)
{
   aCmd->setArgDefault(1, 0);
   aCmd->setArgDefault(2, true);
   unsigned tValue1 = 0;
   unsigned tValue2 = 0;
   tValue1 = 0x00000000;
   tValue1 = 0x0000ffff;
   Prn::print(0, "%08x", tValue1);

   int tBitNum = aCmd->argInt(1);
   bool tBitValue = aCmd->argBool(2);
   tValue2 = Ris::setBit(tValue1, tBitNum, tBitValue);

   Prn::print(0, "%08x", tValue2);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executeGo4(Ris::CmdLineCmd* aCmd)
{
   aCmd->setArgDefault(1, 0);
   aCmd->setArgDefault(2, true);
   unsigned char tValue = 0x0f;
   Prn::print(0, "%02x", tValue);

   int tBitNum = aCmd->argInt(1);
   bool tBitValue = aCmd->argBool(2);
   Ris::setBit(&tValue, tBitNum, tBitValue);

   Prn::print(0, "%02x", tValue);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executeGo5(Ris::CmdLineCmd* aCmd)
{
   printf("printf\n");
   Prn::print(0, "Prn::print 0");
   Prn::print(Prn::Show1, "Prn::print Prn::Show1");
   Prn::print(Prn::Show2, "Prn::print Prn::Show");
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executeGo6(Ris::CmdLineCmd* aCmd)
{
   Prn::print(0, "Ris::portableGetCurrentDir() %s", Ris::portableGetCurrentDir());
   Prn::print(0, "Ris::portableGetBinDir() %s", Ris::portableGetBinDir());
   double tTime = Ris::getProgramTime();
   unsigned int tTimeMS = Ris::getCpuTimeUIntMS();

   Prn::print(0, "Ris::getProgramTime           %10.6f", tTime);
   Prn::print(0, "Ris::getCpuTimeUIntMs         %10d", tTimeMS);
}

