/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2016 - Daniel De Matteis
 *  Copyright (C) 2012-2015 - Michael Lelli
 *  Copyright (C) 2014-2015 - Jay McCarthy
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

#include <stdarg.h>
#include <math.h>

#include <lists/dir_list.h>
#include <file/file_path.h>
#include <retro_inline.h>
#include <retro_assert.h>
#include <queues/message_queue.h>
#ifdef HAVE_THREADS
#include <rthreads/rthreads.h>
#endif
#include <queues/task_queue.h>
#include <string/stdstring.h>
#include <features/features_cpu.h>

#include <compat/strl.h>

#ifdef HAVE_CHEEVOS
#include "cheevos.h"
#endif
#include "autosave.h"
#include "core_info.h"
#include "configuration.h"
#include "movie.h"
#include "retroarch.h"
#include "runloop.h"
#include "file_path_special.h"
#include "managers/core_option_manager.h"
#include "managers/cheat_manager.h"
#include "managers/state_manager.h"
#include "system.h"
#include "list_special.h"
#include "audio/audio_driver.h"
#include "camera/camera_driver.h"
#include "record/record_driver.h"
#include "input/input_driver.h"
#include "ui/ui_companion_driver.h"
#include "core.h"

#include "msg_hash.h"

#include "input/input_keyboard.h"
#include "tasks/tasks_internal.h"

#ifdef HAVE_MENU
#include "menu/menu_driver.h"
#endif

#ifdef HAVE_NETPLAY
#include "network/netplay/netplay.h"
#endif

#if defined(HAVE_HTTPSERVER) && defined(HAVE_ZLIB)
#include "httpserver/httpserver.h"
#endif

#include "verbosity.h"

#ifdef HAVE_ZLIB
#define DEFAULT_EXT "zip"
#else
#define DEFAULT_EXT ""
#endif

#define runloop_cmd_triggered(cmd, id) BIT64_GET(cmd->state[2].state, id) 

#define runloop_cmd_press(cmd, id)     BIT64_GET(cmd->state[0].state, id)
#define runloop_cmd_pressed(cmd, id)   BIT64_GET(cmd->state[1].state, id)
#ifdef HAVE_MENU
#define runloop_cmd_menu_press(cmd)   (BIT64_GET(cmd->state[2].state, RARCH_MENU_TOGGLE) || \
                                      runloop_cmd_get_state_menu_toggle_button_combo( \
                                            settings, cmd->state[0], \
                                            cmd->state[1], cmd->state[2]))
#endif

struct rarch_dir_list
{
   struct string_list *list;
   size_t ptr;
};

typedef struct event_cmd_state
{
   retro_input_t state[3];
} event_cmd_state_t;

static rarch_dir_list_t runloop_shader_dir;
static char runloop_fullpath[PATH_MAX_LENGTH];
static char runloop_default_shader_preset[PATH_MAX_LENGTH];
static rarch_system_info_t runloop_system;
static unsigned runloop_pending_windowed_scale;
static struct retro_frame_time_callback runloop_frame_time;
static retro_keyboard_event_t runloop_key_event          = NULL;
static retro_keyboard_event_t runloop_frontend_key_event = NULL;
static retro_usec_t runloop_frame_time_last      = 0;
static unsigned runloop_max_frames               = false;
static bool runloop_force_nonblock               = false;
static bool runloop_frame_time_last_enable       = false;
static bool runloop_set_frame_limit              = false;
static bool runloop_paused                       = false;
static bool runloop_idle                         = false;
static bool runloop_exec                         = false;
static bool runloop_slowmotion                   = false;
static bool runloop_shutdown_initiated           = false;
static bool runloop_core_shutdown_initiated      = false;
static bool runloop_perfcnt_enable               = false;
static bool runloop_overrides_active             = false;
static bool runloop_game_options_active          = false;
static core_option_manager_t *runloop_core_options = NULL;
#ifdef HAVE_THREADS
static slock_t *_runloop_msg_queue_lock           = NULL;
#endif
static msg_queue_t *runloop_msg_queue            = NULL;

global_t *global_get_ptr(void)
{
   static struct global g_extern;
   return &g_extern;
}

static void runloop_msg_queue_lock(void)
{
#ifdef HAVE_THREADS
   slock_lock(_runloop_msg_queue_lock);
#endif
}

static void runloop_msg_queue_unlock(void)
{
#ifdef HAVE_THREADS
   slock_unlock(_runloop_msg_queue_lock);
#endif
}

static bool runloop_msg_queue_push_internal(runloop_ctx_msg_info_t *msg_info)
{
   if (!msg_info || !runloop_msg_queue)
      return false;
   msg_queue_push(runloop_msg_queue, msg_info->msg,
         msg_info->prio, msg_info->duration);

   if (ui_companion_is_on_foreground())
   {
      const ui_companion_driver_t *ui = ui_companion_get_ptr();
      if (ui->msg_queue_push)
         ui->msg_queue_push(msg_info->msg,
               msg_info->prio, msg_info->duration, msg_info->flush);
   }
   return true;
}

void runloop_msg_queue_push(const char *msg,
      unsigned prio, unsigned duration,
      bool flush)
{
   runloop_ctx_msg_info_t msg_info;
   settings_t *settings = config_get_ptr();
   if(!settings->video.font_enable)
      return;

   runloop_msg_queue_lock();

   if (flush)
      runloop_ctl(RUNLOOP_CTL_MSG_QUEUE_CLEAR, NULL);

   msg_info.msg      = msg;
   msg_info.prio     = prio;
   msg_info.duration = duration;
   msg_info.flush    = flush;

   runloop_msg_queue_push_internal(&msg_info);

   runloop_msg_queue_unlock();
}

char* runloop_msg_queue_pull(void)
{
   runloop_ctx_msg_info_t msg_info;
   runloop_ctl(RUNLOOP_CTL_MSG_QUEUE_PULL, &msg_info);

   return strdup(msg_info.msg);
}

/* Checks if movie is being played back. */
static bool runloop_check_movie_playback(void)
{
   if (!bsv_movie_ctl(BSV_MOVIE_CTL_END, NULL))
      return false;

   runloop_msg_queue_push(
         msg_hash_to_str(MSG_MOVIE_PLAYBACK_ENDED), 2, 180, false);
   VLOG(@"%s\n", msg_hash_to_str(MSG_MOVIE_PLAYBACK_ENDED));

   command_event(CMD_EVENT_BSV_MOVIE_DEINIT, NULL);

   bsv_movie_ctl(BSV_MOVIE_CTL_UNSET_END, NULL);
   bsv_movie_ctl(BSV_MOVIE_CTL_UNSET_PLAYBACK, NULL);

   return true;
}

/* Checks if movie is being recorded. */
static bool runloop_check_movie_record(void)
{
   if (!bsv_movie_ctl(BSV_MOVIE_CTL_IS_INITED, NULL))
      return false;

   runloop_msg_queue_push(
         msg_hash_to_str(MSG_MOVIE_RECORD_STOPPED), 2, 180, true);
   VLOG(@"%s\n", msg_hash_to_str(MSG_MOVIE_RECORD_STOPPED));

   command_event(CMD_EVENT_BSV_MOVIE_DEINIT, NULL);

   return true;
}

