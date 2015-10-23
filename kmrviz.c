#include "kmrviz.h"

kv_global_state_t GS[1];

const char * const KV_COLORS[] =
  {"orange", "magenta", "cyan", "azure", "green",
   "gold", "brown1", "burlywood1", "peachpuff", "aquamarine",
   "chartreuse", "skyblue", "burlywood", "cadetblue", "chocolate",
   "coral", "cornflowerblue", "cornsilk4", "darkolivegreen1", "darkorange1",
   "khaki3", "lavenderblush2", "lemonchiffon1", "lightblue1", "lightcyan",
   "lightgoldenrod", "lightgoldenrodyellow", "lightpink2", "lightsalmon2", "lightskyblue1",
   "lightsteelblue3", "lightyellow3", "maroon1", "yellowgreen", "red",
   "blue"
  };

#include "control.c"

void
kv_viewport_init(kv_viewport_t * VP) {
  memset(VP, 0, sizeof(kv_viewport_t));
  GdkRGBA white[1];
  gdk_rgba_parse(white, "white");

  /* width, height */
  VP->vpw = VP->vph = 0.0;
  VP->x = VP->y = KV_MARGIN_WHEN_ZOOMFIT;
  
  /* Box */
  VP->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  
  /* Drawing Area */
  VP->darea = gtk_drawing_area_new();
  gtk_box_pack_end(GTK_BOX(VP->box), VP->darea, TRUE, TRUE, 0);
  GtkWidget * darea = VP->darea;
  gtk_widget_override_background_color(GTK_WIDGET(darea), GTK_STATE_FLAG_NORMAL, white);
  gtk_widget_set_can_focus(darea, TRUE);
  gtk_widget_add_events(GTK_WIDGET(darea), GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
  g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_darea_draw_event), (void *) VP);
  g_signal_connect(G_OBJECT(darea), "scroll-event", G_CALLBACK(on_darea_scroll_event), (void *) VP);
  g_signal_connect(G_OBJECT(darea), "button-press-event", G_CALLBACK(on_darea_button_event), (void *) VP);
  g_signal_connect(G_OBJECT(darea), "button-release-event", G_CALLBACK(on_darea_button_event), (void *) VP);
  g_signal_connect(G_OBJECT(darea), "motion-notify-event", G_CALLBACK(on_darea_motion_event), (void *) VP);
  g_signal_connect(G_OBJECT(darea), "configure-event", G_CALLBACK(on_darea_configure_event), (void *) VP);
  g_signal_connect(G_OBJECT(darea), "key-press-event", G_CALLBACK(on_darea_key_press_event), (void *) VP);

  /* zoom ratios */
  VP->zoom_ratio_x = VP->zoom_ratio_y = 1.0;
}

void
kv_viewport_queue_draw(kv_viewport_t * VP) {
  gtk_widget_queue_draw(VP->darea);
}

double
kv_viewport_clip_get_bound_left(kv_viewport_t * VP) {
  return (0 - VP->x) / VP->zoom_ratio_x;
}

double
kv_viewport_clip_get_bound_right(kv_viewport_t * VP) {
  return (VP->vpw - VP->x) / VP->zoom_ratio_x;
}

double
kv_viewport_clip_get_bound_up(kv_viewport_t * VP) {
  return (0 - VP->y) / VP->zoom_ratio_y;
}

double
kv_viewport_clip_get_bound_down(kv_viewport_t * VP) {
  return (VP->vph - VP->y) / VP->zoom_ratio_y;
}

void
kv_gui_init(kv_gui_t * GUI) {
  memset(GUI, 0, sizeof(kv_gui_t));
}

