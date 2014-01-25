#define INVERT_COLORS_KEY 0
#define ANIMATION_DURATION_KEY 1
#define VIBE_KEY 2
#define DEFAULT_INVERT_COLORS_VALUE false
#define DEFALUT_ANIMATION_DURATION_VALUE 3000 //3 seconds - short enough for the animation to complete before the light goes out.
#define DEFAULT_VIBE_VALUE false
bool invert_colors;
int animation_duration;
bool vibe;

static GColor fg_color;
static GColor bg_color;

static void init_settings()
{
	invert_colors = persist_exists(INVERT_COLORS_KEY) ? persist_read_bool(INVERT_COLORS_KEY) : DEFAULT_INVERT_COLORS_VALUE;
	animation_duration = persist_exists(ANIMATION_DURATION_KEY) ? persist_read_int(ANIMATION_DURATION_KEY) : DEFALUT_ANIMATION_DURATION_VALUE;
	vibe = persist_exists(VIBE_KEY) ? persist_read_bool(VIBE_KEY) : DEFAULT_VIBE_VALUE;
}

static void save_settings()
{
    persist_write_bool(INVERT_COLORS_KEY, invert_colors);
    persist_write_int(ANIMATION_DURATION_KEY, animation_duration);
	persist_write_bool(VIBE_KEY, vibe);
}
