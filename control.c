/****************** Processes **************************************/

static void
kv_zoom(kv_viewport_t * VP, double new_zrx, double new_zry, double posx, double posy) {
  posx -= VP->x;
  posy -= VP->y;
  double deltax = posx / VP->zoom_ratio_x * new_zrx - posx;
  double deltay = posy / VP->zoom_ratio_y * new_zry - posy;
  VP->x -= deltax;
  VP->y -= deltay;
  VP->zoom_ratio_x = new_zrx;
  VP->zoom_ratio_y = new_zry;
  kv_viewport_queue_draw(VP);
}

/****************** end of Processes *******************************/



/****************** GUI Callbacks **************************************/

static gboolean
on_window_key_event(_unused_ GtkWidget * widget, GdkEvent * event, _unused_ gpointer user_data) {
  _unused_ GdkModifierType m = gtk_accelerator_get_default_mod_mask();
  GdkEventKey * e = (GdkEventKey *) event;
  switch (e->keyval) {
  case GDK_KEY_Tab: { /* Tab */
    break;
  }
  case GDK_KEY_ISO_Left_Tab: { /* Shift + Tab */
    break;
  }
  default:
    break;
  }
  return FALSE;
}

static void
on_toolbar_zoomfit_button_clicked(_unused_ GtkToolButton * toolbtn, _unused_ gpointer user_data) {
  //kv_do_zoomfit();
}

static gboolean
on_darea_draw_event(_unused_ GtkWidget * widget, cairo_t * cr, gpointer user_data) {
  kv_viewport_t * VP = (kv_viewport_t *) user_data;
  kv_viewport_draw(VP, cr);
  return FALSE;
}

static gboolean
on_darea_scroll_event(_unused_ GtkWidget * widget, _unused_ GdkEventScroll * event, _unused_ gpointer user_data) {
  kv_viewport_t * VP = (kv_viewport_t *) user_data;
  /* Calculate zoom factor */
  double factor = 1.0;
  if (event->direction == GDK_SCROLL_UP)
    factor *= KV_ZOOM_INCREMENT;
  else if (event->direction == GDK_SCROLL_DOWN)
    factor /= KV_ZOOM_INCREMENT;
  /* Apply factor */
  double zoomx = VP->zoom_ratio_x * factor;
  double zoomy = VP->zoom_ratio_y * factor;
  kv_zoom(VP, zoomx, zoomy, event->x, event->y);
  return TRUE;
}

static gboolean
on_darea_button_event(_unused_ GtkWidget * widget, _unused_ GdkEventButton * event, gpointer user_data) {
  kv_viewport_t * VP = (kv_viewport_t *) user_data;

  /* grab focus */
  gtk_widget_grab_focus(VP->darea);
  kv_viewport_queue_draw(VP);
  
  /* drag, click */
  if (event->button == GDK_BUTTON_PRIMARY) { /* left mouse button */
    
    if (event->type == GDK_BUTTON_PRESS) {
      /* Drag */
      VP->drag_on = 1;
      VP->pressx = event->x;
      VP->pressy = event->y;
      VP->accdisx = 0.0;
      VP->accdisy = 0.0;
    }  else if (event->type == GDK_BUTTON_RELEASE) {
      /* Drag */
      VP->drag_on = 0;
      /* Click */
      if (VP->accdisx < KV_SAFE_CLICK_RANGE && VP->accdisy < KV_SAFE_CLICK_RANGE) {
        //double ox = (event->x - VP->x) / VP->zoom_ratio_x;
        //double oy = (event->y - VP->y) / VP->zoom_ratio_y;
        //dv_dag_node_t * node = dv_do_finding_clicked_node(V, ox, oy);
      }
    } else if (event->type == GDK_2BUTTON_PRESS) {
      // nothing to do
    }

  } else if (event->button == GDK_BUTTON_SECONDARY) { /* right mouse button */

    /* TODO: show context menu */
    if (event->type == GDK_BUTTON_PRESS) {

      //double ox = (event->x - VP->x) / VP->zoom_ratio_x;
      //double oy = (event->y - VP->y) / VP->zoom_ratio_y;
      //dv_dag_node_t * node = dv_do_finding_clicked_node(V, ox, oy);
      
    } else if (event->type == GDK_BUTTON_RELEASE) {
      // nothing
    }

  }

  return TRUE;
}

static gboolean
on_darea_motion_event(_unused_ GtkWidget * widget, _unused_ GdkEventMotion * event, gpointer user_data) {
  kv_viewport_t * VP = (kv_viewport_t *) user_data;
  
  /* Drag */
  if (VP->drag_on) {
    double deltax = event->x - VP->pressx;
    double deltay = event->y - VP->pressy;
    VP->x += deltax;
    VP->y += deltay;
    VP->accdisx += deltax;
    VP->accdisy += deltay;
    VP->pressx = event->x;
    VP->pressy = event->y;
    kv_viewport_queue_draw(VP);
  }
  
  return TRUE;
}

static gboolean
on_darea_configure_event(_unused_ GtkWidget * widget, GdkEventConfigure * event, gpointer user_data) {
  kv_viewport_t * VP = (kv_viewport_t *) user_data;
  VP->vpw = event->width;
  VP->vph = event->height;
  return TRUE;
}

static gboolean
on_darea_key_press_event(_unused_ GtkWidget * widget, _unused_ GdkEventConfigure * event, _unused_ gpointer user_data) {
  kv_viewport_t * VP = (kv_viewport_t *) user_data;

  _unused_ GdkModifierType mod = gtk_accelerator_get_default_mod_mask();
  GdkEventKey * e = (GdkEventKey *) event;
  switch (e->keyval) {
  case 65361: /* Left */
    VP->x += 15;
    kv_viewport_queue_draw(VP);
    return TRUE;
  case 65362: /* Up */
    VP->y += 15;
    kv_viewport_queue_draw(VP);
    return TRUE;
  case 65363: /* Right */
    VP->x -= 15;
    kv_viewport_queue_draw(VP);
    return TRUE;
  case 65364: /* Down */
    VP->y -= 15;
    kv_viewport_queue_draw(VP);
    return TRUE;
  default:
    return FALSE;
  }
  return FALSE;        
}

/****************** end of GUI Callbacks *******************************/

