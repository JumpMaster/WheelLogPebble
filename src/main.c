#include <pebble.h>
#include <inttypes.h>

#define KEY_SPEED  0
#define KEY_BATTERY  1
#define KEY_TEMPERATURE  2
#define KEY_FAN_STATE  3


Window *window;

TextLayer *text_layer_speed;
TextLayer *text_layer_mph;
TextLayer *text_layer_battery;
TextLayer *text_layer_temperature;

static Layer *arc_layer;
AppTimer *graphics_timer;
static BitmapLayer *battery_bitmap_layer;
static BitmapLayer *temperature_bitmap_layer;

static GBitmap *battery_bitmap;
static GBitmap *temperature_bitmap;

int speed = 1;
int battery = 1;
int temperature = 1;
int fan_state = 0;

int new_speed = 0;
int new_battery = 99;
int new_temperature = 20;
int new_fan_state = 0;

char charSpeed[3] = "";
char charBattery[5] = "";
char charTemperature[4] = "";

GFont font_square_50;
GFont font_square_20;

int32_t angle_start;
int32_t angle_end;
int32_t angle_speed;

int current_angle;
int target_angle;

void refresh_arc_callback(void *data) {

	int increment = 1;
	
	if (current_angle < target_angle && target_angle-current_angle >= 2) {
		increment = (target_angle-current_angle) / 2;
		current_angle += increment > 8 ? 8 : increment;
	} else if (current_angle > target_angle && current_angle-target_angle >= 2) {
		increment = (current_angle-target_angle) / 2;
		current_angle -= increment > 8 ? 8 : increment;
	} else {
		current_angle = target_angle;
	}

	angle_speed = DEG_TO_TRIGANGLE(current_angle);
	
	layer_mark_dirty(arc_layer);
}

static void update_display() {
	
	if (new_speed != speed) {
		speed = new_speed;

		target_angle = ((speed*125)/100)+235;

		if (target_angle > 485)
			target_angle = 485;

		snprintf(charSpeed, 3, "%02d", speed/10);
		text_layer_set_text(text_layer_speed, charSpeed);

		refresh_arc_callback(NULL);
	}
	
	if (new_battery != battery) {
		battery = new_battery > 100 ? 100 : new_battery;
		snprintf(charBattery, 5, "%d%%", battery);
		text_layer_set_text(text_layer_battery, charBattery);
		
		if (battery > 90)
			battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_100);
		else if (battery > 80)
			battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_90);
		else if (battery > 70)
			battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_80);
		else if (battery > 60)
			battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_70);
		else if (battery > 50)
			battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_60);
		else if (battery > 40)
			battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_50);
		else if (battery > 30)
			battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_40);
		else if (battery > 20)
			battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_30);
		else if (battery > 15)
			battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_20);
		else
			battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_15);
			
		bitmap_layer_set_compositing_mode(battery_bitmap_layer, GCompOpSet);
		bitmap_layer_set_bitmap(battery_bitmap_layer, battery_bitmap);
	}
	
	if (new_temperature != temperature) {
		temperature = new_temperature;
		snprintf(charTemperature, 4, "%dC", temperature);
		text_layer_set_text(text_layer_temperature, charTemperature);
	}
	
	if (fan_state != new_fan_state) {
		APP_LOG(APP_LOG_LEVEL_INFO, "FAN = %d", new_fan_state);
		fan_state = new_fan_state;
		if (fan_state > 0)
			temperature_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TEMP_RED);
		else
			temperature_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TEMP);
		
		bitmap_layer_set_compositing_mode(temperature_bitmap_layer, GCompOpSet);
		bitmap_layer_set_bitmap(temperature_bitmap_layer, temperature_bitmap);
	}
}

static void update_arcs(Layer *layer, GContext *ctx) {	
	GRect inner_bounds = layer_get_bounds(layer);
	GRect outer_bounds = layer_get_bounds(layer);
	
	inner_bounds.origin.x += 4;
	inner_bounds.origin.y += 4;
	inner_bounds.size.h -= 8;
	inner_bounds.size.w -= 8;

	if (current_angle >= 472) // 19
		graphics_context_set_fill_color(ctx, GColorRed);
	else if (current_angle >= 435) // 16
		graphics_context_set_fill_color(ctx, GColorOrange);
	else if (current_angle >= 397) // 13
		graphics_context_set_fill_color(ctx, GColorChromeYellow);
	else if (current_angle >= 360) // 10
		graphics_context_set_fill_color(ctx, GColorSpringBud);
	else // below 10
		graphics_context_set_fill_color(ctx, GColorMediumSpringGreen);
	
	graphics_fill_radial(ctx, outer_bounds, GOvalScaleModeFitCircle, 10, angle_start, angle_speed);
	
	graphics_context_set_fill_color(ctx, GColorLightGray);
	graphics_fill_radial(ctx, inner_bounds, GOvalScaleModeFitCircle, 2, angle_speed, angle_end);
		
	if (current_angle != target_angle)
		graphics_timer = app_timer_register(30, (AppTimerCallback) refresh_arc_callback, NULL);
}

