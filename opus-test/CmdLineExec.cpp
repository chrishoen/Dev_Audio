#include "stdafx.h"

#include "CmdLineExec.h"
#include "TestOne.h"

#include "CmdLineExec.h"

//******************************************************************************
//******************************************************************************
//******************************************************************************

CmdLineExec::CmdLineExec()
{
}
//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::reset()
{
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::execute(Ris::CmdLineCmd* aCmd)
{
   if (aCmd->isCmd("RESET")) reset();
   if (aCmd->isCmd("GO1"))   executeGo1(aCmd);
   if (aCmd->isCmd("GO2"))   executeGo2(aCmd);
   if (aCmd->isCmd("GO3"))   executeGo3(aCmd);
   if (aCmd->isCmd("GO4"))   executeGo4(aCmd);
   if (aCmd->isCmd("GO5"))   executeGo5(aCmd);
   if (aCmd->isCmd("Parms")) executeParms(aCmd);

   if (aCmd->isCmd("SIN"))   doSin1();
   if (aCmd->isCmd("PLAY"))  executePlay(aCmd);
   if (aCmd->isCmd("REC1"))  doRec1();
   if (aCmd->isCmd("REC2"))  doRec2();
   if (aCmd->isCmd("INFO"))  doShowInfo();
   if (aCmd->isCmd("S"))     executeStop(aCmd);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executePlay(Ris::CmdLineCmd* aCmd)
{
   aCmd->setArgDefault(1, 0.0);
   doPlay1(aCmd->argDouble(1));
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executeStop(Ris::CmdLineCmd* aCmd)
{
   doStopSin1();
   doStopPlay1();
   doStopRec1();
   doStopRec2();
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executeGo1(Ris::CmdLineCmd* aCmd)
{
   printf("GO1 GO1 GO1 GO1 GO1 GO1 \n");
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
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executeGo4(Ris::CmdLineCmd* aCmd)
{
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executeGo5(Ris::CmdLineCmd* aCmd)
{
 
}

//******************************************************************************

//******************************************************************************
//******************************************************************************
//******************************************************************************

void CmdLineExec::executeParms(Ris::CmdLineCmd* aCmd)
{
}

