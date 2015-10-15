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


#define _unused_ __attribute__((unused))
#define _static_unused_ static __attribute__((unused))

typedef struct kv_viewport {
  double vpw, vph;
  GtkWidget * box;
  GtkWidget * darea;
  double x, y;
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
} kv_global_state_t;


extern kv_global_state_t GS[];


/* kmrviz.c */
void kv_viewport_init(kv_viewport_t *);
void kv_viewport_queue_draw(kv_viewport_t *);
void kv_viewport_draw(kv_viewport_t *, cairo_t *);

void kv_gui_init(kv_gui_t *);
GtkWidget * kv_gui_get_main_window(kv_gui_t *);
void kv_global_state_init(kv_global_state_t *);


#endif /* _KMRVIZ_H */