GtkWidget *
kv_gui_get_main_window(kv_gui_t * GUI) {
  if (GUI->window)
    return GUI->window;
  
  /* Window */
  GtkWidget * window = GUI->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_maximize(GTK_WINDOW(window));
  gtk_window_set_title(GTK_WINDOW(window), "KMRViz");
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(on_window_key_event), NULL);

  /* Accelerator group */
  GtkAccelGroup * accel_group = GUI->accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
  
  /* Window_Box */
  GtkWidget * window_box = GUI->window_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(window), window_box);
  
  /* Menubar */
  {
    GtkBuilder * builder = GUI->builder = gtk_builder_new();
    GError * gerr = NULL;
    gtk_builder_add_from_file(builder, "gui/menubar.ui", &gerr);
    if (gerr) {
      g_error("ERROR: %s\n", gerr->message);
      exit(1);
    }
    gtk_builder_connect_signals(builder, NULL);
    GtkWidget * menubar = GUI->menubar = (GtkWidget *) gtk_builder_get_object(builder, "menubar");
    gtk_box_pack_start(GTK_BOX(window_box), menubar, FALSE, FALSE, 0);
    gtk_widget_show_all(menubar);
    
    GtkWidget * item;
    
    item = GTK_WIDGET(gtk_builder_get_object(builder, "exit"));
    gtk_widget_add_accelerator(item, "activate", accel_group, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    item = GTK_WIDGET(gtk_builder_get_object(builder, "toolbox"));
    GUI->onmenubar.toolbox = GTK_CHECK_MENU_ITEM(item);
    gtk_widget_add_accelerator(item, "activate", accel_group, GDK_KEY_t, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    item = GTK_WIDGET(gtk_builder_get_object(builder, "infobox"));
    GUI->onmenubar.infobox = GTK_CHECK_MENU_ITEM(item);
    gtk_widget_add_accelerator(item, "activate", accel_group, GDK_KEY_i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    item = GTK_WIDGET(gtk_builder_get_object(builder, "zoomfit_hor"));
    gtk_widget_add_accelerator(item, "activate", accel_group, GDK_KEY_h, 0, GTK_ACCEL_VISIBLE); 
    item = GTK_WIDGET(gtk_builder_get_object(builder, "zoomfit_ver"));
    gtk_widget_add_accelerator(item, "activate", accel_group, GDK_KEY_v, 0, GTK_ACCEL_VISIBLE); 
    item = GTK_WIDGET(gtk_builder_get_object(builder, "zoomfit_full"));
    gtk_widget_add_accelerator(item, "activate", accel_group, GDK_KEY_f, 0, GTK_ACCEL_VISIBLE); 
  }

  /* Toolbar */
  {
    GtkWidget * toolbar = GUI->toolbar = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(window_box), toolbar, FALSE, FALSE, 0);
    
    /* toolbox button */
    {
      GtkToolItem * btn_settings = GUI->ontoolbar.toolbox = gtk_toggle_tool_button_new();
      gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btn_settings, -1);
      gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(btn_settings), "preferences-system");
      gtk_widget_set_tooltip_text(GTK_WIDGET(btn_settings), "Show toolbox (Ctrl+T)");
      gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(btn_settings), FALSE);
      g_signal_connect(G_OBJECT(btn_settings), "toggled", G_CALLBACK(on_toolbar_toolbox_button_toggled), NULL);
    }

    /* infobox button */
    {
      GtkToolItem * btn_info = GUI->ontoolbar.infobox = gtk_toggle_tool_button_new();
      gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btn_info, -1);
      gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(btn_info), "dialog-info");
      gtk_widget_set_tooltip_text(GTK_WIDGET(btn_info), "Show info box (Ctrl+I)");
      gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(btn_info), FALSE);
      g_signal_connect(G_OBJECT(btn_info), "toggled", G_CALLBACK(on_toolbar_infobox_button_toggled), NULL);
    }

    /* zoomfit button */
    {      
      gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);
      GtkToolItem * btn_zoomfit = gtk_menu_tool_button_new(NULL, NULL);
      gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btn_zoomfit, -1);
      gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(btn_zoomfit), "zoom-fit-best");
      gtk_widget_set_tooltip_text(GTK_WIDGET(btn_zoomfit), "Zoom fully fit (F)");
      g_signal_connect(G_OBJECT(btn_zoomfit), "clicked", G_CALLBACK(on_toolbar_zoomfit_button_clicked), (void *) 0);

      GtkWidget * menu = gtk_menu_new();
      gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(btn_zoomfit), menu);
      GtkWidget * item;
      
      item = gtk_menu_item_new_with_label("Zoom horizontally fit (H)");
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
      g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_toolbar_zoomfit_button_clicked), (void *) 1);
      
      item = gtk_menu_item_new_with_label("Zoom vertically fit (V)");
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
      g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_toolbar_zoomfit_button_clicked), (void *) 2);

      gtk_widget_show_all(menu);      
    }
  }
  
  /* Main box */
  {
    GtkWidget * main_box = GUI->main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(window_box), main_box, TRUE, TRUE, 0);
    GtkWidget * left_sidebar = GUI->left_sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_box_pack_start(GTK_BOX(GUI->main_box), left_sidebar, FALSE, FALSE, 0);
  }

  /* Statusbars */
  {
    GtkWidget * statusbar_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(window_box), statusbar_box, FALSE, FALSE, 0);
    GtkWidget * statusbar_box_2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(statusbar_box), statusbar_box_2, TRUE, TRUE, 0);
    gtk_box_set_homogeneous(GTK_BOX(statusbar_box_2), TRUE);
    GUI->statusbar1 = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(statusbar_box_2), GUI->statusbar1, TRUE, TRUE, 0);
    gtk_widget_set_tooltip_text(GTK_WIDGET(GUI->statusbar1), "Interaction status");
  
    GtkWidget * statusbar_box_3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(statusbar_box_2), statusbar_box_3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(statusbar_box_3), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 0);
    GUI->statusbar2 = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(statusbar_box_3), GUI->statusbar2, TRUE, TRUE, 0);
    gtk_widget_set_tooltip_text(GTK_WIDGET(GUI->statusbar2), "Selection status");
  
    GUI->statusbar3 = gtk_statusbar_new();
    gtk_box_pack_end(GTK_BOX(statusbar_box), GUI->statusbar3, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(GTK_WIDGET(GUI->statusbar3), "Memory pool status");
    gtk_box_pack_end(GTK_BOX(statusbar_box), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 0);
  }

  return window;
}

