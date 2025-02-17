/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2016 - Daniel De Matteis
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>
#include <string.h>

#include <retro_assert.h>
#include <gfx/scaler/pixconv.h>
#include <gfx/scaler/scaler.h>
#ifdef HAVE_THREADS
#include <rthreads/rthreads.h>
#endif

#include <retro_common_api.h>
#include <file/config_file.h>
#include <features/features_cpu.h>
#include <file/file_path.h>
#include <math.h>
#include <retro_math.h>
#include <retro_miscellaneous.h>
#include "../libretro-common/formats/image.h"

#include "video_thread_wrapper.h"
#include "../frontend/frontend_driver.h"
#include "video_context_driver.h"
#include "../record/record_driver.h"
#include "../config.def.h"
#include "../retroarch.h"
#include "../runloop.h"
#include "../performance_counters.h"
#include "../list_special.h"
#include "../core.h"
#include "../system.h"
#include "../command.h"
#include "../msg_hash.h"
#include "../libretro-common/include/formats/image.h"

#ifdef HAVE_MENU
#include "../menu/menu_setting.h"
#endif

#include "../verbosity.h"

/**
 * video_driver_find_handle:
 * @idx                : index of driver to get handle to.
 *
 * Returns: handle to video driver at index. Can be NULL
 * if nothing found.
 **/
const void *video_driver_find_handle(int idx)
{
   const void *drv = video_drivers[idx];
   if (!drv)
      return NULL;
   return drv;
}

/**
 * video_driver_find_ident:
 * @idx                : index of driver to get handle to.
 *
 * Returns: Human-readable identifier of video driver at index. Can be NULL
 * if nothing found.
 **/
const char *video_driver_find_ident(int idx)
{
   const video_driver_t *drv = video_drivers[idx];
   if (!drv)
      return NULL;
   return drv->ident;
}

/**
 * config_get_video_driver_options:
 *
 * Get an enumerated list of all video driver names, separated by '|'.
 *
 * Returns: string listing of all video driver names, separated by '|'.
 **/
const char* config_get_video_driver_options(void)
{
   return char_list_new_special(STRING_LIST_VIDEO_DRIVERS, NULL);
}

static bool hw_render_context_is_vulkan(enum retro_hw_context_type type)
{
   return type == RETRO_HW_CONTEXT_VULKAN;
}

static bool hw_render_context_is_gl(enum retro_hw_context_type type)
{
   switch (type)
   {
      case RETRO_HW_CONTEXT_OPENGL:
      case RETRO_HW_CONTEXT_OPENGLES2:
      case RETRO_HW_CONTEXT_OPENGL_CORE:
      case RETRO_HW_CONTEXT_OPENGLES3:
      case RETRO_HW_CONTEXT_OPENGLES_VERSION:
         return true;
      default:
         break;
   }

   return false;
}

/**
 * video_driver_get_ptr:
 *
 * Use this if you need the real video driver
 * and driver data pointers.
 *
 * Returns: video driver's userdata.
 **/
void *video_driver_get_ptr(bool force_nonthreaded_data)
{
#ifdef HAVE_THREADS
   settings_t *settings = config_get_ptr();

   if (settings->video.threaded
         && !video_driver_is_hw_context()
         && !force_nonthreaded_data)
      return video_thread_get_ptr(NULL);
#endif

   return video_driver_data;
}

const char *video_driver_get_ident(void)
{
   return (current_video) ? current_video->ident : NULL;
}

const video_poke_interface_t *video_driver_get_poke(void)
{
   return video_driver_poke;
}

/**
 * video_driver_get_current_framebuffer:
 *
 * Gets pointer to current hardware renderer framebuffer object.
 * Used by RETRO_ENVIRONMENT_SET_HW_RENDER.
 *
 * Returns: pointer to hardware framebuffer object, otherwise 0.
 **/
uintptr_t video_driver_get_current_framebuffer(void)
{
   if (!video_driver_poke || !video_driver_poke->get_current_framebuffer)
      return 0;
   return video_driver_poke->get_current_framebuffer(video_driver_data);
}

retro_proc_address_t video_driver_get_proc_address(const char *sym)
{
   if (!video_driver_poke || !video_driver_poke->get_proc_address)
      return NULL;
   return video_driver_poke->get_proc_address(video_driver_data, sym);
}

bool video_driver_set_shader(enum rarch_shader_type type,
      const char *path)
{
   if (!current_video->set_shader)
      return false;
   return current_video->set_shader(video_driver_data, type, path);
}

static void deinit_video_filter(void)
{
   rarch_softfilter_free(video_driver_state.filter.filter);
#ifdef _3DS
   linearFree(video_driver_state.filter.buffer);
#else
   free(video_driver_state.filter.buffer);
#endif
   memset(&video_driver_state.filter, 0,
         sizeof(video_driver_state.filter));
}

static void init_video_filter(enum retro_pixel_format colfmt)
{
   unsigned width, height, pow2_x, pow2_y, maxsize;
   struct retro_game_geometry *geom = NULL;
   settings_t *settings             = config_get_ptr();
   struct retro_system_av_info *av_info =
      video_viewport_get_system_av_info();

   deinit_video_filter();

   if (!*settings->path.softfilter_plugin)
      return;

   /* Deprecated format. Gets pre-converted. */
   if (colfmt == RETRO_PIXEL_FORMAT_0RGB1555)
      colfmt = RETRO_PIXEL_FORMAT_RGB565;

   if (video_driver_is_hw_context())
   {
      WLOG("Cannot use CPU filters when hardware rendering is used.\n");
      return;
   }

   if (av_info)
      geom = (struct retro_game_geometry*)&av_info->geometry;

   if (!geom)
      return;

   width   = geom->max_width;
   height  = geom->max_height;

   video_driver_state.filter.filter = rarch_softfilter_new(
         settings->path.softfilter_plugin,
         RARCH_SOFTFILTER_THREADS_AUTO, colfmt, width, height);

   if (!video_driver_state.filter.filter)
   {
      ELOG("Failed to load filter.\n");
      return;
   }

   rarch_softfilter_get_max_output_size(video_driver_state.filter.filter,
         &width, &height);

   pow2_x                              = next_pow2(width);
   pow2_y                              = next_pow2(height);
   maxsize                             = MAX(pow2_x, pow2_y);
   video_driver_state.filter.scale     = maxsize / RARCH_SCALE_BASE;
   video_driver_state.filter.out_rgb32 = rarch_softfilter_get_output_format(
         video_driver_state.filter.filter) == RETRO_PIXEL_FORMAT_XRGB8888;

   video_driver_state.filter.out_bpp   = 
      video_driver_state.filter.out_rgb32 ?
      sizeof(uint32_t) : sizeof(uint16_t);

   /* TODO: Aligned output. */
#ifdef _3DS
   video_driver_state.filter.buffer    = linearMemAlign(width 
         * height * video_driver_state.filter.out_bpp, 0x80);
#else
   video_driver_state.filter.buffer    = malloc(width 
         * height * video_driver_state.filter.out_bpp);
#endif
   if (!video_driver_state.filter.buffer)
      goto error;

   return;

error:
   ELOG("Softfilter initialization failed.\n");
   deinit_video_filter();
}

