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

#define FORMAT PA_SAMPLE_U8
#define RATE 44100

static void context_state_cb(pa_context* context, void* mainloop);
static void stream_state_cb(pa_stream* s, void* mainloop);
static void stream_success_cb(pa_stream* stream, int success, void* userdata);
static void stream_write_cb(pa_stream* stream, size_t requested_bytes, void* userdata);

void context_state_cb(pa_context* context, void* mainloop)
{
   pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop, 0);
}

void stream_state_cb(pa_stream* s, void* mainloop)
{
   pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop, 0);
}

void stream_write_cb(pa_stream* stream, size_t requested_bytes, void* userdata)
{
   size_t bytes_remaining = requested_bytes;
   while (bytes_remaining > 0)
   {
      uint8_t* buffer = NULL;
      size_t bytes_to_fill = 44100;
      size_t i;

      if (bytes_to_fill > bytes_remaining) bytes_to_fill = bytes_remaining;

      pa_stream_begin_write(stream, (void**)&buffer, &bytes_to_fill);

      for (i = 0; i < bytes_to_fill; i += 2)
      {
         buffer[i] = (i % 100) * 40 / 100 + 44;
         buffer[i + 1] = (i % 100) * 40 / 100 + 44;
      }

      pa_stream_write(stream, buffer, bytes_to_fill, NULL, 0LL, PA_SEEK_RELATIVE);

      bytes_remaining -= bytes_to_fill;

      printf("stream_write_cb %d %d %d\n", (int)requested_bytes, (int)bytes_to_fill, (int)bytes_remaining);
   }
}

void stream_success_cb(pa_stream* stream, int success, void* userdata) {
   return;
}


static pa_threaded_mainloop* mainloop;
static pa_mainloop_api* mainloop_api;
static pa_context* context;
static pa_stream* stream;

void doRun3()
{
   int retval;

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

   // Create a playback stream
   pa_sample_spec sample_specifications;
   sample_specifications.format = FORMAT;
   sample_specifications.rate = RATE;
   sample_specifications.channels = 2;

   pa_channel_map map;
   pa_channel_map_init_stereo(&map);

   stream = pa_stream_new(context, "Playback", &sample_specifications, &map);
   pa_stream_set_state_callback(stream, stream_state_cb, mainloop);
   pa_stream_set_write_callback(stream, stream_write_cb, mainloop);

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
   pa_stream_connect_playback(stream, NULL, &buffer_attr, stream_flags, NULL, NULL);

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

void doStop3()
{
   printf("stopping\n");
   pa_threaded_mainloop_stop(mainloop);
   pa_stream_disconnect(stream);
   pa_context_disconnect(context);
   printf("stopped\n");
}
