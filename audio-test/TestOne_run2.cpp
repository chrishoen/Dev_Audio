/*==============================================================================
Description:
==============================================================================*/

//******************************************************************************
//******************************************************************************
//******************************************************************************
#include "stdafx.h"

#include <pulse/pulseaudio.h>

#include "TestOne.h"

static void context_on_state_change(pa_context* ctx, pa_threaded_mainloop* pa)
{
   pa_threaded_mainloop_signal(pa, 0);
}

static void stream_on_state_change(pa_stream* strm, pa_threaded_mainloop* pa)
{
   pa_threaded_mainloop_signal(pa, 0);
}

static void context_poll_unless(pa_threaded_mainloop* pa, pa_context* ctx, pa_context_state_t state)
{
   for (;;) {
      pa_context_state_t s;
      pa_threaded_mainloop_lock(pa);
      s = pa_context_get_state(ctx);
      pa_threaded_mainloop_unlock(pa);
      if (s == state)
         break;
      pa_threaded_mainloop_wait(pa);
   }
}

static void stream_poll_unless(pa_threaded_mainloop* pa, pa_stream* strm, pa_stream_state_t state)
{
   for (;;) {
      pa_stream_state_t s;
      pa_threaded_mainloop_lock(pa);
      s = pa_stream_get_state(strm);
      pa_threaded_mainloop_unlock(pa);
      if (s == state)
         break;
      pa_threaded_mainloop_wait(pa);
   }
}

static void myMain(pa_threaded_mainloop* pa)
{
   pa_mainloop_api* api = pa_threaded_mainloop_get_api(pa);
   pa_context* ctx = pa_context_new(api, "default");
   pa_stream* strm = NULL;
   int err;

   {
      pa_threaded_mainloop_lock(pa);
      err = pa_context_connect(ctx, NULL, (pa_context_flags_t)0, NULL);
      if (err < 0) {
         pa_threaded_mainloop_unlock(pa);
         fprintf(stderr, "Could not connect to the server (%s)\n",
            pa_strerror(err));
         return;
      }
      pa_context_set_state_callback(ctx,
         (pa_context_notify_cb_t)context_on_state_change, pa);
      pa_threaded_mainloop_unlock(pa);
   }

   context_poll_unless(pa, ctx, PA_CONTEXT_READY);

   {
      pa_threaded_mainloop_lock(pa);
      {
         pa_sample_spec ss;
         ss.channels = 1;
         ss.format = PA_SAMPLE_FLOAT32LE;
         ss.rate = 22050;
         strm = pa_stream_new(ctx, "default", &ss, NULL);
         if (!strm) {
            pa_threaded_mainloop_unlock(pa);
            fprintf(stderr, "Failed to create a new stream\n");
            goto out;
         }
      }

      pa_stream_set_state_callback(strm,
         (pa_stream_notify_cb_t)stream_on_state_change, pa);
      pa_threaded_mainloop_unlock(pa);
   }

   context_poll_unless(pa, ctx, PA_CONTEXT_READY);

   {
      pa_threaded_mainloop_lock(pa);
      err = pa_stream_connect_playback(strm, NULL, NULL, (pa_stream_flags_t)0, NULL, NULL);
      if (err < 0) {
         pa_threaded_mainloop_unlock(pa);
         fprintf(stderr, "Failed to connect the stream to sink (%s)\n",
            pa_strerror(err));
         goto out;
      }
      pa_threaded_mainloop_unlock(pa);
   }

   {
      float* samples;
      size_t nbytes;
      int count = 0;
      int period = 2;
      int iteration = 0;
      for (;;) {
         size_t nsamples;
         size_t i;

         context_poll_unless(pa, ctx, PA_CONTEXT_READY);
         stream_poll_unless(pa, strm, PA_STREAM_READY);

         pa_threaded_mainloop_lock(pa);
         err = pa_stream_begin_write(strm, (void**)&samples, &nbytes);
         if (err < 0) {
            pa_threaded_mainloop_unlock(pa);
            fprintf(stderr, "WTF? (%s)\n", pa_strerror(err));
            goto out;
         }
         nsamples = nbytes / sizeof(*samples);
         for (i = 0; i < nsamples; i++) {
            if (count < period / 2)
               samples[i] = 0.5;
            else
               samples[i] = -0.5;
            count++;
            if (count == period) {
               count = 0;
               iteration++;
               if (iteration > 10) {
                  iteration = 0;
                  period++;
               }
            }
         }
         if (pa_stream_write(strm, samples, nsamples * sizeof(*samples),
            NULL, 0, PA_SEEK_RELATIVE) < 0) {
            pa_threaded_mainloop_unlock(pa);
            fprintf(stderr, "WTF?\n");
            goto out;
         }
         pa_threaded_mainloop_unlock(pa);
      }
   }

out:
   pa_threaded_mainloop_lock(pa);
   if (strm)
      pa_stream_disconnect(strm);
   pa_context_disconnect(ctx);
   pa_threaded_mainloop_unlock(pa);
}

void TestOne::doRun2()
{
   pa_threaded_mainloop* pa = pa_threaded_mainloop_new();
   if (!pa) {
      fprintf(stderr, "Failed to initialize pulseaudio\n");
      return;
   }

   if (pa_threaded_mainloop_start(pa) < 0) {
      fprintf(stderr, "Failed to start the main loop\n");
      return;
   }

   myMain(pa);

   pa_threaded_mainloop_free(pa);
   return;
}