static void init_video_input(const input_driver_t *tmp)
{
   const input_driver_t **input = input_get_double_ptr();
   if (*input)
      return;

   /* Video driver didn't provide an input driver,
    * so we use configured one. */
   VLOG("Graphics driver did not initialize an input driver. Attempting to pick a suitable driver.\n");

   if (tmp)
      *input = tmp;
   else
      input_driver_find_driver();

   /* This should never really happen as tmp (driver.input) is always
    * found before this in find_driver_input(), or we have aborted
    * in a similar fashion anyways. */
   if (!input_get_ptr())
      goto error;

   if (input_driver_init())
      return;

error:
   ELOG("Cannot initialize input driver. Exiting ...\n");
   retroarch_fail(1, "init_video_input()");
}

/**
 * video_monitor_compute_fps_statistics:
 *
 * Computes monitor FPS statistics.
 **/
static void video_monitor_compute_fps_statistics(void)
{
   double avg_fps       = 0.0;
   double stddev        = 0.0;
   unsigned samples     = 0;
   settings_t *settings = config_get_ptr();

   if (settings->video.threaded)
   {
      VLOG("Monitor FPS estimation is disabled for threaded video.\n");
      return;
   }

   if (video_driver_state.frame_time.count < 
         (2 * MEASURE_FRAME_TIME_SAMPLES_COUNT))
   {
      RARCH_LOG(
            "Does not have enough samples for monitor refresh rate estimation. Requires to run for at least %u frames.\n",
            2 * MEASURE_FRAME_TIME_SAMPLES_COUNT);
      return;
   }

   if (video_monitor_fps_statistics(&avg_fps, &stddev, &samples))
   {
      VLOG("Average monitor Hz: %.6f Hz. (%.3f %% frame time deviation, based on %u last samples).\n",
            avg_fps, 100.0 * stddev, samples);
   }
}

static void deinit_pixel_converter(void)
{
   if (!video_driver_scaler_ptr)
      return;

   scaler_ctx_gen_reset(video_driver_scaler_ptr->scaler);

   if (video_driver_scaler_ptr->scaler)
      free(video_driver_scaler_ptr->scaler);
   video_driver_scaler_ptr->scaler     = NULL;

   if (video_driver_scaler_ptr->scaler_out)
      free(video_driver_scaler_ptr->scaler_out);
   video_driver_scaler_ptr->scaler_out = NULL;

   if (video_driver_scaler_ptr)
      free(video_driver_scaler_ptr);
   video_driver_scaler_ptr             = NULL;
}

static bool uninit_video_input(void)
{
   command_event(CMD_EVENT_OVERLAY_DEINIT, NULL);

   if (!video_driver_is_video_cache_context())
      video_driver_deinit_hw_context();

   if (
         !input_driver_owns_driver() &&
         !input_driver_is_data_ptr_same(video_driver_data)
      )
      input_driver_deinit();

   if (
         !video_driver_owns_driver()
         && video_driver_data 
         && current_video && current_video->free
      )
      current_video->free(video_driver_data);

   deinit_pixel_converter();
   deinit_video_filter();

   command_event(CMD_EVENT_SHADER_DIR_DEINIT, NULL);
   video_monitor_compute_fps_statistics();

   return true;
}

static bool init_video_pixel_converter(unsigned size)
{
   struct retro_hw_render_callback *hwr =
      video_driver_get_hw_context();

   /* If pixel format is not 0RGB1555, we don't need to do
    * any internal pixel conversion. */
   if (video_driver_get_pixel_format() != RETRO_PIXEL_FORMAT_0RGB1555)
      return true;

   /* No need to perform pixel conversion for HW rendering contexts. */
   if (hwr && hwr->context_type != RETRO_HW_CONTEXT_NONE)
      return true;

   WLOG("0RGB1555 pixel format is deprecated, and will be slower. For 15/16-bit, RGB565 format is preferred.\n");

   video_driver_scaler_ptr = (video_pixel_scaler_t*)
      calloc(1, sizeof(*video_driver_scaler_ptr));

   if (!video_driver_scaler_ptr)
      goto error;

   video_driver_scaler_ptr->scaler = (struct scaler_ctx*)
      calloc(1, sizeof(*video_driver_scaler_ptr->scaler));

   if (!video_driver_scaler_ptr->scaler)
      goto error;

   video_driver_scaler_ptr->scaler->scaler_type = SCALER_TYPE_POINT;
   video_driver_scaler_ptr->scaler->in_fmt      = SCALER_FMT_0RGB1555;

   /* TODO: Pick either ARGB8888 or RGB565 depending on driver. */
   video_driver_scaler_ptr->scaler->out_fmt     = SCALER_FMT_RGB565;

   if (!scaler_ctx_gen_filter(video_driver_scaler_ptr->scaler))
      goto error;

   video_driver_scaler_ptr->scaler_out = 
      calloc(sizeof(uint16_t), size * size);

   if (!video_driver_scaler_ptr->scaler_out)
      goto error;

   return true;

error:
   deinit_pixel_converter();
   deinit_video_filter();

   return false;
}

