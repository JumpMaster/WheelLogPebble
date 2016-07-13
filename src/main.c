#include <pebble.h>
#include <display.h>

#define KEY_SPEED  0
#define KEY_BATTERY  1
#define KEY_TEMPERATURE  2
#define KEY_FAN_STATE  3
#define KEY_BT_STATE  4
#define KEY_MAX_SPEED 5

static bool DEBUG = true;

static Window *window;

bool has_vibrated = false;
int vibe_speed;
int max_speed;

static VibePattern vibe = {
  		.durations = (uint32_t[]) { 300, 150, 300, 150, 500 },
		.num_segments = 5,
};

// Persistant Storage Keys
enum {
	PS_MAX_SPEED,
	PS_VIBE_SPEED,
	PS_USE_ONBOARD_HORN
};

int speed = 1;
int battery = 1;
int temperature = 1;
int fan_state = 1;
int bt_state = 1;

int new_speed = 0;
int new_battery = 50;
int new_temperature = 30;
int new_fan_state = 0;
int new_bt_state = 0;

char charSpeed[3] = "";
char charBattery[5] = "";
char charTemperature[5] = "";

bool use_onboard_horn;

static void send(int key, int value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_int(iter, key, &value, sizeof(int), true);

  app_message_outbox_send();
}

static void update_display() {
	
	if (new_speed != speed) {
		speed = new_speed;
		
		if (vibe_speed > 0 && !has_vibrated && speed >= vibe_speed)
		{
			has_vibrated = true;
			vibes_enqueue_custom_pattern(vibe);
		}
		else if (has_vibrated && speed < vibe_speed)
			has_vibrated = false;

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
}

static void received_handler(DictionaryIterator *iter, void *context) {

	Tuple *speed_tuple = dict_find(iter, KEY_SPEED);
	Tuple *temperature_tuple = dict_find(iter, KEY_TEMPERATURE);
	Tuple *battery_tuple = dict_find(iter, KEY_BATTERY);
	Tuple *fan_state_tuple = dict_find(iter, KEY_FAN_STATE);
	Tuple *bt_state_tuple = dict_find(iter, KEY_BT_STATE);
	Tuple *max_speed_tuple = dict_find(iter, MESSAGE_KEY_max_speed);
	Tuple *vibe_speed_tuple = dict_find(iter, MESSAGE_KEY_vibe_speed);
	Tuple *use_onboard_horn_tuple = dict_find(iter, MESSAGE_KEY_use_onboard_horn);

	if(speed_tuple)
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
		max_speed = max_speed_tuple->value->int32;
		persist_write_int(PS_MAX_SPEED, max_speed);
		new_speed = speed;
		speed = 0;
		update_angles(max_speed);
	}
	
	if (vibe_speed_tuple) {
		vibe_speed = vibe_speed_tuple->value->int32;
		persist_write_int(PS_VIBE_SPEED, vibe_speed);
	}
	
	if (use_onboard_horn_tuple) {
		use_onboard_horn = use_onboard_horn_tuple->value->int32 == 1;
		persist_write_bool(PS_USE_ONBOARD_HORN, use_onboard_horn);
	}

	// 	APP_LOG(APP_LOG_LEVEL_INFO, "SPEED = %d", new_speed);
	update_display();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (DEBUG) {
		new_speed = speed+5;
		update_display();
		return;
	}
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (use_onboard_horn)
		send(MESSAGE_KEY_play_horn, 0);
	else {
		send(MESSAGE_KEY_play_mp3_horn, 0);
	}
}

static void long_select_click_handler(ClickRecognizerRef recognizer, void *context) {
	// Lauch the Android companion app
	send(MESSAGE_KEY_start_app, 0);
	vibes_short_pulse();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (DEBUG) {
		if (speed > 5)
			new_speed = speed-5;
		else
			new_speed = 0;

		update_display();
		return;
	}
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
	vibe_speed = persist_exists(PS_VIBE_SPEED) ? persist_read_int(PS_VIBE_SPEED) : 280;
	use_onboard_horn = persist_exists(PS_USE_ONBOARD_HORN) ? persist_read_bool(PS_USE_ONBOARD_HORN) : false;
}

void handle_init(void) {
	
	window = window_create();

	load_persistent_data();
  update_angles(max_speed);
	draw_display(window);
	
	window_set_background_color(window, GColorBlack);
	window_stack_push(window, true);
	
	window_set_click_config_provider(window, click_config_provider);
	
	const int inbox_size = 128;
	const int outbox_size = 128;
	app_message_open(inbox_size, outbox_size);
	app_message_register_inbox_received(received_handler);
	
	update_display();
	update_time();
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
	text_layer_destroy(text_layer_time);
	text_layer_destroy(text_layer_speed);
	text_layer_destroy(text_layer_mph);
	text_layer_destroy(text_layer_temperature);
	text_layer_destroy(text_layer_battery);
	bitmap_layer_destroy(battery_bitmap_layer);
	bitmap_layer_destroy(temperature_bitmap_layer);
	bitmap_layer_destroy(bt_bitmap_layer);
	
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