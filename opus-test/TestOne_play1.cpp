/*==============================================================================
Description:
==============================================================================*/

//******************************************************************************
//******************************************************************************
//******************************************************************************
#include "stdafx.h"

#include <stdio.h>
#include <assert.h>
#include <pulse/pulseaudio.h>
#include <opus/opusfile.h>

//******************************************************************************
//******************************************************************************
//******************************************************************************
//static const char* cFilePath = "/opt/prime/single/kashmir1.opus";

static pa_threaded_mainloop* mMainLoop = 0;
static pa_mainloop_api* mMainLoopApi = 0;
static pa_context* mContext = 0;
static pa_stream* mStream = 0;

pa_sample_spec mSampleSpec;

static const char* cFilePath = "/opt/prime/tmp/record.opus";
static OggOpusFile* mFile = 0;
static int mWriteCount = 0;

//******************************************************************************
//******************************************************************************
//******************************************************************************

static void context_state_cb(pa_context* aContext, void* aMainloop)
{
   pa_threaded_mainloop_signal((pa_threaded_mainloop*)aMainloop, 0);
}

static void stream_state_cb(pa_stream* aStream, void* aMainloop)
{
   pa_threaded_mainloop_signal((pa_threaded_mainloop*)aMainloop, 0);
}

static void stream_underflow_cb(pa_stream* aStream, void* aUserData)
{
   printf("underflow\n");
}

static void stream_overflow_cb(pa_stream* aStream, void* aUserData)
{
   printf("overflow\n");
}

static void stream_success_cb(pa_stream* aStream, int aSuccess, void* aUserData)
{
   printf("success %d\n", aSuccess);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

static void stream_write_cb(pa_stream* aStream, size_t aRequestedBytes, void* aUserData)
{
   size_t tBytesRemaining = aRequestedBytes;
   while (tBytesRemaining > 0)
   {
      // Begin write buffer to stream.
      int tRet = 0;
      short* tBuffer = NULL;
      size_t tBytesToFill = tBytesRemaining;
      int tMin = 0;
      int tMax = 0;

      pa_stream_begin_write(aStream, (void**)&tBuffer, &tBytesToFill);

      int tSamplesToFill = (int)tBytesToFill / 2;

      // Read from file into buffer. 
      int tSamplesRead = op_read(mFile, tBuffer, tSamplesToFill, NULL);
      if (tSamplesRead < 0)
      {
         printf("read error %d\n", tSamplesRead);
         return;
      }
      int tBytesRead = tSamplesRead * 2;

      // Metrics.
      for (int i = 0; i < tSamplesRead; i++)
      {
         short tValue = tBuffer[i];
         if (tValue < tMin) tMin = tValue;
         if (tValue > tMax) tMax = tValue;
      }

      // Write buffer to stream.
      pa_stream_write(aStream, tBuffer, tBytesRead, NULL, 0LL, PA_SEEK_RELATIVE);

      tBytesRemaining -= tBytesRead;

      printf("stream_write_cb %d %d %d %d $ %d %d\n",
         mWriteCount++,
         (int)aRequestedBytes,
         (int)tBytesToFill,
         (int)tBytesRemaining,
         tMin,tMax);
   }
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void doPlay1(double aSkip)
{
   int tRet;

   // Open opus file.
   printf("opening opus file %s\n", cFilePath);
   int tError = 0;
   mFile = op_open_file(cFilePath, &tError);
   if (tError)
   {
      printf("opus file FAIL %d\n", tError);
      return;
   }

   // Seek opus file.
   long long tSkip = aSkip * 48000;
   printf("seeking opus file %lld\n", tSkip);
   tError = op_pcm_seek(mFile, tSkip);
   if (tError)
   {
      printf("opus seek FAIL %d\n", tError);
      return;
   }

   // Get a mainloop and its context
   mMainLoop = pa_threaded_mainloop_new();
   mMainLoopApi = pa_threaded_mainloop_get_api(mMainLoop);
   mContext = pa_context_new(mMainLoopApi, "pcm-playback");

   // Set a callback so we can wait for the context to be ready
   pa_context_set_state_callback(mContext, &context_state_cb, mMainLoop);

   // Lock the mainloop so that it does not run and crash before the context is ready
   pa_threaded_mainloop_lock(mMainLoop);

   // Start the mainloop
   tRet = pa_threaded_mainloop_start(mMainLoop);
   if (tRet)
   {
      printf("pa_threaded_mainloop_start FAIL\n");
      return;
   }
   tRet = pa_context_connect(mContext, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL);
   if (tRet)
   {
      printf("pa_context_connect FAIL\n");
      return;
   }

   // Wait for the context to be ready
   while (1)
   {
      pa_context_state_t context_state = pa_context_get_state(mContext);
      assert(PA_CONTEXT_IS_GOOD(context_state));
      if (context_state == PA_CONTEXT_READY) break;
      pa_threaded_mainloop_wait(mMainLoop);
   }

   printf("ready\n");

   // Create a playback stream
   mSampleSpec.rate = 48000;
   mSampleSpec.channels = 1;
   mSampleSpec.format = PA_SAMPLE_S16LE;

   mStream = pa_stream_new(mContext, "Playback", &mSampleSpec, NULL);
   pa_stream_set_state_callback(mStream, stream_state_cb, mMainLoop);
   pa_stream_set_write_callback(mStream, stream_write_cb, mMainLoop);
   pa_stream_set_underflow_callback(mStream, stream_underflow_cb, mMainLoop);
   pa_stream_set_overflow_callback(mStream, stream_overflow_cb, mMainLoop);

   // recommended settings, i.e. server uses sensible values
   pa_buffer_attr tBufferAttr;
   tBufferAttr.maxlength = (uint32_t)-1;
   tBufferAttr.tlength = (uint32_t)-1;
   tBufferAttr.prebuf = (uint32_t)-1;
   tBufferAttr.minreq = (uint32_t)-1;

   // Settings copied as per the chromium browser source
   pa_stream_flags_t tStreamFlags;
   tStreamFlags = (pa_stream_flags_t)(
      PA_STREAM_START_CORKED | PA_STREAM_INTERPOLATE_TIMING |
      PA_STREAM_NOT_MONOTONIC | PA_STREAM_AUTO_TIMING_UPDATE |
      PA_STREAM_ADJUST_LATENCY);

   // Connect stream to the default audio output sink
   pa_stream_connect_playback(mStream, NULL, &tBufferAttr, tStreamFlags, NULL, NULL);

   // Wait for the stream to be ready
   while (1)
   {
      pa_stream_state_t tStreamState = pa_stream_get_state(mStream);
      if (tStreamState == PA_STREAM_READY) break;
      pa_threaded_mainloop_wait(mMainLoop);
   }

   pa_threaded_mainloop_unlock(mMainLoop);

   // Uncork the stream so it will start playing
   pa_stream_cork(mStream, 0, stream_success_cb, mMainLoop);

   printf("running\n");
}

void doStopPlay1()
{
   if (mMainLoop == 0) return;
   printf("stopping\n");
   pa_threaded_mainloop_stop(mMainLoop);
   pa_stream_disconnect(mStream);
   pa_context_disconnect(mContext);
   printf("stopped\n");
   mMainLoop = 0;
   mContext = 0;
   mStream = 0;
}