GtkWidget *
kv_gui_get_toolbox_sidebox(kv_gui_t * GUI) {
  if (GUI->toolbox.sidebox)
    return GUI->toolbox.sidebox;
  GtkWidget * sidebox = GUI->toolbox.sidebox = gtk_frame_new("Toolbox");
  gtk_container_set_border_width(GTK_CONTAINER(sidebox), 5);
  g_object_ref(sidebox);  

  GtkWidget * sidebox_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add(GTK_CONTAINER(sidebox), sidebox_box);
  gtk_container_set_border_width(GTK_CONTAINER(sidebox_box), 3);

  GtkWidget * box;
  
  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
  gtk_box_pack_start(GTK_BOX(sidebox_box), box, FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(box), 3);

  GtkWidget * align_start = GUI->toolbox.align_start = gtk_check_button_new_with_label("Align start times");
  gtk_box_pack_start(GTK_BOX(box), align_start, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(align_start), GS->align_start);
  g_signal_connect(G_OBJECT(align_start), "toggled", G_CALLBACK(on_toolbox_align_start_toggled), (void *) NULL);

  gtk_widget_show_all(sidebox);
  return sidebox;
}

GtkWidget *
kv_gui_get_infobox_sidebox(kv_gui_t * GUI) {
  if (GUI->infobox.sidebox)
    return GUI->infobox.sidebox;
  GtkWidget * sidebox = GUI->infobox.sidebox = gtk_frame_new("Info box");
  gtk_container_set_border_width(GTK_CONTAINER(sidebox), 5);
  g_object_ref(sidebox);

  GtkWidget * sidebox_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_size_request(GTK_WIDGET(sidebox_box), 190, -1);
  gtk_container_add(GTK_CONTAINER(sidebox), sidebox_box);
  gtk_container_set_border_width(GTK_CONTAINER(sidebox_box), 3);

  GtkWidget * box;
  
  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
  gtk_box_pack_start(GTK_BOX(sidebox_box), box, FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(box), 3);
  GtkWidget * type = GUI->infobox.type = gtk_label_new("Kind:");
  gtk_box_pack_start(GTK_BOX(box), type, FALSE, FALSE, 0);

  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
  gtk_box_pack_start(GTK_BOX(sidebox_box), box, FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(box), 3);
  GtkWidget * start_t = GUI->infobox.start_t = gtk_label_new("Start time:");
  gtk_box_pack_start(GTK_BOX(box), start_t, FALSE, FALSE, 0);

  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
  gtk_box_pack_start(GTK_BOX(sidebox_box), box, FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(box), 3);
  GtkWidget * end_t = GUI->infobox.end_t = gtk_label_new("End time:");
  gtk_box_pack_start(GTK_BOX(box), end_t, FALSE, FALSE, 0);

  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
  gtk_box_pack_start(GTK_BOX(sidebox_box), box, FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(box), 3);
  GtkWidget * span = GUI->infobox.span = gtk_label_new("Span:");
  gtk_box_pack_start(GTK_BOX(box), span, FALSE, FALSE, 0);

  gtk_widget_show_all(sidebox);
  return sidebox;
}