static bool init_video(void)
{
   unsigned max_dim, scale, width, height;
   video_viewport_t *custom_vp            = NULL;
   const input_driver_t *tmp              = NULL;
   const struct retro_game_geometry *geom = NULL;
   rarch_system_info_t *system            = NULL;
   video_info_t video                     = {0};
   static uint16_t dummy_pixels[32]       = {0};
   settings_t *settings                   = config_get_ptr();
   struct retro_system_av_info *av_info   =
      video_viewport_get_system_av_info();

   runloop_ctl(RUNLOOP_CTL_SYSTEM_INFO_GET, &system);

   init_video_filter(video_driver_state.pix_fmt);
   command_event(CMD_EVENT_SHADER_DIR_INIT, NULL);

   if (av_info)
      geom      = (const struct retro_game_geometry*)&av_info->geometry;

   if (!geom)
   {
      ELOG("AV geometry not initialized, cannot initialize video driver.\n");
      goto error;
   }

   max_dim   = MAX(geom->max_width, geom->max_height);
   scale     = next_pow2(max_dim) / RARCH_SCALE_BASE;
   scale     = MAX(scale, 1);

   if (video_driver_state.filter.filter)
      scale = video_driver_state.filter.scale;

   /* Update core-dependent aspect ratio values. */
   video_driver_set_viewport_square_pixel();
   video_driver_set_viewport_core();
   video_driver_set_viewport_config();

   /* Update CUSTOM viewport. */
   custom_vp = video_viewport_get_custom();

   if (settings->video.aspect_ratio_idx == ASPECT_RATIO_CUSTOM)
   {
      float default_aspect = aspectratio_lut[ASPECT_RATIO_CORE].value;
      aspectratio_lut[ASPECT_RATIO_CUSTOM].value =
         (custom_vp->width && custom_vp->height) ?
         (float)custom_vp->width / custom_vp->height : default_aspect;
   }

   video_driver_set_aspect_ratio_value(
      aspectratio_lut[settings->video.aspect_ratio_idx].value);

   if (settings->video.fullscreen)
   {
      width  = settings->video.fullscreen_x;
      height = settings->video.fullscreen_y;
   }
   else
   {
      if (settings->video.force_aspect)
      {
         /* Do rounding here to simplify integer scale correctness. */
         unsigned base_width =
            roundf(geom->base_height * video_driver_get_aspect_ratio());
         width  = roundf(base_width * settings->video.scale);
      }
      else
         width  = roundf(geom->base_width   * settings->video.scale);
      height = roundf(geom->base_height * settings->video.scale);
   }

   if (width && height)
      VLOG("Video @ %ux%u\n", width, height);
   else
      VLOG("Video @ fullscreen\n");

   video_driver_display_type_set(RARCH_DISPLAY_NONE);
   video_driver_display_set(0);
   video_driver_window_set(0);

   if (!init_video_pixel_converter(RARCH_SCALE_BASE * scale))
   {
      ELOG("Failed to initialize pixel converter.\n");
      goto error;
   }

   video.width        = width;
   video.height       = height;
   video.fullscreen   = settings->video.fullscreen;
   video.vsync        = settings->video.vsync && !runloop_ctl(RUNLOOP_CTL_IS_NONBLOCK_FORCED, NULL);
   video.force_aspect = settings->video.force_aspect;
#ifdef GEKKO
   video.viwidth      = settings->video.viwidth;
   video.vfilter      = settings->video.vfilter;
#endif
   video.smooth       = settings->video.smooth;
   video.input_scale  = scale;
   video.rgb32        = video_driver_state.filter.filter ?
      video_driver_state.filter.out_rgb32 :
      (video_driver_state.pix_fmt == RETRO_PIXEL_FORMAT_XRGB8888);

   /* Reset video frame count */
   video_driver_frame_count = 0;

   tmp = input_get_ptr();
   /* Need to grab the "real" video driver interface on a reinit. */
   video_driver_find_driver();

#ifdef HAVE_THREADS
   if (settings->video.threaded 
         && !video_driver_is_hw_context())
   {
      /* Can't do hardware rendering with threaded driver currently. */
      VLOG("Starting threaded video driver ...\n");

      if (!video_init_thread((const video_driver_t**)&current_video,
               &video_driver_data,
               input_get_double_ptr(), input_driver_get_data_ptr(),
               current_video, &video))
      {
         ELOG("Cannot open threaded video driver ... Exiting ...\n");
         goto error;
      }
   }
   else
#endif
      video_driver_data = current_video->init(&video, input_get_double_ptr(),
            input_driver_get_data_ptr());

   if (!video_driver_data)
   {
      ELOG("Cannot open video driver ... Exiting ...\n");
      goto error;
   }

   video_driver_poke = NULL;
   if (current_video->poke_interface)
      current_video->poke_interface(video_driver_data, &video_driver_poke);

   if (current_video->viewport_info && (!custom_vp->width ||
            !custom_vp->height))
   {
      /* Force custom viewport to have sane parameters. */
      custom_vp->width = width;
      custom_vp->height = height;

      video_driver_get_viewport_info(custom_vp);
   }

   video_driver_set_rotation(
            (settings->video.rotation + system->rotation) % 4);

   current_video->suppress_screensaver(video_driver_data,
         settings->ui.suspend_screensaver_enable);

   init_video_input(tmp);

   command_event(CMD_EVENT_OVERLAY_DEINIT, NULL);
   command_event(CMD_EVENT_OVERLAY_INIT, NULL);

   video_driver_cached_frame_set(&dummy_pixels, 4, 4, 8);

#if defined(PSP)
   video_driver_set_texture_frame(&dummy_pixels, false, 1, 1, 1.0f);
#endif

   return true;

error:
   retroarch_fail(1, "init_video()");
   return false;
}

bool video_driver_set_viewport(unsigned width, unsigned height,
      bool force_fullscreen, bool allow_rotate)
{
   if (!current_video || !current_video->set_viewport)
      return false;
   current_video->set_viewport(video_driver_data, width, height,
         force_fullscreen, allow_rotate);
   return true;
}

bool video_driver_set_rotation(unsigned rotation)
{
   if (!current_video || !current_video->set_rotation)
      return false;
   current_video->set_rotation(video_driver_data, rotation);
   return true;
}

bool video_driver_set_video_mode(unsigned width,
      unsigned height, bool fullscreen)
{
   gfx_ctx_mode_t mode;

   if (video_driver_poke && video_driver_poke->set_video_mode)
   {
      video_driver_poke->set_video_mode(video_driver_data,
            width, height, fullscreen);
      return true;
   }

   mode.width      = width;
   mode.height     = height;
   mode.fullscreen = fullscreen;

   return video_context_driver_set_video_mode(&mode);
}

bool video_driver_get_video_output_size(unsigned *width, unsigned *height)
{
   if (!video_driver_poke || !video_driver_poke->get_video_output_size)
      return false;
   video_driver_poke->get_video_output_size(video_driver_data,
         width, height);
   return true;
}

void video_driver_set_osd_msg(const char *msg,
      const struct font_params *params, void *font)
{
   if (video_driver_poke && video_driver_poke->set_osd_msg)
      video_driver_poke->set_osd_msg(video_driver_data, msg, params, font);
}

void video_driver_set_texture_enable(bool enable, bool fullscreen)
{
   if (video_driver_poke && video_driver_poke->set_texture_enable)
      video_driver_poke->set_texture_enable(video_driver_data,
            enable, fullscreen);
}

void video_driver_set_texture_frame(const void *frame, bool rgb32,
      unsigned width, unsigned height, float alpha)
{
#ifdef HAVE_MENU
   if (video_driver_poke && video_driver_poke->set_texture_frame)
      video_driver_poke->set_texture_frame(video_driver_data,
            frame, rgb32, width, height, alpha);
#endif
}

#ifdef HAVE_OVERLAY
bool video_driver_overlay_interface(const video_overlay_interface_t **iface)
{
   if (!current_video || !current_video->overlay_interface)
      return false;
   current_video->overlay_interface(video_driver_data, iface);
   return true;
}
#endif

void *video_driver_read_frame_raw(unsigned *width,
   unsigned *height, size_t *pitch)
{
   if (!current_video || !current_video->read_frame_raw)
      return NULL;
   return current_video->read_frame_raw(video_driver_data, width,
         height, pitch);
}

void video_driver_set_filtering(unsigned index, bool smooth)
{
   if (video_driver_poke && video_driver_poke->set_filtering)
      video_driver_poke->set_filtering(video_driver_data, index, smooth);
}

void video_driver_cached_frame_set(const void *data, unsigned width,
      unsigned height, size_t pitch)
{
   video_driver_set_cached_frame_ptr(data);
   video_driver_state.frame_cache.width  = width;
   video_driver_state.frame_cache.height = height;
   video_driver_state.frame_cache.pitch  = pitch;
}

void video_driver_cached_frame_get(const void **data, unsigned *width,
      unsigned *height, size_t *pitch)
{
   if (data)
      *data    = video_driver_state.frame_cache.data;
   if (width)
      *width  = video_driver_state.frame_cache.width;
   if (height)
      *height = video_driver_state.frame_cache.height;
   if (pitch)
      *pitch  = video_driver_state.frame_cache.pitch;
}

void video_driver_get_size(unsigned *width, unsigned *height)
{
   if (width)
      *width  = video_driver_state.video_width;
   if (height)
      *height = video_driver_state.video_height;
}

void video_driver_set_size(unsigned *width, unsigned *height)
{
   if (width)
      video_driver_state.video_width  = *width;
   if (height)
      video_driver_state.video_height = *height;
}

/**
 * video_monitor_set_refresh_rate:
 * @hz                 : New refresh rate for monitor.
 *
 * Sets monitor refresh rate to new value.
 **/
void video_monitor_set_refresh_rate(float hz)
{
   char msg[128];
   settings_t *settings = config_get_ptr();

   snprintf(msg, sizeof(msg),
         "Setting refresh rate to: %.3f Hz.", hz);
   runloop_msg_queue_push(msg, 1, 180, false);
   VLOG("%s\n", msg);

   settings->video.refresh_rate = hz;
}

