/**
 * @file lv_conf.h
 * Configuration file for LVGL
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/*Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888)*/
#define LV_COLOR_DEPTH   16

/*Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface (e.g. SPI)*/
#define LV_COLOR_16_SWAP 0

/*Enable more chroma keying modes. Requires 32-bit color depth.*/
#define LV_COLOR_CHROMA_KEY_HEX \
  0x00ff00 /*Images with this color will not be drawn (with chroma keying)*/

/*=========================
   MEMORY SETTINGS
 *=========================*/

/*1: use custom malloc/free, 0: use the built-in `lv_mem_alloc()` and `lv_mem_free()`*/
#define LV_MEM_CUSTOM 0
#if LV_MEM_CUSTOM == 1
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h> /*Header for the dynamic memory function*/
#define LV_MEM_CUSTOM_ALLOC   malloc     /*Wrapper to malloc*/
#define LV_MEM_CUSTOM_FREE    free       /*Wrapper to free*/
#define LV_MEM_CUSTOM_REALLOC realloc    /*Wrapper to realloc*/
#endif

/*Size of the memory available for `lv_mem_alloc()` in bytes (>= 2kB)*/
#define LV_MEM_SIZE              (32U * 1024U) /*[bytes]*/

/*Set an address for the memory pool instead of allocating it as a normal array. Can be in external
 * SRAM too.*/
#define LV_MEM_ADR               0 /*0: unused*/
/*Instead of an address give a memory allocator that will be called to get a memory pool for LVGL.
 * E.g. a custom allocator*/
#define LV_MEM_POOL_ALLOC        NULL

/*====================
   HAL SETTINGS
 *====================*/

/*Default display refresh period. LVD will redraw changed areas with this period time*/
#define LV_DISP_DEF_REFR_PERIOD  30 /*[ms]*/

/*Input device read period in milliseconds*/
#define LV_INDEV_DEF_READ_PERIOD 30 /*[ms]*/

/*Use a custom tick source.
 *It removes the need to manually update the tick with `lv_tick_inc()`)*/
#define LV_TICK_CUSTOM           0
#if LV_TICK_CUSTOM
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h" /*Header for the system time function*/
#define LV_TICK_CUSTOM_SYS_TIME_EXPR \
  (millis()) /*Expression evaluating to current system time in ms*/
             /*If using lvgl as ESP32 component
              *   #define LV_TICK_CUSTOM_INCLUDE "esp_timer.h"
              *   #define LV_TICK_CUSTOM_SYS_TIME_EXPR ((esp_timer_get_time() / 1000LL))*/
#endif

/*Default Dot Per Inch. Used to initialize default sizes such as widgets sized, style paddings.
 *(Not so important, you can adjust it to modify default sizes and spaces)*/
#define LV_DPI_DEF      130 /*[px/inch]*/

/*=======================
 * FEATURE CONFIGURATION
 *=======================*/

/*-------------
 * Drawing
 *-----------*/

/*Enable complex draw engine.
 *Required to draw shadow, gradient, rounded corners, arc, skew, image transformations or any
 *masks*/
#define LV_DRAW_COMPLEX 1
#if LV_DRAW_COMPLEX != 0
/*Allow buffering some shadow calculation.
 *LV_DRAW_COMPLEX should be 1 for this to work*/
#define LV_SHADOW_CACHE_SIZE 0

/*Set number of maximally cached circle data.
 *The circumference of 1/4 circle are saved for anti-aliasing
 *radius * LV_CIRCLE_CACHE_SIZE for each radius is allocated*/
#define LV_CIRCLE_CACHE_SIZE 4
#endif

/*Default image cache size. Image caching keeps the images opened.
 *If only the built-in image formats are used there is no real advantage of caching.
 *(I.e. no new image decoder is added)
 *With complex image decoders (e.g. PNG or JPG) caching can save the continuous open/decode of
 *images. However the opened images might consume additional RAM. 0: to disable caching*/
#define LV_IMG_CACHE_DEF_SIZE 0

/*Number of stops allowed per gradient. Increase this to allow more stops.
 *This adds (sizeof(lv_color_t) + 1) bytes per additional stop*/
#define LV_GRADIENT_MAX_STOPS 2

