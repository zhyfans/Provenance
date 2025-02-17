/*  RetroArch - A frontend for libretro.
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
#include <stdint.h>
#include <string.h>

#include "../msg_hash.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Winvalid-source-encoding"
#endif

 /* IMPORTANT:
  * For non-english characters to work without proper unicode support,
  * we need this file to be encoded in ISO 8859-1 (Latin1), not UTF-8.
  * If you save this file as UTF-8, you'll break non-english characters
  * (e.g. German "Umlauts" and Portugese diacritics).
 */
/* DO NOT REMOVE THIS. If it causes build failure, it's because you saved the file as UTF-8. Read the above comment. */
extern const char force_iso_8859_1[sizeof("�������")==7+1 ? 1 : -1];

int menu_hash_get_help_fr_enum(enum msg_hash_enums msg, char *s, size_t len)
{
   int ret = 0;

   (void)sizeof(force_iso_8859_1);

   switch (msg)
   {
      case MSG_UNKNOWN:
      default:
         ret = -1;
         break;
   }

   return ret;
}

int menu_hash_get_help_fr(uint32_t hash, char *s, size_t len)
{
   int ret = 0;

   (void)sizeof(force_iso_8859_1);

   switch (hash)
   {
      case 0:
      default:
         ret = -1;
         break;
   }

   return ret;
}