/**
 * video_monitor_fps_statistics
 * @refresh_rate       : Monitor refresh rate.
 * @deviation          : Deviation from measured refresh rate.
 * @sample_points      : Amount of sampled points.
 *
 * Gets the monitor FPS statistics based on the current
 * runtime.
 *
 * Returns: true (1) on success.
 * false (0) if:
 * a) threaded video mode is enabled
 * b) less than 2 frame time samples.
 * c) FPS monitor enable is off.
 **/
bool video_monitor_fps_statistics(double *refresh_rate,
      double *deviation, unsigned *sample_points)
{
   unsigned i;
   retro_time_t accum   = 0, avg, accum_var = 0;
   settings_t *settings = config_get_ptr();
   unsigned samples      = MIN(MEASURE_FRAME_TIME_SAMPLES_COUNT,
         video_driver_state.frame_time.count);

   if (settings->video.threaded || (samples < 2))
      return false;

   /* Measure statistics on frame time (microsecs), *not* FPS. */
   for (i = 0; i < samples; i++)
      accum += video_driver_state.frame_time.samples[i];

#if 0
   for (i = 0; i < samples; i++)
      VLOG("Interval #%u: %d usec / frame.\n",
            i, (int)video_driver_state.frame_time.samples[i]);
#endif

   avg = accum / samples;

   /* Drop first measurement. It is likely to be bad. */
   for (i = 0; i < samples; i++)
   {
      retro_time_t diff = video_driver_state.frame_time.samples[i] - avg;
      accum_var += diff * diff;
   }

   *deviation     = sqrt((double)accum_var / (samples - 1)) / avg;
   *refresh_rate  = 1000000.0 / avg;
   *sample_points = samples;

   return true;
}


/**
 * video_monitor_get_fps:
 * @buf           : string suitable for Window title
 * @size          : size of buffer.
 * @buf_fps       : string of raw FPS only (optional).
 * @size_fps      : size of raw FPS buffer.
 *
 * Get the amount of frames per seconds.
 *
 * Returns: true if framerate per seconds could be obtained,
 * otherwise false.
 *
 **/
bool video_monitor_get_fps(char *buf, size_t size,
      char *buf_fps, size_t size_fps)
{
   static retro_time_t curr_time;
   static retro_time_t fps_time;
   retro_time_t        new_time  = cpu_features_get_time_usec();

   *buf = '\0';

   if (video_driver_frame_count)
   {
      static float last_fps;
      bool ret             = false;
      settings_t *settings = config_get_ptr();
      unsigned write_index = video_driver_state.frame_time.count++ &
         (MEASURE_FRAME_TIME_SAMPLES_COUNT - 1);

      video_driver_state.frame_time.samples[write_index] = new_time - fps_time;
      fps_time = new_time;

      if ((video_driver_frame_count % FPS_UPDATE_INTERVAL) == 0)
      {
         char frames_text[64];

         last_fps = TIME_TO_FPS(curr_time, new_time, FPS_UPDATE_INTERVAL);
         curr_time = new_time;

         fill_pathname_noext(buf,
               video_driver_title_buf,
               " || ",
               size);

         if (settings->fps_show)
         {
            char fps_text[64];
            snprintf(fps_text, sizeof(fps_text), " FPS: %6.1f || ", last_fps);
            strlcat(buf, fps_text, size);
         }

         strlcat(buf, "Frames: ", size);

         snprintf(frames_text, sizeof(frames_text), STRING_REP_UINT64,
               (unsigned long long)video_driver_frame_count);

         strlcat(buf, frames_text, size);
         ret = true;
      }

      if (buf_fps && settings->fps_show)
         snprintf(buf_fps, size_fps, "FPS: %6.1f || %s: " STRING_REP_UINT64,
               last_fps,
               msg_hash_to_str(MSG_FRAMES),
               (unsigned long long)video_driver_frame_count);

      return ret;
   }

   curr_time = fps_time = new_time;
   strlcpy(buf, video_driver_title_buf, size);
   if (buf_fps)
      strlcpy(buf_fps, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NOT_AVAILABLE), size_fps);

   return true;
}

float video_driver_get_aspect_ratio(void)
{
   return video_driver_state.aspect_ratio;
}

void video_driver_set_aspect_ratio_value(float value)
{
   video_driver_state.aspect_ratio = value;
}

static bool video_driver_frame_filter(const void *data,
      unsigned width, unsigned height,
      size_t pitch,
      unsigned *output_width, unsigned *output_height,
      unsigned *output_pitch)
{
   static struct retro_perf_counter softfilter_process = {0};
   settings_t *settings = config_get_ptr();

   performance_counter_init(&softfilter_process, "softfilter_process");

   if (!video_driver_state.filter.filter || !data)
      return false;

   rarch_softfilter_get_output_size(video_driver_state.filter.filter,
         output_width, output_height, width, height);

   *output_pitch = (*output_width) * video_driver_state.filter.out_bpp;

   performance_counter_start(&softfilter_process);
   rarch_softfilter_process(video_driver_state.filter.filter,
         video_driver_state.filter.buffer, *output_pitch,
         data, width, height, pitch);
   performance_counter_stop(&softfilter_process);

   if (settings->video.post_filter_record)
      recording_dump_frame(video_driver_state.filter.buffer,
            *output_width, *output_height, *output_pitch);

   return true;
}

rarch_softfilter_t *video_driver_frame_filter_get_ptr(void)
{
   return video_driver_state.filter.filter;
}

enum retro_pixel_format video_driver_get_pixel_format(void)
{
   return video_driver_state.pix_fmt;
}

void video_driver_set_pixel_format(enum retro_pixel_format fmt)
{
   video_driver_state.pix_fmt = fmt;
}

/**
 * video_driver_cached_frame:
 *
 * Renders the current video frame.
 **/
static bool video_driver_cached_frame(void)
{
   retro_ctx_frame_info_t info;
   void *recording  = recording_driver_get_data_ptr();

   if (runloop_ctl(RUNLOOP_CTL_IS_IDLE, NULL))
      return false; /* Maybe return false here for indication of idleness? */

   /* Cannot allow recording when pushing duped frames. */
   recording_driver_clear_data_ptr();

   /* Not 100% safe, since the library might have
    * freed the memory, but no known implementations do this.
    * It would be really stupid at any rate ...
    */
   info.data        = NULL;
   info.width       = video_driver_state.frame_cache.width;
   info.height      = video_driver_state.frame_cache.height;
   info.pitch       = video_driver_state.frame_cache.pitch;

   if (video_driver_state.frame_cache.data != RETRO_HW_FRAME_BUFFER_VALID)
      info.data = video_driver_state.frame_cache.data;

   core_frame(&info);

   recording_driver_set_data_ptr(recording);

   return true;
}

void video_driver_monitor_adjust_system_rates(void)
{
   float timing_skew;
   const struct retro_system_timing *info = NULL;
   struct retro_system_av_info *av_info   =
      video_viewport_get_system_av_info();
   settings_t *settings                   = config_get_ptr();

   runloop_ctl(RUNLOOP_CTL_UNSET_NONBLOCK_FORCED, NULL);

   if  (av_info)
      info = (const struct retro_system_timing*)&av_info->timing;

   if (!info || info->fps <= 0.0)
      return;

   timing_skew = fabs(1.0f - info->fps / settings->video.refresh_rate);

   /* We don't want to adjust pitch too much. If we have extreme cases,
    * just don't readjust at all. */
   if (timing_skew <= settings->audio.max_timing_skew)
      return;

   VLOG("Timings deviate too much. Will not adjust. (Display = %.2f Hz, Game = %.2f Hz)\n",
         settings->video.refresh_rate,
         (float)info->fps);

   if (info->fps <= settings->video.refresh_rate)
      return;

   /* We won't be able to do VSync reliably when game FPS > monitor FPS. */
   runloop_ctl(RUNLOOP_CTL_SET_NONBLOCK_FORCED, NULL);
   VLOG("Game FPS > Monitor FPS. Cannot rely on VSync.\n");
}

