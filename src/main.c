#include <pebble.h>
#include <settings.h>

//uncomment to disable app logging
#undef APP_LOG
#define APP_LOG(...)

//unsigned int debug_heap_used = 0;
	
#define PADDING 1
#define TEXT_HEIGHT 74
#define TEXT_WIDTH 144
#define NUM_HEIGHT 102
#define NUM_WIDTH 41
	
#define CURVE AnimationCurveEaseIn
	
GRect hour_in_frame;
GRect minute_tens_in_frame;
GRect minute_ones_in_frame;
GRect hour_frame;
GRect minute_tens_frame;
GRect minute_ones_frame;
GRect hour_out_frame;
GRect minute_tens_out_frame;
GRect minute_ones_out_frame;
GRect minute_clip_frame;

Window *window;
Layer *root_layer;

Layer *minutes_clip_layer;

BitmapLayer *hour_layer_odd;
BitmapLayer *minute_tens_layer_odd;
BitmapLayer *minute_ones_layer_odd;
BitmapLayer *hour_layer_even;
BitmapLayer *minute_tens_layer_even;
BitmapLayer *minute_ones_layer_even;

static PropertyAnimation *minute_tens_animation_odd = NULL;
static PropertyAnimation *minute_ones_animation_odd = NULL;
static PropertyAnimation *hour_animation_odd = NULL;
static PropertyAnimation *minute_tens_animation_even = NULL;
static PropertyAnimation *minute_ones_animation_even = NULL;
static PropertyAnimation *hour_animation_even = NULL;

GBitmap *minute_image[10];
GBitmap *hour_image[12];

const int MINUTE_IMAGE_IDS[10] = {RESOURCE_ID_NUM_ZERO_WHITE, RESOURCE_ID_NUM_ONE_WHITE, RESOURCE_ID_NUM_TWO_WHITE, 
								  RESOURCE_ID_NUM_THREE_WHITE, RESOURCE_ID_NUM_FOUR_WHITE, RESOURCE_ID_NUM_FIVE_WHITE, 
								  RESOURCE_ID_NUM_SIX_WHITE, RESOURCE_ID_NUM_SEVEN_WHITE, RESOURCE_ID_NUM_EIGHT_WHITE, 
								  RESOURCE_ID_NUM_NINE_WHITE};

bool minute_image_loaded[10] = {false, false, false, false, false, false, false, false, false, false};
	
const int HOUR_IMAGE_IDS[12] = {RESOURCE_ID_TEXT_TWELVE_WHITE, RESOURCE_ID_TEXT_ONE_WHITE, RESOURCE_ID_TEXT_TWO_WHITE, 
								RESOURCE_ID_TEXT_THREE_WHITE, RESOURCE_ID_TEXT_FOUR_WHITE, RESOURCE_ID_TEXT_FIVE_WHITE,
								RESOURCE_ID_TEXT_SIX_WHITE, RESOURCE_ID_TEXT_SEVEN_WHITE, RESOURCE_ID_TEXT_EIGHT_WHITE,
								RESOURCE_ID_TEXT_NINE_WHITE, RESOURCE_ID_TEXT_TEN_WHITE, RESOURCE_ID_TEXT_ELEVEN_WHITE};

bool hour_image_loaded[12] = {false, false, false, false, false, false, false, false, false, false, false, false};

static void load_minute_image(int i)
{
	if(!minute_image_loaded[i])
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "loading minute image %d", i);
		minute_image[i] = gbitmap_create_with_resource(MINUTE_IMAGE_IDS[i]);
		minute_image_loaded[i] = true;
	}
}

static void load_hour_image(int i)
{
	if(!hour_image_loaded[i])
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "loading hour image %d", i);
		hour_image[i] = gbitmap_create_with_resource(HOUR_IMAGE_IDS[i]);
		hour_image_loaded[i] = true;
	}
}

