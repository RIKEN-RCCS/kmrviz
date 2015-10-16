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


#define _unused_ __attribute__((unused))
#define _static_unused_ static __attribute__((unused))

#define KV_STRING_LENGTH 100
#define KV_RADIUS 10
#define KV_MARGIN_WHEN_ZOOMFIT 15
#define KV_SAFE_CLICK_RANGE 2
#define KV_ZOOM_INCREMENT 1.20
#define KV_LINE_WIDTH 2.5
#define KV_NUM_COLORS 34
#define KV_GAP_BETWEEN_TIMELINES 2

/*-- Copy from kmrtrace.h --*/
typedef enum {
  kmr_trace_event_map_start, /* map phase starts */
  kmr_trace_event_map_end, /* map phase ends */
  kmr_trace_event_shuffle_start,
  kmr_trace_event_shuffle_end,
  kmr_trace_event_reduce_start,
  kmr_trace_event_reduce_end,
  
  kmr_trace_event_mapper_start,
  kmr_trace_event_mapper_end,
  kmr_trace_event_reducer_start,
  kmr_trace_event_reducer_end,
} kmr_trace_event_t;
/*--------*/

typedef struct kv_trace_entry {
  double t;
  int e;
} kv_trace_entry_t;

typedef struct kv_trace {
  int rank;
  double start_t;
  double end_t;
  long n;
  kv_trace_entry_t * e;
} kv_trace_t;

typedef struct kv_trace_set {
  int n;
  kv_trace_t * traces;
  double start_t;
  double end_t;
} kv_trace_set_t;

typedef struct kv_viewport {
  double vpw, vph;
  GtkWidget * box;
  GtkWidget * darea;
  double x, y;
  double zoom_ratio_x, zoom_ratio_y;
  int drag_on;
  double pressx, pressy;
  double accdisx, accdisy;
} kv_viewport_t;

typedef struct kv_gui {
  /* Main window */
  GtkWidget * window;
  GtkWidget * window_box;
  GtkWidget * menubar;
  GtkWidget * toolbar;
  GtkWidget * main_box;
  GtkWidget * statusbar1;
  GtkWidget * statusbar2;
  GtkWidget * statusbar3;
  
  /* Utilities */
  GtkAccelGroup * accel_group;  
  GtkBuilder * builder;
} kv_gui_t;

typedef struct kv_global_state {
  kv_gui_t GUI[1];
  kv_viewport_t VP[1];
  kv_trace_set_t ts[1];
} kv_global_state_t;


extern kv_global_state_t GS[];
extern const char * const KV_COLORS[];


/* kmrviz.c */
void kv_viewport_init(kv_viewport_t *);
void kv_viewport_queue_draw(kv_viewport_t *);

void kv_gui_init(kv_gui_t *);
GtkWidget * kv_gui_get_main_window(kv_gui_t *);

void kv_global_state_init(kv_global_state_t *);


/* draw.c */
void kv_viewport_draw(kv_viewport_t *, cairo_t *);


#endif /* _KMRVIZ_H */
