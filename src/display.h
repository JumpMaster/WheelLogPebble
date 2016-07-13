#include <pebble.h>

void draw_display();
void destroy_display();
void update_arc(int speed);
void update_angles(int max_speed);

TextLayer *text_layer_time;
TextLayer *text_layer_speed;
TextLayer *text_layer_mph;
TextLayer *text_layer_battery;
TextLayer *text_layer_temperature;
Layer *arc_layer;

BitmapLayer *battery_bitmap_layer;
BitmapLayer *temperature_bitmap_layer;
BitmapLayer *bt_bitmap_layer;

GBitmap *battery_bitmap;
GBitmap *temperature_bitmap;
GBitmap *bt_bitmap;