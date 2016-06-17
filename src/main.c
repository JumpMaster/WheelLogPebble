#include <pebble.h>
#include <inttypes.h>

#define KEY_SPEED  0
#define KEY_BATTERY  1
#define KEY_TEMPERATURE  2
#define KEY_FAN_IS_ON  3


Window *window;

TextLayer *text_layer;
TextLayer *text_layer_mph;
TextLayer *text_layer_battery;
TextLayer *text_layer_temperature;

static Layer *s_layer;
AppTimer *graphics_timer;
static BitmapLayer *battery_bitmap_layer;
static BitmapLayer *temperature_bitmap_layer;

static GBitmap *battery_bitmap;
static GBitmap *temperature_bitmap;

int speed = 0;
int battery;
int temperature;

char *charSpeed = "";

GFont font_square_50;
GFont font_square_20;

int32_t angle_start;
int32_t angle_end;
int32_t angle_speed;
int current_angle;
int target_angle;

char *itoa(int num)
{
	static char buff[3] = {};
	int length = 0;
	char *string = buff;
  
	if(num >= 0) {
		if (num < 10)
			length = 1;
		else
			length = 2;
		
		if (length == 1) {
			buff[0] = '0';
			buff[1] = '0' + (num % 10);
			buff[2] = '\0';
		} else {
			buff[1] = '0' + (num % 10);
			buff[0] = '0' + ((num/10) % 10);
			buff[2] = '\0';
		}
	} else {
		buff[0] = '0';
		buff[1] = '0';
		buff[2] = '\0';
	}
  return string;
}

void refresh_callback(void *data) {

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
	
	layer_mark_dirty(s_layer);
}

static void update_speed() {
	target_angle = ((speed*125)/100)+235;
	
	if (target_angle > 485)
		target_angle = 485;

	text_layer_set_text(text_layer, itoa(speed/10));

	refresh_callback(NULL);
}

static void update_display(Layer *layer, GContext *ctx) {	
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
		graphics_timer = app_timer_register(30, (AppTimerCallback) refresh_callback, NULL);
}

static void received_handler(DictionaryIterator *iter, void *context) {
	Tuple *speed_tuple = dict_find(iter, KEY_SPEED);
	if(speed_tuple) {
		speed = speed_tuple->value->int32;
		update_speed();
	}
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	speed += 5;
	update_speed();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	speed = 200;
	update_speed();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (speed > 5)
		speed -= 5;
	else
		speed = 0;
	
	update_speed();
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

	s_layer = layer_create(GRect(10, 5, 124, 124));
	layer_add_child(window_get_root_layer(window), s_layer);
	layer_set_update_proc(s_layer, update_display);
	
	text_layer = text_layer_create(GRect(0, 35, 144, 50));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorWhite);
	font_square_50 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_50));
	text_layer_set_font(text_layer, font_square_50);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));

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
 	text_layer_set_text(text_layer_battery, "95%");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_battery));

	
	text_layer_temperature = text_layer_create(GRect(10, 143, 50, 20));
	text_layer_set_text_alignment(text_layer_temperature, GTextAlignmentLeft);
	text_layer_set_background_color(text_layer_temperature, GColorClear);
	text_layer_set_text_color(text_layer_temperature, GColorWhite);
	text_layer_set_font(text_layer_temperature, font_square_20);
 	text_layer_set_text(text_layer_temperature, "45C");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_temperature));
	
	battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
	battery_bitmap_layer = bitmap_layer_create(GRect(95, 125, 30, 20));
	bitmap_layer_set_compositing_mode(battery_bitmap_layer, GCompOpSet);
	bitmap_layer_set_bitmap(battery_bitmap_layer, battery_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(battery_bitmap_layer));
	
	temperature_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TEMP);
	temperature_bitmap_layer = bitmap_layer_create(GRect(15, 122, 32, 32));
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
	
	update_speed();
}

void handle_deinit(void) {
	text_layer_destroy(text_layer);
	text_layer_destroy(text_layer_mph);
	text_layer_destroy(text_layer_temperature);
	text_layer_destroy(text_layer_battery);
	bitmap_layer_destroy(battery_bitmap_layer);
	bitmap_layer_destroy(temperature_bitmap_layer);
	layer_destroy(s_layer);
	
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