#include "kmrviz.h"

double
kv_scale_down(double t) {
  return t / 1.5E8;
}

static void
kv_lookup_color(int value, double * r, double * g, double * b, double * a) {
  GdkRGBA color;
  gdk_rgba_parse(&color, KV_COLORS[(value + KV_NUM_COLORS) % KV_NUM_COLORS]);
  *r = color.red;
  *g = color.green;
  *b = color.blue;
  *a = color.alpha;
}

static int
kv_viewport_clip_trim(kv_viewport_t * VP, double * xx, double * yy, double * ww, double * hh) {
  double bound_left = kv_viewport_clip_get_bound_left(VP);
  double bound_right = kv_viewport_clip_get_bound_right(VP);
  double bound_up = kv_viewport_clip_get_bound_up(VP);
  double bound_down = kv_viewport_clip_get_bound_down(VP);

  double x = *xx;
  double y = *yy;
  double w = *ww;
  double h = *hh;
  if (x < bound_right && x + w > bound_left && y < bound_down && y + h > bound_up) {
    if (x < bound_left) {
      w -= (bound_left - x);
      x = bound_left;
    }
    if (x + w > bound_right)
      w = bound_right - x;
    if (y < bound_up) {
      h -= (bound_up - y);
      y = bound_up;
    }
    if (y + h > bound_down)
      h = bound_down - y;
    *xx = x;
    *yy = y;
    *ww = w;
    *hh = h;
    return 1;
  }
  return 0;
}

static void
kv_draw_slash(cairo_t * cr, kv_viewport_t * VP, kv_trace_t * trace, kv_trace_entry_t * e) {
  int i = trace - GS->ts->traces;
  double y = i * (2 * KV_RADIUS + KV_GAP_BETWEEN_TIMELINES);
  double x = KV_TIMELINE_START_X + kv_scale_down(e->t - trace->start_t);
  double h = 2 * KV_RADIUS;
  double w = 0.0;
  GdkRGBA color;
  kv_lookup_color(e->e / 2, &color.red, &color.green, &color.blue, &color.alpha);

  if (kv_viewport_clip_trim(VP, &x, &y, &w, &h)) {
    cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
    cairo_set_line_width(cr, KV_LINE_WIDTH / VP->zoom_ratio_x);
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x, y + h);
    cairo_stroke(cr);
  }
}

static void
kv_draw_box(cairo_t * cr, kv_viewport_t * VP, kv_trace_t * trace, kv_trace_entry_t * e1, kv_trace_entry_t * e2) {
  assert(e1->e + 1 == e2->e);
  GdkRGBA color;
  kv_lookup_color(e1->e / 2, &color.red, &color.green, &color.blue, &color.alpha);
  
  int i = trace - GS->ts->traces;
  double x = KV_TIMELINE_START_X + kv_scale_down(e1->t - trace->start_t);
  double y = i * (2 * KV_RADIUS + KV_GAP_BETWEEN_TIMELINES);
  double w = kv_scale_down(e2->t - e1->t);
  double h = 2 * KV_RADIUS;

  if (kv_viewport_clip_trim(VP, &x, &y, &w, &h)) {
    cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha * 0.6);
    cairo_set_line_width(cr, KV_LINE_WIDTH);// / VP->zoom_ratio_x);
    cairo_rectangle(cr, x, y, w, h);
    cairo_fill(cr);
  }
}

void
kv_viewport_draw(kv_viewport_t * VP, cairo_t * cr) {
  kv_trace_set_t * ts = GS->ts;
  //printf("duration %.0lf\n", ts->end_t - ts->start_t);
  
  cairo_save(cr);
  cairo_rectangle(cr, 0, 0, VP->vpw, VP->vph);
  cairo_clip(cr);

  cairo_translate(cr, VP->x, VP->y);
  cairo_scale(cr, VP->zoom_ratio_x, VP->zoom_ratio_y);

  /* Rank numbers */
  {
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);
    char s[KV_STRING_LENGTH];
    double x, y;
    x = 0;
    y = KV_RADIUS * 1.3;
    int i;
    for (i = 0; i < ts->n; i++) {
      sprintf(s, "Rank %d", ts->traces[i].rank);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, s);
      y += 2 * KV_RADIUS + KV_GAP_BETWEEN_TIMELINES;
    }

    /* Lines */
    {
      cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
      cairo_set_line_width(cr, KV_LINE_WIDTH / 2 / VP->zoom_ratio_x);
      int i;
      for (i = 0; i < ts->n; i++) {
        kv_trace_t * trace = &ts->traces[i];
        double y = KV_RADIUS + i * (2 * KV_RADIUS + KV_GAP_BETWEEN_TIMELINES);
        double x = KV_TIMELINE_START_X;
        //x += kv_scale_down(trace->start_t);
        double w = kv_scale_down(trace->end_t - trace->start_t);
        double h = 0.0;
        if (kv_viewport_clip_trim(VP, &x, &y, &w, &h)) {
            cairo_move_to(cr, x, y);
            cairo_line_to(cr, x + w, y);
            cairo_stroke(cr);
        }
      }
    }
  }

  /* Events */
  double basex = KV_TIMELINE_START_X;
  {
    double x, y;
    double r = KV_RADIUS;
    int i;
    for (i = 0; i < ts->n; i++) {
      y = KV_RADIUS + i * (2 * KV_RADIUS + KV_GAP_BETWEEN_TIMELINES);
      kv_trace_t * trace = &ts->traces[i];
      
      /* trace's start, end */
      cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
      cairo_set_line_width(cr, KV_LINE_WIDTH / VP->zoom_ratio_x);
      x = basex;// + kv_scale_down(trace->start_t);
      cairo_move_to(cr, x, y - r);
      cairo_line_to(cr, x, y + r);
      cairo_stroke(cr);
      x = basex + kv_scale_down(trace->end_t - trace->start_t);
      cairo_move_to(cr, x, y - r);
      cairo_line_to(cr, x, y + r);
      cairo_stroke(cr);
      
      /* trace's events */
      /*
      int j;
      for (j = 0; j < trace->n; j++) {
        kv_trace_entry_t * e = &trace->e[j];
        kv_draw_slash(cr, VP, trace, e);
      }
      */

      /* trace's events */
      int jb = 0;
      int j = 0;
      while (j < trace->n) {
        while (trace->e[j].e % 2 != 1 && j < trace->n)
          j++;
        int jj = j - 1;
        while (j < trace->n && jj >= jb) {
          while (trace->e[jj].e != trace->e[j].e - 1) {
            kv_draw_slash(cr, VP, trace, &trace->e[jj]);
            jj--;
          }
          if (jj >= jb)
            kv_draw_box(cr, VP, trace, &trace->e[jj], &trace->e[j]);
          j++;
          jj--;
        }
        jb = j;
      }
    }
  }

  cairo_restore(cr);
}