static bool runloop_check_movie_init(void)
{
   char msg[128]              = {0};
   char path[PATH_MAX_LENGTH] = {0};
   settings_t *settings       = config_get_ptr();

   if (bsv_movie_ctl(BSV_MOVIE_CTL_IS_INITED, NULL))
      return false;

   settings->rewind_granularity = 1;

   if (settings->state_slot > 0)
      snprintf(path, sizeof(path), "%s%d",
            bsv_movie_get_path(), settings->state_slot);
   else
      strlcpy(path, bsv_movie_get_path(), sizeof(path));

   strlcat(path,
         file_path_str(FILE_PATH_BSV_EXTENSION),
         sizeof(path));

   snprintf(msg, sizeof(msg), "%s \"%s\".",
         msg_hash_to_str(MSG_STARTING_MOVIE_RECORD_TO),
         path);

   bsv_movie_init_handle(path, RARCH_MOVIE_RECORD);

   if (!bsv_movie_ctl(BSV_MOVIE_CTL_IS_INITED, NULL))
      return false;

   if (bsv_movie_ctl(BSV_MOVIE_CTL_IS_INITED, NULL))
   {
      runloop_msg_queue_push(msg, 2, 180, true);
      VLOG(@"%s \"%s\".\n",
            msg_hash_to_str(MSG_STARTING_MOVIE_RECORD_TO),
            path);
   }
   else
   {
      runloop_msg_queue_push(
            msg_hash_to_str(MSG_FAILED_TO_START_MOVIE_RECORD),
            2, 180, true);
      ELOG(@"%s\n",
            msg_hash_to_str(MSG_FAILED_TO_START_MOVIE_RECORD));
   }

   return true;
}

static bool runloop_check_movie(void)
{
   if (bsv_movie_ctl(BSV_MOVIE_CTL_PLAYBACK_ON, NULL))
      return runloop_check_movie_playback();
   if (!bsv_movie_ctl(BSV_MOVIE_CTL_IS_INITED, NULL))
      return runloop_check_movie_init();
   return runloop_check_movie_record();
}

/* Checks if slowmotion toggle/hold was being pressed and/or held. */
static bool runloop_check_slowmotion(bool *ptr)
{
   settings_t *settings  = config_get_ptr();
   if (!ptr)
      return false;

   runloop_slowmotion   = *ptr;

   if (!runloop_slowmotion)
      return false;

   if (settings->video.black_frame_insertion)
      video_driver_cached_frame_render();

   if (state_manager_frame_is_reversed())
      runloop_msg_queue_push(msg_hash_to_str(MSG_SLOW_MOTION_REWIND), 2, 30, true);
   else
      runloop_msg_queue_push(msg_hash_to_str(MSG_SLOW_MOTION), 2, 30, true);
   return true;
}

#ifdef HAVE_MENU
static bool runloop_cmd_get_state_menu_toggle_button_combo(
      settings_t *settings,
      retro_input_t input, retro_input_t old_input,
      retro_input_t trigger_input)
{
   switch (settings->input.menu_toggle_gamepad_combo)
   {
      case INPUT_TOGGLE_NONE:
         return false;
      case INPUT_TOGGLE_DOWN_Y_L_R:
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_DOWN))
            return false;
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_Y))
            return false;
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_L))
            return false;
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_R))
            return false;
         break;
      case INPUT_TOGGLE_L3_R3:
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_L3))
            return false;
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_R3))
            return false;
         break;
      case INPUT_TOGGLE_L1_R1_START_SELECT:
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_START))
            return false;
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_SELECT))
            return false;
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_L))
            return false;
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_R))
            return false;
         break;
      case INPUT_TOGGLE_START_SELECT:
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_START))
            return false;
         if (!BIT64_GET(input.state, RETRO_DEVICE_ID_JOYPAD_SELECT))
            return false;
         break;
   }

   input_driver_set_flushing_input();
   return true;
}
#endif

/**
 * runloop_check_pause:
 * @pressed              : was libretro pause key pressed?
 * @frameadvance_pressed : was frameadvance key pressed?
 *
 * Check if libretro pause key was pressed. If so, pause or
 * unpause the libretro core.
 *
 * Returns: true if libretro pause key was toggled, otherwise false.
 **/
static bool runloop_check_pause(settings_t *settings,
      bool focus, bool pause_pressed,
      bool frameadvance_pressed)
{
   static bool old_focus    = true;
   enum event_command cmd   = CMD_EVENT_NONE;
   bool old_is_paused       = runloop_ctl(RUNLOOP_CTL_IS_PAUSED, NULL);

   /* FRAMEADVANCE will set us into pause mode. */
   pause_pressed |= !old_is_paused && frameadvance_pressed;

   if (focus && pause_pressed)
      cmd = CMD_EVENT_PAUSE_TOGGLE;
   else if (focus && !old_focus)
      cmd = CMD_EVENT_UNPAUSE;
   else if (!focus && old_focus)
      cmd = CMD_EVENT_PAUSE;

   old_focus = focus;

   if (cmd != CMD_EVENT_NONE)
      command_event(cmd, NULL);

   if (runloop_ctl(RUNLOOP_CTL_IS_PAUSED, NULL) == old_is_paused)
      return false;

   return true;
}

/**
 * runloop_check_fast_forward_button:
 * @fastforward_pressed  : is fastforward key pressed?
 * @hold_pressed         : is fastforward key pressed and held?
 * @old_hold_pressed     : was fastforward key pressed and held the last frame?
 *
 * Checks if the fast forward key has been pressed for this frame.
 *
 **/
static void runloop_check_fast_forward_button(bool fastforward_pressed,
      bool hold_pressed, bool old_hold_pressed)
{
   /* To avoid continous switching if we hold the button down, we require
    * that the button must go from pressed to unpressed back to pressed
    * to be able to toggle between then.
    */
   if (fastforward_pressed)
   {
      if (input_driver_is_nonblock_state())
         input_driver_unset_nonblock_state();
      else
         input_driver_set_nonblock_state();
   }
   else if (old_hold_pressed != hold_pressed)
   {
      if (hold_pressed)
         input_driver_set_nonblock_state();
      else
         input_driver_unset_nonblock_state();
   }
   else
      return;

   driver_ctl(RARCH_DRIVER_CTL_SET_NONBLOCK_STATE, NULL);
}

/**
 * runloop_check_stateslots:
 * @pressed_increase     : is state slot increase key pressed?
 * @pressed_decrease     : is state slot decrease key pressed?
 *
 * Checks if the state increase/decrease keys have been pressed
 * for this frame.
 **/