void video_driver_menu_settings(void **list_data, void *list_info_data,
      void *group_data, void *subgroup_data, const char *parent_group)
{
#ifdef HAVE_MENU
   rarch_setting_t **list                    = (rarch_setting_t**)list_data;
   rarch_setting_info_t *list_info           = (rarch_setting_info_t*)list_info_data;
   rarch_setting_group_info_t *group_info    = (rarch_setting_group_info_t*)group_data;
   rarch_setting_group_info_t *subgroup_info = (rarch_setting_group_info_t*)subgroup_data;
   global_t                        *global   = global_get_ptr();

   (void)list;
   (void)list_info;
   (void)group_info;
   (void)subgroup_info;
   (void)global;

#if defined(GEKKO) || defined(__CELLOS_LV2__)
   CONFIG_ACTION(
         list, list_info,
         msg_hash_to_str(MENU_ENUM_LABEL_SCREEN_RESOLUTION),
         msg_hash_to_str(MENU_ENUM_LABEL_VALUE_SCREEN_RESOLUTION),
         group_info,
         subgroup_info,
         parent_group);
   menu_settings_list_current_add_enum_idx(list, list_info, MENU_ENUM_LABEL_SCREEN_RESOLUTION);
#endif
#if defined(__CELLOS_LV2__)
   CONFIG_BOOL(
         list, list_info,
         &global->console.screen.pal60_enable,
         msg_hash_to_str(MENU_ENUM_LABEL_PAL60_ENABLE),
         msg_hash_to_str(MENU_ENUM_LABEL_VALUE_PAL60_ENABLE),
         false,
         msg_hash_to_str(MENU_ENUM_LABEL_VALUE_OFF),
         msg_hash_to_str(MENU_ENUM_LABEL_VALUE_ON),
         group_info,
         subgroup_info,
         parent_group,
         general_write_handler,
         general_read_handler,
         SD_FLAG_NONE);
   menu_settings_list_current_add_enum_idx(list, list_info, MENU_ENUM_LABEL_PAL60_ENABLE);
#endif
#if defined(GEKKO) || defined(_XBOX360)
   CONFIG_UINT(
         list, list_info,
         &global->console.screen.gamma_correction,
         msg_hash_to_str(MENU_ENUM_LABEL_VIDEO_GAMMA),
         msg_hash_to_str(MENU_ENUM_LABEL_VALUE_VIDEO_GAMMA),
         0,
         group_info,
         subgroup_info,
         parent_group,
         general_write_handler,
         general_read_handler);
   menu_settings_list_current_add_cmd(
         list,
         list_info,
         CMD_EVENT_VIDEO_APPLY_STATE_CHANGES);
   menu_settings_list_current_add_range(
         list,
         list_info,
         0,
         MAX_GAMMA_SETTING,
         1,
         true,
         true);
   settings_data_list_current_add_flags(list, list_info,
         SD_FLAG_CMD_APPLY_AUTO|SD_FLAG_ADVANCED);
   menu_settings_list_current_add_enum_idx(list, list_info, MENU_ENUM_LABEL_VIDEO_GAMMA);
#endif
#if defined(_XBOX1) || defined(HW_RVL)
   CONFIG_BOOL(
         list, list_info,
         &global->console.softfilter_enable,
         msg_hash_to_str(MENU_ENUM_LABEL_VIDEO_SOFT_FILTER),
         msg_hash_to_str(MENU_ENUM_LABEL_VALUE_VIDEO_SOFT_FILTER),
         false,
         msg_hash_to_str(MENU_ENUM_LABEL_VALUE_OFF),
         msg_hash_to_str(MENU_ENUM_LABEL_VALUE_ON),
         group_info,
         subgroup_info,
         parent_group,
         general_write_handler,
         general_read_handler,
         SD_FLAG_NONE);
   menu_settings_list_current_add_cmd(
         list,
         list_info,
         CMD_EVENT_VIDEO_APPLY_STATE_CHANGES);
   menu_settings_list_current_add_enum_idx(list, list_info, MENU_ENUM_LABEL_VIDEO_SOFT_FILTER);
#endif
#ifdef _XBOX1
   CONFIG_UINT(
         list, list_info,
         &settings->video.swap_interval,
         msg_hash_to_str(MENU_ENUM_LABEL_VIDEO_FILTER_FLICKER),
         msg_hash_to_str(MENU_ENUM_LABEL_VALUE_VIDEO_FILTER_FLICKER),
         0,
         group_info,
         subgroup_info,
         parent_group,
         general_write_handler,
         general_read_handler);
   menu_settings_list_current_add_range(list, list_info, 0, 5, 1, true, true);
   menu_settings_list_current_add_enum_idx(list, list_info, MENU_ENUM_LABEL_VIDEO_FILTER_FLICKER);
#endif
#endif
}

/* Graphics driver requires RGBA byte order data (ABGR on little-endian)
 * for 32-bit.
 * This takes effect for overlay and shader cores that wants to load
 * data into graphics driver. Kinda hackish to place it here, it is only
 * used for GLES.
 * TODO: Refactor this better. */
static struct retro_hw_render_callback hw_render;
static const struct retro_hw_render_context_negotiation_interface *hw_render_context_negotiation;

static bool video_driver_use_rgba                = false;
static bool video_driver_data_own                = false;
static bool video_driver_active                  = false;
static video_driver_frame_t frame_bak            = NULL;
/* If set during context deinit, the driver should keep
 * graphics context alive to avoid having to reset all 
 * context state. */
static bool video_driver_cache_context           = false;
/* Set to true by driver if context caching succeeded. */
static bool video_driver_cache_context_ack       = false;
static uint8_t *video_driver_record_gpu_buffer   = NULL;
#ifdef HAVE_THREADS
static slock_t *display_lock                     = NULL;
#endif

void video_driver_lock(void)
{
#ifdef HAVE_THREADS
   if (!display_lock)
      return;
   slock_lock(display_lock);
#endif
}

void video_driver_unlock(void)
{
#ifdef HAVE_THREADS
   if (!display_lock)
      return;
   slock_unlock(display_lock);
#endif
}

void video_driver_lock_free(void)
{
#ifdef HAVE_THREADS
   slock_free(display_lock);
   display_lock = NULL;
#endif
}

void video_driver_lock_new(void)
{
   video_driver_lock_free();
#ifdef HAVE_THREADS
   if (!display_lock)
      display_lock = slock_new();
   retro_assert(display_lock);
#endif
}

void video_driver_destroy(void)
{
   video_driver_use_rgba          = false;
   video_driver_data_own          = false;
   video_driver_active            = false;
   video_driver_cache_context     = false;
   video_driver_cache_context_ack = false;
   video_driver_record_gpu_buffer = NULL;
   current_video                  = NULL;
}

void video_driver_set_cached_frame_ptr(const void *data)
{
   if (data)
      video_driver_state.frame_cache.data = data;
}

void video_driver_set_stub_frame(void)
{
   frame_bak            = current_video->frame;
   current_video->frame = video_null.frame;
}

void video_driver_unset_stub_frame(void)
{
   if (frame_bak != NULL)
      current_video->frame = frame_bak;

   frame_bak = NULL;
}

