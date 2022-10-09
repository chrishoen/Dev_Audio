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

static void context_state_cb(pa_context* context, void* mainloop)
{
   pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop, 0);
}

static void stream_state_cb(pa_stream* s, void* mainloop)
{
   pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop, 0);
}

static void stream_underflow_cb(pa_stream* s, void* userdata)
{
   printf("underflow\n");
}

static void stream_overflow_cb(pa_stream* s, void* userdata)
{
   printf("overflow\n");
}

static void stream_success_cb(pa_stream* stream, int success, void* userdata)
{
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
FILE* mFile = 0;
static int read_count = 0;

static void stream_read_cb(pa_stream* stream, size_t length, void* userdata)
{
   int total_samples = 0;
   int count = 0;
   while (pa_stream_readable_size(stream) > 0)
   {
      // Read.
      int retval = 0;
      short* peek_sample_buffer = 0;
      size_t bytes_to_peek = length;

      // Stream peek. 
      pa_stream_peek(stream, (const void**)&peek_sample_buffer, &bytes_to_peek);
      int samples_to_peek = bytes_to_peek / 2;
      total_samples += samples_to_peek;

      // Write the samples to the raw file.
      fwrite(peek_sample_buffer, 2, samples_to_peek, mFile);

      // Stream drop.
      pa_stream_drop(stream);
      count++;
   }

   printf("stream_read_cb %d %d %d\n",
      read_count++,
      total_samples,
      count);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

void doRec1()
{
   int retval;
   int error;

   // Open raw file.
   printf("opening raw record file %s\n", cFilePath);
   mFile = fopen(cFilePath, "wb");

   // Get a mainloop and its context
   mainloop = pa_threaded_mainloop_new();
   mainloop_api = pa_threaded_mainloop_get_api(mainloop);
   context = pa_context_new(mainloop_api, "pcm-playback");

   // Set a callback so we can wait for the context to be ready
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
      pa_context_state_t context_state = pa_context_get_state(context);
      assert(PA_CONTEXT_IS_GOOD(context_state));
      if (context_state == PA_CONTEXT_READY) break;
      pa_threaded_mainloop_wait(mainloop);
   }

   printf("ready\n");

   // Create a playback stream
   pa_sample_spec sample_spec;
   sample_spec.rate = 48000;
   sample_spec.channels = 1;
   sample_spec.format = PA_SAMPLE_S16LE;

   stream = pa_stream_new(context, "Record", &sample_spec, NULL);
   pa_stream_set_state_callback(stream, stream_state_cb, mainloop);
   pa_stream_set_read_callback(stream, stream_read_cb, mainloop);
   pa_stream_set_underflow_callback(stream, stream_underflow_cb, mainloop);
   pa_stream_set_overflow_callback(stream, stream_overflow_cb, mainloop);

   // recommended settings, i.e. server uses sensible values
   pa_buffer_attr buffer_attr;
   buffer_attr.maxlength = (uint32_t)-1;
   buffer_attr.tlength = (uint32_t)-1;
   buffer_attr.prebuf = (uint32_t)-1;
   buffer_attr.minreq = (uint32_t)-1;

   // Settings copied as per the chromium browser source
   pa_stream_flags_t stream_flags;
   stream_flags = (pa_stream_flags_t)(
      PA_STREAM_START_CORKED | PA_STREAM_INTERPOLATE_TIMING |
      PA_STREAM_NOT_MONOTONIC | PA_STREAM_AUTO_TIMING_UPDATE |
      PA_STREAM_ADJUST_LATENCY);

   // Connect stream to the default audio output sink
   retval = pa_stream_connect_record(stream, NULL, &buffer_attr, stream_flags);
   if (retval)
   {
      printf("pa_stream_connect_record %d\n", retval);
      return;
   }
   printf("connected\n");

   // Wait for the stream to be ready
   while (1)
   {
      pa_stream_state_t stream_state = pa_stream_get_state(stream);
      assert(PA_STREAM_IS_GOOD(stream_state));
      if (stream_state == PA_STREAM_READY) break;
      pa_threaded_mainloop_wait(mainloop);
   }

   pa_threaded_mainloop_unlock(mainloop);

   // Uncork the stream so it will start playing
   pa_stream_cork(stream, 0, stream_success_cb, mainloop);

   printf("running\n");
}

void doStopRec1()
{
   if (mainloop == 0) return;
   printf("stopping\n");
   pa_threaded_mainloop_stop(mainloop);
   pa_stream_disconnect(stream);
   pa_context_disconnect(context);

   fclose(mFile);

   printf("stopped\n");
   mainloop = 0;
}