static void runloop_check_stateslots(settings_t *settings,
      bool pressed_increase, bool pressed_decrease)
{
   char msg[128]         = {0};

   /* Save state slots */
   if (pressed_increase)
      settings->state_slot++;
   else if (pressed_decrease)
   {
      if (settings->state_slot > 0)
         settings->state_slot--;
   }
   else
      return;

   snprintf(msg, sizeof(msg), "%s: %d",
         msg_hash_to_str(MSG_STATE_SLOT),
         settings->state_slot);

   runloop_msg_queue_push(msg, 2, 180, true);

   VLOG(@"%s\n", msg);
}

static void shader_dir_free(rarch_dir_list_t *dir_list)
{
   if (!dir_list)
      return;

   dir_list_free(dir_list->list);
   dir_list->list = NULL;
   dir_list->ptr  = 0;
}

static bool shader_dir_init(rarch_dir_list_t *dir_list)
{
   unsigned i;
   settings_t *settings  = config_get_ptr();

   if (!*settings->directory.video_shader)
      return false;

   dir_list->list = dir_list_new_special(settings->directory.video_shader, DIR_LIST_SHADERS, NULL);

   if (!dir_list->list || dir_list->list->size == 0)
   {
      command_event(CMD_EVENT_SHADER_DIR_DEINIT, NULL);
      return false;
   }

   dir_list->ptr  = 0;
   dir_list_sort(dir_list->list, false);

   for (i = 0; i < dir_list->list->size; i++)
      VLOG(@"%s \"%s\"\n",
            msg_hash_to_str(MSG_FOUND_SHADER),
            dir_list->list->elems[i].data);
   return true;
}

/**
 * runloop_check_shader_dir:
 * @pressed_next         : was next shader key pressed?
 * @pressed_previous     : was previous shader key pressed?
 *
 * Checks if any one of the shader keys has been pressed for this frame:
 * a) Next shader index.
 * b) Previous shader index.
 *
 * Will also immediately apply the shader.
 **/
static void runloop_check_shader_dir(rarch_dir_list_t *dir_list,
      bool pressed_next, bool pressed_prev)
{

   char msg[128]               = {0};
   const char *shader          = NULL;
   enum rarch_shader_type type = RARCH_SHADER_NONE;

   if (!dir_list || !dir_list->list)
      return;

   if (pressed_next)
   {
      dir_list->ptr = (dir_list->ptr + 1) %
         dir_list->list->size;
   }
   else if (pressed_prev)
   {
      if (dir_list->ptr == 0)
         dir_list->ptr = dir_list->list->size - 1;
      else
         dir_list->ptr--;
   }
   else
      return;

   shader   = dir_list->list->elems[dir_list->ptr].data;

   switch (msg_hash_to_file_type(msg_hash_calculate(path_get_extension(shader))))
   {
      case FILE_TYPE_SHADER_GLSL:
      case FILE_TYPE_SHADER_PRESET_GLSLP:
         type = RARCH_SHADER_GLSL;
         break;
      case FILE_TYPE_SHADER_SLANG:
      case FILE_TYPE_SHADER_PRESET_SLANGP:
         type = RARCH_SHADER_SLANG;
         break;
      case FILE_TYPE_SHADER_CG:
      case FILE_TYPE_SHADER_PRESET_CGP:
         type = RARCH_SHADER_CG;
         break;
      default:
         return;
   }

   snprintf(msg, sizeof(msg), "%s #%u: \"%s\".",
         msg_hash_to_str(MSG_SHADER),
         (unsigned)dir_list->ptr, shader);
   runloop_msg_queue_push(msg, 2, 120, true);

   VLOG(@"%s \"%s\".\n",
         msg_hash_to_str(MSG_APPLYING_SHADER),
         shader);

   if (!video_driver_set_shader(type, shader))
      WLOG(@"%s\n", msg_hash_to_str(MSG_FAILED_TO_APPLY_SHADER));
}

/**
 * rarch_game_specific_options:
 *
 * Returns: true (1) if a game specific core 
 * options path has been found,
 * otherwise false (0).
 **/
static bool rarch_game_specific_options(char **output)
{
   char game_path[PATH_MAX_LENGTH] = {0};
   config_file_t *option_file      = NULL;
   
   if (!retroarch_validate_game_options(game_path,
            sizeof(game_path), false))
         return false;

   option_file = config_file_new(game_path);
   if (!option_file)
      return false;

   config_file_free(option_file);
   
   VLOG(@"%s %s\n",
         msg_hash_to_str(MSG_GAME_SPECIFIC_CORE_OPTIONS_FOUND_AT),
         game_path);
   *output = strdup(game_path);
   return true;
}

static bool runloop_is_focused(void)
{
   settings_t *settings      = config_get_ptr();
   if (settings->pause_nonactive)
      return video_driver_is_focused();
   return true;
}

static bool runloop_check_pause_state(event_cmd_state_t *cmd)
{
   bool check_is_oneshot = false;

   if (!cmd)
      return false;

   check_is_oneshot      = runloop_cmd_triggered(cmd,
         RARCH_FRAMEADVANCE) 
      || runloop_cmd_press(cmd, RARCH_REWIND);

   if (!runloop_ctl(RUNLOOP_CTL_IS_PAUSED, NULL))
      return true;

   if (runloop_cmd_triggered(cmd, RARCH_FULLSCREEN_TOGGLE_KEY))
   {
      command_event(CMD_EVENT_FULLSCREEN_TOGGLE, NULL);
      video_driver_cached_frame_render();
   }

   if (!check_is_oneshot)
      return false;

   return true;
}

static bool runloop_check_idle_state(event_cmd_state_t *cmd)
{
   settings_t *settings      = config_get_ptr();
   bool focused              =  runloop_is_focused();

   runloop_check_pause(settings, focused,
         runloop_cmd_triggered(cmd, RARCH_PAUSE_TOGGLE),
         runloop_cmd_triggered(cmd, RARCH_FRAMEADVANCE));

   if (!runloop_check_pause_state(cmd) || !focused)
      return false;
   return true;
}

