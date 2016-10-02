#if defined(PBL_ROUND)

#include <pebble.h>
#include "display.h"

static GFont font_square_50;
static GFont font_square_20;
static GFont font_square_15;

TextLayer *text_layer_rt_title;
TextLayer *text_layer_d_title;
TextLayer *text_layer_ts_title;

void set_angles(int *start, int *end) {
	*start = (int) 0;
	*end = (int) 360;
}

void draw_display(Window **window, Layer **gui_layer, Layer **details_layer, TextLayer **text_layer_time, TextLayer **text_layer_speed,
	TextLayer **text_layer_mph, TextLayer **text_layer_battery, TextLayer **text_layer_temperature,
	BitmapLayer **battery_bitmap_layer, BitmapLayer **temperature_bitmap_layer,BitmapLayer **bt_bitmap_layer, Layer **arc_layer,
	TextLayer **text_layer_ride_time, TextLayer **text_layer_distance, TextLayer **text_layer_top_speed) {

  Layer *window_layer = window_get_root_layer(*window);
  GRect window_bounds = layer_get_bounds(window_layer);
	*gui_layer = layer_create(window_bounds);
	*details_layer = layer_create(window_bounds);
  
	font_square_50 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_50));
	font_square_20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_20));
	font_square_15 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_15));
  
	*text_layer_time = text_layer_create(GRect(0, 20, 180, 50));
	text_layer_set_font(*text_layer_time, font_square_15);

	*arc_layer = layer_create(layer_get_bounds(window_get_root_layer(*window)));
	
	*text_layer_speed = text_layer_create(GRect(0, 30, 180, 50));
	text_layer_set_font(*text_layer_speed, font_square_50);

	*text_layer_mph = text_layer_create(GRect(0, 82, 180, 20));
	text_layer_set_font(*text_layer_mph, font_square_20);
	
	*text_layer_battery = text_layer_create(GRect(92, 105, 72, 20));
	text_layer_set_font(*text_layer_battery, font_square_20);

	*text_layer_temperature = text_layer_create(GRect(20, 105, 72, 20));
	text_layer_set_font(*text_layer_temperature, font_square_20);
	
	*battery_bitmap_layer = bitmap_layer_create(GRect(103, 132, 30, 14));
	
	*temperature_bitmap_layer = bitmap_layer_create(GRect(45, 133, 32, 16));
	
	*bt_bitmap_layer = bitmap_layer_create(GRect(130, 50, 24, 24));
	
	text_layer_rt_title = text_layer_create(GRect(0, 70, 180, 22));
	*text_layer_ride_time = text_layer_create(GRect(0, 88, 180, 22));
	text_layer_set_background_color(text_layer_rt_title, GColorClear);
	text_layer_set_background_color(*text_layer_ride_time, GColorClear);
	text_layer_set_text_color(text_layer_rt_title, GColorWhite);
	text_layer_set_text_color(*text_layer_ride_time, GColorLightGray);
	text_layer_set_text_alignment(text_layer_rt_title, GTextAlignmentCenter);
	text_layer_set_text_alignment(*text_layer_ride_time, GTextAlignmentCenter);
	text_layer_set_text(text_layer_rt_title, "RIDE TIME");
	text_layer_set_font(text_layer_rt_title, font_square_20);
	text_layer_set_font(*text_layer_ride_time, font_square_20);
	
	text_layer_d_title = text_layer_create(GRect(0, 26, 180, 22));
	*text_layer_distance = text_layer_create(GRect(0, 44, 180, 22));
	text_layer_set_background_color(text_layer_d_title, GColorClear);
	text_layer_set_background_color(*text_layer_distance, GColorClear);
	text_layer_set_text_color(text_layer_d_title, GColorWhite);
	text_layer_set_text_color(*text_layer_distance, GColorLightGray);
	text_layer_set_text_alignment(text_layer_d_title, GTextAlignmentCenter);
	text_layer_set_text_alignment(*text_layer_distance, GTextAlignmentCenter);
	text_layer_set_text(text_layer_d_title, "DISTANCE");
	text_layer_set_font(text_layer_d_title, font_square_20);
	text_layer_set_font(*text_layer_distance, font_square_20);
	
	text_layer_ts_title = text_layer_create(GRect(0, 114, 180, 22));
	*text_layer_top_speed = text_layer_create(GRect(0, 132, 180, 22));
	text_layer_set_background_color(text_layer_ts_title, GColorClear);
	text_layer_set_background_color(*text_layer_top_speed, GColorClear);
	text_layer_set_text_color(text_layer_ts_title, GColorWhite);
	text_layer_set_text_color(*text_layer_top_speed, GColorLightGray);
	text_layer_set_text_alignment(text_layer_ts_title, GTextAlignmentCenter);
	text_layer_set_text_alignment(*text_layer_top_speed, GTextAlignmentCenter);
	text_layer_set_text(text_layer_ts_title, "TOP SPEED");
	text_layer_set_font(text_layer_ts_title, font_square_20);
	text_layer_set_font(*text_layer_top_speed, font_square_20);
	
	layer_add_child(*details_layer, text_layer_get_layer(text_layer_rt_title));
	layer_add_child(*details_layer, text_layer_get_layer(*text_layer_ride_time));
	layer_add_child(*details_layer, text_layer_get_layer(text_layer_d_title));
	layer_add_child(*details_layer, text_layer_get_layer(*text_layer_distance));
	layer_add_child(*details_layer, text_layer_get_layer(text_layer_ts_title));
	layer_add_child(*details_layer, text_layer_get_layer(*text_layer_top_speed));
}

void destroy_display() {
  text_layer_destroy(text_layer_rt_title);
	text_layer_destroy(text_layer_d_title);
	text_layer_destroy(text_layer_ts_title);
	fonts_unload_custom_font(font_square_15);
	fonts_unload_custom_font(font_square_20);
	fonts_unload_custom_font(font_square_50);
}

#endif