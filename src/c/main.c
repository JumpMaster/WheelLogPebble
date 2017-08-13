#include <pebble.h>
#include "display.h"
#include "pebble_process_info.h"
extern const PebbleProcessInfo __pbl_app_info;

static int KEY_SPEED = 0;
static int KEY_BATTERY = 1;
static int KEY_TEMPERATURE = 2;
static int KEY_FAN_STATE = 3;
static int KEY_BT_STATE = 4;
static int KEY_VIBE_ALERT = 5;
static int KEY_USE_MPH = 6;
static int KEY_MAX_SPEED = 7;
static int KEY_RIDE_TIME = 8;
static int KEY_DISTANCE = 9;
static int KEY_TOP_SPEED = 10;
static int KEY_READY = 11;

static int ALARM_SPEED = 0;
static int ALARM_CURRENT = 1;

static Window *window;
static Layer *gui_layer;
static Layer *details_layer;
static Layer *incoming_layer;
static Layer *outgoing_layer;

static char unit_mph[4] = "mph";
static char unit_kmh[5] = "km/h";
static char unit_km[3] = "km";
static char unit_mi[3] = "mi";

static VibePattern vibe_speed = {
	.durations = (uint32_t[]) { 300, 150, 300, 150, 500 },
	.num_segments = 5,
};