static void unload_minute_image(int i)
{
	if(minute_image_loaded[i]) 
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "unloading minute image %d", i);
		gbitmap_destroy(minute_image[i]);
		minute_image_loaded[i] = false;
	}
}

static void unload_hour_image(int i)
{
	if(hour_image_loaded[i])
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "unloading hour image %d", i);
		gbitmap_destroy(hour_image[i]);
		hour_image_loaded[i] = false;
	}
}

static void unload_unused_images(int current_ones, int current_tens, int current_hour)
{
	int previous_ones = current_ones - 1;
	int previous_tens = current_tens - 1;
	int previous_hour = current_hour - 1;
	
	if(previous_ones < 0){ previous_ones = 9;}
	if(previous_tens < 0){ previous_tens = 5;}
	if(previous_hour < 0){ previous_hour = 11;}

	APP_LOG(APP_LOG_LEVEL_DEBUG, "unloading unused images");
	
	for( int i = 0; i < 12; i++)
	{
		if(i < 10 && i != current_ones && i != current_tens && i != previous_ones && i != previous_tens)
		{
			unload_minute_image(i);
		}
		if(i != current_hour && i != previous_hour)
		{
			unload_hour_image(i);
		}
	}
}

static void animate_minute_ones(int m, int delay, int duration)
{
	property_animation_destroy(minute_ones_animation_even);
	minute_ones_animation_even = NULL;
	property_animation_destroy(minute_ones_animation_odd);
	minute_ones_animation_odd = NULL;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Animating minute_ones %d", m);
	load_minute_image(m);

	if(m%2 == 0)
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "%d is even", m);
		bitmap_layer_set_bitmap(minute_ones_layer_even, minute_image[m]);
		minute_ones_animation_even = property_animation_create_layer_frame(bitmap_layer_get_layer(minute_ones_layer_even), &minute_ones_in_frame, &minute_ones_frame);
		minute_ones_animation_odd = property_animation_create_layer_frame(bitmap_layer_get_layer(minute_ones_layer_odd), &minute_ones_frame, &minute_ones_out_frame);
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "%d is odd", m);
		bitmap_layer_set_bitmap(minute_ones_layer_odd, minute_image[m]);
		minute_ones_animation_odd = property_animation_create_layer_frame(bitmap_layer_get_layer(minute_ones_layer_odd), &minute_ones_in_frame, &minute_ones_frame);
		minute_ones_animation_even = property_animation_create_layer_frame(bitmap_layer_get_layer(minute_ones_layer_even), &minute_ones_frame, &minute_ones_out_frame);
	}
	animation_set_curve((Animation*)minute_ones_animation_odd, CURVE);
	animation_set_delay((Animation*)minute_ones_animation_odd, delay);
	animation_set_duration((Animation*)minute_ones_animation_odd, duration);
	
	animation_set_curve((Animation*)minute_ones_animation_even, CURVE);
	animation_set_delay((Animation*)minute_ones_animation_even, delay);
	animation_set_duration((Animation*)minute_ones_animation_even, duration);

	animation_schedule((Animation*)minute_ones_animation_even);
	animation_schedule((Animation*)minute_ones_animation_odd);
}

