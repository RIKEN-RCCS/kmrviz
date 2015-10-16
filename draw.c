#include "kmrviz.h"

static double
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

static void
kv_draw_slash(cairo_t * cr, kv_viewport_t * VP, kv_trace_t * trace, kv_trace_entry_t * e) {
  int i = trace - GS->ts->traces;
  double y = KV_RADIUS + i * (2 * KV_RADIUS + KV_GAP_BETWEEN_TIMELINES);
  double x = kv_scale_down(e->t - trace->start_t);
  GdkRGBA color;
  kv_lookup_color(e->e / 2, &color.red, &color.green, &color.blue, &color.alpha);
  cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
  cairo_set_line_width(cr, KV_LINE_WIDTH / VP->zoom_ratio_x);
  cairo_move_to(cr, x, y - KV_RADIUS);
  cairo_line_to(cr, x, y + KV_RADIUS);
  cairo_stroke(cr);        
}

static void
kv_draw_box(cairo_t * cr, kv_viewport_t * VP, kv_trace_t * trace, kv_trace_entry_t * e1, kv_trace_entry_t * e2) {
  int i = trace - GS->ts->traces;
  double y = KV_RADIUS + i * (2 * KV_RADIUS + KV_GAP_BETWEEN_TIMELINES);
  double x1 = kv_scale_down(e1->t - trace->start_t);
  double x2 = kv_scale_down(e2->t - trace->start_t);
  GdkRGBA color;
  kv_lookup_color(e1->e / 2, &color.red, &color.green, &color.blue, &color.alpha);
  assert(e1->e + 1 == e2->e);
  cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha * 0.6);
  cairo_set_line_width(cr, KV_LINE_WIDTH / VP->zoom_ratio_x);
  cairo_rectangle(cr, x1, y - KV_RADIUS, x2 - x1, 2 * KV_RADIUS);
  cairo_fill(cr);
}

void
kv_viewport_draw(kv_viewport_t * VP, cairo_t * cr) {
  kv_trace_set_t * ts = GS->ts;
  //printf("duration %.0lf\n", ts->end_t - ts->start_t);
  cairo_save(cr);
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
      cairo_set_line_width(cr, KV_LINE_WIDTH / 2);
      int i;
      for (i = 0; i < ts->n; i++) {
        kv_trace_t * trace = &ts->traces[i];
        double y = KV_RADIUS + i * (2 * KV_RADIUS + KV_GAP_BETWEEN_TIMELINES);
        double x = 55;
        //x += kv_scale_down(trace->start_t);
        cairo_move_to(cr, x, y);
        x += kv_scale_down(trace->end_t - trace->start_t);
        cairo_line_to(cr, x, y);
        cairo_stroke(cr);
      }
    }
  }

  /* Events */
  cairo_translate(cr, 55, 0);
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
      x = 0.0;// + kv_scale_down(trace->start_t);
      cairo_move_to(cr, x, y - r);
      cairo_line_to(cr, x, y + r);
      cairo_stroke(cr);
      x = kv_scale_down(trace->end_t - trace->start_t);
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
        while (trace->e[j].e % 2 != 1 && j < trace->n) {
          kv_draw_slash(cr, VP, trace, &trace->e[j]);
          j++;
        }
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