/*Default gradient buffer size.
 *When LV_GRAD_CACHE is 0, the buffer size is `LV_GRADIENT_MAX_STOPS * sizeof(lv_color_t)`
 *When LV_GRAD_CACHE is 1, the buffer size is `LV_GRADIENT_MAX_STOPS * sizeof(lv_color_t) * 2`*/
#define LV_GRADIENT_BUF_SIZE  256

/*Allow dithering the gradients (to achieve visual smooth color gradients on limited color depth
 *display) LV_DITHER_GRADIENT implies LV_DITHER_ERROR_DIFFUSION is 0*/
#define LV_DITHER_GRADIENT    0
#if LV_DITHER_GRADIENT
/*Add support for error diffusion dithering.
 *Error diffusion dithering gets a much better visual result, but implies more CPU consumption and
 *memory when drawing. The increase in memory consumption is (32 bits per object)*/
#define LV_DITHER_ERROR_DIFFUSION 0
#endif

/*Maximum buffer size to allocate for rotation.
 *Only used if software rotation is enabled in the display driver.*/
#define LV_DISP_ROT_MAX_BUF    (10 * 1024)

/*-------------
 * GPU
 *-----------*/

/*Use Arm's 2D acceleration library Arm-2D */
#define LV_USE_GPU_ARM2D       0

/*Use STM32's DMA2D (aka Chrom Art) GPU*/
#define LV_USE_GPU_STM32_DMA2D 0
#if LV_USE_GPU_STM32_DMA2D
/*Must be defined to include path of CMSIS header of target processor
e.g. "stm32f769xx.h" or "stm32f429xx.h"*/
#define LV_GPU_DMA2D_CMSIS_INCLUDE
#endif

/*Use SWM341's DMA2D GPU*/
#define LV_USE_GPU_SWM341_DMA2D 0
#if LV_USE_GPU_SWM341_DMA2D
#define LV_GPU_SWM341_DMA2D_INCLUDE "SWM341.h"
#endif

/*Use NXP's PXP GPU iMX RTxxx platforms*/
#define LV_USE_GPU_NXP_PXP 0
#if LV_USE_GPU_NXP_PXP
/*1: Add default bare metal and FreeRTOS interrupt handling routines for PXP (lv_gpu_nxp_pxp_osa.c)
 *   and call lv_gpu_nxp_pxp_init() automatically during lv_init(). Note that symbol
 *SDK_OS_FREE_RTOS has to be defined in order to use FreeRTOS OSA, otherwise bare-metal
 *implementation is selected. 0: lv_gpu_nxp_pxp_init() has to be called manually before lv_init()
 */
#define LV_USE_GPU_NXP_PXP_AUTO_INIT 0
#endif

/*Use NXP's VG-Lite GPU iMX RTxxx platforms*/
#define LV_USE_GPU_NXP_VG_LITE 0

/*Use SDL renderer API*/
#define LV_USE_GPU_SDL         0
#if LV_USE_GPU_SDL
#define LV_GPU_SDL_INCLUDE_PATH      <SDL2/SDL.h>
/*Texture cache size, the number of textures to be cached simultaneously.
 *Increasing this value may improve rendering performance but also increases memory usage.*/
#define LV_GPU_SDL_LRU_SIZE          (1024 * 1024 * 6)
/*Custom blend mode for mask drawing, disable if you need to link with older SDL2 lib*/
#define LV_GPU_SDL_CUSTOM_BLEND_MODE (SDL_VERSION_ATLEAST(2, 0, 6))
#endif

/*-------------
 * Logging
 *-----------*/

/*Enable the log module*/
#define LV_USE_LOG 1
#if LV_USE_LOG

/*How important log should be added:
 *LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
 *LV_LOG_LEVEL_INFO        Log important events
 *LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
 *LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
 *LV_LOG_LEVEL_USER        Only logs added by the user
 *LV_LOG_LEVEL_NONE        Do not log anything*/
#define LV_LOG_LEVEL            LV_LOG_LEVEL_WARN

/*1: Print the log with 'printf';
 *0: User need to register a callback with `lv_log_register_print_cb()`*/
#define LV_LOG_PRINTF           1

/*Enable/disable LV_LOG_TRACE in modules that produces a huge number of logs*/
#define LV_LOG_TRACE_MEM        1
#define LV_LOG_TRACE_TIMER      1
#define LV_LOG_TRACE_INDEV      1
#define LV_LOG_TRACE_DISP_REFR  1
#define LV_LOG_TRACE_EVENT      1
#define LV_LOG_TRACE_OBJ_CREATE 1
#define LV_LOG_TRACE_STYLE      1
#define LV_LOG_TRACE_LAYOUT     1