static void animate_minute_tens(int t, int delay, int duration)
{
	property_animation_destroy(minute_tens_animation_even);
	minute_ones_animation_even = NULL;
	property_animation_destroy(minute_tens_animation_odd);
	minute_ones_animation_odd = NULL;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Animating minute_tens %d", t);
	load_minute_image(t);

	if(t%2 == 0)
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "%d is even", t);
		bitmap_layer_set_bitmap(minute_tens_layer_even, minute_image[t]);
		minute_tens_animation_even = property_animation_create_layer_frame(bitmap_layer_get_layer(minute_tens_layer_even), &minute_tens_in_frame, &minute_tens_frame);
		minute_tens_animation_odd = property_animation_create_layer_frame(bitmap_layer_get_layer(minute_tens_layer_odd), &minute_tens_frame, &minute_tens_out_frame);
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "%d is odd", t);
		bitmap_layer_set_bitmap(minute_tens_layer_odd, minute_image[t]);
		minute_tens_animation_odd = property_animation_create_layer_frame(bitmap_layer_get_layer(minute_tens_layer_odd), &minute_tens_in_frame, &minute_tens_frame);
		minute_tens_animation_even = property_animation_create_layer_frame(bitmap_layer_get_layer(minute_tens_layer_even), &minute_tens_frame, &minute_tens_out_frame);
	}
	animation_set_curve((Animation*)minute_tens_animation_odd, CURVE);
	animation_set_delay((Animation*)minute_tens_animation_odd, delay);
	animation_set_duration((Animation*)minute_tens_animation_odd, duration);

	animation_set_curve((Animation*)minute_tens_animation_even, CURVE);
	animation_set_delay((Animation*)minute_tens_animation_even, delay);
	animation_set_duration((Animation*)minute_tens_animation_even, duration);

	animation_schedule((Animation*)minute_tens_animation_odd);
	animation_schedule((Animation*)minute_tens_animation_even);
}

static void animate_hour(int h, int delay, int duration)
{
	property_animation_destroy(hour_animation_even);
	hour_animation_even = NULL;
	property_animation_destroy(hour_animation_odd);
	hour_animation_odd = NULL;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Animating hour %d", h);
	load_hour_image(h);

	if(h%2 == 0)
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "%d is even", h);
		bitmap_layer_set_bitmap(hour_layer_even, hour_image[h]);
		hour_animation_even = property_animation_create_layer_frame(bitmap_layer_get_layer(hour_layer_even), &hour_in_frame, &hour_frame);
		hour_animation_odd = property_animation_create_layer_frame(bitmap_layer_get_layer(hour_layer_odd), &hour_frame, &hour_out_frame);
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "%d is odd", h);
		bitmap_layer_set_bitmap(hour_layer_odd, hour_image[h]);			
		hour_animation_odd = property_animation_create_layer_frame(bitmap_layer_get_layer(hour_layer_odd), &hour_in_frame, &hour_frame);
		hour_animation_even = property_animation_create_layer_frame(bitmap_layer_get_layer(hour_layer_even), &hour_frame, &hour_out_frame);
	}
	animation_set_curve((Animation*)hour_animation_odd, CURVE);
	animation_set_delay((Animation*)hour_animation_odd, delay);
	animation_set_duration((Animation*)hour_animation_odd, duration);

	animation_set_curve((Animation*)hour_animation_even, CURVE);
	animation_set_delay((Animation*)hour_animation_even, delay);
	animation_set_duration((Animation*)hour_animation_even, duration);

	animation_schedule((Animation*)hour_animation_odd);
	animation_schedule((Animation*)hour_animation_even);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
	APP_LOG(APP_LOG_LEVEL_INFO, "tick");
	APP_LOG(APP_LOG_LEVEL_WARNING, "Heap used at start of tick: %u", heap_bytes_used());
	APP_LOG(APP_LOG_LEVEL_WARNING, "Heap free at start of tick: %u", heap_bytes_free());

	//APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "Heap used by '' = %i", (int)(heap_bytes_used() - debug_heap_used));	debug_heap_used = heap_bytes_used();

	int hour = (tick_time->tm_hour)%12; //twelve hour mode only
	int minute = tick_time->tm_min;
	int minute_tens = minute/10;
	int minute_ones = minute%10;
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "hour = %d", hour);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "minute = %d", minute);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "minute_tens = %d", minute_tens);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "minute_ones = %d",minute_ones);

	unload_unused_images(minute_ones, minute_tens, hour);
	
	int delay = 0;
	int duration = 2*(animation_duration/3);

	animate_minute_ones(minute_ones, delay, duration);
	
	if(minute_ones == 0)
	{
		delay += (animation_duration/6);
		animate_minute_tens(minute_tens, delay, duration);
	}
	
	if(minute == 0)
	{		
		delay += (animation_duration/6);
		animate_hour(hour, delay, duration);
	}
	
	if(vibrate && minute == 0)
	{
		vibes_double_pulse();
	}
}

