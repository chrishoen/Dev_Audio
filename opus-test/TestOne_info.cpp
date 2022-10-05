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
static OggOpusFile* mFile = 0;

void doShowInfo()
{
   printf("opening opus file %s\n", cFilePath);

   int tError = 0;
   mFile = op_open_file(cFilePath, &tError);
   printf("open status %d\n", tError);

   int channel_count = op_channel_count(mFile, 0);
   long long raw_total = op_raw_total(mFile, 0);
   long long pcm_total = op_pcm_total(mFile, 0);
   int bitrate = op_bitrate(mFile, 0);

   printf("\n");
   printf("channel_count   %d\n", channel_count);
   printf("raw_total       %lld\n", raw_total);
   printf("pcm_total       %lld\n", pcm_total);
   printf("bitrate         %d\n", bitrate);


}

