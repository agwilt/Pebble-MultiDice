#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_show_layer;
static TextLayer *s_info_layer;
static ActionBarLayer *action_bar;
static StatusBarLayer *s_status_bar;

static bool s_get_letter = true;
static char s_buffer[] = "A";
static char s_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char *s_info_line = "Random Letter\n(99 left)";
static size_t s_lnum = 26;
static GBitmap *icon_change_mode, *icon_get_random, *icon_reset_letters;
static const uint32_t s_reset_segments[] = { 50, 50, 50, 50, 50 };
static VibePattern s_reset_pattern = {
	.durations = s_reset_segments,
	.num_segments = ARRAY_LENGTH(s_reset_segments),
};
static const uint32_t s_mode_segments[] = { 50,};
static VibePattern s_mode_pattern = {
	.durations = s_mode_segments,
	.num_segments = ARRAY_LENGTH(s_mode_segments),
};

static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void reset_alphabet();
static void update_letter();
static void update_number();
static void select_click_handler(ClickRecognizerRef recognizer, void *context);
static void set_info_layer();
static void up_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_click_handler(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);

static void main_window_load(Window *window)
{
	// Get information about the Window
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	// Status bar setup
	s_status_bar = status_bar_layer_create();
	layer_set_frame(status_bar_layer_get_layer(s_status_bar), GRect(0, 0, bounds.size.w-ACTION_BAR_WIDTH, STATUS_BAR_LAYER_HEIGHT));

	// Main layer setup
	s_show_layer = text_layer_create(
		GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w-ACTION_BAR_WIDTH, 50));
	text_layer_set_background_color(s_show_layer, GColorClear);
	text_layer_set_text_color(s_show_layer, GColorBlack);
	text_layer_set_font(s_show_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(s_show_layer, GTextAlignmentCenter);

	// Info-line layer setup
	s_info_layer = text_layer_create(
		GRect(0, PBL_IF_ROUND_ELSE(142, 135), bounds.size.w-ACTION_BAR_WIDTH, 50));
	text_layer_set_background_color(s_info_layer, GColorClear);
	text_layer_set_text_color(s_info_layer, GColorBlack);
	text_layer_set_font(s_info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(s_info_layer, GTextAlignmentCenter);
	
	// Initialise action bar
	action_bar = action_bar_layer_create();
	action_bar_layer_add_to_window(action_bar, window);
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
	// Set the icons:
	icon_change_mode = gbitmap_create_with_resource(RESOURCE_ID_MODE);
	icon_get_random = gbitmap_create_with_resource(RESOURCE_ID_NEXT);
	icon_reset_letters = gbitmap_create_with_resource(RESOURCE_ID_RESET);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon_change_mode);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon_get_random);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon_reset_letters);

	// Add it as a child layer to the Window's root layer
	layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
	layer_add_child(window_layer, text_layer_get_layer(s_show_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_info_layer));
}

static void main_window_unload(Window *window)
{
	// Destroy TextLayer
	text_layer_destroy(s_show_layer);
	text_layer_destroy(s_info_layer);
	action_bar_layer_destroy(action_bar);
}

static void reset_alphabet()
{
	size_t i;
	for (i = 0; i < 26 - 1; i++) {
		size_t j = i + rand() / (RAND_MAX / (26 - i) + 1);
		char c = s_alphabet[j];
		s_alphabet[j] = s_alphabet[i];
		s_alphabet[i] = c;
	}
	s_lnum = 0;
	set_info_layer();
	text_layer_set_text(s_show_layer, "");
}

static void update_letter()
{
	if (s_lnum >= 26) {
		vibes_enqueue_custom_pattern(s_reset_pattern);
		reset_alphabet();
	}
	s_buffer[0] = s_alphabet[s_lnum++];
//	text_layer_set_text(s_info_layer, s_alphabet+s_lnum);
	text_layer_set_text(s_show_layer, s_buffer);
	set_info_layer();
}

static void update_number()
{
	s_buffer[0] = (rand() / (RAND_MAX / 6 + 1))+'1';
	text_layer_set_text(s_show_layer, s_buffer);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
	// A single click has just occured
	if (s_get_letter) update_letter();
	else update_number();
}

static void set_info_layer()
{
	if (s_get_letter) {
		snprintf(s_info_line, 24, "Random Letter\n(%d left)", 26 - (int) s_lnum);
		text_layer_set_text(s_info_layer, s_info_line);
	} else {
		text_layer_set_text(s_info_layer, "Random Number\n(1..6)");
	}
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	// A single click has just occured
	vibes_enqueue_custom_pattern(s_mode_pattern);
	s_get_letter = !s_get_letter;
	
	set_info_layer();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	// A single click has just occured
	vibes_enqueue_custom_pattern(s_reset_pattern);
	reset_alphabet();
}

static void click_config_provider(void *context) {
 	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
 	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
 	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void init() {
	// Create main Window element and assign to pointer
	s_main_window = window_create();

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});

	// Register with TickTimerService
	window_set_click_config_provider(s_main_window, click_config_provider);

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);

	// Make sure the time is displayed from the start
	reset_alphabet();
	update_letter();
	set_info_layer();
}

static void deinit() {
	// Destroy Window
	window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}