static void init_frames()
{
	//frame in which minutes are visible
	minute_clip_frame = GRect(144-((2*(PADDING+NUM_WIDTH))+PADDING),168-(NUM_HEIGHT+PADDING),2*(PADDING+NUM_WIDTH),(NUM_HEIGHT+PADDING));

	//frame layers animate in from
	minute_ones_in_frame = GRect((NUM_WIDTH+PADDING+PADDING),0-(NUM_HEIGHT+PADDING),NUM_WIDTH,NUM_HEIGHT); //relative to the clipping layer //double padding between numbers
	minute_tens_in_frame = GRect(0,0-(NUM_HEIGHT+PADDING),NUM_WIDTH,NUM_HEIGHT); //relative to the clipping layer
	hour_in_frame = GRect(PADDING+144,PADDING,TEXT_WIDTH,TEXT_HEIGHT);
	
	//frame layers display in
	minute_ones_frame = GRect((NUM_WIDTH+PADDING+PADDING),0,NUM_WIDTH,NUM_HEIGHT); //relative to the clipping layer
	minute_tens_frame = GRect(0,0,NUM_WIDTH,NUM_HEIGHT); //relative to the clipping layer
	hour_frame = GRect(PADDING,PADDING,TEXT_WIDTH,TEXT_HEIGHT);
	
	//frame layers animate out to
	minute_ones_out_frame = GRect((NUM_WIDTH+PADDING+PADDING),NUM_HEIGHT+PADDING,NUM_WIDTH,NUM_HEIGHT); //relative to the clipping layer
	minute_tens_out_frame = GRect(0,NUM_HEIGHT+PADDING,NUM_WIDTH,NUM_HEIGHT); //relative to the clipping layer
	hour_out_frame = GRect(PADDING-144,PADDING,TEXT_WIDTH,TEXT_HEIGHT);
}

static void init_layers()
{
	minutes_clip_layer = layer_create(minute_clip_frame);
	layer_set_clips(minutes_clip_layer, true);
	layer_add_child(root_layer, minutes_clip_layer);
	
	hour_layer_odd = bitmap_layer_create(hour_frame);
	bitmap_layer_set_compositing_mode(hour_layer_odd, GCompOpOr);
	bitmap_layer_set_alignment(hour_layer_odd, GAlignLeft);
	layer_add_child(root_layer, bitmap_layer_get_layer(hour_layer_odd));

	minute_ones_layer_odd = bitmap_layer_create(minute_ones_frame);
	bitmap_layer_set_compositing_mode(minute_ones_layer_odd, GCompOpOr);
	bitmap_layer_set_alignment(minute_ones_layer_odd, GAlignCenter);
	layer_add_child(minutes_clip_layer, bitmap_layer_get_layer(minute_ones_layer_odd));

	minute_tens_layer_odd = bitmap_layer_create(minute_tens_frame);
	bitmap_layer_set_compositing_mode(minute_tens_layer_odd, GCompOpOr);
	bitmap_layer_set_alignment(minute_tens_layer_odd, GAlignRight);
	layer_add_child(minutes_clip_layer, bitmap_layer_get_layer(minute_tens_layer_odd));

	hour_layer_even = bitmap_layer_create(hour_frame);
	bitmap_layer_set_compositing_mode(hour_layer_even, GCompOpOr);
	bitmap_layer_set_alignment(hour_layer_even, GAlignLeft);
	layer_add_child(root_layer, bitmap_layer_get_layer(hour_layer_even));

	minute_ones_layer_even = bitmap_layer_create(minute_ones_frame);
	bitmap_layer_set_compositing_mode(minute_ones_layer_even, GCompOpOr);
	bitmap_layer_set_alignment(minute_ones_layer_even, GAlignCenter);
	layer_add_child(minutes_clip_layer, bitmap_layer_get_layer(minute_ones_layer_even));

	minute_tens_layer_even = bitmap_layer_create(minute_tens_frame);
	bitmap_layer_set_compositing_mode(minute_tens_layer_even, GCompOpOr);
	bitmap_layer_set_alignment(minute_tens_layer_even, GAlignRight);
	layer_add_child(minutes_clip_layer, bitmap_layer_get_layer(minute_tens_layer_even));
}