const char *msg_hash_to_str_fr(enum msg_hash_enums msg)
{
   switch (msg)
   {
      case MSG_PROGRAM:
         return "RetroArch";
      case MSG_MOVIE_RECORD_STOPPED:
         return "Arr�t de l'enregistrement vid�o.";
      case MSG_MOVIE_PLAYBACK_ENDED:
         return "Fin de la lecture vid�o";
      case MSG_AUTOSAVE_FAILED:
         return "Impossible d'activer l'enregistrement automatique.";
      case MSG_NETPLAY_FAILED_MOVIE_PLAYBACK_HAS_STARTED:
         return "Lecture en cours. Impossible d'activer le jeu en r�seau.";
      case MSG_NETPLAY_FAILED:
         return "�chec de l'initialisation du jeu en r�seau";
      case MSG_LIBRETRO_ABI_BREAK:
         return "est compil� avec une version diff�rente de la biblioth�que libretro actuelle.";
      case MSG_REWIND_INIT_FAILED_NO_SAVESTATES:
         return "L'impl�mentation ne supporte pas la sauvegarde d'�tat. Impossible d'activer le retour rapide.";
      case MSG_REWIND_INIT_FAILED_THREADED_AUDIO:
         return "L'impl�mentation utilise audio thread. Impossible d'activer le retour rapide.";
      case MSG_REWIND_INIT_FAILED:
         return "�chec de l'initialisation du tampon pour le retour rapide. Cette fonctionnalit� sera d�sactiv�e.";
      case MSG_REWIND_INIT:
         return "Initialisation du tampon pour le retour rapide avec une taille";
      case MSG_CUSTOM_TIMING_GIVEN:
         return "Temps personnalis� attribu�";
      case MSG_VIEWPORT_SIZE_CALCULATION_FAILED:
         return "�chec du calcul de la taille du visuel ! Utilisation des donn�es brutes. Cela ne fonctionnera probablement pas bien ...";
      case MSG_HW_RENDERED_MUST_USE_POSTSHADED_RECORDING:
         return "Le core Libretro utilise le rendu mat�riel. Obligation d'utiliser �galement l'enregistrement post-shaded.";
      case MSG_RECORDING_TO:
         return "Enregistrement vers";
      case MSG_DETECTED_VIEWPORT_OF:
         return "D�tection du visuel";
      case MSG_TAKING_SCREENSHOT:
         return "R�alisation d'une copie d'�cran.";
      case MSG_FAILED_TO_TAKE_SCREENSHOT:
         return "�chec de la copie d'�cran.";
      case MSG_FAILED_TO_START_RECORDING:
         return "�chec de l'activation de l'enregistrement.";
      case MSG_RECORDING_TERMINATED_DUE_TO_RESIZE:
         return "Enregistrement interrompu � cause du redimensionnement.";
      case MSG_USING_LIBRETRO_DUMMY_CORE_RECORDING_SKIPPED:
         return "Utilisation d'un core libretro simple. Ignore l'enregistrement.";
      case MSG_UNKNOWN:
         return "Inconnu";
      case MSG_LOADING_CONTENT_FILE:
         return "Chargement du contenu";
      case MSG_RECEIVED:
         return "Re�u";
      case MSG_UNRECOGNIZED_COMMAND:
         return "Commande non reconnue";
      case MSG_SENDING_COMMAND:
         return "Envoi commande";
      case MSG_GOT_INVALID_DISK_INDEX:
         return "Index du disque invalide.";
      case MSG_FAILED_TO_REMOVE_DISK_FROM_TRAY:
         return "Impossible d'�jecter le disque du lecteur.";
      case MSG_REMOVED_DISK_FROM_TRAY:
         return "�jection du disque du lecteur.";
      case MSG_VIRTUAL_DISK_TRAY:
         return "Lecteur de disque virtuel.";
      case MSG_FAILED_TO:
         return "�chec de";
      case MSG_TO:
         return "de";
      case MSG_SAVING_RAM_TYPE:
         return "Sauvegarde du type de RAM";
      case MSG_SAVING_STATE:
         return "Sauvegarde savestate";
      case MSG_LOADING_STATE:
         return "Chargement savestate";
      case MSG_FAILED_TO_LOAD_MOVIE_FILE:
         return "�chec du chargement du fichier vid�o";
      case MSG_FAILED_TO_LOAD_CONTENT:
         return "�chec du chargement du fichier";
      case MSG_COULD_NOT_READ_CONTENT_FILE:
         return "Impossible de lire le contenu du fichier";
      case MSG_GRAB_MOUSE_STATE:
         return "Capture la souris";
      case MSG_PAUSED:
         return "Pause.";
      case MSG_UNPAUSED:
         return "Relanc�.";
      case MSG_FAILED_TO_LOAD_OVERLAY:
         return "Impossible de charger l'overlay.";
      case MSG_FAILED_TO_UNMUTE_AUDIO:
         return "Impossible de remettre le son.";
      case MSG_AUDIO_MUTED:
         return "Son coup�.";
      case MSG_AUDIO_UNMUTED:
         return "Remise du son.";
      case MSG_RESET:
         return "Reset";
      case MSG_FAILED_TO_LOAD_STATE:
         return "Impossible de charger la savestate � partir de";
      case MSG_FAILED_TO_SAVE_STATE_TO:
         return "Impossible de sauvegarder la savestate vers";
      case MSG_FAILED_TO_UNDO_LOAD_STATE:
         return "Aucun savestate de retour arri�re trouv�";
      case MSG_FAILED_TO_UNDO_SAVE_STATE:
         return "Impossible de sauvegarder les informations de savestate de retour arri�re";
      case MSG_FAILED_TO_SAVE_SRAM:
         return "Impossible de sauvegarder la SRAM";
      case MSG_STATE_SIZE:
         return "Taille savestate";
      case MSG_FOUND_SHADER:
         return "Shader trouv�";
      case MSG_SRAM_WILL_NOT_BE_SAVED:
         return "SRAM ne sera pas sauvegard�e.";
      case MSG_BLOCKING_SRAM_OVERWRITE:
         return "Bloque l'�crasement de la SRAM";
      case MSG_CORE_DOES_NOT_SUPPORT_SAVESTATES:
         return "Le core ne supporte pas les savestates.";
      case MSG_SAVED_STATE_TO_SLOT:
         return "Savestate vers slot";
      case MSG_SAVED_SUCCESSFULLY_TO:
         return "Sauvegarde r�ussie vers";
      case MSG_BYTES:
         return "octets";
      case MSG_CONFIG_DIRECTORY_NOT_SET:
         return "R�pertoire de configuration non d�fini. Impossible de sauvegarder le nouveau fichier.";
      case MSG_SKIPPING_SRAM_LOAD:
         return "Ignore le chargement de la SRAM.";
      case MSG_APPENDED_DISK:
         return "Disque fusionn�";
      case MSG_STARTING_MOVIE_PLAYBACK:
         return "D�marrage de la lecture vid�o.";
      case MSG_FAILED_TO_REMOVE_TEMPORARY_FILE:
         return "Impossible de supprimer le fichier temporaire";
      case MSG_REMOVING_TEMPORARY_CONTENT_FILE:
         return "Suppression du fichier temporaire";
      case MSG_LOADED_STATE_FROM_SLOT:
         return "Chargement du savestate � partir du slot";
      case MSG_COULD_NOT_PROCESS_ZIP_FILE:
         return "Impossible de traiter le fichier ZIP.";
      case MSG_SCANNING_OF_DIRECTORY_FINISHED:
         return "Analyse des dossiers termin�e";
      case MSG_SCANNING:
         return "Analyse";
      case MSG_REDIRECTING_CHEATFILE_TO:
         return "Redirection du fichier triche vers";
      case MSG_REDIRECTING_SAVEFILE_TO:
         return "Redirection de la sauvegarde vers";
      case MSG_REDIRECTING_SAVESTATE_TO:
         return "Redirection de la savestate vers";
      case MSG_SHADER:
         return "Shader";
      case MSG_APPLYING_SHADER:
         return "Application du shader";
      case MSG_FAILED_TO_APPLY_SHADER:
         return "Impossible d'appliquer le shader.";
      case MSG_STARTING_MOVIE_RECORD_TO:
         return "D�marrage de l'enregistrement vid�o vers";
      case MSG_FAILED_TO_START_MOVIE_RECORD:
         return "Impossible de d�marrer l'enregistrement vid�o.";
      case MSG_STATE_SLOT:
         return "State slot";
      case MSG_RESTARTING_RECORDING_DUE_TO_DRIVER_REINIT:
         return "Red�marrage de l'enregistrement � cause de la r�initialisation du pilote.";
      case MSG_SLOW_MOTION:
         return "Ralenti.";
      case MSG_SLOW_MOTION_REWIND:
         return "Rembobinage ralenti.";
      case MSG_REWINDING:
         return "Rembobinage.";
      case MSG_REWIND_REACHED_END:
         return "Atteinte de la fin du tampon de rembobinage.";
      case MSG_CHEEVOS_HARDCORE_MODE_ENABLE:
         return "Mode mat�riel activ� : savestate et rembobinage sont d�sactiv�s.";
      case MSG_TASK_FAILED:
         return "�chec";
      case MSG_DOWNLOADING:
         return "T�l�chargement";
      case MSG_EXTRACTING:
         return "Extraction";
      case MENU_ENUM_LABEL_VALUE_OVERLAY_AUTOLOAD_PREFERRED:
         return "Charger l'overlay pr�f�r� automatiquement";
      case MENU_ENUM_LABEL_VALUE_UPDATE_CORE_INFO_FILES:
         return "Mettre � jour les informations des coeurs";
      case MENU_ENUM_LABEL_VALUE_DOWNLOAD_CORE_CONTENT:
         return "T�l�charger du contenu";
      case MENU_ENUM_LABEL_VALUE_SCAN_THIS_DIRECTORY:
         return "<Scanner ce dossier>";
      case MENU_ENUM_LABEL_VALUE_SCAN_FILE:
         return "Scanner un fichier";
      case MENU_ENUM_LABEL_VALUE_SCAN_DIRECTORY:
         return "Scanner un dossier";
      case MENU_ENUM_LABEL_VALUE_ADD_CONTENT_LIST:
         return "Ajouter du contenu";
      case MENU_ENUM_LABEL_VALUE_INFORMATION_LIST:
         return "Informations";
      case MENU_ENUM_LABEL_VALUE_USE_BUILTIN_PLAYER:
         return "Utiliser le lecteur vid�o embarqu�";
      case MENU_ENUM_LABEL_VALUE_CONTENT_SETTINGS:
         return "Menu rapide";
      case MENU_ENUM_LABEL_VALUE_RDB_ENTRY_CRC32:
         return "CRC32";
      case MENU_ENUM_LABEL_VALUE_RDB_ENTRY_MD5:
         return "MD5";
      case MENU_ENUM_LABEL_VALUE_LOAD_CONTENT_LIST:
         return "Charger du contenu";
      case MENU_ENUM_LABEL_VALUE_LOAD_ARCHIVE:
         return "Charger l'archive";
      case MENU_ENUM_LABEL_VALUE_OPEN_ARCHIVE:
         return "Ouvrir l'archive";
      case MENU_ENUM_LABEL_VALUE_ASK_ARCHIVE:
         return "Demander";
      case MENU_ENUM_LABEL_VALUE_PRIVACY_SETTINGS:
         return "Confidentialit�";
      case MENU_ENUM_LABEL_VALUE_HORIZONTAL_MENU:
         return "Horizontal Menu";
      case MENU_ENUM_LABEL_VALUE_NO_SETTINGS_FOUND:
         return "Pas de r�glages trouv�s.";
      case MENU_ENUM_LABEL_VALUE_NO_PERFORMANCE_COUNTERS:
         return "Pas de compteurs de performance.";
      case MENU_ENUM_LABEL_VALUE_DRIVER_SETTINGS:
         return "Pilotes";
      case MENU_ENUM_LABEL_VALUE_CONFIGURATION_SETTINGS:
         return "Configurations";
      case MENU_ENUM_LABEL_VALUE_CORE_SETTINGS:
         return "Coeurs";
      case MENU_ENUM_LABEL_VALUE_VIDEO_SETTINGS:
         return "Vid�o";
      case MENU_ENUM_LABEL_VALUE_LOGGING_SETTINGS:
         return "Journaux";
      case MENU_ENUM_LABEL_VALUE_SAVING_SETTINGS:
         return "Sauvegardes";
      case MENU_ENUM_LABEL_VALUE_REWIND_SETTINGS:
         return "Rembobinage";
      case MENU_ENUM_LABEL_VALUE_SHADER:
         return "Shader";
      case MENU_ENUM_LABEL_VALUE_CHEAT:
         return "Triche";
      case MENU_ENUM_LABEL_VALUE_USER:
         return "Utilisateur";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_BGM_ENABLE:
         return "Musique du syst�me activ�e";
      case MENU_ENUM_LABEL_VALUE_RETROPAD:
         return "RetroPad";
      case MENU_ENUM_LABEL_VALUE_RETROKEYBOARD:
         return "RetroKeyboard";
      case MENU_ENUM_LABEL_AUDIO_BLOCK_FRAMES:
         return "audio_block_frames";
      case MENU_ENUM_LABEL_VALUE_AUDIO_BLOCK_FRAMES:
         return "Block Frames";
      case MENU_ENUM_LABEL_VALUE_INPUT_DESCRIPTOR_LABEL_SHOW: /* FIXME/UPDATE */
         return "Afficher les remaps du coeur";
      case MENU_ENUM_LABEL_VALUE_INPUT_DESCRIPTOR_HIDE_UNBOUND:
         return "Cacher les remaps non mapp�s des coeurs";
      case MENU_ENUM_LABEL_VALUE_VIDEO_FONT_ENABLE:
         return "Afficher les messages d'info";
      case MENU_ENUM_LABEL_VALUE_VIDEO_FONT_PATH:
         return "Police des messages d'info";
      case MENU_ENUM_LABEL_VALUE_VIDEO_FONT_SIZE:
         return "Taille du texte des messages";
      case MENU_ENUM_LABEL_VALUE_VIDEO_MESSAGE_POS_X:
         return "Position X";
      case MENU_ENUM_LABEL_VALUE_VIDEO_MESSAGE_POS_Y:
         return "Position Y";
      case MENU_ENUM_LABEL_VALUE_VIDEO_SOFT_FILTER:
         return "Filtre doux activ�";
      case MENU_ENUM_LABEL_VIDEO_FILTER_FLICKER:
         return "video_filter_flicker";
      case MENU_ENUM_LABEL_VALUE_VIDEO_FILTER_FLICKER:
         return "Filtre anti-scintillement";
      case MENU_ENUM_LABEL_VALUE_DIRECTORY_CONTENT:
         return "<Dossier de contenu>";
      case MENU_ENUM_LABEL_VALUE_UNKNOWN:
         return "Inconnu";
      case MENU_ENUM_LABEL_VALUE_DONT_CARE:
         return "Peu importe";
      case MENU_ENUM_LABEL_VALUE_LINEAR:
         return "Lin�aire";
      case MENU_ENUM_LABEL_VALUE_NEAREST:
         return "Au plus proche";
      case MENU_ENUM_LABEL_VALUE_DIRECTORY_DEFAULT:
         return "<Par d�faut>";
      case MENU_ENUM_LABEL_VALUE_DIRECTORY_NONE:
         return "<Aucun>";
      case MENU_ENUM_LABEL_VALUE_NOT_AVAILABLE:
         return "Indisponible";
      case MENU_ENUM_LABEL_VALUE_INPUT_REMAPPING_DIRECTORY:
         return "Dossier de remaps d'entr�es";
      case MENU_ENUM_LABEL_VALUE_JOYPAD_AUTOCONFIG_DIR:
         return "Dossier des autoconfigs d'entr�es";
      case MENU_ENUM_LABEL_VALUE_RECORDING_CONFIG_DIRECTORY:
         return "Dossier des r�glages de capture vid�o";
      case MENU_ENUM_LABEL_VALUE_RECORDING_OUTPUT_DIRECTORY:
         return "Dossier d'enregistrement des vid�os";
      case MENU_ENUM_LABEL_VALUE_SCREENSHOT_DIRECTORY:
         return "Dossier des captures d'�cran";
      case MENU_ENUM_LABEL_VALUE_PLAYLIST_DIRECTORY:
         return "Dossier des playlists";
      case MENU_ENUM_LABEL_VALUE_SAVEFILE_DIRECTORY:
         return "Dossier des sauvegardes";
      case MENU_ENUM_LABEL_VALUE_SAVESTATE_DIRECTORY:
         return "Dossier des sauvegardes rapides";
      case MENU_ENUM_LABEL_VALUE_STDIN_CMD_ENABLE:
         return "Commandes stdin";
      case MENU_ENUM_LABEL_VALUE_VIDEO_DRIVER:
         return "Pilote vid�o";
      case MENU_ENUM_LABEL_VALUE_RECORD_ENABLE:
         return "Autoriser les captures vid�o";
      case MENU_ENUM_LABEL_VALUE_VIDEO_GPU_RECORD:
         return "Captures vid�o via le GPU";
      case MENU_ENUM_LABEL_VALUE_RECORD_PATH: /* FIXME/UPDATE */
         return "Chemin de l'enregistrement";
      case MENU_ENUM_LABEL_VALUE_RECORD_USE_OUTPUT_DIRECTORY:
         return "Utiliser le dossier d'enregistrement";
      case MENU_ENUM_LABEL_VALUE_RECORD_CONFIG:
         return "Configuration de capture";
      case MENU_ENUM_LABEL_VALUE_VIDEO_POST_FILTER_RECORD:
         return "Activer les filtres de traitement";
      case MENU_ENUM_LABEL_VALUE_CORE_ASSETS_DIRECTORY:
         return "Dossier des t�l�chargements";
      case MENU_ENUM_LABEL_VALUE_ASSETS_DIRECTORY:
         return "Dossier des assets";
      case MENU_ENUM_LABEL_VALUE_DYNAMIC_WALLPAPERS_DIRECTORY:
         return "Dossier des fonds d'�cran dynamiques";
      case MENU_ENUM_LABEL_VALUE_THUMBNAILS_DIRECTORY:
         return "Dossier des vignettes";
      case MENU_ENUM_LABEL_VALUE_RGUI_BROWSER_DIRECTORY:
         return "Dossier racine de navigation";
      case MENU_ENUM_LABEL_VALUE_RGUI_CONFIG_DIRECTORY:
         return "Dossier des fichiers de configuration";
      case MENU_ENUM_LABEL_VALUE_LIBRETRO_INFO_PATH:
         return "Dossier des informations des coeurs";
      case MENU_ENUM_LABEL_VALUE_LIBRETRO_DIR_PATH:
         return "Dossier des coeurs";
      case MENU_ENUM_LABEL_VALUE_CURSOR_DIRECTORY:
         return "Dossier des curseurs";
      case MENU_ENUM_LABEL_VALUE_CONTENT_DATABASE_DIRECTORY:
         return "Dossier des bases de donn�es de contenus";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_DIRECTORY:
         return "Dossier syst�me/BIOS";
      case MENU_ENUM_LABEL_VALUE_CHEAT_DATABASE_PATH:
         return "Dossier des fichiers de triche";
      case MENU_ENUM_LABEL_VALUE_CACHE_DIRECTORY: /* FIXME/UPDATE */
         return "Dossier d'extraction";
      case MENU_ENUM_LABEL_VALUE_AUDIO_FILTER_DIR:
         return "Dossier des filtres audio";
      case MENU_ENUM_LABEL_VALUE_VIDEO_SHADER_DIR:
         return "Dossier des shaders vid�o";
      case MENU_ENUM_LABEL_VALUE_VIDEO_FILTER_DIR:
         return "Dossier des filtres vid�o";
      case MENU_ENUM_LABEL_VALUE_OVERLAY_DIRECTORY:
         return "Dossier des overlays";
      case MENU_ENUM_LABEL_VALUE_OSK_OVERLAY_DIRECTORY:
         return "Dossier des overlays claviers";
      case MENU_ENUM_LABEL_VALUE_NETPLAY_CLIENT_SWAP_INPUT:
         return "Inverser les entr�es du jeu en r�seau";
      case MENU_ENUM_LABEL_VALUE_NETPLAY_SPECTATOR_MODE_ENABLE:
         return "Activer le mode spectateur";
      case MENU_ENUM_LABEL_VALUE_NETPLAY_IP_ADDRESS:
         return "Adresse IP";
      case MENU_ENUM_LABEL_VALUE_NETPLAY_TCP_UDP_PORT:
         return "Port TCP/UDP du jeu en r�seau";
      case MENU_ENUM_LABEL_VALUE_NETPLAY_ENABLE:
         return "Autoriser le jeu en r�seau";
      case MENU_ENUM_LABEL_VALUE_NETPLAY_DELAY_FRAMES:
         return "Netplay Delay Frames";
      case MENU_ENUM_LABEL_VALUE_NETPLAY_MODE:
         return "Activer le mode client";
      case MENU_ENUM_LABEL_VALUE_RGUI_SHOW_START_SCREEN:
         return "Afficher l'�cran de d'aide";
      case MENU_ENUM_LABEL_VALUE_TITLE_COLOR:
         return "Couleur du titre du menu";
      case MENU_ENUM_LABEL_VALUE_ENTRY_HOVER_COLOR:
         return "Couleur de l'entr�e active";
      case MENU_ENUM_LABEL_VALUE_TIMEDATE_ENABLE:
         return "Afficher la date et l'heure";
      case MENU_ENUM_LABEL_VALUE_THREADED_DATA_RUNLOOP_ENABLE:
         return "Boucle de donn�es thread�e";
      case MENU_ENUM_LABEL_VALUE_ENTRY_NORMAL_COLOR:
         return "Couleur des entr�es du menu";
      case MENU_ENUM_LABEL_VALUE_SHOW_ADVANCED_SETTINGS:
         return "Afficher les r�glages avanc�s";
      case MENU_ENUM_LABEL_VALUE_COLLAPSE_SUBGROUPS_ENABLE:
         return "Fusionner les sous-groupes";
      case MENU_ENUM_LABEL_VALUE_MOUSE_ENABLE:
         return "Support de la souris";
      case MENU_ENUM_LABEL_VALUE_POINTER_ENABLE:
         return "Support du tactile";
      case MENU_ENUM_LABEL_VALUE_CORE_ENABLE:
         return "Afficher le coeur actuel";
      case MENU_ENUM_LABEL_VALUE_DPI_OVERRIDE_ENABLE:
         return "Personnaliser le DPI";
      case MENU_ENUM_LABEL_VALUE_DPI_OVERRIDE_VALUE:
         return "Valeur du DPI personnalis�";
      case MENU_ENUM_LABEL_VALUE_SUSPEND_SCREENSAVER_ENABLE:
         return "D�sactiver l'�conomiseur d'�cran";
      case MENU_ENUM_LABEL_VALUE_VIDEO_DISABLE_COMPOSITION:
         return "D�sactiver le compositeur du bureau";
      case MENU_ENUM_LABEL_VALUE_PAUSE_NONACTIVE:
         return "Ne pas fonctionner en arri�re-plan";
      case MENU_ENUM_LABEL_VALUE_UI_COMPANION_START_ON_BOOT:
         return "UI Companion Start On Boot";
      case MENU_ENUM_LABEL_VALUE_UI_MENUBAR_ENABLE:
         return "Menubar";
      case MENU_ENUM_LABEL_VALUE_ARCHIVE_MODE:
         return "Mode d'ouverture des archives";
      case MENU_ENUM_LABEL_VALUE_NETWORK_CMD_ENABLE:
         return "Commandes r�seau";
      case MENU_ENUM_LABEL_VALUE_NETWORK_CMD_PORT:
         return "Port des commandes r�seau";
      case MENU_ENUM_LABEL_VALUE_HISTORY_LIST_ENABLE:
         return "Afficher l'historique";
      case MENU_ENUM_LABEL_VALUE_CONTENT_HISTORY_SIZE:
         return "Taille de l'historique";
      case MENU_ENUM_LABEL_VALUE_VIDEO_REFRESH_RATE_AUTO:
         return "Fr�quence estim�e de l'�cran";
      case MENU_ENUM_LABEL_VALUE_DUMMY_ON_CORE_SHUTDOWN:
         return "Utiliser un faux coeur lorsqu'il n'y en a pas";
      case MENU_ENUM_LABEL_VALUE_CORE_SET_SUPPORTS_NO_CONTENT_ENABLE: /* TODO/FIXME */
         return "Ne pas d�marrer de coeur automatiquement";
      case MENU_ENUM_LABEL_VALUE_FRAME_THROTTLE_ENABLE:
         return "Limiter la vitesse d'ex�cution";
      case MENU_ENUM_LABEL_VALUE_FASTFORWARD_RATIO:
         return "Vitesse de l'avance rapide";
      case MENU_ENUM_LABEL_VALUE_AUTO_REMAPS_ENABLE:
         return "Charger les fichiers remaps automatiquement";
      case MENU_ENUM_LABEL_VALUE_SLOWMOTION_RATIO:
         return "Taux de ralentissement";
      case MENU_ENUM_LABEL_VALUE_CORE_SPECIFIC_CONFIG:
         return "Avoir une configuration par-coeur";
      case MENU_ENUM_LABEL_VALUE_GAME_SPECIFIC_OPTIONS:
         return "Options du coeur par-jeu";
      case MENU_ENUM_LABEL_VALUE_AUTO_OVERRIDES_ENABLE:
         return "Charger les fichiers d'override automatiquement";
      case MENU_ENUM_LABEL_VALUE_CONFIG_SAVE_ON_EXIT:
         return "Sauver la config en quittant";
      case MENU_ENUM_LABEL_VALUE_VIDEO_SMOOTH:
         return "Filtre bilin�aire (HW)";
      case MENU_ENUM_LABEL_VALUE_VIDEO_GAMMA:
         return "Gamma";
      case MENU_ENUM_LABEL_VALUE_VIDEO_ALLOW_ROTATE:
         return "Autoriser la rotation";
      case MENU_ENUM_LABEL_VALUE_VIDEO_HARD_SYNC:
         return "Synchroniser le GPU au CPU";
      case MENU_ENUM_LABEL_VALUE_VIDEO_SWAP_INTERVAL:
         return "Intervale de synchronisation verticale";
      case MENU_ENUM_LABEL_VALUE_VIDEO_VSYNC:
         return "Synchronisation verticale";
      case MENU_ENUM_LABEL_VALUE_VIDEO_THREADED:
         return "Threader l'affichage";
      case MENU_ENUM_LABEL_VALUE_VIDEO_ROTATION:
         return "Rotation";
      case MENU_ENUM_LABEL_VALUE_VIDEO_GPU_SCREENSHOT:
         return "Activer les captures d'�cran GPU";
      case MENU_ENUM_LABEL_VALUE_VIDEO_CROP_OVERSCAN:
         return "Tronquer l'overscan (Reload)";
      case MENU_ENUM_LABEL_VALUE_VIDEO_ASPECT_RATIO_INDEX:
         return "Rapport d'aspect";
      case MENU_ENUM_LABEL_VALUE_VIDEO_ASPECT_RATIO_AUTO:
         return "Format d'image automatique";
      case MENU_ENUM_LABEL_VALUE_VIDEO_FORCE_ASPECT:
         return "Forcer le format d'image";
      case MENU_ENUM_LABEL_VALUE_VIDEO_REFRESH_RATE:
         return "Fr�quence de rafraichissement";
      case MENU_ENUM_LABEL_VALUE_VIDEO_FORCE_SRGB_DISABLE:
         return "D�sactiver sRGB FBO";
      case MENU_ENUM_LABEL_VALUE_VIDEO_WINDOWED_FULLSCREEN:
         return "Mode plein �cran fen�tr�";
      case MENU_ENUM_LABEL_VALUE_PAL60_ENABLE:
         return "Utiliser le mode PAL60";
      case MENU_ENUM_LABEL_VALUE_VIDEO_VFILTER:
         return "Deflicker"; /* TODO */
      case MENU_ENUM_LABEL_VALUE_VIDEO_VI_WIDTH:
         return "Set VI Screen Width"; /* TODO */
      case MENU_ENUM_LABEL_VALUE_VIDEO_BLACK_FRAME_INSERTION:
         return "Ins�rer des frames noires";
      case MENU_ENUM_LABEL_VALUE_SORT_SAVEFILES_ENABLE:
         return "Classer les sauvegardes par dossier";
      case MENU_ENUM_LABEL_VALUE_SORT_SAVESTATES_ENABLE:
         return "Classer les savestates par dossier";
      case MENU_ENUM_LABEL_VALUE_VIDEO_FULLSCREEN:
         return "Plein �cran";
      case MENU_ENUM_LABEL_VALUE_VIDEO_SCALE:
         return "Zoom (en fen�tre)";
      case MENU_ENUM_LABEL_VALUE_VIDEO_SCALE_INTEGER:
         return "Aligner aux pixels de l'�cran";
      case MENU_ENUM_LABEL_VALUE_PERFCNT_ENABLE:
         return "Compteurs de performance";
      case MENU_ENUM_LABEL_VALUE_LIBRETRO_LOG_LEVEL:
         return "Verbosit� des journaux des coeurs";
      case MENU_ENUM_LABEL_VALUE_LOG_VERBOSITY:
         return "Verbosit� des journaux";
      case MENU_ENUM_LABEL_VALUE_SAVESTATE_AUTO_LOAD:
         return "Charger automatiquement les savestates";
      case MENU_ENUM_LABEL_VALUE_SAVESTATE_AUTO_INDEX:
         return "Indexer automatiquement les savestates";
      case MENU_ENUM_LABEL_VALUE_SAVESTATE_AUTO_SAVE:
         return "Sauvegarde automatique";
      case MENU_ENUM_LABEL_VALUE_AUTOSAVE_INTERVAL:
         return "Intervale de sauvegarde SaveRAM";
      case MENU_ENUM_LABEL_VALUE_BLOCK_SRAM_OVERWRITE:
         return "Ne pas �craser la SaveRAM en chargeant la savestate";
      case MENU_ENUM_LABEL_VALUE_VIDEO_SHARED_CONTEXT:
         return "Partager le contexte mat�riel";
      case MENU_ENUM_LABEL_VALUE_RESTART_RETROARCH:
         return "Red�marrer RetroArch";
      case MENU_ENUM_LABEL_VALUE_NETPLAY_NICKNAME:
         return "Nom d'utilisateur";
      case MENU_ENUM_LABEL_VALUE_USER_LANGUAGE:
         return "Langage";
      case MENU_ENUM_LABEL_VALUE_CAMERA_ALLOW:
         return "Autoriser la cam�ra";
      case MENU_ENUM_LABEL_VALUE_LOCATION_ALLOW:
         return "Autoriser la localisation";
      case MENU_ENUM_LABEL_VALUE_PAUSE_LIBRETRO:
         return "Pauser le contenu quand le menu est activ�";
      case MENU_ENUM_LABEL_VALUE_INPUT_OSK_OVERLAY_ENABLE:
         return "Afficher l'overlay clavier";
      case MENU_ENUM_LABEL_VALUE_INPUT_OVERLAY_ENABLE:
         return "Activer les overlays";
      case MENU_ENUM_LABEL_VALUE_VIDEO_MONITOR_INDEX:
         return "�cran";
      case MENU_ENUM_LABEL_VALUE_VIDEO_FRAME_DELAY:
         return "D�layer les frames";
      case MENU_ENUM_LABEL_VALUE_INPUT_DUTY_CYCLE:
         return "Rapport de cycle";
      case MENU_ENUM_LABEL_VALUE_INPUT_TURBO_PERIOD:
         return "D�lai du turbo";
      case MENU_ENUM_LABEL_VALUE_INPUT_AXIS_THRESHOLD:
         return "Seuil des axes";
      case MENU_ENUM_LABEL_VALUE_INPUT_REMAP_BINDS_ENABLE:
         return "Autoriser le remapping des entr�es";
      case MENU_ENUM_LABEL_VALUE_INPUT_MAX_USERS:
         return "Nombre d'utilisateurs";
      case MENU_ENUM_LABEL_VALUE_INPUT_AUTODETECT_ENABLE:
         return "Activer l'autoconfiguration";
      case MENU_ENUM_LABEL_VALUE_AUDIO_OUTPUT_RATE:
         return "Fr�quence de sortie (KHz)";
      case MENU_ENUM_LABEL_VALUE_AUDIO_MAX_TIMING_SKEW:
         return "Limite max de l'ajustement";
      case MENU_ENUM_LABEL_VALUE_CHEAT_NUM_PASSES:
         return "Nombre de passages";
      case MENU_ENUM_LABEL_VALUE_REMAP_FILE_SAVE_CORE:
         return "Charger un fichier remaps de coeur";
      case MENU_ENUM_LABEL_VALUE_REMAP_FILE_SAVE_GAME:
         return "Charger un fichier remap de contenu";
      case MENU_ENUM_LABEL_VALUE_CHEAT_APPLY_CHANGES:
         return "Appliquer les changements";
      case MENU_ENUM_LABEL_VALUE_SHADER_APPLY_CHANGES:
         return "Appliquer les changements";
      case MENU_ENUM_LABEL_VALUE_REWIND_ENABLE:
         return "Activer le rembobinage";
      case MENU_ENUM_LABEL_VALUE_CONTENT_COLLECTION_LIST:
         return "Via les collections";
      case MENU_ENUM_LABEL_VALUE_DETECT_CORE_LIST:
         return "Via les fichiers (d�tecter le coeur)";
      case MENU_ENUM_LABEL_VALUE_DOWNLOADED_FILE_DETECT_CORE_LIST:
         return "Via les t�l�chargements (d�tecter le coeur)";
      case MENU_ENUM_LABEL_VALUE_LOAD_CONTENT_HISTORY:
         return "R�cemment ouvert";
      case MENU_ENUM_LABEL_VALUE_AUDIO_ENABLE:
         return "Activer le son";
      case MENU_ENUM_LABEL_VALUE_FPS_SHOW:
         return "Afficher le FPS";
      case MENU_ENUM_LABEL_VALUE_AUDIO_MUTE:
         return "Muet";
      case MENU_ENUM_LABEL_VALUE_AUDIO_VOLUME:
         return "Volume sonnore (dB)";
      case MENU_ENUM_LABEL_VALUE_AUDIO_SYNC:
         return "Synchroniser le son";
      case MENU_ENUM_LABEL_VALUE_AUDIO_RATE_CONTROL_DELTA:
         return "Delta du taux de contr�le";
      case MENU_ENUM_LABEL_VALUE_VIDEO_SHADER_NUM_PASSES:
         return "Nombre de passages";
      case MENU_ENUM_LABEL_VALUE_CONFIGURATIONS:
         return "Charger une configuration";
      case MENU_ENUM_LABEL_VALUE_REWIND_GRANULARITY:
         return "Pr�cision du rembobinage";
      case MENU_ENUM_LABEL_VALUE_REMAP_FILE_LOAD:
         return "Charger un fichier de remap";
      case MENU_ENUM_LABEL_VALUE_CUSTOM_RATIO:
         return "Forcer une r�solution";
      case MENU_ENUM_LABEL_VALUE_USE_THIS_DIRECTORY:
         return "<Choisir ce dossier>";
      case MENU_ENUM_LABEL_VALUE_RDB_ENTRY_START_CONTENT:
         return "Ex�cuter le contenu";
      case MENU_ENUM_LABEL_VALUE_DISK_OPTIONS:
         return "Disques";
      case MENU_ENUM_LABEL_VALUE_CORE_OPTIONS:
         return "Options";
      case MENU_ENUM_LABEL_VALUE_CORE_CHEAT_OPTIONS:
         return "Triche";
      case MENU_ENUM_LABEL_VALUE_TAKE_SCREENSHOT:
         return "Capturer l �cran";
      case MENU_ENUM_LABEL_VALUE_RESUME:
         return "Reprendre";
      case MENU_ENUM_LABEL_VALUE_DISK_INDEX:
         return "Num�ro du disque";
      case MENU_ENUM_LABEL_VALUE_FRONTEND_COUNTERS:
         return "Compteurs du Frontend";
      case MENU_ENUM_LABEL_VALUE_DISK_IMAGE_APPEND:
         return "Ajouter une image de disque";
      case MENU_ENUM_LABEL_VALUE_DISK_CYCLE_TRAY_STATUS:
         return "�tat du lecteur de disque";
      case MENU_ENUM_LABEL_VALUE_NO_PLAYLIST_ENTRIES_AVAILABLE:
         return "Playlist vide.";
      case MENU_ENUM_LABEL_VALUE_NO_CORE_INFORMATION_AVAILABLE:
         return "Pas d'informations disponibles.";
      case MENU_ENUM_LABEL_VALUE_NO_CORE_OPTIONS_AVAILABLE:
         return "Pas d'options disponibles.";
      case MENU_ENUM_LABEL_VALUE_NO_CORES_AVAILABLE:
         return "Aucun coeur disponible.";
      case MENU_ENUM_LABEL_VALUE_NO_CORE:
         return "Aucun coeur";
      case MENU_ENUM_LABEL_VALUE_DATABASE_MANAGER:
         return "Gestion de la base de donn�es";
      case MENU_ENUM_LABEL_VALUE_CURSOR_MANAGER:
         return "Gestion des curseurs";
      case MENU_ENUM_LABEL_VALUE_MAIN_MENU:
         return "Menu principal";
      case MENU_ENUM_LABEL_VALUE_SETTINGS:
         return "R�glages";
      case MENU_ENUM_LABEL_VALUE_QUIT_RETROARCH:
         return "Quitter RetroArch";
      case MENU_ENUM_LABEL_VALUE_SHUTDOWN:
         return "�teindre";
      case MENU_ENUM_LABEL_VALUE_HELP:
         return "Aide";
      case MENU_ENUM_LABEL_VALUE_SAVE_NEW_CONFIG:
         return "Sauvegarder la configuration";
      case MENU_ENUM_LABEL_VALUE_RESTART_CONTENT:
         return "Red�marrer le contenu";
      case MENU_ENUM_LABEL_VALUE_CORE_UPDATER_LIST:
         return "Mise � jour des coeurs";
      case MENU_ENUM_LABEL_VALUE_CORE_UPDATER_BUILDBOT_URL:
         return "URL du buildbot des coeurs";
      case MENU_ENUM_LABEL_VALUE_BUILDBOT_ASSETS_URL:
         return "URL du buildbot des assets";
      case MENU_ENUM_LABEL_VALUE_NAVIGATION_WRAPAROUND:
         return "Saut-retour";
      case MENU_ENUM_LABEL_VALUE_NAVIGATION_BROWSER_FILTER_SUPPORTED_EXTENSIONS_ENABLE:
         return "Filtrer par extentions support�es";
      case MENU_ENUM_LABEL_VALUE_CORE_UPDATER_AUTO_EXTRACT_ARCHIVE:
         return "Extraire automatiquement";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFORMATION:
         return "Informations du syst�me";
      case MENU_ENUM_LABEL_VALUE_ONLINE_UPDATER:
         return "Mises � jour";
      case MENU_ENUM_LABEL_VALUE_CORE_INFORMATION:
         return "Informations sur le coeur";
      case MENU_ENUM_LABEL_VALUE_DIRECTORY_NOT_FOUND:
         return "Dossier non trouv�.";
      case MENU_ENUM_LABEL_VALUE_NO_ITEMS:
         return "Vide.";
      case MENU_ENUM_LABEL_VALUE_CORE_LIST:
         return "Charger un coeur";
      case MENU_ENUM_LABEL_VALUE_LOAD_CONTENT:
         return "Via les fichiers";
      case MENU_ENUM_LABEL_VALUE_CLOSE_CONTENT:
         return "Quitter";
      case MENU_ENUM_LABEL_VALUE_MANAGEMENT:
         return "Gestion avanc�e";
      case MENU_ENUM_LABEL_VALUE_SAVE_STATE:
         return "Sauvegarder une savestate";
      case MENU_ENUM_LABEL_VALUE_LOAD_STATE:
         return "Charger une savestate";
      case MENU_ENUM_LABEL_VALUE_RESUME_CONTENT:
         return "Reprendre";
      case MENU_ENUM_LABEL_VALUE_INPUT_DRIVER:
         return "Pilote des entr�es";
      case MENU_ENUM_LABEL_VALUE_AUDIO_DRIVER:
         return "Pilote audio";
      case MENU_ENUM_LABEL_VALUE_JOYPAD_DRIVER:
         return "Pilote des manettes";
      case MENU_ENUM_LABEL_VALUE_AUDIO_RESAMPLER_DRIVER:
         return "Pilote de r�-�chantillonage audio";
      case MENU_ENUM_LABEL_VALUE_RECORD_DRIVER:
         return "Pilote de capture vid�o";
      case MENU_ENUM_LABEL_VALUE_MENU_DRIVER:
         return "Pilote de menu";
      case MENU_ENUM_LABEL_VALUE_CAMERA_DRIVER:
         return "Pilote de cam�ra";
      case MENU_ENUM_LABEL_VALUE_LOCATION_DRIVER:
         return "Pilote de localisation";
      case MENU_ENUM_LABEL_VALUE_UNABLE_TO_READ_COMPRESSED_FILE:
         return "Impossible de lire l'archive.";
      case MENU_ENUM_LABEL_VALUE_OVERLAY_SCALE:
         return "Zoom de l'overlay";
      case MENU_ENUM_LABEL_VALUE_OVERLAY_PRESET:
         return "Pr�r�glages de l'overlay";
      case MENU_ENUM_LABEL_VALUE_AUDIO_LATENCY:
         return "Latence audio (ms)";
      case MENU_ENUM_LABEL_VALUE_AUDIO_DEVICE:
         return "Carte son";
      case MENU_ENUM_LABEL_VALUE_KEYBOARD_OVERLAY_PRESET:
         return "Pr�r�glages d'overlay clavier";
      case MENU_ENUM_LABEL_VALUE_OVERLAY_OPACITY:
         return "Transparence de l'overlay";
      case MENU_ENUM_LABEL_VALUE_MENU_WALLPAPER:
         return "Fond d'�cran";
      case MENU_ENUM_LABEL_VALUE_DYNAMIC_WALLPAPER:
         return "Fond d'�cran dynamique";
      case MENU_ENUM_LABEL_VALUE_THUMBNAILS:
         return "Vignettes";
      case MENU_ENUM_LABEL_VALUE_CORE_INPUT_REMAPPING_OPTIONS:
         return "Remap d'entr�es";
      case MENU_ENUM_LABEL_VALUE_SHADER_OPTIONS:
         return "Shaders";
      case MENU_ENUM_LABEL_VALUE_NO_SHADER_PARAMETERS:
         return "Aucun param�tres.";
      case MENU_ENUM_LABEL_VALUE_VIDEO_FILTER:
         return "Filtre vid�o";
      case MENU_ENUM_LABEL_VALUE_AUDIO_DSP_PLUGIN:
         return "Module DSP";
      case MENU_ENUM_LABEL_VALUE_STARTING_DOWNLOAD:
         return "T�l�chargement de : ";
      case MENU_ENUM_LABEL_VALUE_OFF:
         return "OFF";
      case MENU_ENUM_LABEL_VALUE_ON:
         return "ON";
      case MENU_ENUM_LABEL_VALUE_UPDATE_ASSETS:
         return "Mettre � jour les assets";
      case MENU_ENUM_LABEL_VALUE_UPDATE_CHEATS:
         return "Mettre � jour les codes de triche";
      case MENU_ENUM_LABEL_VALUE_UPDATE_AUTOCONFIG_PROFILES:
         return "Mettre � jour les profils d'autoconfiguration";
      case MENU_ENUM_LABEL_VALUE_UPDATE_AUTOCONFIG_PROFILES_HID:
         return "Mettre � jour les profils d'autoconfiguration (HID)";
      case MENU_ENUM_LABEL_VALUE_UPDATE_DATABASES:
         return "Mettre � jour les bases de donn�es";
      case MENU_ENUM_LABEL_VALUE_UPDATE_OVERLAYS:
         return "Mettre � jour les overlays";
      case MENU_ENUM_LABEL_VALUE_UPDATE_CG_SHADERS:
         return "Mettre � jour les shaders CG";
      case MENU_ENUM_LABEL_VALUE_UPDATE_GLSL_SHADERS:
         return "Mettre � jour les shaders GLSL";
      case MENU_ENUM_LABEL_VALUE_CORE_INFO_CORE_NAME:
         return "Nom";
      case MENU_ENUM_LABEL_VALUE_CORE_INFO_CORE_LABEL:
         return "Label";
      case MENU_ENUM_LABEL_VALUE_CORE_INFO_SYSTEM_NAME:
         return "Syst�me";
      case MENU_ENUM_LABEL_VALUE_CORE_INFO_SYSTEM_MANUFACTURER:
         return "Fabricant du syst�me";
      case MENU_ENUM_LABEL_VALUE_CORE_INFO_CATEGORIES:
         return "Cat�gories";
      case MENU_ENUM_LABEL_VALUE_CORE_INFO_AUTHORS:
         return "Auteurs";
      case MENU_ENUM_LABEL_VALUE_CORE_INFO_PERMISSIONS:
         return "Permissions";
      case MENU_ENUM_LABEL_VALUE_CORE_INFO_LICENSES:
         return "Licence(s)";
      case MENU_ENUM_LABEL_VALUE_CORE_INFO_SUPPORTED_EXTENSIONS:
         return "Extensions support�es";
      case MENU_ENUM_LABEL_VALUE_CORE_INFO_FIRMWARE:
         return "Firmware";
      case MENU_ENUM_LABEL_VALUE_CORE_INFO_CORE_NOTES:
         return "Notes";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_BUILD_DATE:
         return "Date de build";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_GIT_VERSION:
         return "Version git";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_CPU_FEATURES:
         return "Fonctionnalit�s du CPU";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_FRONTEND_IDENTIFIER:
         return "Identifiant frontend";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_FRONTEND_NAME:
         return "Nom du frontend";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_FRONTEND_OS:
         return "OS du frontend";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_RETRORATING_LEVEL:
         return "Niveau RetroRating";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_POWER_SOURCE:
         return "Alimentation";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_POWER_SOURCE_NO_SOURCE:
         return "Non aliment�";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_POWER_SOURCE_CHARGING:
         return "En chargement";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_POWER_SOURCE_CHARGED:
         return "Charg�";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_POWER_SOURCE_DISCHARGING:
         return "D�charg�";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_VIDEO_CONTEXT_DRIVER:
         return "Pilote du contexte vid�o";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_DISPLAY_METRIC_MM_WIDTH:
         return "Largeur d'�cran (mm)";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_DISPLAY_METRIC_MM_HEIGHT:
         return "Hauteur d'�cran (mm)";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_DISPLAY_METRIC_DPI:
         return "DPI de l'�cran";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_LIBRETRODB_SUPPORT:
         return "Support de libretroDB";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_OVERLAY_SUPPORT:
         return "Support des overlays";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_COMMAND_IFACE_SUPPORT:
         return "Support de l'interface de commandes";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_NETWORK_COMMAND_IFACE_SUPPORT:
         return "Support des commandes r�seau";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_COCOA_SUPPORT:
         return "Support de Cocoa";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_RPNG_SUPPORT:
         return "Support des PNGs (RPNG)";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_SDL_SUPPORT:
         return "Support de SDL1.2";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_SDL2_SUPPORT:
         return "Support de SDL2";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_OPENGL_SUPPORT:
         return "Support d'OpenGL";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_OPENGLES_SUPPORT:
         return "Support d'OpenGL ES";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_THREADING_SUPPORT:
         return "Support du threading";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_KMS_SUPPORT:
         return "Support de KMS/EGL";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_UDEV_SUPPORT:
         return "Support de udev";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_OPENVG_SUPPORT:
         return "Support d'OpenVG";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_EGL_SUPPORT:
         return "Support d'EGL";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_X11_SUPPORT:
         return "Support de X11";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_WAYLAND_SUPPORT:
         return "Support de Wayland";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_XVIDEO_SUPPORT:
         return "Support de XVideo";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_ALSA_SUPPORT:
         return "Support d'ALSA";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_OSS_SUPPORT:
         return "Support d'OSS";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_OPENAL_SUPPORT:
         return "Support d'OpenAL";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_OPENSL_SUPPORT:
         return "Support d'OpenSL";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_RSOUND_SUPPORT:
         return "Support de RSound";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_ROARAUDIO_SUPPORT:
         return "Support de RoarAudio";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_JACK_SUPPORT:
         return "Support de JACK";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_PULSEAUDIO_SUPPORT:
         return "Support de PulseAudio";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_DSOUND_SUPPORT:
         return "Support de DirectSoundt";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_XAUDIO2_SUPPORT:
         return "Support de XAudio2";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_ZLIB_SUPPORT:
         return "Support de Zlib";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_7ZIP_SUPPORT:
         return "Support de 7zip";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_DYLIB_SUPPORT:
         return "Support des biblioth�ques dynamiques";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_CG_SUPPORT:
         return "Support de CG";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_GLSL_SUPPORT:
         return "Support de GLSL";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_HLSL_SUPPORT:
         return "Support de HLSL";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_LIBXML2_SUPPORT:
         return "Support du parser XML libxml2";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_SDL_IMAGE_SUPPORT:
         return "Support de SDL_Image";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_FBO_SUPPORT:
         return "Support d'OpenGL/Direct3D render-to-texture (shaders multi-passages)";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_FFMPEG_SUPPORT:
         return "Support de FFmpeg";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_CORETEXT_SUPPORT:
         return "Support de CoreText";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_FREETYPE_SUPPORT:
         return "Support de FreeType";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_NETPLAY_SUPPORT:
         return "Support du jeu en r�seau";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_PYTHON_SUPPORT:
         return "Support de Python (scripting des shaders)";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_V4L2_SUPPORT:
         return "Support de Video4Linux2";
      case MENU_ENUM_LABEL_VALUE_SYSTEM_INFO_LIBUSB_SUPPORT:
         return "Support de libusb";
      case MENU_ENUM_LABEL_VALUE_YES:
         return "Oui";
      case MENU_ENUM_LABEL_VALUE_NO:
         return "Non";
      case MENU_ENUM_LABEL_VALUE_BACK:
         return "Retour";
      case MENU_ENUM_LABEL_VALUE_SCREEN_RESOLUTION:
         return "R�solution d'�cran";
      case MENU_ENUM_LABEL_VALUE_DISABLED:
         return "D�sactiv�";
      case MENU_ENUM_LABEL_VALUE_PORT:
         return "Port";
      case MENU_ENUM_LABEL_VALUE_NONE:
         return "Aucun";
      case MENU_ENUM_LABEL_VALUE_RDB_ENTRY_DEVELOPER:
         return "D�veloppeur";
      case MENU_ENUM_LABEL_VALUE_RDB_ENTRY_PUBLISHER:
         return "�diteur";
      case MENU_ENUM_LABEL_VALUE_RDB_ENTRY_DESCRIPTION:
         return "Description";
      case MENU_ENUM_LABEL_VALUE_RDB_ENTRY_NAME:
         return "Nom";
      case MENU_ENUM_LABEL_VALUE_RDB_ENTRY_ORIGIN:
         return "Origine";
      case MENU_ENUM_LABEL_VALUE_RDB_ENTRY_FRANCHISE:
         return "Franchise";
      case MENU_ENUM_LABEL_VALUE_RDB_ENTRY_RELEASE_MONTH:
         return "Mois de sortie";
      case MENU_ENUM_LABEL_VALUE_RDB_ENTRY_RELEASE_YEAR:
         return "Ann�e de sortie";
      case MENU_ENUM_LABEL_VALUE_TRUE:
         return "Vrai";
      case MENU_ENUM_LABEL_VALUE_FALSE:
         return "Faux";
      case MENU_ENUM_LABEL_VALUE_MISSING:
         return "Manquant";
      case MENU_ENUM_LABEL_VALUE_PRESENT:
         return "Pr�sent";
      case MENU_ENUM_LABEL_VALUE_OPTIONAL:
         return "Optionnel";
      case MENU_ENUM_LABEL_VALUE_REQUIRED:
         return "Requis";
      case MENU_ENUM_LABEL_VALUE_STATUS:
         return "Statut";
      case MENU_ENUM_LABEL_VALUE_AUDIO_SETTINGS:
         return "Audio";
      case MENU_ENUM_LABEL_VALUE_INPUT_SETTINGS:
         return "Entr�es";
      case MENU_ENUM_LABEL_VALUE_ONSCREEN_DISPLAY_SETTINGS:
         return "Messages d'info";
      case MENU_ENUM_LABEL_VALUE_OVERLAY_SETTINGS:
         return "Overlays";
      case MENU_ENUM_LABEL_VALUE_MENU_SETTINGS:
         return "Menu";
      case MENU_ENUM_LABEL_VALUE_MULTIMEDIA_SETTINGS:
         return "Multim�dia";
      case MENU_ENUM_LABEL_VALUE_USER_INTERFACE_SETTINGS:
         return "Interface graphique";
      case MENU_ENUM_LABEL_VALUE_MENU_FILE_BROWSER_SETTINGS:
         return "Navigateur de fichiers";
      case MENU_ENUM_LABEL_VALUE_CORE_UPDATER_SETTINGS:
         return "Mises � jour";
      case MENU_ENUM_LABEL_VALUE_NETWORK_SETTINGS:
         return "R�seau";
      case MENU_ENUM_LABEL_VALUE_PLAYLIST_SETTINGS:
         return "Playlists";
      case MENU_ENUM_LABEL_VALUE_USER_SETTINGS:
         return "Utilisateur";
      case MENU_ENUM_LABEL_VALUE_DIRECTORY_SETTINGS:
         return "Dossiers";
      case MENU_ENUM_LABEL_VALUE_RECORDING_SETTINGS:
         return "Capture video";
      case MENU_ENUM_LABEL_VALUE_NO_INFORMATION_AVAILABLE:
         return "Pas d'informations disponibles.";
      case MENU_ENUM_LABEL_VALUE_INPUT_USER_BINDS:
         return "Entr�es utilisateur %u";
      case MENU_ENUM_LABEL_VALUE_LANG_ENGLISH:
         return "Anglais";
      case MENU_ENUM_LABEL_VALUE_LANG_JAPANESE:
         return "Japonais";
      case MENU_ENUM_LABEL_VALUE_LANG_FRENCH:
         return "Fran�ais";
      case MENU_ENUM_LABEL_VALUE_LANG_SPANISH:
         return "Espagnol";
      case MENU_ENUM_LABEL_VALUE_LANG_GERMAN:
         return "Allemand";
      case MENU_ENUM_LABEL_VALUE_LANG_ITALIAN:
         return "Italien";
      case MENU_ENUM_LABEL_VALUE_LANG_DUTCH:
         return "N�erlandais";
      case MENU_ENUM_LABEL_VALUE_LANG_PORTUGUESE:
         return "Portuguais";
      case MENU_ENUM_LABEL_VALUE_LANG_RUSSIAN:
         return "Russe";
      case MENU_ENUM_LABEL_VALUE_LANG_KOREAN:
         return "Cor�en";
      case MENU_ENUM_LABEL_VALUE_LANG_CHINESE_TRADITIONAL:
         return "Chinois (Traditionnel)";
      case MENU_ENUM_LABEL_VALUE_LANG_CHINESE_SIMPLIFIED:
         return "Chinois (Simplifi�)";
      case MENU_ENUM_LABEL_VALUE_LANG_ESPERANTO:
         return "Esperanto";
      case MENU_ENUM_LABEL_VALUE_LEFT_ANALOG:
         return "Stick analogique gauche";
      case MENU_ENUM_LABEL_VALUE_RIGHT_ANALOG:
         return "Stick analogique droite";
      case MENU_ENUM_LABEL_VALUE_INPUT_HOTKEY_BINDS:
         return "Racourcis d'entr�es";
      case MENU_ENUM_LABEL_VALUE_FRAME_THROTTLE_SETTINGS:
         return "Vitesse d'affichage";
      case MENU_ENUM_LABEL_VALUE_SEARCH:
         return "Recherche :";
      case MENU_ENUM_LABEL_VALUE_USE_BUILTIN_IMAGE_VIEWER:
         return "Utiliser le lecteur d'image embarqu�";
      case MENU_ENUM_LABEL_VALUE_HELP_LIST:
         return "Aide";
      case MENU_ENUM_LABEL_VALUE_START_CORE:
         return "D�marrer le coeur";
      case MENU_ENUM_LABEL_VALUE_START_NET_RETROPAD:
         return "Mode manette � distance";
      case MENU_ENUM_LABEL_VALUE_SAVE_CURRENT_CONFIG:
         return "Sauvegarder la configuration actuelle";
      case MENU_ENUM_LABEL_VALUE_SETTINGS_TAB:
         return "R�glages";
      case MENU_ENUM_LABEL_VALUE_HISTORY_TAB:
         return "Historique";
      case MENU_ENUM_LABEL_VALUE_ADD_TAB:
         return "Scanner";
      case MENU_ENUM_LABEL_VALUE_DEBUG_PANEL_ENABLE:
         return "Activer le panneau de d�bogage";
      case MENU_ENUM_LABEL_VALUE_INPUT_OVERLAY_HIDE_IN_MENU:
         return "Cacher l'overlay dans le menu";
      case MENU_ENUM_LABEL_CONTENT_HISTORY_SIZE:
         return "Taille de l'historique";
      case MENU_ENUM_LABEL_VALUE_ACCOUNTS_LIST:
         return "Comptes en ligne";
      case MENU_ENUM_LABEL_VALUE_MENU_LINEAR_FILTER:
         return "Filtre lin�aire";
      case MENU_ENUM_LABEL_VALUE_XMB_SCALE_FACTOR:
         return "XMB : Zoom";
      case MENU_ENUM_LABEL_VALUE_XMB_ALPHA_FACTOR:
         return "XMB : Transparence";
      case MENU_ENUM_LABEL_VALUE_XMB_FONT:
         return "XMB : Police";
      case MENU_ENUM_LABEL_VALUE_XMB_THEME:
         return "XMB : Theme";
      case MENU_ENUM_LABEL_VALUE_XMB_MENU_COLOR_THEME:
         return "D�grad� de font d'�cran";
      case MENU_ENUM_LABEL_VALUE_MATERIALUI_MENU_COLOR_THEME:
         return "D�grad� de font d'�cran";
      case MENU_ENUM_LABEL_VALUE_XMB_SHADOWS_ENABLE:
         return "Ombres pour les icones";
      case MENU_ENUM_LABEL_VALUE_XMB_RIBBON_ENABLE:
         return "Font d'�cran anim�";
      case MENU_ENUM_LABEL_VALUE_NETWORK_REMOTE_ENABLE:
         return "Manette r�seau";
      case MENU_ENUM_LABEL_VALUE_RUN:
         return "Lancer";
      case MENU_ENUM_LABEL_VALUE_STATE_SLOT:
         return "Slot de savestate";

      case MENU_ENUM_LABEL_VALUE_UNDO_LOAD_STATE:
         return "Annuler charger une savestate";
      case MENU_ENUM_LABEL_VALUE_UNDO_SAVE_STATE:
         return "Annuler sauvegarder une savestate";
      default:
         break;
   }

   return "null";
}