static bool runloop_check_state(event_cmd_state_t *cmd,
      rarch_dir_list_t *shader_dir)
{
   bool tmp                  = false;
   settings_t *settings      = config_get_ptr();

   if (!cmd || runloop_ctl(RUNLOOP_CTL_IS_IDLE, NULL))
      return false;

   if (runloop_cmd_triggered(cmd, RARCH_SCREENSHOT))
      command_event(CMD_EVENT_TAKE_SCREENSHOT, NULL);

   if (runloop_cmd_triggered(cmd, RARCH_MUTE))
      command_event(CMD_EVENT_AUDIO_MUTE_TOGGLE, NULL);

   if (runloop_cmd_triggered(cmd, RARCH_OSK))
   {
      if (input_keyboard_ctl(
               RARCH_INPUT_KEYBOARD_CTL_IS_LINEFEED_ENABLED, NULL))
         input_keyboard_ctl(
               RARCH_INPUT_KEYBOARD_CTL_UNSET_LINEFEED_ENABLED, NULL);
      else
         input_keyboard_ctl(
               RARCH_INPUT_KEYBOARD_CTL_SET_LINEFEED_ENABLED, NULL);
   }

   if (runloop_cmd_press(cmd, RARCH_VOLUME_UP))
      command_event(CMD_EVENT_VOLUME_UP, NULL);
   else if (runloop_cmd_press(cmd, RARCH_VOLUME_DOWN))
      command_event(CMD_EVENT_VOLUME_DOWN, NULL);

#ifdef HAVE_NETPLAY
   tmp = runloop_cmd_triggered(cmd, RARCH_NETPLAY_FLIP);
   netplay_driver_ctl(RARCH_NETPLAY_CTL_FLIP_PLAYERS, &tmp);
   tmp = runloop_cmd_triggered(cmd, RARCH_FULLSCREEN_TOGGLE_KEY);
   netplay_driver_ctl(RARCH_NETPLAY_CTL_FULLSCREEN_TOGGLE, &tmp);
#endif
   if (!runloop_check_idle_state(cmd))
      return false;

   runloop_check_fast_forward_button(
         runloop_cmd_triggered(cmd, RARCH_FAST_FORWARD_KEY),
         runloop_cmd_press    (cmd, RARCH_FAST_FORWARD_HOLD_KEY),
         runloop_cmd_pressed  (cmd, RARCH_FAST_FORWARD_HOLD_KEY));
   runloop_check_stateslots(settings,
         runloop_cmd_triggered(cmd, RARCH_STATE_SLOT_PLUS),
         runloop_cmd_triggered(cmd, RARCH_STATE_SLOT_MINUS)
         );

   if (runloop_cmd_triggered(cmd, RARCH_SAVE_STATE_KEY))
      command_event(CMD_EVENT_SAVE_STATE, NULL);
   else if (runloop_cmd_triggered(cmd, RARCH_LOAD_STATE_KEY))
      command_event(CMD_EVENT_LOAD_STATE, NULL);

#ifdef HAVE_CHEEVOS
   if(!settings->cheevos.hardcore_mode_enable)
#endif
      state_manager_check_rewind(runloop_cmd_press(cmd, RARCH_REWIND));

   tmp = runloop_cmd_press(cmd, RARCH_SLOWMOTION);

   runloop_check_slowmotion(&tmp);

   if (runloop_cmd_triggered(cmd, RARCH_MOVIE_RECORD_TOGGLE))
      runloop_check_movie();
   
   if (runloop_cmd_triggered(cmd, RARCH_SHADER_NEXT) || 
      runloop_cmd_triggered(cmd, RARCH_SHADER_PREV))
   {
      runloop_check_shader_dir(shader_dir,
            runloop_cmd_triggered(cmd, RARCH_SHADER_NEXT),
            runloop_cmd_triggered(cmd, RARCH_SHADER_PREV));
   }

   if (runloop_cmd_triggered(cmd, RARCH_DISK_EJECT_TOGGLE))
      command_event(CMD_EVENT_DISK_EJECT_TOGGLE, NULL);
   else if (runloop_cmd_triggered(cmd, RARCH_DISK_NEXT))
      command_event(CMD_EVENT_DISK_NEXT, NULL);
   else if (runloop_cmd_triggered(cmd, RARCH_DISK_PREV))
      command_event(CMD_EVENT_DISK_PREV, NULL);

   if (runloop_cmd_triggered(cmd, RARCH_RESET))
      command_event(CMD_EVENT_RESET, NULL);

   cheat_manager_state_checks(
         runloop_cmd_triggered(cmd, RARCH_CHEAT_INDEX_PLUS),
         runloop_cmd_triggered(cmd, RARCH_CHEAT_INDEX_MINUS),
                              
                              static bool runloop_is_frame_count_end(void)
                              {
                                 uint64_t *frame_count =
                                    video_driver_get_frame_count_ptr();
                                 return runloop_max_frames && (*frame_count >= runloop_max_frames);
                              }

                              bool runloop_ctl
         runloop_cmd_triggered(cmd, RARCH_CHEAT_TOGGLE));

   return true;
}(enum runloop_ctl_state state, void *data)
{

   switch (state)
   {
      case RUNLOOP_CTL_SHADER_DIR_DEINIT:
         shader_dir_free(&runloop_shader_dir);
         break;
      case RUNLOOP_CTL_SHADER_DIR_INIT:
         return shader_dir_init(&runloop_shader_dir);
      case RUNLOOP_CTL_SYSTEM_INFO_INIT:
         core_get_system_info(&runloop_system.info);

         if (!runloop_system.info.library_name)
            runloop_system.info.library_name = msg_hash_to_str(MSG_UNKNOWN);
         if (!runloop_system.info.library_version)
            runloop_system.info.library_version = "v0";

         video_driver_set_title_buf();

         strlcpy(runloop_system.valid_extensions,
               runloop_system.info.valid_extensions ?
               runloop_system.info.valid_extensions : DEFAULT_EXT,
               sizeof(runloop_system.valid_extensions));
         break;
      case RUNLOOP_CTL_GET_CORE_OPTION_SIZE:
         {
            unsigned *idx = (unsigned*)data;
            if (!idx)
               return false;
            *idx = core_option_manager_size(runloop_core_options);
         }
         break;
      case RUNLOOP_CTL_HAS_CORE_OPTIONS:
         return runloop_core_options;
      case RUNLOOP_CTL_CORE_OPTIONS_LIST_GET:
         {
            core_option_manager_t **coreopts = (core_option_manager_t**)data;
            if (!coreopts)
               return false;
            *coreopts = runloop_core_options;
         }
         break;
      case RUNLOOP_CTL_SYSTEM_INFO_GET:
         {
            rarch_system_info_t **system = (rarch_system_info_t**)data;
            if (!system)
               return false;
            *system = &runloop_system;
         }
         break;
      case RUNLOOP_CTL_SYSTEM_INFO_FREE:

         /* No longer valid. */
         if (runloop_system.subsystem.data)
            free(runloop_system.subsystem.data);
         runloop_system.subsystem.data = NULL;
         runloop_system.subsystem.size = 0;
         
         if (runloop_system.ports.data)
            free(runloop_system.ports.data);
         runloop_system.ports.data = NULL;
         runloop_system.ports.size = 0;
         
         if (runloop_system.mmaps.descriptors)
            free((void *)runloop_system.mmaps.descriptors);
         runloop_system.mmaps.descriptors     = NULL;
         runloop_system.mmaps.num_descriptors = 0;
         
         runloop_key_event          = NULL;
         runloop_frontend_key_event = NULL;

         audio_driver_unset_callback();
         memset(&runloop_system, 0, sizeof(rarch_system_info_t));
         break;
      case RUNLOOP_CTL_SET_FRAME_TIME_LAST:
         runloop_frame_time_last_enable = true;
         break;
      case RUNLOOP_CTL_UNSET_FRAME_TIME_LAST:
         if (!runloop_ctl(RUNLOOP_CTL_IS_FRAME_TIME_LAST, NULL))
            return false;
         runloop_frame_time_last        = 0;
         runloop_frame_time_last_enable = false;
         break;
      case RUNLOOP_CTL_SET_OVERRIDES_ACTIVE:
         runloop_overrides_active = true;
         break;
      case RUNLOOP_CTL_UNSET_OVERRIDES_ACTIVE:
         runloop_overrides_active = false; 
         break;
      case RUNLOOP_CTL_IS_OVERRIDES_ACTIVE:
         return runloop_overrides_active;
      case RUNLOOP_CTL_SET_GAME_OPTIONS_ACTIVE:
         runloop_game_options_active = true;
         break;
      case RUNLOOP_CTL_UNSET_GAME_OPTIONS_ACTIVE:
         runloop_game_options_active = false;
         break;
      case RUNLOOP_CTL_IS_GAME_OPTIONS_ACTIVE:
         return runloop_game_options_active;
      case RUNLOOP_CTL_IS_FRAME_TIME_LAST:
         return runloop_frame_time_last_enable;
      case RUNLOOP_CTL_SET_FRAME_LIMIT:
         runloop_set_frame_limit = true;
         break;
      case RUNLOOP_CTL_UNSET_FRAME_LIMIT:
         runloop_set_frame_limit = false;
         break;
      case RUNLOOP_CTL_SHOULD_SET_FRAME_LIMIT:
         return runloop_set_frame_limit;
      case RUNLOOP_CTL_GET_PERFCNT:
         {
            bool **perfcnt = (bool**)data;
            if (!perfcnt)
               return false;
            *perfcnt = &runloop_perfcnt_enable;
         }
         break;
      case RUNLOOP_CTL_SET_PERFCNT_ENABLE:
         runloop_perfcnt_enable = true;
         break;
      case RUNLOOP_CTL_UNSET_PERFCNT_ENABLE:
         runloop_perfcnt_enable = false;
         break;
      case RUNLOOP_CTL_IS_PERFCNT_ENABLE:
         return runloop_perfcnt_enable;
      case RUNLOOP_CTL_SET_NONBLOCK_FORCED:
         runloop_force_nonblock = true;
         break;
      case RUNLOOP_CTL_UNSET_NONBLOCK_FORCED:
         runloop_force_nonblock = false;
         break;
      case RUNLOOP_CTL_IS_NONBLOCK_FORCED:
         return runloop_force_nonblock;
      case RUNLOOP_CTL_SET_FRAME_TIME:
         {
            const struct retro_frame_time_callback *info =
               (const struct retro_frame_time_callback*)data;
#ifdef HAVE_NETPLAY
            global_t *global = global_get_ptr();

            /* retro_run() will be called in very strange and
             * mysterious ways, have to disable it. */
            if (global->netplay.enable)
               return false;
#endif
            runloop_frame_time = *info;
         }
         break;
      case RUNLOOP_CTL_GET_WINDOWED_SCALE:
         {
            unsigned **scale = (unsigned**)data;
            if (!scale)
               return false;
            *scale       = (unsigned*)&runloop_pending_windowed_scale;
         }
         break;
      case RUNLOOP_CTL_SET_WINDOWED_SCALE:
         {
            unsigned *idx = (unsigned*)data;
            if (!idx)
               return false;
            runloop_pending_windowed_scale = *idx;
         }
         break;
      case RUNLOOP_CTL_SET_LIBRETRO_PATH:
         {
            const char *fullpath = (const char*)data;
            if (!fullpath)
               return false;
            config_set_active_core_path(fullpath);
         }
         break;
      case RUNLOOP_CTL_CLEAR_CONTENT_PATH:
         *runloop_fullpath = '\0';
         break;
      case RUNLOOP_CTL_GET_CONTENT_PATH:
         {
            char **fullpath = (char**)data;
            if (!fullpath)
               return false;
            *fullpath       = (char*)runloop_fullpath;
         }
         break;
      case RUNLOOP_CTL_SET_CONTENT_PATH:
         {
            const char *fullpath = (const char*)data;
            if (!fullpath)
               return false;
            strlcpy(runloop_fullpath, fullpath, sizeof(runloop_fullpath));
         }
         break;
      case RUNLOOP_CTL_CLEAR_DEFAULT_SHADER_PRESET:
         *runloop_default_shader_preset = '\0';
         break;
      case RUNLOOP_CTL_GET_DEFAULT_SHADER_PRESET:
         {
            char **preset = (char**)data;
            if (!preset)
               return false;
            *preset       = (char*)runloop_default_shader_preset;
         }
         break;
      case RUNLOOP_CTL_SET_DEFAULT_SHADER_PRESET:
         {
            const char *preset = (const char*)data;
            if (!preset)
               return false;
            strlcpy(runloop_default_shader_preset, preset,
               sizeof(runloop_default_shader_preset));
         }
         break;
      case RUNLOOP_CTL_FRAME_TIME_FREE:
         memset(&runloop_frame_time, 0, sizeof(struct retro_frame_time_callback));
         runloop_frame_time_last           = 0;
         runloop_max_frames                = 0;
         break;
      case RUNLOOP_CTL_STATE_FREE:
         runloop_perfcnt_enable            = false;
         runloop_idle                      = false;
         runloop_paused                    = false;
         runloop_slowmotion                = false;
         runloop_frame_time_last_enable    = false;
         runloop_set_frame_limit           = false;
         runloop_overrides_active          = false;
         runloop_ctl(RUNLOOP_CTL_FRAME_TIME_FREE, NULL);
         break;
      case RUNLOOP_CTL_GLOBAL_FREE:
         {
            global_t *global = NULL;
            command_event(CMD_EVENT_TEMPORARY_CONTENT_DEINIT, NULL);
            command_event(CMD_EVENT_SUBSYSTEM_FULLPATHS_DEINIT, NULL);
            command_event(CMD_EVENT_RECORD_DEINIT, NULL);
            command_event(CMD_EVENT_LOG_FILE_DEINIT, NULL);

            rarch_ctl(RARCH_CTL_UNSET_BLOCK_CONFIG_READ, NULL);
            runloop_ctl(RUNLOOP_CTL_CLEAR_CONTENT_PATH,  NULL);
            runloop_overrides_active   = false;

            core_unset_input_descriptors();

            global = global_get_ptr();
            memset(global, 0, sizeof(struct global));
            retroarch_override_setting_free_state();
            config_free_state();
         }
         break;
      case RUNLOOP_CTL_CLEAR_STATE:
         driver_ctl(RARCH_DRIVER_CTL_DEINIT,  NULL);
         runloop_ctl(RUNLOOP_CTL_STATE_FREE,  NULL);
         runloop_ctl(RUNLOOP_CTL_GLOBAL_FREE, NULL);
         break;
      case RUNLOOP_CTL_SET_MAX_FRAMES:
         {
            unsigned *ptr = (unsigned*)data;
            if (!ptr)
               return false;
            runloop_max_frames = *ptr;
         }
         break;
      case RUNLOOP_CTL_IS_IDLE:
         return runloop_idle;
      case RUNLOOP_CTL_SET_IDLE:
         {
            bool *ptr = (bool*)data;
            if (!ptr)
               return false;
            runloop_idle = *ptr;
         }
         break;
      case RUNLOOP_CTL_IS_SLOWMOTION:
         return runloop_slowmotion;
      case RUNLOOP_CTL_SET_SLOWMOTION:
         {
            bool *ptr = (bool*)data;
            if (!ptr)
               return false;
            runloop_slowmotion = *ptr;
         }
         break;
      case RUNLOOP_CTL_SET_PAUSED:
         {
            bool *ptr = (bool*)data;
            if (!ptr)
               return false;
            runloop_paused = *ptr;
         }
         break;
      case RUNLOOP_CTL_IS_PAUSED:
         return runloop_paused;
      case RUNLOOP_CTL_MSG_QUEUE_PULL:
         runloop_msg_queue_lock();
         {
            const char **ret = (const char**)data;
            if (!ret)
               return false;
            *ret = msg_queue_pull(runloop_msg_queue);
         }
         runloop_msg_queue_unlock();
         break;
      case RUNLOOP_CTL_MSG_QUEUE_FREE:
#ifdef HAVE_THREADS
         slock_free(_runloop_msg_queue_lock);
         _runloop_msg_queue_lock = NULL;
#endif
         break;
      case RUNLOOP_CTL_MSG_QUEUE_CLEAR:
         msg_queue_clear(runloop_msg_queue);
         break;
      case RUNLOOP_CTL_MSG_QUEUE_DEINIT:
         if (!runloop_msg_queue)
            return true;

         runloop_msg_queue_lock();

         msg_queue_free(runloop_msg_queue);

         runloop_msg_queue_unlock();
         runloop_ctl(RUNLOOP_CTL_MSG_QUEUE_FREE, NULL);

         runloop_msg_queue = NULL;
         break;
      case RUNLOOP_CTL_MSG_QUEUE_INIT:
         runloop_ctl(RUNLOOP_CTL_MSG_QUEUE_DEINIT, NULL);
         runloop_msg_queue = msg_queue_new(8);
         retro_assert(runloop_msg_queue);

#ifdef HAVE_THREADS
         _runloop_msg_queue_lock = slock_new();
         retro_assert(_runloop_msg_queue_lock);
#endif
         break;
      case RUNLOOP_CTL_TASK_INIT:
         {
#ifdef HAVE_THREADS
            settings_t *settings = config_get_ptr();
            bool threaded_enable = settings->threaded_data_runloop_enable;
#else
            bool threaded_enable = false;
#endif
            task_queue_ctl(TASK_QUEUE_CTL_DEINIT, NULL);
            task_queue_ctl(TASK_QUEUE_CTL_INIT, &threaded_enable);
         }
         break;
      case RUNLOOP_CTL_SET_CORE_SHUTDOWN:
         runloop_core_shutdown_initiated = true;
         break;
      case RUNLOOP_CTL_UNSET_CORE_SHUTDOWN:
         runloop_core_shutdown_initiated = false;
         break;
      case RUNLOOP_CTL_IS_CORE_SHUTDOWN:
         return runloop_core_shutdown_initiated;
      case RUNLOOP_CTL_SET_SHUTDOWN:
         runloop_shutdown_initiated = true;
         break;
      case RUNLOOP_CTL_UNSET_SHUTDOWN:
         runloop_shutdown_initiated = false;
         break;
      case RUNLOOP_CTL_IS_SHUTDOWN:
         return runloop_shutdown_initiated;
      case RUNLOOP_CTL_SET_EXEC:
         runloop_exec = true;
         break;
      case RUNLOOP_CTL_UNSET_EXEC:
         runloop_exec = false;
         break;
      case RUNLOOP_CTL_IS_EXEC:
         return runloop_exec;
      case RUNLOOP_CTL_DATA_DEINIT:
         task_queue_ctl(TASK_QUEUE_CTL_DEINIT, NULL);
         break;
      case RUNLOOP_CTL_IS_CORE_OPTION_UPDATED:
         if (!runloop_core_options)
            return false;
         return  core_option_manager_updated(runloop_core_options);
      case RUNLOOP_CTL_CORE_OPTION_PREV:
         {
            unsigned *idx = (unsigned*)data;
            if (!idx)
               return false;
            core_option_manager_prev(runloop_core_options, *idx);
            if (ui_companion_is_on_foreground())
               ui_companion_driver_notify_refresh();
         }
         break;
      case RUNLOOP_CTL_CORE_OPTION_NEXT:
         {
            unsigned *idx = (unsigned*)data;
            if (!idx)
               return false;
            core_option_manager_next(runloop_core_options, *idx);
            if (ui_companion_is_on_foreground())
               ui_companion_driver_notify_refresh();
         }
         break;
      case RUNLOOP_CTL_CORE_OPTIONS_GET:
         {
            struct retro_variable *var = (struct retro_variable*)data;

            if (!runloop_core_options || !var)
               return false;

            VLOG(@"Environ GET_VARIABLE %s:\n", var->key);
            core_option_manager_get(runloop_core_options, var);
            VLOG(@"\t%s\n", var->value ? var->value : 
                  msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NOT_AVAILABLE));
         }
         break;
      case RUNLOOP_CTL_CORE_OPTIONS_INIT:
         {
            char *game_options_path           = NULL;
            bool ret                          = false;
            char buf[PATH_MAX_LENGTH]         = {0};
            global_t *global                  = global_get_ptr();
            settings_t *settings              = config_get_ptr();
            const char *options_path          = settings->path.core_options;
            const struct retro_variable *vars = 
               (const struct retro_variable*)data;

            if (string_is_empty(options_path) 
                  && !string_is_empty(global->path.config))
            {
               fill_pathname_resolve_relative(buf, global->path.config,
                     file_path_str(FILE_PATH_CORE_OPTIONS_CONFIG), sizeof(buf));
               options_path = buf;
            }


            if (settings->game_specific_options)
               ret = rarch_game_specific_options(&game_options_path);

            if(ret)
            {
               runloop_ctl(RUNLOOP_CTL_SET_GAME_OPTIONS_ACTIVE, NULL);
               runloop_core_options = 
                  core_option_manager_new(game_options_path, vars);
               free(game_options_path);
            }
            else
            {
               runloop_ctl(RUNLOOP_CTL_UNSET_GAME_OPTIONS_ACTIVE, NULL);
               runloop_core_options = 
                  core_option_manager_new(options_path, vars);
            }

         }
         break;
      case RUNLOOP_CTL_CORE_OPTIONS_FREE:
         if (runloop_core_options)
            core_option_manager_free(runloop_core_options);
         runloop_core_options          = NULL;
         break;
      case RUNLOOP_CTL_CORE_OPTIONS_DEINIT:
         {
            global_t *global                  = global_get_ptr();
            if (!runloop_core_options)
               return false;

            /* check if game options file was just created and flush
               to that file instead */
            if(global && !string_is_empty(global->path.core_options_path))
            {
               core_option_manager_flush_game_specific(runloop_core_options,
                     global->path.core_options_path);
               global->path.core_options_path[0] = '\0';
            }
            else
               core_option_manager_flush(runloop_core_options);

            if (runloop_ctl(RUNLOOP_CTL_IS_GAME_OPTIONS_ACTIVE, NULL))
               runloop_ctl(RUNLOOP_CTL_UNSET_GAME_OPTIONS_ACTIVE, NULL);

            runloop_ctl(RUNLOOP_CTL_CORE_OPTIONS_FREE, NULL);
         }
         break;
      case RUNLOOP_CTL_KEY_EVENT_GET:
         {
            retro_keyboard_event_t **key_event = 
               (retro_keyboard_event_t**)data;
            if (!key_event)
               return false;
            *key_event = &runloop_key_event;
         }
         break;
      case RUNLOOP_CTL_FRONTEND_KEY_EVENT_GET:
         {
            retro_keyboard_event_t **key_event = 
               (retro_keyboard_event_t**)data;
            if (!key_event)
               return false;
            *key_event = &runloop_frontend_key_event;
         }
         break;
      case RUNLOOP_CTL_HTTPSERVER_INIT:
#if defined(HAVE_HTTPSERVER) && defined(HAVE_ZLIB)
         httpserver_init(8888);
#endif
         break;
      case RUNLOOP_CTL_HTTPSERVER_DESTROY:
#if defined(HAVE_HTTPSERVER) && defined(HAVE_ZLIB)
         httpserver_destroy();
#endif
         break;
      case RUNLOOP_CTL_NONE:
      default:
         break;
   }

   return true;
}