void
kv_gui_update_infobox(kv_timeline_box_t * box) {
  if (!GS->infobox_shown) return;
  char s[KV_STRING_LENGTH];
  sprintf(s, "Kind: %s", kv_trace_event_get_kind(box->start_e->e));
  gtk_label_set_text(GTK_LABEL(GS->GUI->infobox.type), s);
  sprintf(s, "Start time: %.0lf", box->start_e->t - GS->TS->start_t);
  gtk_label_set_text(GTK_LABEL(GS->GUI->infobox.start_t), s);
  sprintf(s, "End time: %.0lf", box->end_e->t - GS->TS->start_t);
  gtk_label_set_text(GTK_LABEL(GS->GUI->infobox.end_t), s);
  sprintf(s, "Span: %.0lf", box->end_e->t - box->start_e->t);
  gtk_label_set_text(GTK_LABEL(GS->GUI->infobox.span), s);
}

void
kv_global_state_init(kv_global_state_t * GS) {
  memset(GS, 0, sizeof(kv_global_state_t));
  kv_gui_init(GS->GUI);
  kv_viewport_init(GS->VP);
  GS->align_start = 1;
}

static void
kv_open_gui(_unused_ int argc, _unused_ char * argv[]) {
  /* Main window */
  GtkWidget * window = kv_gui_get_main_window(GS->GUI);

  /* Viewport */
  kv_viewport_t * VP = GS->VP;
  gtk_box_pack_end(GTK_BOX(GS->GUI->main_box), VP->box, TRUE, TRUE, 0);

  /* Run main loop */
  gtk_widget_show_all(window);
}

_static_unused_ int
kv_read_trace_txt(char * filename, kv_trace_t * trace) {
  FILE * fs = fopen(filename, "r");
  if (fs == NULL) {
    fprintf(stderr, "fopen: %d\n", errno);
    return 0;
  }
  printf("Trace %s:\n", filename);

  fscanf(fs, "rank: %d\n", &trace->rank);
  fscanf(fs, "start_t: %lf\n", &trace->start_t);
  fscanf(fs, "end_t: %lf\n", &trace->end_t);
  fscanf(fs, "n: %ld\n", &trace->n);
  printf("  rank=%d\n"
         "  start_t=%.0lf\n"
         "  end_t  =%.0lf\n"
         "  n=%ld\n",
         trace->rank, trace->start_t, trace->end_t, trace->n);
  trace->e = (kv_trace_entry_t *) malloc( trace->n * sizeof(kv_trace_entry_t) );
  int i;
  for (i = 0; i < trace->n; i++) {
    fscanf(fs, "event %d at t=%lf\n", &(trace->e[i].e), &(trace->e[i].t));
    if (i == 0 || i == trace->n - 1)
      printf("  e[%d]=(%.0lf,%d)\n",
             i, trace->e[i].t, trace->e[i].e);
  }
  return 1;
}

