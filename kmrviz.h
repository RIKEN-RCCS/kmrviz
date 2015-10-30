/*
 * kmrviz.h (first created: 2015/10/15)
 */

#ifndef _KMRVIZ_H
#define _KMRVIZ_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <cairo.h>
#include <cairo-ps.h>
#include <cairo-svg.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <execinfo.h>

#include <stdint.h>
#include <byteswap.h>


#define _unused_ __attribute__((unused))
#define _static_unused_ static __attribute__((unused))

#define KV_STRING_LENGTH 100
#define KV_RADIUS 10
#define KV_MARGIN_WHEN_ZOOMFIT 15
#define KV_SAFE_CLICK_RANGE 2
#define KV_ZOOM_INCREMENT 1.20
#define KV_LINE_WIDTH 2.0
#define KV_NUM_COLORS 37
#define KV_GAP_BETWEEN_TIMELINES 2
#define KV_TIMELINE_START_X 75
#define KV_NESTED_DECREASE_RATE 0.9

/*-- Copy from kmrtrace.h --*/
typedef enum {
  kmr_trace_event_start,
  kmr_trace_event_end,
  kmr_trace_event_map,
  kmr_trace_event_map_once,
  kmr_trace_event_shuffle,
  kmr_trace_event_reduce,
  kmr_trace_event_sort,
  
  kmr_trace_event_max,

  /*
  kmr_trace_event_map_start,
  kmr_trace_event_map_end,
  kmr_trace_event_shuffle_start,
  kmr_trace_event_shuffle_end,
  kmr_trace_event_reduce_start,
  kmr_trace_event_reduce_end,
  kmr_trace_event_map_once_start,
  kmr_trace_event_map_once_end,
  kmr_trace_event_sort_start,
  kmr_trace_event_sort_end,
  
  kmr_trace_event_trace_start,
  kmr_trace_event_trace_end,
  
  kmr_trace_event_mapper_start,
  kmr_trace_event_mapper_end,
  kmr_trace_event_reducer_start,
  kmr_trace_event_reducer_end,
  */
} kmr_trace_event_t;
/*--------*/


typedef struct kv_trace_entry {
  double t;
  int e;
  long kvi_element_count;
  long kvo_element_count;
  int offset;
} kv_trace_entry_t;

typedef struct kv_trace {
  int rank;
  double start_t;
  double end_t;
  long n;
  kv_trace_entry_t * e;
  struct kv_trace * next;
} kv_trace_t;

typedef struct kv_trace_set {
  double start_t;
  double end_t;
  double t_span;
  int n;
  kv_trace_t * head;
  kv_trace_t * tail;
} kv_trace_set_t;


typedef struct kv_timeline_slash {
  kv_trace_entry_t * e;
  double x;
  struct kv_timeline_slash * next;
} kv_timeline_slash_t;

typedef struct kv_timeline_box {
  kv_trace_entry_t * start_e;
  kv_trace_entry_t * end_e;
  double x;
  double w;
  struct kv_timeline_box * child;
  struct kv_timeline_box * next;
  int focused;
} kv_timeline_box_t;

typedef struct kv_timeline {
  kv_trace_t * trace;
  double y;
  double h;
  kv_timeline_box_t * box;
  kv_timeline_slash_t * slash;
  struct kv_timeline * next;
} kv_timeline_t;

typedef struct kv_timeline_set {
  kv_trace_set_t * TS;
  int n;
  kv_timeline_t * head;
  kv_timeline_t * tail;
} kv_timeline_set_t;


typedef struct kv_viewport {
  double vpw, vph;
  GtkWidget * box;
  GtkWidget * darea;
  double x, y;
  double zoom_ratio_x, zoom_ratio_y;
  int drag_on;
  double pressx, pressy;
  double accdisx, accdisy;
  kv_timeline_box_t * last_hovered_box;
} kv_viewport_t;

typedef struct kv_gui {
  /* Main window */
  GtkWidget * window;
  GtkWidget * window_box;
  GtkWidget * menubar;
  GtkWidget * toolbar;
  GtkWidget * main_box;
  GtkWidget * left_sidebar;
  GtkWidget * statusbar1;
  GtkWidget * statusbar2;
  GtkWidget * statusbar3;
  
  /* Utilities */
  GtkAccelGroup * accel_group;  
  GtkBuilder * builder;

  /* On toolbar */
  struct {
    GtkToolItem * toolbox;
    GtkToolItem * infobox;
    GtkToolItem * replaybox;
  } ontoolbar;

  /* On menubar */
  struct {
    GtkCheckMenuItem * toolbox;
    GtkCheckMenuItem * infobox;
    GtkCheckMenuItem * replaybox;
  } onmenubar;

  /* Sideboxes */
  struct toolbox {
    GtkWidget * sidebox;
    GtkWidget * align_start;
    GtkWidget * legend;
  } toolbox;
  struct infobox {
    GtkWidget * sidebox;
    GtkWidget * type;
    GtkWidget * start_t;
    GtkWidget * end_t;
    GtkWidget * span;
    GtkWidget * kvi_ne;
    GtkWidget * kvo_ne;
  } infobox;
  struct replaybox {
    GtkWidget * sidebox;
    GtkWidget * enable;
    GtkWidget * scale;
    GtkWidget * entry;
    GtkWidget * time_step_entry;
    GtkWidget * filledranks;
  } replaybox;
} kv_gui_t;

