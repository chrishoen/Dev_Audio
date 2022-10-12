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
static void stream_success_cb(pa_stream* aStream, int aSuccess, void* aUserData)
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

static const char* cFilePath = "/opt/prime/tmp/record.wav";
static const char* cDeviceName = "alsa_input.usb-046d_HD_Pro_Webcam_C920_51F943AF-02.analog-stereo";
//static const char* cDeviceName = "alsa_input.hw_0_0";
static SNDFILE* mFile = NULL;
static int mReadCount = 0;
static bool mShowFlag = false;
static bool mWriteFlag = true;

//******************************************************************************
//******************************************************************************
//******************************************************************************

static void stream_read_cb(pa_stream* stream, size_t length, void* userdata)
{
   int total_samples = 0;
   int count = 0;

   // Read.
   int tRet = 0;
   short* peek_sample_buffer = 0;
   size_t bytes_to_peek = length;
   int tMin = 0;
   int tMax = 0;

   // Stream peek. 
   pa_stream_peek(stream, (const void**)&peek_sample_buffer, &bytes_to_peek);
   int samples_to_peek = bytes_to_peek / 2;
   // Metrics.
   for (int i = 0; i < samples_to_peek; i++)
   {
      short tValue = peek_sample_buffer[i];
      if (tValue < tMin) tMin = tValue;
      if (tValue > tMax) tMax = tValue;
   }
   // Write the samples to the wav file.
   if (mWriteFlag)
   {
      sf_write_raw(mFile, peek_sample_buffer, bytes_to_peek);
   }

   // Stream drop.
   pa_stream_drop(stream);

   if (mShowFlag)
   {
      printf("stream_read_cb %d %d $ %4d %4d\n",
         mReadCount++,
         samples_to_peek,
         tMin, tMax);
   }
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

static void stream_state_cb(pa_stream* s, void* userdata)
{
   switch (pa_stream_get_state(s))
   {
   case PA_STREAM_CREATING:
      printf("stream_state_cb creating\n");
      break;
   case PA_STREAM_TERMINATED:
      printf("stream_state_cb terminated\n");
      break;

   case PA_STREAM_READY:
      printf("stream_state_cb ready\n");
      const pa_buffer_attr* a;
      char cmt[PA_CHANNEL_MAP_SNPRINT_MAX], sst[PA_SAMPLE_SPEC_SNPRINT_MAX];

      if (!(a = pa_stream_get_buffer_attr(s)))
      {
         printf("pa_stream_get_buffer_attr failed: %s\n", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
      }
      else
      {
         printf("Buffer metrics: maxlength=%u, fragsize=%u\n", a->maxlength, a->fragsize);
      }

      printf("Using sample spec '%s', channel map '%s'.\n",
         pa_sample_spec_snprint(sst, sizeof(sst), pa_stream_get_sample_spec(s)),
         pa_channel_map_snprint(cmt, sizeof(cmt), pa_stream_get_channel_map(s)));

      printf("Connected to device %s (%u, %ssuspended).\n",
         pa_stream_get_device_name(s),
         pa_stream_get_device_index(s),
         pa_stream_is_suspended(s) ? "" : "not ");

      printf("stream_state_cb ready done\n");
      pa_threaded_mainloop_signal((pa_threaded_mainloop*)mMainLoop, 0);
      break;

   case PA_STREAM_FAILED:
   default:
      printf("Stream error: %s\n", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
      break;
   }
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

static void context_state_cb(pa_context* c, void* userdata)
{
   int tRet = 0;
   switch (pa_context_get_state(c)) {
   case PA_CONTEXT_CONNECTING:
      printf("context connecting\n");
      break;
   case PA_CONTEXT_AUTHORIZING:
      printf("context authorizing\n");
      break;
   case PA_CONTEXT_SETTING_NAME:
      printf("context setting name\n");
      break;

   case PA_CONTEXT_READY: {
      printf("context ready\n");

      // Create a stream
      mStream = pa_stream_new(mContext, "Record", &mSampleSpec, NULL);
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
      pa_stream_flags_t stream_flags;
      stream_flags = (pa_stream_flags_t)(
         PA_STREAM_START_CORKED | PA_STREAM_INTERPOLATE_TIMING |
         PA_STREAM_NOT_MONOTONIC | PA_STREAM_AUTO_TIMING_UPDATE |
         PA_STREAM_ADJUST_LATENCY);
      stream_flags = (pa_stream_flags_t)0;
      // Connect stream to the default audio output sink
   // tRet = pa_stream_connect_record(stream, NULL, &buffer_attr, stream_flags);
   // tRet = pa_stream_connect_record(stream, cDeviceName, NULL, stream_flags);
      tRet = pa_stream_connect_record(mStream, NULL, NULL, stream_flags);
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
      printf("Context failure: %s"), pa_strerror(pa_context_errno(c));
      break;
   }

}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void doRec2(bool aShowFlag)
{
   int tRet;
   int error;

   // Set the global sample spec.
   mSampleSpec.rate = 44100;
   mSampleSpec.channels = 1;
   mSampleSpec.format = PA_SAMPLE_S16LE;

   // Set file info.
   SF_INFO sfi;
   memset(&sfi, 0, sizeof(sfi));
   sfi.samplerate = mSampleSpec.rate;
   sfi.channels = mSampleSpec.channels;
   sfi.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
   sfi.frames = mSampleSpec.rate * mSampleSpec.channels * 2;
   if (!sf_format_check(&sfi))
   {
      printf("sf_format_check FAIL\n");
      return;
   }

   // Open file.
   mFile = sf_open(cFilePath, SFM_WRITE, &sfi);
   if (!mFile)
   {
      printf("sf_open FAIL\n");
      return;
   }
   printf("openned record file %s\n", cFilePath);

   // Get a mainloop and its context
   mShowFlag = aShowFlag;
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
      pa_context_state_t context_state = pa_context_get_state(mContext);
      assert(PA_CONTEXT_IS_GOOD(context_state));
      if (context_state == PA_CONTEXT_READY)
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
         pa_stream_state_t stream_state = pa_stream_get_state(mStream);
         pa_threaded_mainloop_unlock(mMainLoop);
         assert(PA_STREAM_IS_GOOD(stream_state));
         if (stream_state == PA_STREAM_READY)
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

void doStopRec2()
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

   sf_close(mFile);

   printf("stopped\n");
   mMainLoop = 0;
   mContext = 0;
   mStream = 0;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void doShowRec2()
{
   if (!mMainLoop) return;
   printf("show2*****************************************\n");
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