#ifdef HAVE_OVERLAY
static void runloop_iterate_linefeed_overlay(settings_t *settings)
{
   static char prev_overlay_restore = false;
   bool osk_enable = input_driver_is_onscreen_keyboard_enabled();

   if (osk_enable && !input_keyboard_ctl(
            RARCH_INPUT_KEYBOARD_CTL_IS_LINEFEED_ENABLED, NULL))
   {
      input_driver_unset_onscreen_keyboard_enabled();
      prev_overlay_restore  = true;
      command_event(CMD_EVENT_OVERLAY_DEINIT, NULL);
   }
   else if (!osk_enable && input_keyboard_ctl(
            RARCH_INPUT_KEYBOARD_CTL_IS_LINEFEED_ENABLED, NULL))
   {
      input_driver_set_onscreen_keyboard_enabled();
      prev_overlay_restore  = false;
      command_event(CMD_EVENT_OVERLAY_INIT, NULL);
   }
   else if (prev_overlay_restore)
   {
      if (!settings->input.overlay_hide_in_menu)
         command_event(CMD_EVENT_OVERLAY_INIT, NULL);
      prev_overlay_restore = false;
   }
}
#endif


/* Loads dummy core instead of exiting RetroArch completely.
 * Aborts core shutdown if invoked. */