_static_unused_ int
kv_read_trace_bin(char * filename, kv_trace_t * trace) {
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "open: %d\n", errno);
    return 0;
  }
  struct stat statbuf[1];
  int err = fstat(fd, statbuf);
  if (err < 0) {
    fprintf(stderr, "fstat: %d\n", errno);
    return 0;    
  }
  printf("Trace %s:\n", filename);
  printf("  st_size = %ld bytes (%0.0lfMB)\n",
         (long) statbuf->st_size,
         ((double) statbuf->st_size) / (1024.0 * 1024.0));

  void * dp = mmap(0, statbuf->st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (!dp) {
    fprintf(stderr, "mmap: error\n");
    return 0;
  }

  /* endian checker */
  //int data_endianness = KV_BIG_ENDIAN;
  int data_endianness = KV_GET_FIRST_BYTE(dp);
  dp += sizeof(uint32_t);
  int host_checker = KV_ENDIAN_CHECKER;
  int host_endianness = KV_GET_FIRST_BYTE(&host_checker);
  int do_swap_byte = (host_endianness != data_endianness);
  //printf("data endian: %d, host endian: %d, do swap byte: %d\n", data_endianness, host_endianness, do_swap_byte);
  
  /* rank */
  if (!do_swap_byte) {
    trace->rank = *((int *) dp);
  } else {
    kv_swap_bytes(dp, &trace->rank, sizeof(int));
  }
  dp += sizeof(int);
  
  /* start_t */
  if (!do_swap_byte) {
    trace->start_t = *((double *) dp);
  } else {
    kv_swap_bytes(dp, &trace->start_t, sizeof(double));
  }
  dp += sizeof(double);

  /* end_t */
  if (!do_swap_byte) {
    trace->end_t = *((double *) dp);
  } else {
    kv_swap_bytes(dp, &trace->end_t, sizeof(double));
  }
  dp += sizeof(double);

  /* n */
  if (!do_swap_byte) {
    trace->n = *((long *) dp);
  } else {
    kv_swap_bytes(dp, &trace->n, sizeof(long));
  }
  dp += sizeof(long);

  
  printf("  rank=%d, t=(%.0lf,%.0lf, n=%ld\n",
         trace->rank, trace->start_t, trace->end_t, trace->n);
  
  /* e */
  trace->e = (kv_trace_entry_t *) malloc( trace->n * sizeof(kv_trace_entry_t) );
  int i;
  for (i = 0; i < trace->n; i++) {
    /* e.t */
    if (!do_swap_byte) {
      trace->e[i].t = *((double *) dp);
    } else {
      kv_swap_bytes(dp, &(trace->e[i].t), sizeof(double));
    }
    dp += sizeof(double);
    /* e.e */
    if (!do_swap_byte) {
      trace->e[i].e = *((int *) dp);
    } else {
      kv_swap_bytes(dp, &(trace->e[i].e), sizeof(int));
    }
    dp += sizeof(int);
    /*
    if (i == 0 || i == trace->n - 1)
      printf("  e[%d]=(%.0lf,%d)\n",
             i, trace->e[i].t, trace->e[i].e);
    */
  }
  return 1;
}

static int
kv_read_trace(char * filename, kv_trace_t * trace) {
  int ret = 0;
  char * subs;
  if ( (subs = strstr(filename, "txt")) && (strlen(subs) == 3) ) {
    if (kv_read_trace_txt(filename, trace))
      ret = 1;
  } else if ( (subs = strstr(filename, "bin")) && (strlen(subs) == 3) ) {
    if (kv_read_trace_bin(filename, trace))
      ret = 1;
  } else {
    if (kv_read_trace_bin(filename, trace))
      ret = 1;
  }
  return ret;
}

