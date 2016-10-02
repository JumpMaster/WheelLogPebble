#include <pebble.h>

void set_angles(int *start, int *end);

void draw_display(Window **window, Layer **gui_layer, Layer **details_layer, TextLayer **text_layer_time, TextLayer **text_layer_speed,
	TextLayer **text_layer_mph, TextLayer **text_layer_battery, TextLayer **text_layer_temperature,
	BitmapLayer **battery_bitmap_layer, BitmapLayer **temperature_bitmap_layer, BitmapLayer **bt_bitmap_layer, Layer **arc_layer,
	TextLayer **text_layer_ride_time, TextLayer **text_layer_distance, TextLayer **text_layer_top_speed);

void destroy_display();