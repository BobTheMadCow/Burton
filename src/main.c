#include <pebble.h>
#include <settings.h>

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

BitmapLayer *hour_layer[2];
BitmapLayer *minute_tens_layer[2];
BitmapLayer *minute_ones_layer[2];

static InverterLayer *inverter;

static PropertyAnimation *minute_tens_animation[2];
static PropertyAnimation *minute_ones_animation[2];
static PropertyAnimation *hour_animation[2];

GBitmap *minute_image[10];
GBitmap *hour_image[12];

static void run_animation(PropertyAnimation *animation, Layer *layer, GRect from_frame, GRect to_frame, int delay, int duration)
{
	animation = property_animation_create_layer_frame(layer, &from_frame, &to_frame);
	animation_set_curve((Animation*)animation, CURVE);
	animation_set_delay((Animation*)animation, delay);
	animation_set_duration((Animation*)animation, duration);
	animation_schedule((Animation*)animation);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
	int hour = (tick_time->tm_hour)%12; //twelve hour mode only
	int minute = tick_time->tm_min;
	int minute_tens = minute/10;
	int minute_ones = minute%10;
	
	int delay = 0;
	int duration = 2*(animation_duration/3);

	int prev_one, next_one;
	int prev_ten, next_ten;
	int prev_hour, next_hour;

	if(minute_ones%2 == 1)
	{
		prev_one = 0;
		next_one = 1;
	}
	else
	{
		prev_one = 1;
		next_one = 0;
	}
	bitmap_layer_set_bitmap(minute_ones_layer[next_one], minute_image[minute_ones]);
	
	//run_animation(PropertyAnimation *animation, Layer *layer_in, GRect *from_frame, GRect *to_frame)
	run_animation(minute_ones_animation[prev_one],bitmap_layer_get_layer(minute_ones_layer[prev_one]), minute_ones_frame, minute_ones_out_frame, delay, duration);
	run_animation(minute_ones_animation[next_one],bitmap_layer_get_layer(minute_ones_layer[next_one]), minute_ones_in_frame, minute_ones_frame, delay, duration);

	delay += (animation_duration/6);
	
	if(minute_ones == 0)
	{
		if(minute_tens%2 == 1)
		{
			prev_ten = 0;
			next_ten = 1;
		}
		else
		{
			prev_ten = 1;
			next_ten = 0;
		}
		bitmap_layer_set_bitmap(minute_tens_layer[next_ten], minute_image[minute_tens]);
		
		run_animation(minute_tens_animation[prev_ten],bitmap_layer_get_layer(minute_tens_layer[prev_ten]), minute_tens_frame, minute_tens_out_frame, delay, duration);
		run_animation(minute_tens_animation[next_ten],bitmap_layer_get_layer(minute_tens_layer[next_ten]), minute_tens_in_frame, minute_tens_frame, delay, duration);
	}

	delay += (animation_duration/6);
	
	if(minute == 0)
	{
		if(hour%2 == 1)
		{
			prev_hour = 0;
			next_hour = 1;
		}
		else
		{
			prev_hour = 1;
			next_hour = 0;
		}
		bitmap_layer_set_bitmap(hour_layer[next_hour], hour_image[hour]);
	
		run_animation(hour_animation[prev_hour],bitmap_layer_get_layer(hour_layer[prev_hour]), hour_frame, hour_out_frame, delay, duration);
		run_animation(hour_animation[next_hour],bitmap_layer_get_layer(hour_layer[next_hour]), hour_in_frame, hour_frame, delay, duration);
	}
	
	if(vibe && tick_time->tm_min == 0)
	{
		vibes_double_pulse();
	}
}

static void init_images()
{
	minute_image[1] = gbitmap_create_with_resource(RESOURCE_ID_NUM_ONE_WHITE);
	minute_image[2] = gbitmap_create_with_resource(RESOURCE_ID_NUM_TWO_WHITE);
	minute_image[3] = gbitmap_create_with_resource(RESOURCE_ID_NUM_THREE_WHITE);
	minute_image[4] = gbitmap_create_with_resource(RESOURCE_ID_NUM_FOUR_WHITE);
	minute_image[5] = gbitmap_create_with_resource(RESOURCE_ID_NUM_FIVE_WHITE);
	minute_image[6] = gbitmap_create_with_resource(RESOURCE_ID_NUM_SIX_WHITE);
	minute_image[7] = gbitmap_create_with_resource(RESOURCE_ID_NUM_SEVEN_WHITE);
	minute_image[8] = gbitmap_create_with_resource(RESOURCE_ID_NUM_EIGHT_WHITE);
	minute_image[9] = gbitmap_create_with_resource(RESOURCE_ID_NUM_NINE_WHITE);
	minute_image[0] = gbitmap_create_with_resource(RESOURCE_ID_NUM_ZERO_WHITE);
	
	hour_image[1] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_ONE_WHITE);
	hour_image[2] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_TWO_WHITE);
	hour_image[3] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_THREE_WHITE);
	hour_image[4] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_FOUR_WHITE);
	hour_image[5] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_FIVE_WHITE);
	hour_image[6] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_SIX_WHITE);
	hour_image[7] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_SEVEN_WHITE);
	hour_image[8] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_EIGHT_WHITE);
	hour_image[9] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_NINE_WHITE);
	hour_image[10] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_TEN_WHITE);
	hour_image[11] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_ELEVEN_WHITE);
	hour_image[0] = gbitmap_create_with_resource(RESOURCE_ID_TEXT_TWELVE_WHITE);
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