static void received_handler(DictionaryIterator *iter, void *context) {
	Tuple *speed_tuple = dict_find(iter, KEY_SPEED);
	Tuple *temperature_tuple = dict_find(iter, KEY_TEMPERATURE);
	Tuple *battery_tuple = dict_find(iter, KEY_BATTERY);
	Tuple *fan_state_tuple = dict_find(iter, KEY_FAN_STATE);
	
	if(speed_tuple)
		new_speed = speed_tuple->value->int32;
	
	if (temperature_tuple)
		new_temperature = temperature_tuple->value->int32;
	
	if (battery_tuple)
		new_battery = battery_tuple->value->int32;
	
	if (fan_state_tuple)
		new_fan_state = fan_state_tuple->value->int32;
	
	update_display();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	new_speed += 5;
	update_display();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	new_speed = 200;
	update_display();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (new_speed > 5)
		new_speed -= 5;
	else
		new_speed = 0;
	
	update_display();
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

void handle_init(void) {
	angle_start = DEG_TO_TRIGANGLE(235);
	angle_end = DEG_TO_TRIGANGLE(485);
	angle_speed = angle_start;
	current_angle = 235;
	
	window = window_create();

	arc_layer = layer_create(GRect(10, 5, 124, 124));
	layer_add_child(window_get_root_layer(window), arc_layer);
	layer_set_update_proc(arc_layer, update_arcs);
	
	text_layer_speed = text_layer_create(GRect(0, 35, 144, 50));
	text_layer_set_text_alignment(text_layer_speed, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_speed, GColorClear);
	text_layer_set_text_color(text_layer_speed, GColorWhite);
	font_square_50 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_50));
	text_layer_set_font(text_layer_speed, font_square_50);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_speed));

	text_layer_mph = text_layer_create(GRect(0, 85, 144, 20));
	text_layer_set_text_alignment(text_layer_mph, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_mph, GColorClear);
	text_layer_set_text_color(text_layer_mph, GColorWhite);
	font_square_20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_20));
	text_layer_set_font(text_layer_mph, font_square_20);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_mph));
 	text_layer_set_text(text_layer_mph, "MPH");
	
	text_layer_battery = text_layer_create(GRect(69, 143, 70, 20));
	text_layer_set_text_alignment(text_layer_battery, GTextAlignmentRight);
	text_layer_set_background_color(text_layer_battery, GColorClear);
	text_layer_set_text_color(text_layer_battery, GColorWhite);
	text_layer_set_font(text_layer_battery, font_square_20);
//  	text_layer_set_text(text_layer_battery, "95%");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_battery));

	
	text_layer_temperature = text_layer_create(GRect(10, 143, 50, 20));
	text_layer_set_text_alignment(text_layer_temperature, GTextAlignmentLeft);
	text_layer_set_background_color(text_layer_temperature, GColorClear);
	text_layer_set_text_color(text_layer_temperature, GColorWhite);
	text_layer_set_font(text_layer_temperature, font_square_20);
//  	text_layer_set_text(text_layer_temperature, "45C");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_temperature));
	
	battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_100);
	battery_bitmap_layer = bitmap_layer_create(GRect(95, 128, 30, 14));
	bitmap_layer_set_compositing_mode(battery_bitmap_layer, GCompOpSet);
	bitmap_layer_set_bitmap(battery_bitmap_layer, battery_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(battery_bitmap_layer));
	
	temperature_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TEMP);
	temperature_bitmap_layer = bitmap_layer_create(GRect(15, 127, 32, 16));
	bitmap_layer_set_compositing_mode(temperature_bitmap_layer, GCompOpSet);
	bitmap_layer_set_bitmap(temperature_bitmap_layer, temperature_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(temperature_bitmap_layer));
	
	window_set_background_color(window, GColorBlack);
	window_stack_push(window, true);
	
	window_set_click_config_provider(window, click_config_provider);
	
	const int inbox_size = 128;
	const int outbox_size = 128;
	app_message_open(inbox_size, outbox_size);
	app_message_register_inbox_received(received_handler);
	
	update_display();
}

void handle_deinit(void) {
	text_layer_destroy(text_layer_speed);
	text_layer_destroy(text_layer_mph);
	text_layer_destroy(text_layer_temperature);
	text_layer_destroy(text_layer_battery);
	bitmap_layer_destroy(battery_bitmap_layer);
	bitmap_layer_destroy(temperature_bitmap_layer);
	layer_destroy(arc_layer);
	
	fonts_unload_custom_font(font_square_20);
	fonts_unload_custom_font(font_square_50);

	gbitmap_destroy(battery_bitmap);
	gbitmap_destroy(temperature_bitmap);
	
	window_destroy(window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}