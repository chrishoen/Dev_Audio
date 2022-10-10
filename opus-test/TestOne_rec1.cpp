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

static void stream_suspended_cb(pa_stream* s, void* mainloop)
{
   printf("stream_suspended_cb\n");
}
static void stream_moved_cb(pa_stream* s, void* mainloop)
{
   printf("stream_moved_cb\n");
}
static void stream_underflow_cb(pa_stream* s, void* userdata)
{
   printf("stream_underflow_cb\n");
}
static void stream_overflow_cb(pa_stream* s, void* userdata)
{
   printf("stream_overflow_cb\n");
}
static void stream_started_cb(pa_stream* s, void* userdata)
{
   printf("stream_started_cb\n");
}
static void stream_event_cb(pa_stream* s, const char* name, pa_proplist* pl, void* userdata)
{
   printf("stream_event_cb\n");
}
static void stream_buffer_attr_cb(pa_stream* stream, void* userdata)
{
   printf("stream_buffer_attr_cb\n");
}
static void stream_success_cb(pa_stream* stream, int success, void* userdata)
{
   printf("stream_success_cb %d\n", success);
   return;
}


//******************************************************************************
//******************************************************************************
//******************************************************************************

static pa_threaded_mainloop* mainloop = 0;
static pa_mainloop_api* mainloop_api = 0;
static pa_context* context = 0;
static pa_stream* stream = 0;

static const char* cFilePath = "/opt/prime/tmp/record.raw";
static const char* cDeviceName = "alsa_input.usb-046d_HD_Pro_Webcam_C920_51F943AF-02.analog-stereo";
//static const char* cDeviceName = "alsa_input.hw_0_0";
FILE* mFile = 0;
static int read_count = 0;
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
   int retval = 0;
   short* peek_sample_buffer = 0;
   size_t bytes_to_peek = length;
   int tMin = 0;
   int tMax = 0;

   // Stream peek. 
   pa_stream_peek(stream, (const void**)&peek_sample_buffer, &bytes_to_peek);
   int samples_to_peek = bytes_to_peek / 2;
   total_samples += samples_to_peek;
   // Metrics.
   for (int i = 0; i < samples_to_peek; i++)
   {
      short tValue = peek_sample_buffer[i];
      if (tValue < tMin) tMin = tValue;
      if (tValue > tMax) tMax = tValue;
   }
   // Write the samples to the raw file.
   if (mWriteFlag)
   {
      fwrite(peek_sample_buffer, 2, samples_to_peek, mFile);
   }

   // Stream drop.
   pa_stream_drop(stream);

   if (mShowFlag)
   {
      printf("stream_read_cb %d %d $ %4d %4d\n",
         read_count++,
         total_samples,
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
      pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop, 0);
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
   int retval = 0;
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
      pa_sample_spec sample_spec;
      sample_spec.rate = 32000;
      sample_spec.channels = 1;
      sample_spec.format = PA_SAMPLE_S16LE;
      stream = pa_stream_new(context, "Record", &sample_spec, NULL);
      printf("pa_stream_new PASS\n");

      // Assign stream callbacks.
      pa_stream_set_state_callback(stream, stream_state_cb, NULL);
      pa_stream_set_read_callback(stream, stream_read_cb, NULL);
      pa_stream_set_suspended_callback(stream, stream_suspended_cb, NULL);
      pa_stream_set_moved_callback(stream, stream_moved_cb, NULL);
      pa_stream_set_underflow_callback(stream, stream_underflow_cb, NULL);
      pa_stream_set_overflow_callback(stream, stream_overflow_cb, NULL);
      pa_stream_set_started_callback(stream, stream_started_cb, NULL);
      pa_stream_set_event_callback(stream, stream_event_cb, NULL);
      pa_stream_set_buffer_attr_callback(stream, stream_buffer_attr_cb, NULL);
      printf("set stream callbacks PASS\n");

      // Connect stream.
      pa_stream_flags_t stream_flags;
      stream_flags = (pa_stream_flags_t)(
         PA_STREAM_START_CORKED | PA_STREAM_INTERPOLATE_TIMING |
         PA_STREAM_NOT_MONOTONIC | PA_STREAM_AUTO_TIMING_UPDATE |
         PA_STREAM_ADJUST_LATENCY);
      stream_flags = (pa_stream_flags_t)0;
      // Connect stream to the default audio output sink
   // retval = pa_stream_connect_record(stream, NULL, &buffer_attr, stream_flags);
   // retval = pa_stream_connect_record(stream, cDeviceName, NULL, stream_flags);
      retval = pa_stream_connect_record(stream, NULL, NULL, stream_flags);
      if (retval)
      {
         printf("pa_stream_connect_record FAIL %d %s\n", retval, pa_strerror(retval));
         return;
      }
      printf("pa_stream_connect_record PASS\n");
      printf("context ready done\n");
      // Signal the main thread.
      pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop, 0);
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