bool video_driver_supports_recording(void)
{
   settings_t *settings = config_get_ptr();
   return settings->video.gpu_record && current_video->read_viewport;
}

bool video_driver_supports_viewport_read(void)
{
   settings_t *settings = config_get_ptr();
   return (settings->video.gpu_screenshot ||
         (video_driver_is_hw_context() && !current_video->read_frame_raw))
      && current_video->read_viewport && current_video->viewport_info;
}

bool video_driver_supports_read_frame_raw(void)
{
   return current_video->read_frame_raw;
}

void video_driver_set_viewport_config(void)
{
   settings_t *settings = config_get_ptr();
   struct retro_system_av_info *av_info = video_viewport_get_system_av_info();

   if (settings->video.aspect_ratio < 0.0f)
   {
      struct retro_game_geometry *geom = &av_info->geometry;

      if (!geom)
         return;

      if (geom->aspect_ratio > 0.0f && settings->video.aspect_ratio_auto)
         aspectratio_lut[ASPECT_RATIO_CONFIG].value = geom->aspect_ratio;
      else
      {
         unsigned base_width  = geom->base_width;
         unsigned base_height = geom->base_height;

         /* Get around division by zero errors */
         if (base_width == 0)
            base_width = 1;
         if (base_height == 0)
            base_height = 1;
         aspectratio_lut[ASPECT_RATIO_CONFIG].value = 
            (float)base_width / base_height; /* 1:1 PAR. */
      }
   }
   else
   {
      aspectratio_lut[ASPECT_RATIO_CONFIG].value = 
         settings->video.aspect_ratio;
   }
}

void video_driver_set_viewport_square_pixel(void)
{
   unsigned len, highest, i, aspect_x, aspect_y;
   unsigned width, height;
   struct retro_game_geometry *geom     = NULL;
   struct retro_system_av_info *av_info = 
      video_viewport_get_system_av_info();

   if (av_info)
      geom = &av_info->geometry;

   if (!geom)
      return;

   width  = geom->base_width;
   height = geom->base_height;

   if (width == 0 || height == 0)
      return;

   len      = MIN(width, height);
   highest  = 1;

   for (i = 1; i < len; i++)
   {
      if ((width % i) == 0 && (height % i) == 0)
         highest = i;
   }

   aspect_x = width / highest;
   aspect_y = height / highest;

   snprintf(aspectratio_lut[ASPECT_RATIO_SQUARE].name,
         sizeof(aspectratio_lut[ASPECT_RATIO_SQUARE].name),
         "%u:%u (1:1 PAR)", aspect_x, aspect_y);

   aspectratio_lut[ASPECT_RATIO_SQUARE].value = (float)aspect_x / aspect_y;
}

void video_driver_set_viewport_core(void)
{
   struct retro_system_av_info *av_info = 
      video_viewport_get_system_av_info();
   struct retro_game_geometry *geom = &av_info->geometry;

   if (!geom || geom->base_width <= 0.0f || geom->base_height <= 0.0f)
      return;

   /* Fallback to 1:1 pixel ratio if none provided */
   if (geom->aspect_ratio > 0.0f)
   {
      aspectratio_lut[ASPECT_RATIO_CORE].value = geom->aspect_ratio;
   }
   else
   {
      aspectratio_lut[ASPECT_RATIO_CORE].value = 
         (float)geom->base_width / geom->base_height;
   }
}

void video_driver_reset_custom_viewport(void)
{
   struct video_viewport *custom_vp = video_viewport_get_custom();
   if (!custom_vp)
      return;

   custom_vp->width  = 0;
   custom_vp->height = 0;
   custom_vp->x      = 0;
   custom_vp->y      = 0;
}

void video_driver_set_rgba(void)
{
   video_driver_lock();
   video_driver_use_rgba = true;
   image_texture_set_rgba();
   video_driver_unlock();
}

void video_driver_unset_rgba(void)
{
   video_driver_lock();
   video_driver_use_rgba = false;
   image_texture_unset_rgba();
   video_driver_unlock();
}

bool video_driver_supports_rgba(void)
{
   bool tmp;
   video_driver_lock();
   tmp = video_driver_use_rgba;
   video_driver_unlock();
   return tmp;
}

bool video_driver_get_next_video_out(void)
{
   if (!video_driver_poke)
      return false;

   if (!video_driver_poke->get_video_output_next)
      return video_context_driver_get_video_output_next();
   video_driver_poke->get_video_output_next(video_driver_data);
   return true;
}

bool video_driver_get_prev_video_out(void)
{
   if (!video_driver_poke)
      return false;

   if (!video_driver_poke->get_video_output_prev)
      return video_context_driver_get_video_output_prev();
   video_driver_poke->get_video_output_prev(video_driver_data);
   return true;
}

bool video_driver_init(void)
{
   video_driver_lock_new();
   return init_video();
}

void video_driver_destroy_data(void)
{
   video_driver_data = NULL;
}

void video_driver_deinit(void)
{
   uninit_video_input();
   video_driver_lock_free();
   video_driver_data = NULL;
}

void video_driver_monitor_reset(void)
{
   video_driver_state.frame_time.count = 0;
}

void video_driver_set_aspect_ratio(void)
{
   settings_t *settings = config_get_ptr();
   if (!video_driver_poke || !video_driver_poke->set_aspect_ratio)
      return;
   video_driver_poke->set_aspect_ratio(
         video_driver_data, settings->video.aspect_ratio_idx);
}

void video_driver_show_mouse(void)
{
   if (!video_driver_poke)
      return;
   if (video_driver_poke->show_mouse)
      video_driver_poke->show_mouse(video_driver_data, true);
}

void video_driver_hide_mouse(void)
{
   if (!video_driver_poke)
      return;
   if (video_driver_poke->show_mouse)
      video_driver_poke->show_mouse(video_driver_data, false);
}

void video_driver_set_nonblock_state(bool toggle)
{
   if (current_video->set_nonblock_state)
      current_video->set_nonblock_state(video_driver_data, toggle);
}

bool video_driver_find_driver(void)
{
   settings_t *settings = config_get_ptr();
   int i;
   driver_ctx_info_t drv;

   if (video_driver_is_hw_context())
   {
      struct retro_hw_render_callback *hwr = video_driver_get_hw_context();

      current_video                        = NULL;

      if (hwr && hw_render_context_is_vulkan(hwr->context_type))
      {
#if defined(HAVE_VULKAN)
         VLOG("Using HW render, Vulkan driver forced.\n");
         current_video = &video_vulkan;
#endif
      }

      if (hwr && hw_render_context_is_gl(hwr->context_type))
      {
#if defined(HAVE_OPENGL) && defined(HAVE_FBO)
         VLOG("Using HW render, OpenGL driver forced.\n");
         current_video = &video_gl;
#endif
      }

      if (current_video)
         return true;
   }

   if (frontend_driver_has_get_video_driver_func())
   {
      current_video = (video_driver_t*)frontend_driver_get_video_driver();

      if (current_video)
         return true;
      WLOG("Frontend supports get_video_driver() but did not specify one.\n");
   }

   drv.label = "video_driver";
   drv.s     = settings->video.driver;

   driver_ctl(RARCH_DRIVER_CTL_FIND_INDEX, &drv);

   i = drv.len;

   if (i >= 0)
      current_video = (video_driver_t*)video_driver_find_handle(i);
   else
   {
      unsigned d;
      ELOG("Couldn't find any video driver named \"%s\"\n",
            settings->video.driver);
      RARCH_LOG_OUTPUT("Available video drivers are:\n");
      for (d = 0; video_driver_find_handle(d); d++)
         RARCH_LOG_OUTPUT("\t%s\n", video_driver_find_ident(d));
      WLOG("Going to default to first video driver...\n");

      current_video = (video_driver_t*)video_driver_find_handle(0);

      if (!current_video)
         retroarch_fail(1, "find_video_driver()");
   }
   return true;
}