static int runloop_iterate_time_to_exit_load_dummy(void)
{
   content_ctx_info_t content_info = {0};
   if (!task_push_content_load_default(
         NULL, NULL,
         &content_info,
         CORE_TYPE_DUMMY,
         CONTENT_MODE_LOAD_NOTHING_WITH_DUMMY_CORE,
         NULL, NULL))
      return -1;

   runloop_ctl(RUNLOOP_CTL_UNSET_SHUTDOWN,      NULL);
   runloop_ctl(RUNLOOP_CTL_UNSET_CORE_SHUTDOWN, NULL);

   return 1;
}

/* Time to exit out of the main loop?
 * Reasons for exiting:
 * a) Shutdown environment callback was invoked.
 * b) Quit key was pressed.
 * c) Frame count exceeds or equals maximum amount of frames to run.
 * d) Video driver no longer alive.
 * e) End of BSV movie and BSV EOF exit is true. (TODO/FIXME - explain better)
 */
static INLINE int runloop_iterate_time_to_exit(bool quit_key_pressed)
{
   settings_t *settings          = NULL;
   bool time_to_exit             = runloop_ctl(RUNLOOP_CTL_IS_SHUTDOWN, NULL);
   time_to_exit                  = time_to_exit || quit_key_pressed;
   time_to_exit                  = time_to_exit || !video_driver_is_alive();
   time_to_exit                  = time_to_exit || bsv_movie_ctl(BSV_MOVIE_CTL_END_EOF, NULL);
   time_to_exit                  = time_to_exit || runloop_is_frame_count_end();
   time_to_exit                  = time_to_exit || runloop_ctl(RUNLOOP_CTL_IS_EXEC, NULL);

   if (!time_to_exit)
      return 1;

   if (runloop_ctl(RUNLOOP_CTL_IS_EXEC, NULL))
      runloop_ctl(RUNLOOP_CTL_UNSET_EXEC, NULL);

   if (!runloop_ctl(RUNLOOP_CTL_IS_CORE_SHUTDOWN, NULL))
      return -1;

   /* Quits out of RetroArch main loop. */

   settings = config_get_ptr();

   if (settings->load_dummy_on_core_shutdown)
      return runloop_iterate_time_to_exit_load_dummy();

   return -1;
}

