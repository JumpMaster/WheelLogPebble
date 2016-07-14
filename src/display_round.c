#if defined(PBL_ROUND)

#include <pebble.h>
#include <display.h>

static GFont font_square_50;
static GFont font_square_20;
static GFont font_square_15;

int get_angle_start_deg() {
	return 0;
}

int get_angle_end_deg() {
	return 360;
}

void draw_display(Window **window, TextLayer **text_layer_time, TextLayer **text_layer_speed,
	TextLayer **text_layer_mph, TextLayer **text_layer_battery, TextLayer **text_layer_temperature,
	BitmapLayer **battery_bitmap_layer, BitmapLayer **temperature_bitmap_layer,BitmapLayer **bt_bitmap_layer, Layer **arc_layer) {

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
}

void destroy_display() {
	fonts_unload_custom_font(font_square_15);
	fonts_unload_custom_font(font_square_20);
	fonts_unload_custom_font(font_square_50);
}

#endif