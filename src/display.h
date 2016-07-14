#include <pebble.h>

int get_angle_start_deg();
int get_angle_end_deg();

void draw_display(Window **window, TextLayer **text_layer_time, TextLayer **text_layer_speed,
	TextLayer **text_layer_mph, TextLayer **text_layer_battery, TextLayer **text_layer_temperature,
	BitmapLayer **battery_bitmap_layer, BitmapLayer **temperature_bitmap_layer,BitmapLayer **bt_bitmap_layer, Layer **arc_layer);

void destroy_display();