#ifdef HAVE_MENU
static int runloop_iterate_menu(enum menu_action action, unsigned *sleep_ms)
{
   menu_ctx_iterate_t iter;
   settings_t *settings    = config_get_ptr();
   bool focused            = runloop_is_focused() &&
      !ui_companion_is_on_foreground();
   bool is_idle            = runloop_ctl(RUNLOOP_CTL_IS_IDLE, NULL);

   iter.action             = action;

   if (!menu_driver_ctl(RARCH_MENU_CTL_ITERATE, &iter))
      rarch_ctl(RARCH_CTL_MENU_RUNNING_FINISHED, NULL);

   if (focused || !is_idle)
      menu_driver_ctl(RARCH_MENU_CTL_RENDER, NULL);

   if (!focused || is_idle)
   {
      *sleep_ms = 10;
      return 1;
   }

   if (!settings->menu.throttle_framerate && !settings->fastforward_ratio)
      return 0;

   return -1;
}
#endif

/**
 * runloop_iterate:
 *
 * Run Libretro core in RetroArch for one frame.
 *
 * Returns: 0 on success, 1 if we have to wait until 
 * button input in order to wake up the loop, 
 * -1 if we forcibly quit out of the RetroArch iteration loop.
 **/
int runloop_iterate(unsigned *sleep_ms)
{
   unsigned i;
   event_cmd_state_t    cmd;
   retro_time_t current, target, to_sleep_ms;
   static retro_input_t last_input              = {0};
   event_cmd_state_t   *cmd_ptr                 = &cmd;
   static retro_time_t frame_limit_minimum_time = 0.0;
   static retro_time_t frame_limit_last_time    = 0.0;
   settings_t *settings                         = config_get_ptr();

   cmd.state[1]                                 = last_input;
   cmd.state[0]                                 = input_keys_pressed();
   last_input                                   = cmd.state[0];

   runloop_ctl(RUNLOOP_CTL_UNSET_FRAME_TIME_LAST, NULL);

   if (runloop_ctl(RUNLOOP_CTL_SHOULD_SET_FRAME_LIMIT, NULL))
   {
      struct retro_system_av_info *av_info = 
         video_viewport_get_system_av_info();
      float fastforward_ratio              = 
         (settings->fastforward_ratio == 0.0f) 
         ? 1.0f : settings->fastforward_ratio;

      frame_limit_last_time    = cpu_features_get_time_usec();
      frame_limit_minimum_time = (retro_time_t)roundf(1000000.0f 
            / (av_info->timing.fps * fastforward_ratio));

      runloop_ctl(RUNLOOP_CTL_UNSET_FRAME_LIMIT, NULL);
   }

   if (input_driver_is_flushing_input())
   {
      input_driver_unset_flushing_input();
      if (cmd.state[0].state)
      {
         cmd.state[0].state = 0;

         /* If core was paused before entering menu, evoke
          * pause toggle to wake it up. */
         if (runloop_ctl(RUNLOOP_CTL_IS_PAUSED, NULL))
            BIT64_SET(cmd.state[0].state, RARCH_PAUSE_TOGGLE);
         input_driver_set_flushing_input();
      }
   }
   
   if (runloop_frame_time.callback)
   {
      /* Updates frame timing if frame timing callback is in use by the core.
       * Limits frame time if fast forward ratio throttle is enabled. */

      retro_time_t current     = cpu_features_get_time_usec();
      retro_time_t delta       = current - runloop_frame_time_last;
      bool is_locked_fps       = (runloop_ctl(RUNLOOP_CTL_IS_PAUSED, NULL) ||
                                  input_driver_is_nonblock_state()) |
                                  !!recording_driver_get_data_ptr();


      if (!runloop_frame_time_last || is_locked_fps)
         delta = runloop_frame_time.reference;

      if (!is_locked_fps && runloop_ctl(RUNLOOP_CTL_IS_SLOWMOTION, NULL))
         delta /= settings->slowmotion_ratio;

      runloop_frame_time_last = current;

      if (is_locked_fps)
         runloop_frame_time_last = 0;

      runloop_frame_time.callback(delta);
   }

   cmd.state[2].state      = cmd.state[0].state & ~cmd.state[1].state;  /* trigger  */

   if (runloop_cmd_triggered(cmd_ptr, RARCH_OVERLAY_NEXT))
      command_event(CMD_EVENT_OVERLAY_NEXT, NULL);

   if (runloop_cmd_triggered(cmd_ptr, RARCH_FULLSCREEN_TOGGLE_KEY))
   {
      bool fullscreen_toggled = !runloop_ctl(RUNLOOP_CTL_IS_PAUSED, NULL);
#ifdef HAVE_MENU
      fullscreen_toggled = fullscreen_toggled || 
         menu_driver_ctl(RARCH_MENU_CTL_IS_ALIVE, NULL);
#endif

      if (fullscreen_toggled)
         command_event(CMD_EVENT_FULLSCREEN_TOGGLE, NULL);
   }

   if (runloop_cmd_triggered(cmd_ptr, RARCH_GRAB_MOUSE_TOGGLE))
      command_event(CMD_EVENT_GRAB_MOUSE_TOGGLE, NULL);

#ifdef HAVE_MENU
   if (runloop_cmd_menu_press(cmd_ptr) || 
         rarch_ctl(RARCH_CTL_IS_DUMMY_CORE, NULL))
   {
      if (menu_driver_ctl(RARCH_MENU_CTL_IS_ALIVE, NULL))
      {
         if (rarch_ctl(RARCH_CTL_IS_INITED, NULL) && 
               !rarch_ctl(RARCH_CTL_IS_DUMMY_CORE, NULL))
            rarch_ctl(RARCH_CTL_MENU_RUNNING_FINISHED, NULL);
      }
      else
         rarch_ctl(RARCH_CTL_MENU_RUNNING, NULL);
   }
#endif

#ifdef HAVE_OVERLAY
   runloop_iterate_linefeed_overlay(settings);
#endif

   if (runloop_iterate_time_to_exit(
            runloop_cmd_press(cmd_ptr, RARCH_QUIT_KEY)) != 1)
   {
      frame_limit_last_time = 0.0;
      command_event(CMD_EVENT_QUIT, NULL);
      return -1;
   }


#ifdef HAVE_MENU
   if (menu_driver_ctl(RARCH_MENU_CTL_IS_ALIVE, NULL))
   {
      int ret = runloop_iterate_menu((enum menu_action)
      menu_input_frame_retropad(cmd.state[0], cmd.state[2]),
      sleep_ms);

      if (ret == -1)
         goto end;
      return ret;
   }
#endif

   if (!runloop_check_state(&cmd, &runloop_shader_dir))
   {
      /* RetroArch has been paused. */
      core_poll();
      *sleep_ms = 10;
      return 1;
   }

#if defined(HAVE_THREADS)
   autosave_lock();
#endif

#ifdef HAVE_NETPLAY
   netplay_driver_ctl(RARCH_NETPLAY_CTL_PRE_FRAME, NULL);
#endif

   if (bsv_movie_ctl(BSV_MOVIE_CTL_IS_INITED, NULL))
      bsv_movie_ctl(BSV_MOVIE_CTL_SET_FRAME_START, NULL);

   camera_driver_ctl(RARCH_CAMERA_CTL_POLL, NULL);

   /* Update binds for analog dpad modes. */
   for (i = 0; i < settings->input.max_users; i++)
   {
      if (!settings->input.analog_dpad_mode[i])
         continue;

      input_push_analog_dpad(settings->input.binds[i],
            settings->input.analog_dpad_mode[i]);
      input_push_analog_dpad(settings->input.autoconf_binds[i],
            settings->input.analog_dpad_mode[i]);
   }

   if ((settings->video.frame_delay > 0) && 
         !input_driver_is_nonblock_state())
      retro_sleep(settings->video.frame_delay);

   core_run();

#ifdef HAVE_CHEEVOS
   cheevos_test();
#endif

   for (i = 0; i < settings->input.max_users; i++)
   {
      if (!settings->input.analog_dpad_mode[i])
         continue;

      input_pop_analog_dpad(settings->input.binds[i]);
      input_pop_analog_dpad(settings->input.autoconf_binds[i]);
   }

   if (bsv_movie_ctl(BSV_MOVIE_CTL_IS_INITED, NULL))
      bsv_movie_ctl(BSV_MOVIE_CTL_SET_FRAME_END, NULL);

#ifdef HAVE_NETPLAY
   netplay_driver_ctl(RARCH_NETPLAY_CTL_POST_FRAME, NULL);
#endif

#if defined(HAVE_THREADS)
   autosave_unlock();
#endif

   if (!settings->fastforward_ratio)
      return 0;
#ifdef HAVE_MENU
end:
#endif

   current                        = cpu_features_get_time_usec();
   target                         = frame_limit_last_time + 
      frame_limit_minimum_time;
   to_sleep_ms                    = (target - current) / 1000;

   if (to_sleep_ms > 0)
   {
      *sleep_ms = (unsigned)to_sleep_ms;
      /* Combat jitter a bit. */
      frame_limit_last_time += frame_limit_minimum_time;
      return 1;
   }

   frame_limit_last_time  = cpu_features_get_time_usec();

   return 0;
}