void handle_init(void) 
{
	APP_LOG(APP_LOG_LEVEL_WARNING, "Heap used at start of initialisation: %u", heap_bytes_used());
	APP_LOG(APP_LOG_LEVEL_WARNING, "Heap free at start of initialisation: %u", heap_bytes_free());

	load_settings();
	init_settings();
	init_frames();
	
	window = window_create();
	window_set_background_color(window, bg_color);
	window_stack_push(window, true);
	root_layer = window_get_root_layer(window);
	
	init_layers();
	
	time_t now = time(NULL);
    struct tm *time = localtime(&now);
	int start_hour = (time->tm_hour)%12; //twelve hour mode only
	int start_minute = time->tm_min;
	int start_minute_tens = start_minute/10;
	int start_minute_ones = start_minute%10;
	
	if(start_minute == 0)
	{
		if(start_hour == 0)
		{
			start_hour = 11;
		}
		else
		{
			start_hour -= 1;
		}
	}
	
	if(start_minute_ones == 0)
	{
		if(start_minute_tens == 0)
		{
			start_minute_tens = 5;
		}
		else
		{
			start_minute_tens -= 1;
		}
		start_minute_ones = 9;
	}
	else
	{
		start_minute_ones -= 1;
	}

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting initial display time");
	APP_LOG(APP_LOG_LEVEL_DEBUG, "hour = %d", start_hour);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "minute = %d", start_minute);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "minute_tens = %d", start_minute_tens);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "minute_ones = %d",start_minute_ones);

	load_minute_image(start_minute_ones);
	load_minute_image(start_minute_tens);
	load_hour_image(start_hour);

	animate_minute_ones(start_minute_ones, 0, 0);
	animate_minute_tens(start_minute_tens, 0, 0);
	animate_hour(start_hour, 0, 0);

	inverter_layer = inverter_layer_create(GRect(0,0,144,168));
	layer_add_child(root_layer, inverter_layer_get_layer(inverter_layer));
	layer_set_hidden(inverter_layer_get_layer(inverter_layer), !invert_colors); //hide layer to not invert colors
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

void handle_deinit(void) 
{
	save_settings();
	animation_unschedule_all();
	
	bitmap_layer_destroy(hour_layer_odd);
	bitmap_layer_destroy(minute_tens_layer_odd);
	bitmap_layer_destroy(minute_ones_layer_odd);
	bitmap_layer_destroy(hour_layer_even);
	bitmap_layer_destroy(minute_tens_layer_even);
	bitmap_layer_destroy(minute_ones_layer_even);
	
	property_animation_destroy(minute_tens_animation_odd);
	property_animation_destroy(minute_ones_animation_odd);
	property_animation_destroy(hour_animation_odd);
	property_animation_destroy(minute_tens_animation_even);
	property_animation_destroy(minute_ones_animation_even);
	property_animation_destroy(hour_animation_even);

	for(int i = 0; i < 12; i++)
	{
		if(i < 10)
		{
			unload_minute_image(i);			
		}
		unload_hour_image(i);
	}
	inverter_layer_destroy(inverter_layer);
	window_destroy(window);
}

int main(void) 
{
	handle_init();
	app_event_loop();
	handle_deinit();
}