/*1: Enable debug for the memory allocator related topics*/
#define LV_LOG_TRACE_MEM        1

#endif /*LV_USE_LOG*/

/*-------------
 * Asserts
 *-----------*/

/*Enable asserts if an operation is failed or an invalid data is found.
 *If LV_USE_LOG is enabled an error message will be printed on failure*/
#define LV_USE_ASSERT_NULL 1 /*Check if the parameter is NULL. (Very fast, recommended)*/
#define LV_USE_ASSERT_MALLOC \
  1 /*Checks is the memory is successfully allocated or no. (Very fast, recommended)*/
#define LV_USE_ASSERT_STYLE \
  0 /*Check if the styles are properly initialized. (Very fast, recommended)*/
#define LV_USE_ASSERT_MEM_INTEGRITY \
  0 /*Check the integrity of `lv_mem` after critical operations. (Slow)*/
#define LV_USE_ASSERT_OBJ         0 /*Check the object's type and existence (e.g. not deleted). (Slow)*/

/*Add a custom handler when assert happens e.g. to restart the MCU*/
#define LV_ASSERT_HANDLER_INCLUDE <stdint.h>
#define LV_ASSERT_HANDLER \
  while (1)               \
    ; /*Halt by default*/

/*-------------
 * Others
 *-----------*/

/*1: Show CPU usage and FPS count*/
#define LV_USE_PERF_MONITOR 0
#if LV_USE_PERF_MONITOR
#define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT
#endif

/*1: Show the used memory and the memory fragmentation
 * Requires LV_MEM_CUSTOM = 0*/
#define LV_USE_MEM_MONITOR 0
#if LV_USE_MEM_MONITOR
#define LV_USE_MEM_MONITOR_POS LV_ALIGN_BOTTOM_LEFT
#endif

/*1: Draw random colored rectangles over the redrawn areas*/
#define LV_USE_REFR_DEBUG 0

/*Change the built in (v)snprintf functions*/
#define LV_SPRINTF_CUSTOM 0
#if LV_SPRINTF_CUSTOM
#define LV_SPRINTF_INCLUDE <stdio.h>
#define lv_snprintf        snprintf
#define lv_vsnprintf       vsnprintf
#else /*LV_SPRINTF_CUSTOM*/
#define LV_SPRINTF_USE_FLOAT 0
#endif /*LV_SPRINTF_CUSTOM*/

#define LV_USE_USER_DATA 1

/*Garbage Collector settings
 *Used if lvgl is bound to higher level language and the memory is managed by that language*/
#define LV_ENABLE_GC     0
#if LV_ENABLE_GC != 0
#define LV_GC_INCLUDE "gc.h" /*Include Garbage Collector related things*/
#endif                       /*LV_ENABLE_GC*/

/*=====================
 *  COMPILER SETTINGS
 *====================*/

/*For big endian systems set to 1*/
#define LV_BIG_ENDIAN_SYSTEM 0

/*Define a custom attribute to `lv_tick_inc` function*/
#define LV_ATTRIBUTE_TICK_INC

/*Define a custom attribute to `lv_timer_handler` function*/
#define LV_ATTRIBUTE_TIMER_HANDLER

/*Define a custom attribute to `lv_disp_flush_ready` function*/
#define LV_ATTRIBUTE_FLUSH_READY

/*Required alignment size for buffers*/
#define LV_ATTRIBUTE_MEM_ALIGN_SIZE 1

/*Will be added where memories needs to be aligned (with -Os data might not be aligned to boundary
 *by default). E.g. __attribute__((aligned(4)))*/
#define LV_ATTRIBUTE_MEM_ALIGN

/*Attribute to mark large constant arrays for example font's bitmaps*/
#define LV_ATTRIBUTE_LARGE_CONST

/*Compiler prefix for a big array declaration in RAM*/
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY

/*Place performance critical functions into a faster memory (e.g RAM)*/
#define LV_ATTRIBUTE_FAST_MEM

/*Prefix variables that are used in GPU accelerated operations, often these need to be
 *placed in RAM sections that are DMA accessible*/
#define LV_ATTRIBUTE_DMA