static VibePattern vibe_current = {
	.durations = (uint32_t[]) { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
	.num_segments = 11,
};

// Persistant Storage Keys
enum {
	PS_MAX_SPEED,
	PS_USE_MPH
};

int speed = 1;
int battery = 1;
int temperature = 1;
int fan_state = 1;
int bt_state = 1;
int ride_time = 1;
int distance = 1;
int top_speed = 1;

int new_speed = 150;
int new_battery = 50;
int new_temperature = 35;
int new_fan_state = 0;
int new_bt_state = 0;
int new_ride_time = 0;
int new_distance = 0;
int new_top_speed = 0;

char charSpeed[3] = "";
char charBattery[5] = "";
char charTemperature[5] = "";
char charRideTime[9] = "";
char charDistance[9] = "";
char charTopSpeed[10] = "";

int max_speed;
bool use_mph;

enum screen {
	gui,
	details
};

enum direction {
	up,
	down
};

AppTimer *graphics_timer;
AppTimer *transition_timer;

int displayed_screen = gui;
bool transitioning = false;
int transition_direction;
int incoming_screen;
int outgoing_screen;

int light_green_speed;
int yellow_speed;
int orange_speed;
int red_speed;

int angle_start_deg;
int angle_end_deg;
int angle_current_deg;
int angle_target_deg;

int angle_light_green_deg;
int angle_yellow_deg;
int angle_orange_deg;
int angle_red_deg;

int32_t angle_start;
int32_t angle_end;
int32_t angle_current;
int32_t angle_increment;

TextLayer *text_layer_time;
TextLayer *text_layer_speed;
TextLayer *text_layer_mph;
TextLayer *text_layer_battery;
TextLayer *text_layer_temperature;
TextLayer *text_layer_ride_time;
TextLayer *text_layer_distance;
TextLayer *text_layer_top_speed;

BitmapLayer *battery_bitmap_layer;
BitmapLayer *temperature_bitmap_layer;
BitmapLayer *bt_bitmap_layer;

Layer *arc_layer;

GBitmap *battery_bitmap;
GBitmap *temperature_bitmap;
GBitmap *bt_bitmap;

int toMiles(int value) {
	return (value*1000)/16093;
}

void transition_callback(void *data) {
	
	if (!transitioning)
		transitioning = true;
	
	GRect incoming_bounds = layer_get_bounds(incoming_layer);
	GRect outgoing_bounds = layer_get_bounds(outgoing_layer);
	
	if (transition_direction == up) {
		int transition_speed = (-incoming_bounds.origin.y) / 10;
		if (transition_speed <= 0)
			transition_speed = 1;
		if (incoming_bounds.origin.y + transition_speed >= 0 || transition_speed == 0) {
			incoming_bounds.origin.y = 0;
			outgoing_bounds.origin.y = 0 - outgoing_bounds.size.h+1;
		} else {
			incoming_bounds.origin.y += transition_speed;
			outgoing_bounds.origin.y += transition_speed;
		}
	} else {
		int transition_speed = incoming_bounds.origin.y / 10;
		if (transition_speed <= 0)
			transition_speed = 1;
		
		if (incoming_bounds.origin.y - transition_speed <= 0) {
			incoming_bounds.origin.y = 0;
			outgoing_bounds.origin.y = 0 - outgoing_bounds.size.h;
		} else {
			incoming_bounds.origin.y -= transition_speed;
			outgoing_bounds.origin.y -= transition_speed;
		}
	}
	
	layer_set_bounds(incoming_layer, incoming_bounds);
	layer_set_bounds(outgoing_layer, outgoing_bounds);
	
	if (incoming_bounds.origin.y == 0) {
		displayed_screen = incoming_screen;
		transitioning = false;
	} else
		transition_timer = app_timer_register(30, (AppTimerCallback) transition_callback, NULL);
}


void refresh_arc_callback(void *data) {

	int increment = 1;
	
	if (angle_current_deg < angle_target_deg && angle_target_deg-angle_current_deg >= 2) {
		increment = (angle_target_deg-angle_current_deg) / 2;
		angle_current_deg += increment > 8 ? 8 : increment;
	} else if (angle_current_deg > angle_target_deg && angle_current_deg-angle_target_deg >= 2) {
		increment = (angle_current_deg-angle_target_deg) / 2;
		angle_current_deg -= increment > 8 ? 8 : increment;
	} else {
		angle_current_deg = angle_target_deg;
	}

	angle_current = DEG_TO_TRIGANGLE(angle_current_deg);
	
	layer_mark_dirty(arc_layer);
}

void update_arc(int speed) {
  angle_target_deg = ((speed*angle_increment)/100)+angle_start_deg;

	if (angle_target_deg < angle_start_deg)
		angle_target_deg = angle_start_deg;
	else if (angle_target_deg > angle_end_deg)
		angle_target_deg = angle_end_deg;

	refresh_arc_callback(NULL);
}

static void update_arcs(Layer *layer, GContext *ctx) {	
	GRect inner_bounds = layer_get_bounds(layer);
	GRect outer_bounds = layer_get_bounds(layer);
	
	inner_bounds.origin.x += 4;
	inner_bounds.origin.y += 4;
	inner_bounds.size.h -= 8;
	inner_bounds.size.w -= 8;

  #ifdef PBL_COLOR
	if (angle_current_deg > angle_red_deg)
		graphics_context_set_fill_color(ctx, GColorRed);
	else if (angle_current_deg > angle_orange_deg)
		graphics_context_set_fill_color(ctx, GColorOrange);
	else if (angle_current_deg > angle_yellow_deg)
		graphics_context_set_fill_color(ctx, GColorChromeYellow);
	else if (angle_current_deg > angle_light_green_deg) 
		graphics_context_set_fill_color(ctx, GColorSpringBud);
	else
  #endif
	graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorMediumSpringGreen, GColorWhite));
	
	graphics_fill_radial(ctx, outer_bounds, GOvalScaleModeFitCircle, 10, angle_start, angle_current);
	
	graphics_context_set_fill_color(ctx, GColorLightGray);
	graphics_fill_radial(ctx, inner_bounds, GOvalScaleModeFitCircle, 2, angle_current, angle_end);
		
	if (angle_current_deg != angle_target_deg)
		graphics_timer = app_timer_register(30, (AppTimerCallback) refresh_arc_callback, NULL);
}

void update_angles(int max_speed) {
	// All speeds are in KPH
	// To keep speeds as integers they are
	// multiplied by a factor of 10.  e.g. 24.5 KPH == 245
	angle_increment = ((angle_end_deg-angle_start_deg)*100)/max_speed;
	
	light_green_speed = max_speed / 2;
	yellow_speed = light_green_speed + (max_speed / 12)*2;
	orange_speed = light_green_speed + (max_speed / 12)*4;
	red_speed = light_green_speed + (max_speed / 12)*5;
	
	angle_light_green_deg = ((light_green_speed*angle_increment)/100)+angle_start_deg;
	angle_yellow_deg = ((yellow_speed*angle_increment)/100)+angle_start_deg;
	angle_orange_deg = ((orange_speed*angle_increment)/100)+angle_start_deg;
	angle_red_deg = ((red_speed*angle_increment)/100)+angle_start_deg;
}