static int
kv_replace_asterisk(kv_trace_set_t * TS, char * filename) {
  char * pos = filename;
  while (*pos != '\0' && *pos != '*') {
    pos++;
  }
  if (*pos == '*') {
    *pos = '\0';
    char newfn[1000];
    int v = 0;
    while (1) {
      sprintf(newfn, "%s%d%s", filename, v, pos + 1);
      if (!kv_replace_asterisk(TS, newfn))
        break;
      v++;
    }
    *pos = '*';
  } else if (*pos == '\0') {
    kv_trace_t * trace = (kv_trace_t *) malloc(  sizeof(kv_trace_t) );
    if (kv_read_trace(filename, trace)) {
      if (TS->head == NULL) {
        TS->head = TS->tail = trace;
      } else {
        trace->next = NULL;
        TS->tail->next = trace;
        TS->tail = trace;
      }
      TS->n++;
      return 1;
    }
    free(trace);
  }
  return 0;
}

static void
kv_read_traces(int argc, char * argv[], kv_trace_set_t * TS) {
  /* read traces */
  TS->n = 0;
  TS->head = TS->tail = NULL;
  int i;
  for (i = 1; i < argc; i++) {
    char * filename = argv[i];
    kv_replace_asterisk(TS, filename);    
  }
  printf("ntraces = %d\n", TS->n);
  
  /* find min start_t, max end_t */
  TS->start_t = -1;
  TS->end_t = 0;
  TS->t_span = 0;
  kv_trace_t * trace = TS->head;
  while (trace) {
    if (TS->start_t < 0 || trace->start_t < TS->start_t)
      TS->start_t = trace->start_t;
    if (trace->end_t > TS->end_t)
      TS->end_t = trace->end_t;
    if (trace->end_t - trace->start_t > TS->t_span)
      TS->t_span = trace->end_t - trace->start_t;
    trace = trace->next;
  }
  printf("min start=%.0lf\nmax end  =%.0lf\n", TS->start_t, TS->end_t);

  /* adjust all t based on TS->start_t */
  /*
  kv_trace_t * trace = TS->head;
  while (trace) {
    trace->start_t -= TS->start_t;
    trace->end_t -= TS->start_t;
    int j;
    for (j = 0; j < trace->n; j++)
      trace->e[j].t -= TS->start_t;
    trace = trace->next;
  }
  */
}

static void
kv_timeline_insert_slash(kv_timeline_t * tl, kv_trace_entry_t * e) {
  kv_timeline_slash_t * slash = (kv_timeline_slash_t *) malloc( sizeof(kv_timeline_slash_t) );
  slash->e = e;
  slash->next = NULL;
  if (!tl->slash) {
    tl->slash = slash;
    return;
  } else if (tl->slash->e->t > slash->e->t) {
    slash->next = tl->slash;
    tl->slash = slash;
    return;
  }
  kv_timeline_slash_t * s = tl->slash;
  while (s->next && s->next->e->t <= slash->e->t)
    s = s->next;
  slash->next = s->next;
  s->next = slash;
}

static void
kv_timeline_box_init(kv_timeline_box_t * box) {
  memset(box, 0, sizeof(kv_timeline_box_t));
}

static void
kv_insert_box(kv_timeline_box_t ** boxlist, kv_timeline_box_t * mybox, kv_trace_entry_t * e1, kv_trace_entry_t * e2) {
  kv_timeline_box_t * box;
  if (mybox) {
    box = mybox;
  } else {     
    box = (kv_timeline_box_t *) malloc( sizeof(kv_timeline_box_t) );
    kv_timeline_box_init(box);
    box->start_e = e1;
    box->end_e = e2;
    box->child = box->next = NULL;
  }
  if (!*boxlist) {
    *boxlist = box;
    return;
  } else if ((*boxlist)->start_e->t > box->start_e->t) {
    box->next = *boxlist;
    (*boxlist) = box;
    return;
  }
  kv_timeline_box_t * b = *boxlist;
  while (b->next && b->next->start_e->t <= box->start_e->t)
    b = b->next;
  box->next = b->next;
  b->next = box;
}

static void
kv_collect_boxes(kv_timeline_box_t * box) {
  while (box) {
    kv_timeline_box_t * b = box->next;
    while (b && b->start_e->t < box->end_e->t) {
      box->next = b->next;
      b->next = NULL;
      kv_insert_box(&(box->child), b, NULL, NULL);
      b = box->next;
    }
    kv_collect_boxes(box->child);
    box = box->next;
  }
}

