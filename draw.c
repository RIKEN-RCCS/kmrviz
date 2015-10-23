#include "kmrviz.h"

double
kv_scale_down_span(double t) {
  return t / 1.5E8;
}

double
kv_scale_down(kv_trace_t * trace, double t) {
  if (GS->align_start && trace)
    return kv_scale_down_span(t - trace->start_t);
  else
    return kv_scale_down_span(t - GS->TS->start_t);
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
kv_draw_slash(cairo_t * cr, kv_viewport_t * VP, kv_timeline_t * tl, kv_timeline_slash_t * s) {
  if (!s) return;
  double x = s->x;
  double y = tl->y;
  double w = 0.0;
  double h = tl->h;

  if (kv_viewport_clip_trim(VP, &x, &y, &w, &h)) {
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, KV_LINE_WIDTH / VP->zoom_ratio_x);
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x, y + h);
    cairo_stroke(cr);
  }
  kv_draw_slash(cr, VP, tl, s->next);
}

static void
kv_draw_box(cairo_t * cr, kv_viewport_t * VP, kv_timeline_t * tl, kv_timeline_box_t * box, double ratio) {
  if (!box) return;
  GdkRGBA color;
  kv_lookup_color(box->start_e->e / 2, &color.red, &color.green, &color.blue, &color.alpha);

  double x = box->x;
  double y = tl->y;
  double w = box->w;
  double h = tl->h;

  double slip = h * (1 - ratio);
  y += slip / 2.0;
  h -= slip;  
  
  if (kv_viewport_clip_trim(VP, &x, &y, &w, &h)) {
    cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
    cairo_set_line_width(cr, KV_LINE_WIDTH);// / VP->zoom_ratio_x);
    cairo_rectangle(cr, x, y, w, h);
    if (!box->focused)
      cairo_fill(cr);
    else {
      cairo_fill_preserve(cr);
      cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
      cairo_set_line_width(cr, KV_LINE_WIDTH / VP->zoom_ratio_x);
      cairo_stroke(cr);
    }        
  }
  kv_draw_box(cr, VP, tl, box->next, ratio);
  kv_draw_box(cr, VP, tl, box->child, ratio * KV_NESTED_DECREASE_RATE);
}

void
kv_viewport_draw(kv_viewport_t * VP, cairo_t * cr) {
  kv_timeline_set_t * TL = GS->TL;
  
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
    kv_timeline_t * tl = TL->head;
    while (tl) {
      sprintf(s, "Rank %d", tl->trace->rank);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, s);
      y += 2 * KV_RADIUS + KV_GAP_BETWEEN_TIMELINES;
      tl = tl->next;
    }

    /* Lines */
    {
      cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
      cairo_set_line_width(cr, KV_LINE_WIDTH / 5 / VP->zoom_ratio_x);
      double y = KV_RADIUS;
      kv_timeline_t * tl = TL->head;
      while (tl) {
        kv_trace_t * trace = tl->trace;
        double x = KV_TIMELINE_START_X + kv_scale_down(trace, trace->start_t);
        double w = kv_scale_down_span(trace->end_t - trace->start_t);
        double h = 0.0;
        if (kv_viewport_clip_trim(VP, &x, &y, &w, &h)) {
            cairo_move_to(cr, x, y);
            cairo_line_to(cr, x + w, y);
            cairo_stroke(cr);
        }
        y += 2 * KV_RADIUS + KV_GAP_BETWEEN_TIMELINES;
        tl = tl->next;
      }
    }
  }

  /* Timelines: boxes & slashes */
  {
    kv_timeline_t * tl = TL->head;
    while (tl) {
      kv_draw_slash(cr, VP, tl, tl->slash);
      kv_draw_box(cr, VP, tl, tl->box, 1.0);
      tl = tl->next;
    }
  }

  cairo_restore(cr);
}