static void send(int key, int value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_int(iter, key, &value, sizeof(int), true);

  app_message_outbox_send();
}

static void send_ready() {
	int version = (__pbl_app_info.process_version.major * 100) + __pbl_app_info.process_version.minor;
	send(KEY_READY, version);
}

static void update_display() {
	
	if (new_speed != speed) {
		speed = new_speed;

		if (use_mph)
			snprintf(charSpeed, 3, "%02d", toMiles(speed)); // Avoid floating point calculations on Pebble!
		else
			snprintf(charSpeed, 3, "%02d", speed/10);

		text_layer_set_text(text_layer_speed, charSpeed);

    	update_arc(speed);
	}

	if (new_battery != battery) {
		if (new_battery > 100)
			new_battery = 100;
		else if (new_battery < 0)
			new_battery = 0;

		battery = new_battery;
		snprintf(charBattery, 5, "%d%%", battery);
		text_layer_set_text(text_layer_battery, charBattery);
		
		gbitmap_destroy(battery_bitmap);

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
		snprintf(charTemperature, 5, "%dC", temperature);
		text_layer_set_text(text_layer_temperature, charTemperature);
	}
	
	if (fan_state != new_fan_state) {
		fan_state = new_fan_state;
		gbitmap_destroy(temperature_bitmap);
		if (fan_state > 0)
			temperature_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TEMP_RED);
		else
			temperature_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TEMP);
		
		bitmap_layer_set_compositing_mode(temperature_bitmap_layer, GCompOpSet);
		bitmap_layer_set_bitmap(temperature_bitmap_layer, temperature_bitmap);
	}
	
	if (bt_state != new_bt_state) {
		bt_state = new_bt_state;

		if (bt_state > 0)
			layer_set_hidden(bitmap_layer_get_layer(bt_bitmap_layer), true);
		else
			layer_set_hidden(bitmap_layer_get_layer(bt_bitmap_layer), false);
	}
	
	if (new_ride_time != ride_time) {
		ride_time = new_ride_time;
		int seconds = ride_time;
		int hours = seconds / (60 * 60);
		seconds -= hours * (60 * 60);
		int minutes = seconds / 60;
		seconds -= minutes * 60;
		
		snprintf(charRideTime, 9, "%02d:%02d:%02d", hours, minutes, seconds);
		text_layer_set_text(text_layer_ride_time, charRideTime);
	}
	
	if (new_distance != distance) {
		distance = new_distance;
		int value = distance;
		if (use_mph)
			value = toMiles(value*10);
		snprintf(charDistance, 9, "%d.%d %s", value/10, value%10, use_mph ? unit_mi : unit_km);
		text_layer_set_text(text_layer_distance, charDistance);
	}
	
	if (new_top_speed != top_speed) {
		top_speed = new_top_speed;
		int value = top_speed;
		if (use_mph)
			value = toMiles(value*10);
		snprintf(charTopSpeed, 10, "%d.%d %s", value/10, value%10, use_mph ? unit_mph : unit_kmh);
		text_layer_set_text(text_layer_top_speed, charTopSpeed);
	}
}

