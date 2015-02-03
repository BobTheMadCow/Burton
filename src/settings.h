//uncomment to disable app logging
#undef APP_LOG
#define APP_LOG(...)

static AppSync app;
static uint8_t buffer[256];
enum Setting_keys { setting_key_invert_colors = 1, setting_key_animation_duration, setting_key_vibrate };

#define DEFAULT_INVERT_COLORS_VALUE false
#define DEFAULT_ANIMATION_DURATION_VALUE 2000
#define DEFAULT_VIBRATE_VALUE false
	
bool invert_colors;
int animation_duration;
bool vibrate;

GColor	bg_color = GColorBlack;
GColor	fg_color = GColorWhite;
static InverterLayer *inverter_layer;

static void tuple_changed_callback(const uint32_t key, const Tuple* tuple_new, const Tuple* tuple_old, void* context)
{
	  //  we know these values are uint8 format
	int value = tuple_new->value->uint8;
	switch (key) 
	{
    	case setting_key_invert_colors:
			if (invert_colors == true && value != 1)
			{
				APP_LOG(APP_LOG_LEVEL_DEBUG, "Change to invert_colors detected");
        		invert_colors = false;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "invert_colors set to false (%d)", invert_colors);
			}
			else if(invert_colors == false && value != 0)
			{
				APP_LOG(APP_LOG_LEVEL_DEBUG, "Change to invert_colors detected");
				invert_colors = true;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "invert_colors set to true (%d)", invert_colors);
			}
			layer_set_hidden(inverter_layer_get_layer(inverter_layer), !invert_colors);
			break;
		case setting_key_animation_duration:
			if (animation_duration/1000 != value && value >= 0 && value <= 5)
			{
				APP_LOG(APP_LOG_LEVEL_DEBUG, "Change to animation_duration detected");
				animation_duration = value*1000;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "animation_duration set to %d", animation_duration);
			}
			break;
		case setting_key_vibrate:
			if (vibrate == true && value != 1)
			{
				APP_LOG(APP_LOG_LEVEL_DEBUG, "Change to vibrate detected");
        		vibrate = false;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate set to false (%d)", vibrate);
			}
			else if(vibrate == false && value != 0)
			{
				APP_LOG(APP_LOG_LEVEL_DEBUG, "Change to vibrate detected");
				vibrate = true;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate set to true (%d)", vibrate);
			}
			break;
	}
}

static void app_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void* context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "app error %d", app_message_error);
}

static void init_settings()
{
		  //  app communication
	Tuplet tuples[] = {
		TupletInteger(setting_key_invert_colors, invert_colors),
		TupletInteger(setting_key_animation_duration, animation_duration),
		TupletInteger(setting_key_vibrate, vibrate)
	};
	app_message_open(160, 160);
	app_sync_init(&app, buffer, sizeof(buffer), tuples, ARRAY_LENGTH(tuples), tuple_changed_callback, app_error_callback, NULL);
}

static void load_settings()
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Loading settings");
	//	invert_colors = persist_exists(setting_key_invert_colors) ? persist_read_bool(setting_key_invert_colors) : DEFAULT_INVERT_COLORS_VALUE;
	if(persist_exists(setting_key_invert_colors))
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "invert_colors exists! loading value");
		invert_colors = persist_read_bool(setting_key_invert_colors);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "invert_colors = %d", invert_colors);
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "invert_colors doesn't exist! loading default value");
		invert_colors = DEFAULT_INVERT_COLORS_VALUE;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "invert_colors = %d", invert_colors);	
	}
	//	animation_duration = persist_exists(setting_key_animation_duration) ? persist_read_int(setting_key_animation_duration) : DEFAULT_ANIMATION_DURATION_VALUE;
	if(persist_exists(setting_key_animation_duration))
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "animation_duration exists! loading value");
		animation_duration = persist_read_int(setting_key_animation_duration);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "animation_duration = %d", animation_duration);
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "animation_duration doesn't exist! loading default value");
		animation_duration = DEFAULT_ANIMATION_DURATION_VALUE;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "animation_duration = %d", animation_duration);	
	}

	//	vibrate = persist_exists(setting_key_vibrate) ? persist_read_bool(setting_key_vibrate) : DEFAULT_VIBRATE_VALUE;
	if(persist_exists(setting_key_vibrate))
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate exists! loading value");
		vibrate = persist_read_bool(setting_key_vibrate);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate = %d", vibrate);
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate doesn't exist! loading default value");
		vibrate = DEFAULT_VIBRATE_VALUE;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate = %d", vibrate);	
	}
}

static void save_settings()
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Saving settings");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "invert_colors = %d", invert_colors);	
	persist_write_bool(setting_key_invert_colors, invert_colors);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "animation_duration = %d", animation_duration);	
	persist_write_int(setting_key_animation_duration, animation_duration);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate = %d", vibrate);	
	persist_write_bool(setting_key_vibrate, vibrate);
}