void doRec1(bool aShowFlag)
{
   int retval;
   int error;

   // Open raw file.
   printf("opening record file %s\n", cFilePath);
   mFile = fopen(cFilePath, "wb");

   // Get a mainloop and its context
   mShowFlag = aShowFlag;
   mainloop = pa_threaded_mainloop_new();
   mainloop_api = pa_threaded_mainloop_get_api(mainloop);
   context = pa_context_new(mainloop_api, "pcm-playback");
   pa_context_set_state_callback(context, &context_state_cb, mainloop);

   // Lock the mainloop so that it does not run and crash before the context is ready
   pa_threaded_mainloop_lock(mainloop);

   // Start the mainloop
   retval = pa_threaded_mainloop_start(mainloop);
   if (retval)
   {
      printf("pa_threaded_mainloop_start FAIL\n");
      return;
   }
   retval = pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL);
   if (retval)
   {
      printf("pa_context_connect FAIL\n");
      return;
   }

   // Wait for the context to be ready
   while (1)
   {
      printf("context ready loop begin\n");
      pa_context_state_t context_state = pa_context_get_state(context);
      assert(PA_CONTEXT_IS_GOOD(context_state));
      if (context_state == PA_CONTEXT_READY)
      {
         printf("context ready loop ready\n");
         break;
      }
      pa_threaded_mainloop_wait(mainloop);
      printf("context ready loop end\n");
   }

   // Wait for the stream to be ready
   while (1)
   {
      printf("stream ready loop begin\n");
      if (stream)
      {
         pa_threaded_mainloop_lock(mainloop);
         pa_stream_state_t stream_state = pa_stream_get_state(stream);
         pa_threaded_mainloop_unlock(mainloop);
         assert(PA_STREAM_IS_GOOD(stream_state));
         if (stream_state == PA_STREAM_READY)
         {
            printf("stream ready loop ready\n");
            break;
         }
      }
      pa_threaded_mainloop_wait(mainloop);
      printf("stream ready loop end\n");
   }
   printf("stream ready\n");

   pa_threaded_mainloop_unlock(mainloop);

   // Uncork the stream so it will start playing
   pa_stream_cork(stream, 0, stream_success_cb, mainloop);

   printf("running\n");
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void doStopRec1()
{
   if (mainloop == 0) return;
   printf("stopping\n");
   pa_threaded_mainloop_stop(mainloop);
   pa_stream_disconnect(stream);
   pa_context_disconnect(context);

   if (stream)
      pa_stream_unref(stream);

   if (context)
      pa_context_unref(context);

   if (mainloop) {
      pa_signal_done();
      pa_threaded_mainloop_free(mainloop);
   }

   fclose(mFile);

   printf("stopped\n");
   mainloop = 0;
   context = 0;
   stream = 0;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void doShowRec1()
{
   if (!mainloop) return;
   printf("show*****************************************begin\n");
   pa_threaded_mainloop_lock(mainloop);

   switch (pa_stream_get_state(stream))
   {
   case PA_STREAM_READY:
      printf("stream ready\n");
      break;
   default:
      printf("stream not ready\n");
      break;
   }

   printf("Connected to device %s (%u, %ssuspended).\n",
      pa_stream_get_device_name(stream),
      pa_stream_get_device_index(stream),
      pa_stream_is_suspended(stream) ? "" : "not ");

   pa_threaded_mainloop_unlock(mainloop);
   printf("show*****************************************end\n");
}