static void update_settings()
{
	layer_set_hidden(inverter_layer_get_layer(inverter), !invert_colors);
}

void handle_init(void) 
{
	bg_color = GColorBlack;
	fg_color = GColorWhite;
	
	init_settings();
	
	init_images();
	init_frames();
	
	window = window_create();
	window_set_background_color(window, bg_color);
	window_stack_push(window, true);
	root_layer = window_get_root_layer(window);
	
	minutes_clip_layer = layer_create(minute_clip_frame);
	layer_set_clips(minutes_clip_layer, true);
	layer_add_child(root_layer, minutes_clip_layer);
	
	for(int i = 0; i < 2; i++)
	{
		hour_layer[i] = bitmap_layer_create(hour_in_frame);
		bitmap_layer_set_compositing_mode(hour_layer[i], GCompOpOr);
		bitmap_layer_set_alignment(hour_layer[i], GAlignLeft);
		layer_add_child(root_layer, bitmap_layer_get_layer(hour_layer[i]));
	
		minute_ones_layer[i] = bitmap_layer_create(minute_ones_in_frame);
		bitmap_layer_set_compositing_mode(minute_ones_layer[i], GCompOpOr);
		bitmap_layer_set_alignment(minute_ones_layer[i], GAlignCenter);
		layer_add_child(minutes_clip_layer, bitmap_layer_get_layer(minute_ones_layer[i]));
	
		minute_tens_layer[i] = bitmap_layer_create(minute_tens_in_frame);
		bitmap_layer_set_compositing_mode(minute_tens_layer[i], GCompOpOr);
		bitmap_layer_set_alignment(minute_tens_layer[i], GAlignRight);
		layer_add_child(minutes_clip_layer, bitmap_layer_get_layer(minute_tens_layer[i]));
	}

	time_t now = time(NULL);
    struct tm *time = localtime(&now);
	int hour = (time->tm_hour)%12; //twelve hour mode only
	int minute = time->tm_min;
	int minute_tens = minute/10;
	int minute_ones = minute%10;
	
	if(minute == 0)
	{
		if(hour == 0)
		{
			hour = 11;
		}
		else
		{
			hour -= 1;
		}
	}
	
	if(minute_ones == 0)
	{
		if(minute_tens == 0)
		{
			minute_tens = 5;
		}
		else
		{
			minute_tens -= 1;
		}
		minute_ones = 9;
	}
	else
	{
		minute_ones -= 1;
	}
	
	bitmap_layer_set_bitmap(minute_ones_layer[minute_ones%2], minute_image[minute_ones]);
	bitmap_layer_set_bitmap(minute_tens_layer[minute_tens%2], minute_image[minute_tens]);
	bitmap_layer_set_bitmap(hour_layer[hour%2], hour_image[hour]);

	run_animation(minute_ones_animation[minute_ones%2],bitmap_layer_get_layer(minute_ones_layer[minute_ones%2]), minute_ones_in_frame, minute_ones_frame, 0, 0);
	run_animation(minute_tens_animation[minute_tens%2],bitmap_layer_get_layer(minute_tens_layer[minute_tens%2]), minute_tens_in_frame, minute_tens_frame, 0, 0);
	run_animation(hour_animation[hour%2],bitmap_layer_get_layer(hour_layer[hour%2]), hour_in_frame, hour_frame, 0, 0);
	
	inverter = inverter_layer_create(GRect(0,0,144,168));
	layer_add_child(root_layer, inverter_layer_get_layer(inverter));
	layer_set_hidden(inverter_layer_get_layer(inverter), !invert_colors); //hide layer to not invert colors
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);	
}

void handle_deinit(void) 
{
	for(int i = 0; i < 2; i++)
	{
		bitmap_layer_destroy(hour_layer[i]);
		bitmap_layer_destroy(minute_tens_layer[i]);
		bitmap_layer_destroy(minute_ones_layer[i]);
		animation_destroy((Animation*) minute_tens_animation[i]);
		animation_destroy((Animation*) minute_ones_animation[i]);
		animation_destroy((Animation*) hour_animation[i]);
	}
	for(int i = 0; i < 12; i++)
	{
		if(i < 10)
		{
			gbitmap_destroy(minute_image[i]);			
		}
		gbitmap_destroy(hour_image[i]);
	}
	inverter_layer_destroy(inverter);
	window_destroy(window);
	save_settings();
}

int main(void) 
{
	handle_init();
	app_event_loop();
	handle_deinit();
}