static void
kv_build_timelines(kv_trace_set_t * TS, kv_timeline_set_t * TL) {
  TL->TS = TS;
  TL->n = 0;
  TL->head = TL->tail = NULL;
  kv_trace_t * trace = TS->head;
  while (trace) {
    kv_timeline_t * tl = (kv_timeline_t *) malloc( sizeof(kv_timeline_t) );
    tl->trace = trace;
    tl->box = NULL;
    tl->slash = NULL;
    tl->next = NULL;

    kv_trace_entry_t * e;
    e = (kv_trace_entry_t *) malloc( sizeof(kv_trace_entry_t) );
    e->t = trace->start_t;
    e->e = kmr_trace_event_trace_start;
    kv_timeline_insert_slash(tl, e);
    e = (kv_trace_entry_t *) malloc( sizeof(kv_trace_entry_t) );
    e->t = trace->end_t;
    e->e = kmr_trace_event_trace_end;
    kv_timeline_insert_slash(tl, e);
    
    int ibase = 0;
    int i = 0; /* close */
    while (i < trace->n) {
      while (trace->e[i].e % 2 != 1 && i < trace->n)
        i++;
      int j = i - 1; /* open */
      while (j >= ibase) {
        while (trace->e[j].e != trace->e[i].e - 1) {
          kv_timeline_insert_slash(tl, &trace->e[j]);
          j--;
        }
        if (j >= ibase) {
          kv_insert_box(&(tl->box), NULL, &trace->e[j], &trace->e[i]);
          i++;
          j--;
        } else {
          kv_timeline_insert_slash(tl, &trace->e[i]);
          i++;
          break;
        }
      }
      ibase = i;
    }

    kv_collect_boxes(tl->box);
    
    if (TL->head == NULL) {
      TL->head = TL->tail = tl;
    } else {
      TL->tail->next = tl;
      TL->tail = tl;
    }
    TL->n++;
    trace = trace->next;
  }
}

static void
kv_layout_slash(kv_timeline_t * tl, kv_timeline_slash_t * slash) {
  if (!slash) return;
  slash->x = KV_TIMELINE_START_X + kv_scale_down(tl->trace, slash->e->t);
  kv_layout_slash(tl, slash->next);
}

static void
kv_layout_box(kv_timeline_t * tl, kv_timeline_box_t * box) {
  if (!box) return;
  box->x = KV_TIMELINE_START_X + kv_scale_down(tl->trace, box->start_e->t);
  box->w = kv_scale_down_span(box->end_e->t - box->start_e->t);
  kv_layout_box(tl, box->child);
  kv_layout_box(tl, box->next);
}

void
kv_layout_timelines(kv_timeline_set_t * TL) {
  double y = 0.0;
  double h = 2 * KV_RADIUS;
  kv_timeline_t * tl = TL->head;
  while (tl) {
    tl->y = y;
    tl->h = h;
    kv_layout_slash(tl, tl->slash);
    kv_layout_box(tl, tl->box);
    y += 2 * KV_RADIUS + KV_GAP_BETWEEN_TIMELINES;
    tl = tl->next;
  }
}

int
main(int argc, char * argv[]) {
  /* GTK */
  gtk_init(&argc, &argv);

  /* GS */
  kv_global_state_init(GS);

  /* Data */
  kv_read_traces(argc, argv, GS->TS);
  kv_build_timelines(GS->TS, GS->TL);
  kv_layout_timelines(GS->TL);

  /* Open GUI */
  kv_open_gui(argc, argv);
  gtk_widget_grab_focus(GS->VP->darea);
  char s[KV_STRING_LENGTH];
  sprintf(s, "Ranks: %d", GS->TS->n);
  gtk_statusbar_push(GTK_STATUSBAR(GS->GUI->statusbar3), 0, s);
  sprintf(s, "Span: %.0lf ns", GS->TS->t_span);
  gtk_statusbar_push(GTK_STATUSBAR(GS->GUI->statusbar2), 0, s);

  /* Run */
  gtk_main();
  return 1;
}
