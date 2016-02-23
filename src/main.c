#include <pebble.h>
#include <time.h>

//-----REFERENCE VALUES-----
//Pebble default screen size - 144x168 px
//Pebble Time Round screen size - 180x180 px
//--------------------------

static Window *s_main_window; //Always at bottom of window stack, displays "time since checked time" info
static Window *s_time_window; //Pushed & popped from top of window stack on wrist shake, displays time
static TextLayer *s_count_back_layer; //textless TextLayer drawing full black background
static TextLayer *s_count_data_layer; //TextLayer holding count information
static TextLayer *s_count_above_layer; //TextLayer holding "it has been"
static TextLayer *s_count_below_layer; //TextLayer holding "since you last saw the time"
static TextLayer *s_time_back_layer; //same deal
static TextLayer *s_time_time_data_layer; //TextLayer holding current time
static TextLayer *s_time_date_data_layer; //TextLayer holding current date
static TextLayer *s_time_above_layer; //TextLayer holding "it is currently"
static time_t checktime; //struct holding time user last checked watch
static char *days[7] = {"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
static char *months[12] = {"january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december"};

static char* time_between(time_t end, time_t start) {
  static char buffer[11];
  double diff = difftime(end, start);
  if(diff < 3600) {
    if(60 <= diff && diff < 120) {
      strcpy(buffer, "1 minute");
    } 
    else {
      snprintf(buffer, 11, "%d minutes", (int) diff / 60);
    }
  }
  else if(diff < 86400) {
    if(3600 <= diff && diff < 7200) {
      strcpy(buffer, "1 hour");
    } 
    else {
      snprintf(buffer, 11, "%d hours", (int) diff / 3600);
    }
  } 
  else {
    strcpy(buffer, "24+ hours");
  }
  return buffer;
}

static void update_count() {
  time_t curr = time(NULL);
  text_layer_set_text(s_count_data_layer, time_between(curr, checktime));
}

static void update_time() {
  time_t temp = time(NULL);
  struct tm *curr = localtime(&temp);
  static char time_buffer[9];
  strftime(time_buffer, 9, "%I:%M ", curr);
  strcat(time_buffer, curr->tm_hour < 12? "am" : "pm");
  text_layer_set_text(s_time_time_data_layer, time_buffer);
  
  static char date_buffer[27]; //max length of information (corresponds to "on wednesday,\nseptember 10")
  snprintf(date_buffer, 27, "on %s,\n%s %d", days[curr->tm_wday], months[curr->tm_mon], curr->tm_mday);
  text_layer_set_text(s_time_date_data_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if(window_is_loaded(s_time_window)) {
    update_time();
  }
  else {
    update_count();
  }
}


static void format_text_layer(TextLayer *layer, GTextAlignment alignment) {
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_overflow_mode(layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(layer, alignment);
  text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_color(layer, GColorLightGray);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_count_back_layer = text_layer_create(bounds);
  text_layer_set_background_color(s_count_back_layer, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(s_count_back_layer));
  
  s_count_data_layer = text_layer_create(GRect(0, bounds.size.h / 2 - 28, bounds.size.w, 28));
  text_layer_set_background_color(s_count_data_layer, GColorClear);
  text_layer_set_font(s_count_data_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_color(s_count_data_layer, GColorWhite);
  text_layer_set_text_alignment(s_count_data_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_count_data_layer));

  s_count_above_layer = text_layer_create(GRect(bounds.size.w / 2 - PBL_IF_ROUND_ELSE(60, 57), bounds.size.h / 2 - 60, bounds.size.w - PBL_IF_ROUND_ELSE(30, 15), 30));
  format_text_layer(s_count_above_layer, GTextAlignmentLeft);
  text_layer_set_text(s_count_above_layer, "it has been");
  layer_add_child(window_layer, text_layer_get_layer(s_count_above_layer));
  
  s_count_below_layer = text_layer_create(GRect(0, bounds.size.h / 2 + 8, bounds.size.w - PBL_IF_ROUND_ELSE(30, 15), 48));
  format_text_layer(s_count_below_layer, GTextAlignmentRight);
  text_layer_set_text(s_count_below_layer, "since you last\nsaw the time");
  layer_add_child(window_layer, text_layer_get_layer(s_count_below_layer));

}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_count_back_layer);
  text_layer_destroy(s_count_above_layer);
  text_layer_destroy(s_count_below_layer);
  text_layer_destroy(s_count_data_layer);
}

static void time_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_time_back_layer = text_layer_create(bounds);
  text_layer_set_background_color(s_time_back_layer, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(s_time_back_layer));
  
  s_time_time_data_layer = text_layer_create(GRect(0, bounds.size.h / 2 - 28, bounds.size.w, 34));
  text_layer_set_background_color(s_time_time_data_layer, GColorClear);
  text_layer_set_font(s_time_time_data_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_color(s_time_time_data_layer, GColorWhite);
  text_layer_set_text_alignment(s_time_time_data_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_time_data_layer));
  
  s_time_date_data_layer = text_layer_create(GRect(0, bounds.size.h / 2 + 8, bounds.size.w - PBL_IF_ROUND_ELSE(30, 15), 54));
  format_text_layer(s_time_date_data_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_time_date_data_layer));

  s_time_above_layer = text_layer_create(GRect(bounds.size.w / 2 - PBL_IF_ROUND_ELSE(60, 57), bounds.size.h / 2 - 60, bounds.size.w - PBL_IF_ROUND_ELSE(30, 15), 30));
  format_text_layer(s_time_above_layer, GTextAlignmentLeft);
  text_layer_set_text(s_time_above_layer, "it is currently");
  layer_add_child(window_layer, text_layer_get_layer(s_time_above_layer));
}

static void time_window_unload(Window *window) {
  text_layer_destroy(s_time_back_layer);
  text_layer_destroy(s_time_above_layer);
  text_layer_destroy(s_time_time_data_layer);
  text_layer_destroy(s_time_date_data_layer);
}

static void time_window_pop() {
  if(window_is_loaded(s_time_window)) {
    window_stack_pop(true);
    checktime = time(NULL);
    update_count();
  }
}

static void time_window_push(AccelAxisType axis, int32_t direction) {
  if(!window_is_loaded(s_time_window)) {
    window_stack_push(s_time_window, true);
    update_time();
    app_timer_register(5000, time_window_pop, (void *) NULL);
  }
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  s_time_window = window_create();
  window_set_window_handlers(s_time_window, (WindowHandlers) {
    .load = time_window_load,
    .unload = time_window_unload
  });
  
  window_stack_push(s_main_window, true);
  
  checktime = time(NULL);
  
  update_count();
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  accel_tap_service_subscribe(time_window_push);

  window_stack_push(s_time_window, true);
  
  update_time();
 
  app_timer_register(5000, time_window_pop, (void *) NULL);
}

static void deinit() {
  window_destroy(s_main_window);
  window_destroy(s_time_window);
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}