void video_driver_apply_state_changes(void)
{
   if (!video_driver_poke)
      return;
   if (video_driver_poke->apply_state_changes)
      video_driver_poke->apply_state_changes(video_driver_data);
}

bool video_driver_read_viewport(uint8_t *buffer)
{
   if (!current_video->read_viewport)
      return false;
   if (!current_video->read_viewport(video_driver_data, buffer))
      return false;

   return true;
}

bool video_driver_cached_frame_has_valid_framebuffer(void)
{
   if (!video_driver_state.frame_cache.data)
      return false;
   return video_driver_state.frame_cache.data == RETRO_HW_FRAME_BUFFER_VALID;
}

bool video_driver_cached_frame_render(void)
{
   if (!current_video)
      return false;
   return video_driver_cached_frame();
}

bool video_driver_is_alive(void)
{
   if (current_video)
      return current_video->alive(video_driver_data);
   else
      return true;
}

bool video_driver_is_focused(void)
{
   return current_video->focus(video_driver_data);
}

bool video_driver_has_windowed(void)
{
#if defined(RARCH_CONSOLE) || defined(RARCH_MOBILE)
   return false;
#else
   return current_video->has_windowed(video_driver_data);
#endif
}

uint64_t *video_driver_get_frame_count_ptr(void)
{
   return &video_driver_frame_count;
}

bool video_driver_frame_filter_alive(void)
{
   return !!video_driver_state.filter.filter;
}

bool video_driver_frame_filter_is_32bit(void)
{
   return video_driver_state.filter.out_rgb32;
}

void video_driver_default_settings(void)
{
   global_t *global    = global_get_ptr();

   if (!global)
      return;

   global->console.screen.gamma_correction       = DEFAULT_GAMMA;
   global->console.flickerfilter_enable          = false;
   global->console.softfilter_enable             = false;

   global->console.screen.resolutions.current.id = 0;
}

void video_driver_load_settings(config_file_t *conf)
{
   bool tmp_bool    = false;
   global_t *global = global_get_ptr();

   if (!conf)
      return;

   CONFIG_GET_BOOL_BASE(conf, global,
         console.screen.gamma_correction, "gamma_correction");

   if (config_get_bool(conf, "flicker_filter_enable",
         &tmp_bool))
      global->console.flickerfilter_enable = tmp_bool;

   if (config_get_bool(conf, "soft_filter_enable",
         &tmp_bool))
      global->console.softfilter_enable = tmp_bool;

   CONFIG_GET_INT_BASE(conf, global,
         console.screen.soft_filter_index,
         "soft_filter_index");
   CONFIG_GET_INT_BASE(conf, global, 
         console.screen.resolutions.current.id,
         "current_resolution_id");
   CONFIG_GET_INT_BASE(conf, global, 
         console.screen.flicker_filter_index,
         "flicker_filter_index");
}

void video_driver_save_settings(config_file_t *conf)
{
   global_t *global = global_get_ptr();
   if (!conf)
      return;

   config_set_bool(conf, "gamma_correction",
         global->console.screen.gamma_correction);
   config_set_bool(conf, "flicker_filter_enable",
         global->console.flickerfilter_enable);
   config_set_bool(conf, "soft_filter_enable",
         global->console.softfilter_enable);

   config_set_int(conf, "soft_filter_index",
         global->console.screen.soft_filter_index);
   config_set_int(conf, "current_resolution_id",
         global->console.screen.resolutions.current.id);
   config_set_int(conf, "flicker_filter_index",
         global->console.screen.flicker_filter_index);
}

void video_driver_set_own_driver(void)
{
   video_driver_data_own = true;
}

void video_driver_unset_own_driver(void)
{
   video_driver_data_own = false;
}

bool video_driver_owns_driver(void)
{
   return video_driver_data_own;
}

bool video_driver_is_hw_context(void)
{
   return hw_render.context_type != RETRO_HW_CONTEXT_NONE;
}

void video_driver_deinit_hw_context(void)
{
   if (hw_render.context_destroy)
      hw_render.context_destroy();

   memset(&hw_render, 0, sizeof(hw_render));
   hw_render_context_negotiation = NULL;
}

struct retro_hw_render_callback *video_driver_get_hw_context(void)
{
   return &hw_render;
}

const struct retro_hw_render_context_negotiation_interface *video_driver_get_context_negotiation_interface(void)
{
   return hw_render_context_negotiation;
}

void video_driver_set_context_negotiation_interface(const struct retro_hw_render_context_negotiation_interface *iface)
{
   hw_render_context_negotiation = iface;
}

void video_driver_set_video_cache_context(void)
{
   video_driver_cache_context = true;
}

void video_driver_unset_video_cache_context(void)
{
   video_driver_cache_context = false;
}

bool video_driver_is_video_cache_context(void)
{
   return video_driver_cache_context;
}

void video_driver_set_video_cache_context_ack(void)
{
   video_driver_cache_context_ack = true;
}

void video_driver_unset_video_cache_context_ack(void)
{
   video_driver_cache_context_ack = false;
}

bool video_driver_is_video_cache_context_ack(void)
{
   return video_driver_cache_context_ack;
}

void video_driver_set_active(void)
{
   video_driver_active = true;
}

void video_driver_unset_active(void)
{
   video_driver_active = false;
}

bool video_driver_is_active(void)
{
   return video_driver_active;
}

bool video_driver_has_gpu_record(void)
{
   return video_driver_record_gpu_buffer != NULL;
}

uint8_t *video_driver_get_gpu_record(void)
{
   return video_driver_record_gpu_buffer;
}

bool video_driver_gpu_record_init(unsigned size)
{
   video_driver_record_gpu_buffer = (uint8_t*)malloc(size);
   if (!video_driver_record_gpu_buffer)
      return false;
   return true;
}

void video_driver_gpu_record_deinit(void)
{
   free(video_driver_record_gpu_buffer);
   video_driver_record_gpu_buffer = NULL;
}

bool video_driver_get_current_software_framebuffer(struct retro_framebuffer *fb)
{
   if (
         !video_driver_poke || 
         !video_driver_poke->get_current_software_framebuffer)
      return false;
   if (!video_driver_poke->get_current_software_framebuffer(
            video_driver_data, fb))
      return false;

   return true;
}

bool video_driver_get_hw_render_interface(const struct retro_hw_render_interface **iface)
{
   if (
         !video_driver_poke || 
         !video_driver_poke->get_hw_render_interface)
      return false;

   if (!video_driver_poke->get_hw_render_interface(video_driver_data, iface))
      return false;

   return true;
}

bool video_driver_get_viewport_info(struct video_viewport *viewport)
{
   if (!current_video || !current_video->viewport_info)
      return false;
   current_video->viewport_info(video_driver_data, viewport);
   return true;
}

void video_driver_set_title_buf(void)
{
   struct retro_system_info info;
   core_get_system_info(&info);

   fill_pathname_noext(video_driver_title_buf, 
         msg_hash_to_str(MSG_PROGRAM),
         " ",
         sizeof(video_driver_title_buf));
   strlcat(video_driver_title_buf, 
         info.library_name,
         sizeof(video_driver_title_buf));
   strlcat(video_driver_title_buf,
         " ", sizeof(video_driver_title_buf));
   strlcat(video_driver_title_buf,
         info.library_version,
         sizeof(video_driver_title_buf));
}