static void received_handler(DictionaryIterator *iter, void *context) {	
// 	Tuple *t = dict_read_first(iter);
// 	int a = t->key;
// 	int b = MESSAGE_KEY_vibe_alert;
//     APP_LOG(APP_LOG_LEVEL_INFO, "Key is %d", a);
// 	    APP_LOG(APP_LOG_LEVEL_INFO, "Expecting %d", b);

	Tuple *speed_tuple = dict_find(iter, KEY_SPEED);
	Tuple *temperature_tuple = dict_find(iter, KEY_TEMPERATURE);
	Tuple *battery_tuple = dict_find(iter, KEY_BATTERY);
	Tuple *fan_state_tuple = dict_find(iter, KEY_FAN_STATE);
	Tuple *bt_state_tuple = dict_find(iter, KEY_BT_STATE);
	Tuple *use_mph_tuple = dict_find(iter, KEY_USE_MPH);
	Tuple *max_speed_tuple = dict_find(iter, KEY_MAX_SPEED);
	Tuple *vibe_alert_tuple = dict_find(iter, KEY_VIBE_ALERT);
	Tuple *ride_time_tuple = dict_find(iter, KEY_RIDE_TIME);
	Tuple *distance_tuple = dict_find(iter, KEY_DISTANCE);
	Tuple *top_speed_tuple = dict_find(iter, KEY_TOP_SPEED);
	Tuple *ready_tuple = dict_find(iter, KEY_READY);
	
	if (ready_tuple)
		send(MESSAGE_KEY_displayed_screen, displayed_screen);

	if (speed_tuple)
		new_speed = speed_tuple->value->int32;
	
	if (temperature_tuple)
		new_temperature = temperature_tuple->value->int32;
	
	if (battery_tuple)
		new_battery = battery_tuple->value->int32;
	
	if (fan_state_tuple)
		new_fan_state = fan_state_tuple->value->int32;
	
	if (bt_state_tuple)
		new_bt_state = bt_state_tuple->value->int32;
	
	if (max_speed_tuple) {
		max_speed = max_speed_tuple->value->int32 * 10;
		persist_write_int(PS_MAX_SPEED, max_speed);
		new_speed = speed;
		speed = 0;
		update_angles(max_speed);
	}

	if (use_mph_tuple) {
		use_mph = use_mph_tuple->value->int32 == 1;
		persist_write_bool(PS_USE_MPH, use_mph);

		distance = -1;
		top_speed = -1;

		if (use_mph)
			text_layer_set_text(text_layer_mph, unit_mph);
		else
			text_layer_set_text(text_layer_mph, unit_kmh);	
	}
	
	if (vibe_alert_tuple) {
		int vibe_type = vibe_alert_tuple->value->int32;

		if (vibe_type == ALARM_SPEED) // Speed alarm
			vibes_enqueue_custom_pattern(vibe_speed);
		else if (vibe_type == ALARM_CURRENT) // Current Alarm
			vibes_enqueue_custom_pattern(vibe_current);
	}
	
	if (ride_time_tuple)
		new_ride_time = ride_time_tuple->value->int32;
	
	if (distance_tuple)
		new_distance = distance_tuple->value->int32;

	if (top_speed_tuple)
		new_top_speed = top_speed_tuple->value->int32;
	
	update_display();
}

static void start_transition() {
	if (!transitioning) {
		Layer *layer;
		if (displayed_screen == gui) {
			layer = details_layer;
		} else
			layer = gui_layer;
	
		GRect layer_bounds = layer_get_bounds(layer);
		
		if (transition_direction == down)
			layer_bounds.origin.y = layer_bounds.size.h+1;
		else
			layer_bounds.origin.y = 0 - layer_bounds.size.h;


		layer_set_bounds(layer, layer_bounds);
		
		if (displayed_screen == gui) {
			incoming_screen = details;
			outgoing_screen = gui;
			outgoing_layer = gui_layer;
			incoming_layer = details_layer;
		} else {
			incoming_screen = gui;
			outgoing_screen = details;
			outgoing_layer = details_layer;
			incoming_layer = gui_layer;
		}
		transition_callback(NULL);
	} else {
		Layer *temp_layer = outgoing_layer;
		int temp = outgoing_screen;
		outgoing_screen = incoming_screen;
		incoming_screen = temp;
		outgoing_layer = incoming_layer;
		incoming_layer = temp_layer;
	}
	send(MESSAGE_KEY_displayed_screen, incoming_screen);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (transitioning && transition_direction == up)
		return;
	transition_direction = up;
	start_transition();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	send(MESSAGE_KEY_play_horn, 0);
}