/*Export integer constant to binding. This macro is used with constants in the form of LV_<CONST>
 *that should also appear on LVGL binding API such as Micropython.*/
#define LV_EXPORT_CONST_INT(int_value) \
  struct _silence_gcc_warning /*The default value just prevents GCC warning*/

/*Extend the default -32k..32k coordinate range to -4M..4M by using int32_t for coordinates instead
 * of int16_t*/
#define LV_USE_LARGE_COORD               0

/*==================
 *   FONT USAGE
 *===================*/

/*Montserrat fonts with ASCII range and some symbols using bpp = 4
 *https://fonts.google.com/specimen/Montserrat*/
#define LV_FONT_MONTSERRAT_8             0
#define LV_FONT_MONTSERRAT_10            0
#define LV_FONT_MONTSERRAT_12            0
#define LV_FONT_MONTSERRAT_14            1
#define LV_FONT_MONTSERRAT_16            0
#define LV_FONT_MONTSERRAT_18            0
#define LV_FONT_MONTSERRAT_20            0
#define LV_FONT_MONTSERRAT_22            0
#define LV_FONT_MONTSERRAT_24            0
#define LV_FONT_MONTSERRAT_26            0
#define LV_FONT_MONTSERRAT_28            0
#define LV_FONT_MONTSERRAT_30            0
#define LV_FONT_MONTSERRAT_32            0
#define LV_FONT_MONTSERRAT_34            0
#define LV_FONT_MONTSERRAT_36            0
#define LV_FONT_MONTSERRAT_38            0
#define LV_FONT_MONTSERRAT_40            0
#define LV_FONT_MONTSERRAT_42            0
#define LV_FONT_MONTSERRAT_44            0
#define LV_FONT_MONTSERRAT_46            0
#define LV_FONT_MONTSERRAT_48            0

/*Demonstrate special features*/
#define LV_FONT_MONTSERRAT_12_SUBPX      0
#define LV_FONT_MONTSERRAT_28_COMPRESSED 0 /*bpp = 3*/
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW 0 /*Hebrew, Arabic, Perisan letters and all their forms*/
#define LV_FONT_SIMSUN_16_CJK            0 /*1000 most common CJK radicals*/

/*Pixel perfect monospace fonts*/
#define LV_FONT_UNSCII_8                 0
#define LV_FONT_UNSCII_16                0

/*Optionally declare custom fonts here.
 *You can use these fonts as default font too and they will be available globally.
 *E.g. #define LV_FONT_CUSTOM_DECLARE   LV_FONT_DECLARE(my_font_1) LV_FONT_DECLARE(my_font_2)*/
#define LV_FONT_CUSTOM_DECLARE

/*Always set a default font*/
#define LV_FONT_DEFAULT                     &lv_font_montserrat_14

/*Enable handling large font and/or fonts with a lot of characters.
 *Requires 26~640 kB of RAM. Recommend enabling only if fonts with > 20,000 characters are used.*/
#define LV_FONT_FMT_TXT_LARGE               0

/*Enables/disables support for compressed fonts.*/
#define LV_USE_FONT_COMPRESSED              0

/*Enable drawing placeholders when glyph dsc is not found*/
#define LV_USE_FONT_PLACEHOLDER             1

/*=================
 *  TEXT SETTINGS
 *=================*/

/**
 * Select a character encoding for strings.
 * Your IDE or editor should have the same character encoding
 * - LV_TXT_ENC_UTF8
 * - LV_TXT_ENC_ASCII
 */
#define LV_TXT_ENC                          LV_TXT_ENC_UTF8

/*Can break (wrap) texts on these chars*/
#define LV_TXT_BREAK_CHARS                  " ,.;:-_"

/*If a word is at least this long, will break wherever "prettiest"
 *To disable, set to a value <= 0*/
#define LV_TXT_LINE_BREAK_LONG_LEN          0

/*Minimum number of characters in a long word to put on a line before a break.
 *Depends on LV_TXT_LINE_BREAK_LONG_LEN.*/
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN  3

/*Minimum number of characters in a long word to put on a line after a break.
 *Depends on LV_TXT_LINE_BREAK_LONG_LEN.*/
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3

/*The control character to use for signalling text recoloring.*/
#define LV_TXT_COLOR_CMD                    "#"

