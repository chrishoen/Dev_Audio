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
#include <sndfile.h>

#include "risProgramTime.h"
#include "TestOne.h"

//******************************************************************************
//******************************************************************************
//******************************************************************************

static void stream_suspended_cb(pa_stream* aStream, void* aMainLoop)
{
   printf("stream_suspended_cb\n");
}
static void stream_moved_cb(pa_stream* aStream, void* aMainLoop)
{
   printf("stream_moved_cb\n");
}
static void stream_underflow_cb(pa_stream* aStream, void* aUserData)
{
   printf("stream_underflow_cb\n");
}
static void stream_overflow_cb(pa_stream* aStream, void* aUserData)
{
   printf("stream_overflow_cb\n");
}
static void stream_started_cb(pa_stream* aStream, void* aUserData)
{
   printf("stream_started_cb\n");
}
static void stream_event_cb(pa_stream* aStream, const char* aName, pa_proplist* pl, void* aUserData)
{
   printf("stream_event_cb\n");
}
static void stream_buffer_attr_cb(pa_stream* aStream, void* aUserData)
{
   printf("stream_buffer_attr_cb\n");
}
static void stream_success_cb(pa_stream* aStream, int aSuccess,void* aUserData)
{
   printf("stream_success_cb %d\n", aSuccess);
   return;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

static pa_threaded_mainloop* mMainLoop = 0;
static pa_mainloop_api* mMainLoopApi = 0;
static pa_context* mContext = 0;
static pa_stream* mStream = 0;

static pa_sample_spec mSampleSpec;

static const char* cFilePath = "/opt/prime/tmp/record.raw";
static const char* cDeviceName = "alsa_input.usb-046d_HD_Pro_Webcam_C920_51F943AF-02.analog-stereo";
//static const char* cDeviceName = "alsa_input.hw_0_0";

static int mReadCount = 0;

static bool mFirstReadFlag = false;
static double mTime = 0;
static double mLastTime = 0;
static double mDeltaTime = 0;
static double mStartReadTime = 0;

//******************************************************************************
//******************************************************************************
//******************************************************************************

static void stream_read_cb(pa_stream* aStream, size_t aLength, void* aUserData)
{
   // Get times.
   if (mFirstReadFlag)
   {
      mFirstReadFlag = false;
      mStartReadTime = Ris::getProgramTime();
      static double mDeltaTime = 0;
   }
   mLastTime = mTime;
   mTime = Ris::getProgramTime() - mStartReadTime;
   mDeltaTime = mTime - mLastTime;

   // Read.
   int tRet = 0;
   short* tPeekSampleBuffer = 0;
   size_t tBytesToPeek = aLength;
   int tMin = 0;
   int tMax = 0;

   // Stream peek. 
   pa_stream_peek(aStream, (const void**)&tPeekSampleBuffer, &tBytesToPeek);
   int tSamplesToPeek = tBytesToPeek / 2;

   // Metrics.
   for (int i = 0; i < tSamplesToPeek; i++)
   {
      short tValue = tPeekSampleBuffer[i];
      if (tValue < tMin) tMin = tValue;
      if (tValue > tMax) tMax = tValue;
   }

   // Stream drop.
   pa_stream_drop(aStream);

   // Show.
   Prn::print(Prn::Show1, "stream_read_cb %4d $ %.3f  %.3f $ %5d $ %5d %5d",
      mReadCount++,
      mTime, mDeltaTime,
      tSamplesToPeek,
      tMin, tMax);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

static void stream_state_cb(pa_stream* aStream, void* aUserData)
{
   switch (pa_stream_get_state(aStream))
   {
   case PA_STREAM_CREATING:
      printf("stream_state_cb creating\n");
      break;
   case PA_STREAM_TERMINATED:
      printf("stream_state_cb terminated\n");
      break;

   case PA_STREAM_READY:
      printf("stream_state_cb ready\n");
      const pa_buffer_attr* tBuffAttr;
      char cmt[PA_CHANNEL_MAP_SNPRINT_MAX], sst[PA_SAMPLE_SPEC_SNPRINT_MAX];

      if (!(tBuffAttr = pa_stream_get_buffer_attr(aStream)))
      {
         printf("pa_stream_get_buffer_attr failed: %s\n",
            pa_strerror(pa_context_errno(pa_stream_get_context(aStream))));
      }
      else
      {
         printf("Buffer metrics: maxlength=%u, fragsize=%u\n",
            tBuffAttr->maxlength, tBuffAttr->fragsize);
      }

      printf("Using sample spec '%s', channel map '%s'.\n",
         pa_sample_spec_snprint(sst, sizeof(sst), pa_stream_get_sample_spec(aStream)),
         pa_channel_map_snprint(cmt, sizeof(cmt), pa_stream_get_channel_map(aStream)));

      printf("Connected to device %s (%u, %ssuspended).\n",
         pa_stream_get_device_name(aStream),
         pa_stream_get_device_index(aStream),
         pa_stream_is_suspended(aStream) ? "" : "not ");

      printf("stream_state_cb ready done\n");
      pa_threaded_mainloop_signal((pa_threaded_mainloop*)mMainLoop, 0);
      break;

   case PA_STREAM_FAILED:
   default:
      printf("Stream error: %s\n", 
         pa_strerror(pa_context_errno(pa_stream_get_context(aStream))));
      break;
   }
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

static void context_state_cb(pa_context* aContext, void* aUserData)
{
   int tRet = 0;
   switch (pa_context_get_state(aContext))
   {
   case PA_CONTEXT_CONNECTING:
      printf("context connecting\n");
      break;
   case PA_CONTEXT_AUTHORIZING:
      printf("context authorizing\n");
      break;
   case PA_CONTEXT_SETTING_NAME:
      printf("context setting name\n");
      break;

   case PA_CONTEXT_READY:
   {
      printf("context ready\n");

      // Create a stream
      mStream = pa_stream_new(mContext, "Record", &mSampleSpec, NULL);
      if (!mStream)
      {
         printf("pa_stream_new FAIL\n");
         return;
      }
      printf("pa_stream_new PASS\n");

      // Assign stream callbacks.
      pa_stream_set_state_callback(mStream, stream_state_cb, NULL);
      pa_stream_set_read_callback(mStream, stream_read_cb, NULL);
      pa_stream_set_suspended_callback(mStream, stream_suspended_cb, NULL);
      pa_stream_set_moved_callback(mStream, stream_moved_cb, NULL);
      pa_stream_set_underflow_callback(mStream, stream_underflow_cb, NULL);
      pa_stream_set_overflow_callback(mStream, stream_overflow_cb, NULL);
      pa_stream_set_started_callback(mStream, stream_started_cb, NULL);
      pa_stream_set_event_callback(mStream, stream_event_cb, NULL);
      pa_stream_set_buffer_attr_callback(mStream, stream_buffer_attr_cb, NULL);
      printf("set stream callbacks PASS\n");

      // Connect stream.
      pa_stream_flags_t tStreamFlags;
      tStreamFlags = (pa_stream_flags_t)(
         PA_STREAM_START_CORKED | PA_STREAM_INTERPOLATE_TIMING |
         PA_STREAM_NOT_MONOTONIC | PA_STREAM_AUTO_TIMING_UPDATE |
         PA_STREAM_ADJUST_LATENCY);
      tStreamFlags = (pa_stream_flags_t)0;
      // Connect stream to the default audio output sink
   // tRet = pa_stream_connect_record(stream, NULL, &buffer_attr, stream_flags);
   // tRet = pa_stream_connect_record(stream, cDeviceName, NULL, stream_flags);
      tRet = pa_stream_connect_record(mStream, NULL, NULL, tStreamFlags);
      if (tRet)
      {
         printf("pa_stream_connect_record FAIL %d %s\n", tRet, pa_strerror(tRet));
         return;
      }
      printf("pa_stream_connect_record PASS\n");
      printf("context ready done\n");
      // Signal the main thread.
      pa_threaded_mainloop_signal((pa_threaded_mainloop*)mMainLoop, 0);
      break;
   }

   case PA_CONTEXT_TERMINATED:
      printf("context terminated\n");
      break;

   case PA_CONTEXT_FAILED:
   default:
      printf("Context failure: %s"), pa_strerror(pa_context_errno(aContext));
      break;
   }
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void doRec1(bool aShowFlag)
{
   int tRet;

   // Do this first.
   Prn::setFilter(Prn::Show1, true);
   mFirstReadFlag = true;

   // Set the sample spec.
   mSampleSpec.rate = 44100;
   mSampleSpec.channels = 1;
   mSampleSpec.format = PA_SAMPLE_S16LE;

   // Get a mainloop and its context
   mMainLoop = pa_threaded_mainloop_new();
   mMainLoopApi = pa_threaded_mainloop_get_api(mMainLoop);
   mContext = pa_context_new(mMainLoopApi, "pcm-playback");
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
      printf("context ready loop begin\n");
      pa_context_state_t tContextState = pa_context_get_state(mContext);
      if (tContextState == PA_CONTEXT_READY)
      {
         printf("context ready loop ready\n");
         break;
      }
      pa_threaded_mainloop_wait(mMainLoop);
      printf("context ready loop end\n");
   }

   // Wait for the stream to be ready
   while (1)
   {
      printf("stream ready loop begin\n");
      if (mStream)
      {
         pa_threaded_mainloop_lock(mMainLoop);
         pa_stream_state_t tStreamState = pa_stream_get_state(mStream);
         pa_threaded_mainloop_unlock(mMainLoop);
         if (tStreamState == PA_STREAM_READY)
         {
            printf("stream ready loop ready\n");
            break;
         }
      }
      pa_threaded_mainloop_wait(mMainLoop);
      printf("stream ready loop end\n");
   }
   printf("stream ready\n");

   pa_threaded_mainloop_unlock(mMainLoop);

   // Uncork the stream so it will start playing
   pa_stream_cork(mStream, 0, stream_success_cb, mMainLoop);

   printf("running\n");
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void doStopRec1()
{
   if (mMainLoop == 0) return;
   printf("stopping\n");
   pa_threaded_mainloop_stop(mMainLoop);
   pa_stream_disconnect(mStream);
   pa_context_disconnect(mContext);

   if (mStream)
   {
      pa_stream_unref(mStream);
   }

   if (mContext)
   {
      pa_context_unref(mContext);
   }

   if (mMainLoop)
   {
      pa_signal_done();
      pa_threaded_mainloop_free(mMainLoop);
   }

   printf("stopped\n");
   mMainLoop = 0;
   mContext = 0;
   mStream = 0;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void doShowRec1()
{
   if (!mMainLoop) return;
   printf("show1*****************************************\n");
   pa_threaded_mainloop_lock(mMainLoop);

   switch (pa_stream_get_state(mStream))
   {
   case PA_STREAM_READY:
      printf("stream ready\n");
      break;
   default:
      printf("stream not ready\n");
      break;
   }

   printf("Connected to device %s (%u, %ssuspended).\n",
      pa_stream_get_device_name(mStream),
      pa_stream_get_device_index(mStream),
      pa_stream_is_suspended(mStream) ? "" : "not ");

   pa_threaded_mainloop_unlock(mMainLoop);
}
