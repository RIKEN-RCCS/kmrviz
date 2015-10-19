#include "kmrviz.h"

kv_global_state_t GS[1];

const char * const KV_COLORS[] =
  {"orange", "magenta", "cyan", "azure", "green",
   "gold", "brown1", "burlywood1", "peachpuff", "aquamarine",
   "chartreuse", "skyblue", "burlywood", "cadetblue", "chocolate",
   "coral", "cornflowerblue", "cornsilk4", "darkolivegreen1", "darkorange1",
   "khaki3", "lavenderblush2", "lemonchiffon1", "lightblue1", "lightcyan",
   "lightgoldenrod", "lightgoldenrodyellow", "lightpink2", "lightsalmon2", "lightskyblue1",
   "lightsteelblue3", "lightyellow3", "maroon1", "yellowgreen"};

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

    /* zoomfit button */
    {      
      gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);
      GtkToolItem * btn_zoomfit = gtk_tool_button_new(NULL, NULL);
      gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btn_zoomfit, -1);
      gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(btn_zoomfit), "zoom-fit-best");
      gtk_widget_set_tooltip_text(GTK_WIDGET(btn_zoomfit), "Zoom visualization fitly (F)");
      g_signal_connect(G_OBJECT(btn_zoomfit), "clicked", G_CALLBACK(on_toolbar_zoomfit_button_clicked), (void *) 0);
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
  gtk_container_set_border_width(GTK_CONTAINER(sidebox_box), 5);

  GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(sidebox_box), box, FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(box), 5);

  GtkWidget * align_start = GUI->toolbox.align_start = gtk_check_button_new_with_label("Align start times");
  gtk_box_pack_start(GTK_BOX(box), align_start, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(align_start), GS->align_start);
  g_signal_connect(G_OBJECT(align_start), "toggled", G_CALLBACK(on_toolbox_align_start_toggled), (void *) NULL);

  gtk_widget_show_all(sidebox);
  return sidebox;
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

static int
kv_read_trace(char * filename, kv_trace_t * kt) {
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
  kt->rank = *((int *) dp);
  dp += sizeof(int);
  kt->start_t = *((double *) dp);
  dp += sizeof(double);
  kt->end_t = *((double *) dp);
  dp += sizeof(double);
  kt->n = *((long *) dp);
  dp += sizeof(long);
  printf("  rank=%d\n"
         "  start_t=%21.0lf\n"
         "  end_t  =%21.0lf\n"
         "  n=%ld\n",
         kt->rank, kt->start_t, kt->end_t, kt->n);
  kt->e = (kv_trace_entry_t *) malloc( kt->n * sizeof(kv_trace_entry_t) );
  int i;
  for (i = 0; i < kt->n; i++) {
    kt->e[i].t = *((double *) dp);
    dp += sizeof(double);
    kt->e[i].e = *((int *) dp);
    dp += sizeof(int);
    if (i == 0 || i == kt->n - 1)
      printf("  e[%d]=(%21.0lf,%d)\n",
             i, kt->e[i].t, kt->e[i].e);
  }
  return 1;
}

static void
kv_read_traces(int argc, char * argv[], kv_trace_set_t * TS) {
  /* read traces */
  TS->traces = (kv_trace_t *) malloc( (argc - 1) * sizeof(kv_trace_t) );
  TS->n = 0;
  int i;
  for (i = 1; i < argc; i++) {
    if (kv_read_trace(argv[i], &TS->traces[TS->n]))
      TS->n++;
  }
  printf("ntraces = %d\n", TS->n);
  
  /* find min start_t, max end_t */
  TS->start_t = -1;
  TS->end_t = 0;
  TS->t_span = 0;
  for (i = 0; i < TS->n; i++) {
    kv_trace_t * trace = &TS->traces[i];
    if (TS->start_t < 0 || trace->start_t < TS->start_t)
      TS->start_t = trace->start_t;
    if (trace->end_t > TS->end_t)
      TS->end_t = trace->end_t;
    if (trace->end_t - trace->start_t > TS->t_span)
      TS->t_span = trace->end_t - trace->start_t;
  }
  printf("min start=%.0lf\nmax end  =%.0lf\n", TS->start_t, TS->end_t);

  /* adjust all t based on TS->start_t */
  /*
  for (i = 0; i < TS->n; i++) {
    TS->traces[i].start_t -= TS->start_t;
    TS->traces[i].end_t -= TS->start_t;
    int j;
    for (j = 0; j < TS->traces[i].n; j++)      
      TS->traces[i].e[j].t -= TS->start_t;
  }
  */
}

int
main(int argc, char * argv[]) {
  /* GTK */
  gtk_init(&argc, &argv);

  /* GS */
  kv_global_state_init(GS);

  /* Data */
  kv_read_traces(argc, argv, GS->TS);

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