typedef struct kv_global_state {
  kv_gui_t GUI[1];
  kv_viewport_t VP[1];
  kv_trace_set_t TS[1];
  kv_timeline_set_t TL[1];
  int align_start;
  int toolbox_shown;
  int infobox_shown;
  int replaybox_shown;
  int draw_legend;
  int replay_enable;
  double current_time;
} kv_global_state_t;


extern kv_global_state_t GS[];
extern const char * const KV_COLORS[];


/* kmrviz.c */
void kv_viewport_init(kv_viewport_t *);
void kv_viewport_queue_draw(kv_viewport_t *);
double kv_viewport_clip_get_bound_left(kv_viewport_t *);
double kv_viewport_clip_get_bound_right(kv_viewport_t *);
double kv_viewport_clip_get_bound_up(kv_viewport_t *);
double kv_viewport_clip_get_bound_down(kv_viewport_t *);

void kv_gui_init(kv_gui_t *);
GtkWidget * kv_gui_get_main_window(kv_gui_t *);
GtkWidget * kv_gui_get_toolbox_sidebox(kv_gui_t *);
GtkWidget * kv_gui_get_infobox_sidebox(kv_gui_t *);
GtkWidget * kv_gui_get_replaybox_sidebox(kv_gui_t *);
void kv_gui_update_infobox(kv_timeline_box_t *);

void kv_global_state_init(kv_global_state_t *);

void kv_layout_timelines(kv_timeline_set_t *);


/* draw.c */
double kv_scale_down_span(double);
double kv_scale_down(kv_trace_t *, double);
void kv_viewport_draw(kv_viewport_t *, cairo_t *);


_static_unused_ void
kv_swap_bytes(void * input, void * output, int bytes) {
  int i;
  for (i = 0; i < bytes; i++)
    ((char *) output)[i] = ((char *) input)[bytes - 1 - i];
  /*
  if (bytes == 16)
    *((uint16_t *) output) = __bswap_16(*((uint16_t *) input));
  else if (bytes == 32)
    *((uint32_t *) output) = __bswap_32(*((uint32_t *) input));
  else if (bytes == 64)
    *((uint64_t *) output) = __bswap_64(*((uint64_t *) input));
   */
}

#define KV_ENDIAN_CHECKER 0xdeadbeef
#define KV_BIG_ENDIAN     0xde
#define KV_LITTLE_ENDIAN  0xef
#define KV_GET_FIRST_BYTE(x) ((uint8_t *) x)[0]
#define KV_IS_BIG_ENDIAN(x) (KV_GET_FIRST_BYTE(x) == KV_BIG_ENDIAN)
#define KV_IS_LITTLE_ENDIAN(x) (KV_GET_FIRST_BYTE(x) == KV_LITTLE_ENDIAN)

_static_unused_ char *
kv_trace_event_get_kind(kmr_trace_event_t e) {
  switch (e) {
  case kmr_trace_event_start:
    return "trace start";
  case kmr_trace_event_end:
    return "trace end";
  case kmr_trace_event_map:
    return "map";
  case kmr_trace_event_map_once:
    return "map_once";
  case kmr_trace_event_shuffle:
    return "shuffle";
  case kmr_trace_event_reduce:
    return "reduce";
  case kmr_trace_event_sort:
    return "sort";
  case kmr_trace_event_max:
    return "max";
  }
  return NULL;
}

/*
_static_unused_ char *
kv_trace_event_get_kind(kmr_trace_event_t e) {
  switch (e) {
  case kmr_trace_event_map:
  case kmr_trace_event_map_start:
  case kmr_trace_event_map_end:
    return "map";
  case kmr_trace_event_shuffle:
  case kmr_trace_event_shuffle_start:
  case kmr_trace_event_shuffle_end:
    return "shuffle";
  case kmr_trace_event_reduce:
  case kmr_trace_event_reduce_start:
  case kmr_trace_event_reduce_end:
    return "reduce";
  case kmr_trace_event_map_once:
  case kmr_trace_event_map_once_start:
  case kmr_trace_event_map_once_end:
    return "map_once";
  case kmr_trace_event_sort:
  case kmr_trace_event_sort_start:
  case kmr_trace_event_sort_end:
    return "sort";
  case kmr_trace_event_start:
  case kmr_trace_event_end:
  case kmr_trace_event_trace_start:
  case kmr_trace_event_trace_end:
    return "trace";
  }
  return NULL;
}

_static_unused_ char *
kv_trace_event_get_type(kmr_trace_event_t e) {
  switch (e) {
  case kmr_trace_event_map_start:
  case kmr_trace_event_shuffle_start:
  case kmr_trace_event_reduce_start:
  case kmr_trace_event_trace_start:
  case kmr_trace_event_map_once_start:
  case kmr_trace_event_sort_start:
    return "start";
  case kmr_trace_event_map_end:
  case kmr_trace_event_shuffle_end:
  case kmr_trace_event_reduce_end:
  case kmr_trace_event_trace_end:
  case kmr_trace_event_map_once_end:
  case kmr_trace_event_sort_end:
    return "end";
  case kmr_trace_event_start:
  case kmr_trace_event_end:
  case kmr_trace_event_map:
  case kmr_trace_event_map_once:
  case kmr_trace_event_shuffle:
  case kmr_trace_event_reduce:
  case kmr_trace_event_sort:
    return "none";
  }
  return NULL;
}
*/

#endif /* _KMRVIZ_H */
