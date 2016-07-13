#if defined(PBL_RECT)

#include <pebble.h>
#include <display.h>

int32_t angle_increment;

static GFont font_square_50;
static GFont font_square_20;
static GFont font_square_15;

AppTimer *graphics_timer;

int current_angle;
int target_angle;

int light_green_speed;
int yellow_speed;
int orange_speed;
int red_speed;

int light_green_angle;
int yellow_angle;
int orange_angle;
int red_angle;

int32_t angle_start;
int32_t angle_end;
int32_t angle_speed;

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

void update_arc(int speed) {
  target_angle = ((speed*angle_increment)/100)+235;

		if (target_angle < 235)
			target_angle = 235;
		else if (target_angle > 485)
			target_angle = 485;

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
	if (current_angle > red_angle)
		graphics_context_set_fill_color(ctx, GColorRed);
	else if (current_angle > orange_angle)
		graphics_context_set_fill_color(ctx, GColorOrange);
	else if (current_angle > yellow_angle)
		graphics_context_set_fill_color(ctx, GColorChromeYellow);
	else if (current_angle > light_green_angle) 
		graphics_context_set_fill_color(ctx, GColorSpringBud);
	else
	#endif
		graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorMediumSpringGreen, GColorWhite));
	
	graphics_fill_radial(ctx, outer_bounds, GOvalScaleModeFitCircle, 10, angle_start, angle_speed);
	
	graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorLightGray, GColorWhite));
	graphics_fill_radial(ctx, inner_bounds, GOvalScaleModeFitCircle, 2, angle_speed, angle_end);
		
	if (current_angle != target_angle)
		graphics_timer = app_timer_register(30, (AppTimerCallback) refresh_arc_callback, NULL);
}

void update_angles(int max_speed) {
  // All speeds are in KPH
	// To keep speeds as Integers they are
	// multiplied by a factor of 10.  e.g. 24.5 KPH == 245
	angle_increment = 25000/max_speed;
	
	light_green_speed = max_speed / 2;
	yellow_speed = light_green_speed + (max_speed / 12)*2;
	orange_speed = light_green_speed + (max_speed / 12)*4;
	red_speed = light_green_speed + (max_speed / 12)*5;
	
	light_green_angle = ((light_green_speed*angle_increment)/100)+235;
	yellow_angle = ((yellow_speed*angle_increment)/100)+235;
	orange_angle = ((orange_speed*angle_increment)/100)+235;
	red_angle = ((red_speed*angle_increment)/100)+235;
}

void draw_display(Window *window) {
  
  angle_start = DEG_TO_TRIGANGLE(235);
	angle_end = DEG_TO_TRIGANGLE(485);
	angle_speed = angle_start;
	current_angle = 235;
  
  font_square_50 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_50));
	font_square_20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_20));
	font_square_15 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_15));
  
  text_layer_time = text_layer_create(GRect(0, 0, 144, 50));
	text_layer_set_text_alignment(text_layer_time, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_time, GColorClear);
	text_layer_set_text_color(text_layer_time, GColorWhite);
	text_layer_set_font(text_layer_time, font_square_15);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_time));

	arc_layer = layer_create(GRect(10, 22, 124, 124));
	layer_add_child(window_get_root_layer(window), arc_layer);
	layer_set_update_proc(arc_layer, update_arcs);
	
	text_layer_speed = text_layer_create(GRect(0, 45, 144, 50));
	text_layer_set_text_alignment(text_layer_speed, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_speed, GColorClear);
	text_layer_set_text_color(text_layer_speed, GColorWhite);
	text_layer_set_font(text_layer_speed, font_square_50);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_speed));

	text_layer_mph = text_layer_create(GRect(0, 97, 144, 20));
	text_layer_set_text_alignment(text_layer_mph, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_mph, GColorClear);
	text_layer_set_text_color(text_layer_mph, GColorWhite);
	text_layer_set_font(text_layer_mph, font_square_20);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_mph));
 	text_layer_set_text(text_layer_mph, "KPH");
	
	text_layer_battery = text_layer_create(GRect(72, 143, 72, 20));
	text_layer_set_text_alignment(text_layer_battery, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_battery, GColorClear);
	text_layer_set_text_color(text_layer_battery, GColorWhite);
	text_layer_set_font(text_layer_battery, font_square_20);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_battery));

	
	text_layer_temperature = text_layer_create(GRect(0, 143, 72, 20));
	text_layer_set_text_alignment(text_layer_temperature, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer_temperature, GColorClear);
	text_layer_set_text_color(text_layer_temperature, GColorWhite);
	text_layer_set_font(text_layer_temperature, font_square_20);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_temperature));
	
	battery_bitmap_layer = bitmap_layer_create(GRect(93, 128, 30, 14));
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(battery_bitmap_layer));
	
	temperature_bitmap_layer = bitmap_layer_create(GRect(20, 127, 32, 16));
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(temperature_bitmap_layer));
	
	bt_bitmap_layer = bitmap_layer_create(GRect(120, 0, 24, 24));
	bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_DISCONNECTED);
	bitmap_layer_set_compositing_mode(bt_bitmap_layer, GCompOpSet);
	bitmap_layer_set_bitmap(bt_bitmap_layer, bt_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bt_bitmap_layer));
}

void destroy_display() {
  layer_destroy(arc_layer);

  fonts_unload_custom_font(font_square_15);
  fonts_unload_custom_font(font_square_20);
	fonts_unload_custom_font(font_square_50);
}

#endif