/*==============================================================================
Description:
==============================================================================*/

//******************************************************************************
//******************************************************************************
//******************************************************************************
#include "stdafx.h"

#include <stdio.h>
#include <assert.h>
#include <opus/opusfile.h>

static const char* cFilePath = "/opt/prime/single/kashmir1.opus";
static short mBuffer[10000];
static OggOpusFile* mFile = 0;

void doRun21()
{
   printf("opening\n");
   int tError = 0;
   mFile = op_open_file(cFilePath, &tError);
   printf("open status %d\n", tError);
}

void doRun22()
{
   printf("reading\n");
   int tRet = op_read(mFile, mBuffer, 1000, NULL);
   printf("read status %d\n", tRet);
}

void doRun23()
{
   op_free(mFile);
}
