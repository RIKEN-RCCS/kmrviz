/****************** Processes **************************************/


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
  //kv_viewport_t * VP = (kv_viewport_t *) user_data;
  return TRUE;
}

static gboolean
on_darea_button_event(_unused_ GtkWidget * widget, _unused_ GdkEventButton * event, gpointer user_data) {
  kv_viewport_t * VP = (kv_viewport_t *) user_data;

  /* grab focus */
  gtk_widget_grab_focus(VP->darea);
  kv_viewport_queue_draw(VP);
  
  /* node clicking, dag dragging */
  /*
  if ( VP->mV[ CS->activeV - CS->V ] ) {
    dv_do_button_event(CS->activeV, event);
  } else {
    int i;
    for (i = 0; i < CS->nV; i++)
      if ( VP->mV[i] )
        dv_do_button_event(&CS->V[i], event);
  }
  */

  return TRUE;
}

static gboolean
on_darea_motion_event(_unused_ GtkWidget * widget, _unused_ GdkEventMotion * event, gpointer user_data) {
  _unused_ kv_viewport_t * VP = (kv_viewport_t *) user_data;
  
  /*
  if ( CS->activeV && VP->mV[ CS->activeV - CS->V ] ) {
    dv_do_motion_event(CS->activeV, event);
  } else {
    int i;
    for (i = 0; i < CS->nV; i++)
      if ( VP->mV[i] )
        dv_do_motion_event(&CS->V[i], event);
  }
  */
  
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

