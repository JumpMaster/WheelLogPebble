#if defined(PBL_RECT)

#include <pebble.h>
#include <display.h>

static GFont font_square_50;
static GFont font_square_20;
static GFont font_square_15;

int get_angle_start_deg() {
	return 235;
}

int get_angle_end_deg() {
	return 485;
}

void draw_display(Window **window, TextLayer **text_layer_time, TextLayer **text_layer_speed,
	TextLayer **text_layer_mph, TextLayer **text_layer_battery, TextLayer **text_layer_temperature,
	BitmapLayer **battery_bitmap_layer, BitmapLayer **temperature_bitmap_layer,BitmapLayer **bt_bitmap_layer, Layer **arc_layer) {
   
	font_square_50 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_50));
	font_square_20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_20));
	font_square_15 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_15));
  
	*text_layer_time = text_layer_create(GRect(0, 0, 144, 50));
	text_layer_set_font(*text_layer_time, font_square_15);

	*arc_layer = layer_create(GRect(10, 22, 124, 124));
	
	*text_layer_speed = text_layer_create(GRect(0, 45, 144, 50));
	text_layer_set_font(*text_layer_speed, font_square_50);

	*text_layer_mph = text_layer_create(GRect(0, 97, 144, 20));
	text_layer_set_font(*text_layer_mph, font_square_20);
	
	*text_layer_battery = text_layer_create(GRect(72, 143, 72, 20));
	text_layer_set_font(*text_layer_battery, font_square_20);

	*text_layer_temperature = text_layer_create(GRect(0, 143, 72, 20));
	text_layer_set_font(*text_layer_temperature, font_square_20);
	
	*battery_bitmap_layer = bitmap_layer_create(GRect(93, 128, 30, 14));
	
	*temperature_bitmap_layer = bitmap_layer_create(GRect(20, 127, 32, 16));
	
	*bt_bitmap_layer = bitmap_layer_create(GRect(120, 0, 24, 24));
}

void destroy_display() {
	fonts_unload_custom_font(font_square_15);
	fonts_unload_custom_font(font_square_20);
	fonts_unload_custom_font(font_square_50);
}

#endif