/**
 * video_viewport_get_scaled_integer:
 * @vp            : Viewport handle
 * @width         : Width.
 * @height        : Height.
 * @aspect_ratio  : Aspect ratio (in float).
 * @keep_aspect   : Preserve aspect ratio?
 *
 * Gets viewport scaling dimensions based on 
 * scaled integer aspect ratio.
 **/
void video_viewport_get_scaled_integer(struct video_viewport *vp,
      unsigned width, unsigned height,
      float aspect_ratio, bool keep_aspect)
{
   int padding_x        = 0;
   int padding_y        = 0;
   settings_t *settings = config_get_ptr();

   if (!vp)
      return;

   if (settings->video.aspect_ratio_idx == ASPECT_RATIO_CUSTOM)
   {
      struct video_viewport *custom = video_viewport_get_custom();

      if (custom)
      {
         padding_x = width - custom->width;
         padding_y = height - custom->height;
         width     = custom->width;
         height    = custom->height;
      }
   }
   else
   {
      unsigned base_width;
      /* Use system reported sizes as these define the 
       * geometry for the "normal" case. */
      struct retro_system_av_info *av_info = 
         video_viewport_get_system_av_info();
      unsigned base_height = 0;
      
      if (av_info)
         base_height = av_info->geometry.base_height;

      if (base_height == 0)
         base_height = 1;

      /* Account for non-square pixels.
       * This is sort of contradictory with the goal of integer scale,
       * but it is desirable in some cases.
       *
       * If square pixels are used, base_height will be equal to 
       * system->av_info.base_height. */
      base_width = (unsigned)roundf(base_height * aspect_ratio);

      /* Make sure that we don't get 0x scale ... */
      if (width >= base_width && height >= base_height)
      {
         if (keep_aspect)
         {
            /* X/Y scale must be same. */
            unsigned max_scale = MIN(width / base_width,
                  height / base_height);
            padding_x          = width - base_width * max_scale;
            padding_y          = height - base_height * max_scale;
         }
         else
         {
            /* X/Y can be independent, each scaled as much as possible. */
            padding_x = width % base_width;
            padding_y = height % base_height;
         }
      }

      width     -= padding_x;
      height    -= padding_y;
   }

   vp->width  = width;
   vp->height = height;
   vp->x      = padding_x / 2;
   vp->y      = padding_y / 2;
}

struct retro_system_av_info *video_viewport_get_system_av_info(void)
{
   static struct retro_system_av_info av_info;

   return &av_info;
}

struct video_viewport *video_viewport_get_custom(void)
{
   settings_t *settings = config_get_ptr();
   return &settings->video_viewport_custom;
}

unsigned video_pixel_get_alignment(unsigned pitch)
{
   if (pitch & 1)
      return 1;
   if (pitch & 2)
      return 2;
   if (pitch & 4)
      return 4;
   return 8;
}

static bool video_pixel_frame_scale(const void *data,
      unsigned width, unsigned height,
      size_t pitch)
{
   static struct retro_perf_counter video_frame_conv = {0};

   performance_counter_init(&video_frame_conv, "video_frame_conv");

   if (     !data 
         || video_driver_get_pixel_format() != RETRO_PIXEL_FORMAT_0RGB1555)
      return false;
   if (data == RETRO_HW_FRAME_BUFFER_VALID)
      return false;

   performance_counter_start(&video_frame_conv);

   video_driver_scaler_ptr->scaler->in_width      = width;
   video_driver_scaler_ptr->scaler->in_height     = height;
   video_driver_scaler_ptr->scaler->out_width     = width;
   video_driver_scaler_ptr->scaler->out_height    = height;
   video_driver_scaler_ptr->scaler->in_stride     = pitch;
   video_driver_scaler_ptr->scaler->out_stride    = width * sizeof(uint16_t);

   scaler_ctx_scale(video_driver_scaler_ptr->scaler,
         video_driver_scaler_ptr->scaler_out, data);

   performance_counter_stop(&video_frame_conv);

   return true;
}

/**
 * video_driver_frame:
 * @data                 : pointer to data of the video frame.
 * @width                : width of the video frame.
 * @height               : height of the video frame.
 * @pitch                : pitch of the video frame.
 *
 * Video frame render callback function.
 **/
void video_driver_frame(const void *data, unsigned width,
      unsigned height, size_t pitch)
{
   static char video_driver_msg[256];
   unsigned output_width  = 0;
   unsigned output_height = 0;
   unsigned  output_pitch = 0;
   const char *msg        = NULL;
   settings_t *settings   = config_get_ptr();

   runloop_ctl(RUNLOOP_CTL_MSG_QUEUE_PULL,   &msg);

   if (!video_driver_is_active())
      return;

   if (video_driver_scaler_ptr &&
         video_pixel_frame_scale(data, width, height, pitch))
   {
      data                = video_driver_scaler_ptr->scaler_out;
      pitch               = video_driver_scaler_ptr->scaler->out_stride;
   }

   video_driver_cached_frame_set(data, width, height, pitch);

   /* Slightly messy code,
    * but we really need to do processing before blocking on VSync
    * for best possible scheduling.
    */
   if (
         (
             !video_driver_state.filter.filter
          || !settings->video.post_filter_record 
          || !data
          || video_driver_has_gpu_record()
         )
      )
      recording_dump_frame(data, width, height, pitch);

   if (video_driver_frame_filter(data, width, height, pitch,
            &output_width, &output_height, &output_pitch))
   {
      data   = video_driver_state.filter.buffer;
      width  = output_width;
      height = output_height;
      pitch  = output_pitch;
   }

   video_driver_msg[0] = '\0';
   if (msg)
      strlcpy(video_driver_msg, msg, sizeof(video_driver_msg));

   if (!current_video || !current_video->frame(
            video_driver_data, data, width, height,
            video_driver_frame_count,
            pitch, video_driver_msg))
   {
      video_driver_unset_active();
   }

   video_driver_frame_count++;
}

void video_driver_display_type_set(enum rarch_display_type type)
{
   video_driver_display_type = type;
}

uintptr_t video_driver_display_get(void)
{
   return video_driver_display;
}

void video_driver_display_set(uintptr_t idx)
{
   video_driver_display = idx;
}

enum rarch_display_type video_driver_display_type_get(void)
{
   return video_driver_display_type;
}

void video_driver_window_set(uintptr_t idx)
{
   video_driver_window = idx;
}

uintptr_t video_driver_window_get(void)
{
   return video_driver_window;
}

bool video_driver_texture_load(void *data,
      enum texture_filter_type  filter_type,
      uintptr_t *id)
{
#ifdef HAVE_THREADS
   settings_t *settings = config_get_ptr();
#endif

   if (!id || !video_driver_poke || !video_driver_poke->load_texture)
      return false;

   *id = video_driver_poke->load_texture(video_driver_data, data,
#ifdef HAVE_THREADS
         settings->video.threaded
         && !video_driver_is_hw_context(),
#else
         false,
#endif
         filter_type);

   return true;
}

bool video_driver_texture_unload(uintptr_t *id)
{
   if (!video_driver_poke || !video_driver_poke->unload_texture)
      return false;

   video_driver_poke->unload_texture(video_driver_data, *id);
   *id = 0;
   return true;
}