static void long_select_click_handler(ClickRecognizerRef recognizer, void *context) {
	// Lauch the Android companion app
	send(MESSAGE_KEY_start_app, 0);
	vibes_short_pulse();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (transitioning && transition_direction == down)
		return;
	transition_direction = down;
	start_transition();
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
	
	window_long_click_subscribe(BUTTON_ID_SELECT, 1000, long_select_click_handler, NULL);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(text_layer_time, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

void load_persistent_data() {
	max_speed = persist_exists(PS_MAX_SPEED) ? persist_read_int(PS_MAX_SPEED) : 300;
	use_mph = persist_exists(PS_USE_MPH) ? persist_read_bool(PS_USE_MPH) : false;
}

void handle_init(void) {
	
	window = window_create();
	Layer *window_layer = window_get_root_layer(window);
	GRect window_bounds = layer_get_bounds(window_layer);

	set_angles(&angle_start_deg, &angle_end_deg);
	
	angle_start = DEG_TO_TRIGANGLE(angle_start_deg);
	angle_end = DEG_TO_TRIGANGLE(angle_end_deg);
	angle_current = angle_start;
	angle_current_deg = angle_start_deg;
	
	load_persistent_data();
	update_angles(max_speed);
	
	draw_display(&window, &gui_layer, &details_layer, &text_layer_time, &text_layer_speed, &text_layer_mph, &text_layer_battery, &text_layer_temperature,
				 &battery_bitmap_layer, &temperature_bitmap_layer, &bt_bitmap_layer, &arc_layer,
				 &text_layer_ride_time, &text_layer_distance, &text_layer_top_speed);
	
	text_layer_set_text_alignment(text_layer_time, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_time, GColorClear);
	text_layer_set_text_color(text_layer_time, GColorWhite);
	
	text_layer_set_text_alignment(text_layer_speed, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_speed, GColorClear);
	text_layer_set_text_color(text_layer_speed, GColorWhite);

	text_layer_set_text_alignment(text_layer_mph, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_mph, GColorClear);
	text_layer_set_text_color(text_layer_mph, GColorWhite);
	
	if (use_mph)
 		text_layer_set_text(text_layer_mph, unit_mph);
	else
		text_layer_set_text(text_layer_mph, unit_kmh);

	
	text_layer_set_text_alignment(text_layer_battery, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_battery, GColorClear);
	text_layer_set_text_color(text_layer_battery, GColorWhite);

	text_layer_set_text_alignment(text_layer_temperature, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_temperature, GColorClear);
	text_layer_set_text_color(text_layer_temperature, GColorWhite);
	
	bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_DISCONNECTED);
	bitmap_layer_set_compositing_mode(bt_bitmap_layer, GCompOpSet);
	bitmap_layer_set_bitmap(bt_bitmap_layer, bt_bitmap);

	layer_set_update_proc(arc_layer, update_arcs);

	layer_add_child(gui_layer, text_layer_get_layer(text_layer_time));
	layer_add_child(gui_layer, arc_layer);
	layer_add_child(gui_layer, text_layer_get_layer(text_layer_speed));
	layer_add_child(gui_layer, text_layer_get_layer(text_layer_mph));
	layer_add_child(gui_layer, text_layer_get_layer(text_layer_battery));
	layer_add_child(gui_layer, text_layer_get_layer(text_layer_temperature));
	layer_add_child(gui_layer, bitmap_layer_get_layer(temperature_bitmap_layer));
	layer_add_child(gui_layer, bitmap_layer_get_layer(battery_bitmap_layer));
	layer_add_child(gui_layer, bitmap_layer_get_layer(bt_bitmap_layer));
	layer_add_child(window_layer, gui_layer);
	
	GRect details_bounds = layer_get_bounds(details_layer);
	details_bounds.origin.y -= window_bounds.size.h;
	layer_set_bounds(details_layer, details_bounds);
	
	layer_add_child(window_layer, details_layer);
	
	window_set_background_color(window, GColorBlack);
	window_stack_push(window, true);
	
	window_set_click_config_provider(window, click_config_provider);
	
	const int inbox_size = 128;
	const int outbox_size = 128;
	app_message_open(inbox_size, outbox_size);
	app_message_register_inbox_received(received_handler);
	
	update_display();
	update_time();
	send_ready();
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
	text_layer_destroy(text_layer_time);
	text_layer_destroy(text_layer_speed);
	text_layer_destroy(text_layer_mph);
	text_layer_destroy(text_layer_temperature);
	text_layer_destroy(text_layer_battery);
	text_layer_destroy(text_layer_ride_time);
	text_layer_destroy(text_layer_distance);
	text_layer_destroy(text_layer_top_speed);
	bitmap_layer_destroy(battery_bitmap_layer);
	bitmap_layer_destroy(temperature_bitmap_layer);
	bitmap_layer_destroy(bt_bitmap_layer);
	layer_destroy(arc_layer);
	layer_destroy(gui_layer);
	layer_destroy(details_layer);
	
	destroy_display();

	gbitmap_destroy(battery_bitmap);
	gbitmap_destroy(temperature_bitmap);
	gbitmap_destroy(bt_bitmap);
	
	window_destroy(window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}