/*Support bidirectional texts. Allows mixing Left-to-Right and Right-to-Left texts.
 *The direction will be processed according to the Unicode Bidirectional Algorithm:
 *https://www.unicode.org/reports/tr9/*/
#define LV_USE_BIDI                         0
#if LV_USE_BIDI
/*Set the default direction. Supported values:
 *`LV_BASE_DIR_LTR` Left-to-Right
 *`LV_BASE_DIR_RTL` Right-to-Left
 *`LV_BASE_DIR_AUTO` detect texts base direction*/
#define LV_BIDI_BASE_DIR_DEF LV_BASE_DIR_AUTO
#endif

/*Enable Arabic/Persian processing
 *In these languages characters should be replaced with an other form based on their position in the
 *word*/
#define LV_USE_ARABIC_PERSIAN_CHARS  0

/*==================
 *  WIDGETS
 *================*/

/*Documentation of the widgets: https://docs.lvgl.io/latest/en/html/widgets/index.html*/

#define LV_WIDGETS_HAS_DEFAULT_VALUE 1

#define LV_USE_ANIMIMG               1

#define LV_USE_ARC                   1

#define LV_USE_BAR                   1

#define LV_USE_BTN                   1

#define LV_USE_BUTTONMATRIX          1

#define LV_USE_CALENDAR              1
#if LV_USE_CALENDAR
#define LV_CALENDAR_WEEK_STARTS_MONDAY 0
#if LV_CALENDAR_WEEK_STARTS_MONDAY
#define LV_CALENDAR_DEFAULT_DAY_NAMES \
  { "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" }
