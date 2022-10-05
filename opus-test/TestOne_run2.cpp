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

const char* cFilePath = "/opt/prime/single/kashmir1.opus";

void doRun2()
{
   printf("opening\n");

   int tError = 0;
   OggOpusFile* tFile = op_open_file(cFilePath, &tError);
   printf("status %d\n", tError);
}