#else
#define LV_CALENDAR_DEFAULT_DAY_NAMES \
  { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" }
#endif
#define LV_CALENDAR_DEFAULT_MONTH_NAMES                                                    \
  {                                                                                        \
    "January", "February", "March", "April", "May", "June", "July", "August", "September", \
        "October", "November", "December"                                                  \
  }
#define LV_USE_CALENDAR_HEADER_ARROW    1
#define LV_USE_CALENDAR_HEADER_DROPDOWN 1
#endif /*LV_USE_CALENDAR*/

#define LV_USE_CANVAS   1

#define LV_USE_CHART    1

#define LV_USE_CHECKBOX 1

#define LV_USE_DROPDOWN 1 /*Requires: lv_label*/

#define LV_USE_IMG      1 /*Requires: lv_label*/

#define LV_USE_KEYBOARD 1

#define LV_USE_LABEL    1
#if LV_USE_LABEL
#define LV_LABEL_TEXT_SELECTION 1 /*Enable selecting text of the label*/
#define LV_LABEL_LONG_TXT_HINT \
  1 /*Store some extra info in labels to speed up drawing of very long texts*/
#define LV_LABEL_WAIT_CHAR_COUNT 3 /*The count of wait chart*/
#endif

#define LV_USE_LED    1

#define LV_USE_LINE   1

#define LV_USE_LIST   1

#define LV_USE_MENU   1

#define LV_USE_METER  1

#define LV_USE_MSGBOX 1

#define LV_USE_ROLLER 1 /*Requires: lv_label*/
#if LV_USE_ROLLER
#define LV_ROLLER_INF_PAGES 7
#endif

#define LV_USE_SLIDER 1 /*Requires: lv_bar*/

#define LV_USE_SPAN   1
#if LV_USE_SPAN
/*A line text can contain maximum num of span descriptor */
#define LV_SPAN_SNIPPET_STACK_SIZE 64
#endif

#define LV_USE_SPINBOX  1

#define LV_USE_SPINNER  1

#define LV_USE_SWITCH   1

#define LV_USE_TEXTAREA 1 /*Requires: lv_label*/
#if LV_USE_TEXTAREA != 0
#define LV_TEXTAREA_DEF_PWD_SHOW_TIME 1500 /*ms*/
#endif

#define LV_USE_TABLE         1

#define LV_USE_TABVIEW       1

#define LV_USE_TILEVIEW      1

#define LV_USE_WIN           1

/*==================
 * THEMES
 *==================*/

/*A simple, impressive and very complete theme*/
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT

/*0: Light mode; 1: Dark mode*/
#define LV_THEME_DEFAULT_DARK            0

/*1: Enable grow on press*/
#define LV_THEME_DEFAULT_GROW            1

/*Default transition time in [ms]*/
#define LV_THEME_DEFAULT_TRANSITION_TIME 300
#endif /*LV_USE_THEME_DEFAULT*/

/*A very simple theme that is a good starting point for a custom theme*/
#define LV_USE_THEME_BASIC 1

/*A theme designed for monochrome displays*/
#define LV_USE_THEME_MONO  1

/*==================
 * LAYOUTS
 *================*/

/*A layout similar to Flexbox in CSS.*/
#define LV_USE_FLEX        1

/*A layout similar to Grid in CSS.*/
#define LV_USE_GRID        1

/*====================
 * 3RD PARTS LIBRARIES
 *====================*/

/*File system interfaces for common APIs */

/*API for fopen, fread, etc*/
#define LV_USE_FS_STDIO    0
#if LV_USE_FS_STDIO
#define LV_FS_STDIO_LETTER \
  '\0' /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
#define LV_FS_STDIO_PATH \
  "" /*Set the working directory. File/directory paths will be appended to it.*/
#define LV_FS_STDIO_CACHE_SIZE 0 /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for open, read, etc*/
#define LV_USE_FS_POSIX 0
#if LV_USE_FS_POSIX
#define LV_FS_POSIX_LETTER \
  '\0' /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
#define LV_FS_POSIX_PATH \
  "" /*Set the working directory. File/directory paths will be appended to it.*/
#define LV_FS_POSIX_CACHE_SIZE 0 /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for CreateFile, ReadFile, etc*/
#define LV_USE_FS_WIN32 0
#if LV_USE_FS_WIN32
#define LV_FS_WIN32_LETTER \
  '\0' /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
#define LV_FS_WIN32_PATH \
  "" /*Set the working directory. File/directory paths will be appended to it.*/
#define LV_FS_WIN32_CACHE_SIZE 0 /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for FATFS (needs to be added separately). Uses f_open, f_read, etc*/
#define LV_USE_FS_FATFS 0
#if LV_USE_FS_FATFS
#define LV_FS_FATFS_LETTER \
  '\0' /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
#define LV_FS_FATFS_CACHE_SIZE 0 /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*PNG decoder library*/
#define LV_USE_PNG      0

/*BMP decoder library*/
#define LV_USE_BMP      0

/*JPG + split JPG decoder library.
 *Split JPG is a custom format optimized for embedded systems.*/
#define LV_USE_SJPG     0

/*GIF decoder library*/
#define LV_USE_GIF      0

/*QR code library*/
#define LV_USE_QRCODE   0

/*Barcode code library*/
#define LV_USE_BARCODE  0

/*FreeType library*/
#define LV_USE_FREETYPE 0
#if LV_USE_FREETYPE
/*Memory used by FreeType to cache characters [bytes] (-1: no caching)*/
#define LV_FREETYPE_CACHE_SIZE (16 * 1024)
#if LV_FREETYPE_CACHE_SIZE >= 0
/* 1: bitmap cache use the sbit cache, 0:bitmap cache use the image cache. */
/* sbit cache:it is much more memory efficient for small bitmaps(font size < 256) */
/* if font size >= 256, must set LV_FREETYPE_CACHE_SIZE=0 */
#define LV_FREETYPE_SBIT_CACHE     0
/* Maximum number of opened FT_Face/FT_Size objects managed by this cache instance. */
/* (0:use system defaults) */
#define LV_FREETYPE_CACHE_FT_FACES 0
#define LV_FREETYPE_CACHE_FT_SIZES 0
#endif
#endif

/*Rlottie library*/
#define LV_USE_RLOTTIE 0

/*FFmpeg library for image decoding and playing videos
 *Supports all major image formats so do not enable other image decoder with it*/
#define LV_USE_FFMPEG  0
#if LV_USE_FFMPEG
/*Dump input information to stderr*/
#define LV_FFMPEG_DUMP_FORMAT 0
#endif

/*==================
 * OTHERS
 *================*/

/*1: Enable API to take snapshots for widgets*/
#define LV_USE_SNAPSHOT   0

/*1: Enable Monkey test*/
#define LV_USE_MONKEY     0

/*1: Enable grid navigation*/
#define LV_USE_GRIDNAV    0

/*1: Enable lv_obj fragment*/
#define LV_USE_FRAGMENT   0

/*1: Support using images as font in label or span widgets */
#define LV_USE_IMGFONT    0

/*1: Enable an observer pattern implementation*/
#define LV_USE_OBSERVER   0

/*1: Enable Pinyin input method*/
/*Requires: lv_keyboard*/
#define LV_USE_IME_PINYIN 0
#if LV_USE_IME_PINYIN
/*1: Use default thesaurus*/
/*If you do not use the default thesaurus, be sure to use `lv_ime_pinyin_set_dict()` to set a
 * thesaurus*/
#define LV_IME_PINYIN_USE_DEFAULT_DICT 1
/*Set the maximum number of candidate panels that can be displayed*/
/*This needs to be adjusted according to the size of the screen*/
#define LV_IME_PINYIN_CAND_TEXT_NUM    6

/*Use 9 key input(k9)*/
#define LV_IME_PINYIN_USE_K9_MODE      1
#if LV_IME_PINYIN_USE_K9_MODE == 1
#define LV_IME_PINYIN_K9_CAND_TEXT_NUM 3
#endif
#endif

/*==================
 * EXAMPLES
 *==================*/

/*Enable the examples to be built with the library*/
#define LV_BUILD_EXAMPLES 1

/*================--
 * DEVICES
 *===================*/

/*Use SDL to open window on PC and handle mouse and keyboard*/
#define LV_USE_SDL        0
#if LV_USE_SDL
#define LV_SDL_INCLUDE_PATH <SDL2/SDL.h>
#define LV_SDL_RENDER_MODE                                                              \
  LV_DISPLAY_RENDER_MODE_DIRECT /*LV_DISPLAY_RENDER_MODE_DIRECT is recommended for best \
                                   performance*/
#define LV_SDL_BUF_COUNT 1      /*1 or 2*/
#endif

/*Use X11 to open window on Linux desktop and handle mouse and keyboard*/
#define LV_USE_X11 0
#if LV_USE_X11
#define LV_X11_DIRECT_EXIT         1 /*Exit the application when all X11 windows have been closed*/
#define LV_X11_DOUBLE_BUFFER       1 /*Use double buffers for endering*/
/*select only 1 of the following render methods (don't enable both) */
#define LV_X11_RENDER_MODE_PARTIAL 1 /*Partial render mode (draw only the updated areas)*/
#define LV_X11_RENDER_MODE_DIRECT  0 /*Direct render mode (draw directly to the screen)*/
#endif

/*Driver for /dev/fb*/
#define LV_USE_LINUX_FBDEV 0
#if LV_USE_LINUX_FBDEV
#define LV_LINUX_FBDEV_BPP    32
#define LV_LINUX_FBDEV_DEVICE "/dev/fb0"
#endif

/*Use Nuttx to open window and handle touchscreen*/
#define LV_USE_NUTTX                        0

/*Use uGFX to open window and handle touchscreen*/
#define LV_USE_UGFX                         0

/*Use driver for GTK library on Linux using directdraw*/
#define LV_USE_GTK_DIRECT                   0

/*Use driver for GTK library on Linux using indirect draw*/
#define LV_USE_GTK_INDIRECT                 0

/*==================
 * INPUT
 *==================*/

/*Input device driver settings*/
#define LV_INDEV_DEF_READ_PERIOD            30  /*[ms]*/
#define LV_INDEV_DEF_LONG_PRESS_TIME        400 /*[ms]*/
#define LV_INDEV_DEF_LONG_PRESS_REPEAT_TIME 100 /*[ms]*/
#define LV_INDEV_DEF_READ_REPEATED          1   /*Enable repeated reading*/
#define LV_INDEV_DEF_DRAG_THROW             300 /*Drag throw slow-down in [ms]*/
#define LV_INDEV_DEF_DRAG_THROW_MIN_Y       50  /*Drag throw min. delta Y in [px]*/
#define LV_INDEV_DEF_DRAG_THROW_MIN_X       50  /*Drag throw min. delta X in [px]*/

/*Gesture threshold in [px]*/
#define LV_GESTURE_THRESHOLD                50

/*Drag threshold in [px]*/
#define LV_DRAG_THRESHOLD                   10

/*==================
 * ANIMATION
 *==================*/

#define LV_USE_ANIMATION                    1

/*Declare the type of the user data of animations (can be e.g. `void *`, `int`, `struct`)*/
typedef void* lv_anim_user_data_t;

#endif /*LV_CONF_H*/