// [main.c]
// this template is provided for the 2D shooter game.

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <math.h>
#include <ctype.h>

// If defined, logs will be shown on console and written to file.
// If commented out, logs will not be shown nor be saved.
#define LOG_ENABLED

/* Constants. */

// Frame rate (frame per second)
const int FPS = 60;

// At most 4 audios can be played at a time.
const int RESERVE_SAMPLES = 4;
// Same as:
// const int SCENE_MENU = 1;
// const int SCENE_GAME = 2;
enum {
	SCENE_MENU = 1,
	SCENE_GAME = 2,
	// [HACKATHON 3-7]
	// TODO: Declare a new scene id.
	SCENE_S_SELECT = 3,
	SCENE_SETTINGS = 4,
	SCENE_PAUSE = 5,
	SCENE_RESULTS = 6,
	SCENE_INTRO = 7,
	SCENE_INTRO2 = 8
};

/* Input states */

// The active scene id.
int active_scene;
// Keyboard state, whether the key is down or not.
bool key_state[ALLEGRO_KEY_MAX];
// Mouse state, whether the key is down or not.
// 1 is for left, 2 is for right, 3 is for middle.
bool *mouse_state;
// Mouse position.
int mouse_x, mouse_y;
// TODO: More variables to store input states such as joysticks, ...

/* Variables for allegro basic routines. */

ALLEGRO_DISPLAY* game_display;
ALLEGRO_EVENT_QUEUE* game_event_queue;
ALLEGRO_TIMER* game_update_timer;
ALLEGRO_TIMER* beat_timer;

/* Shared resources*/
ALLEGRO_FONT* font_pirulen_120;
ALLEGRO_FONT* font_pirulen_72;
ALLEGRO_FONT* font_pirulen_40;
ALLEGRO_FONT* font_pirulen_30;
ALLEGRO_FONT* font_calibri_35;
ALLEGRO_FONT* font_freescript_40;
// TODO: More shared resources or data that needed to be accessed
// across different scenes.
// common colors
ALLEGRO_COLOR gray_middle;
ALLEGRO_COLOR black;
ALLEGRO_COLOR white;
ALLEGRO_COLOR red;
ALLEGRO_COLOR green;
ALLEGRO_COLOR blue;


// Save Files
FILE* config;
FILE* save;

// UI sound samples
ALLEGRO_SAMPLE* ui_click;
ALLEGRO_SAMPLE* ui_slide;
ALLEGRO_SAMPLE* ui_enter01;
ALLEGRO_SAMPLE* ui_enter02;
ALLEGRO_SAMPLE* ui_back01;
ALLEGRO_SAMPLE* ui_back02;
ALLEGRO_SAMPLE_ID ui_click_id;
ALLEGRO_SAMPLE_ID ui_slide_id;
ALLEGRO_SAMPLE_ID ui_enter01_id;
ALLEGRO_SAMPLE_ID ui_enter02_id;
ALLEGRO_SAMPLE_ID ui_back01_id;
ALLEGRO_SAMPLE_ID ui_back02_id;

// Results BGM
ALLEGRO_SAMPLE* clear_song;
ALLEGRO_SAMPLE* failed_song;
ALLEGRO_SAMPLE_ID clear_song_id;
ALLEGRO_SAMPLE_ID failed_song_id;

/* Menu Scene resources*/
ALLEGRO_BITMAP* main_img_background;
// [HACKATHON 3-1]
// TODO: Declare 2 variables for storing settings images.
// Uncomment and fill in the code below.
ALLEGRO_BITMAP* img_settings;
ALLEGRO_BITMAP* img_settings2;
ALLEGRO_SAMPLE* main_bgm;
ALLEGRO_SAMPLE_ID main_bgm_id;

// INTRO resources
ALLEGRO_BITMAP* allegro_logo;
ALLEGRO_BITMAP* headphones_img;

/* Start Scene resources*/
ALLEGRO_BITMAP* start_img_background;
ALLEGRO_BITMAP* start_img_plane;
ALLEGRO_BITMAP* img_enemy01;
ALLEGRO_SAMPLE* start_bgm;
ALLEGRO_SAMPLE_ID start_bgm_id;
ALLEGRO_BITMAP* img_enemy02;
ALLEGRO_BITMAP* img_enemy03;
ALLEGRO_BITMAP* img_enemy03_a;
// [HACKATHON 2-1]
// TODO: Declare a variable to store your bullet's image.
// Uncomment and fill in the code below.
ALLEGRO_BITMAP* img_bullet;
ALLEGRO_BITMAP* img_red_bullet;
ALLEGRO_BITMAP* img_green_bullet;
ALLEGRO_BITMAP* img_blue_bullet;
ALLEGRO_BITMAP* img_red_s_bullet;
ALLEGRO_BITMAP* img_green_s_bullet;
ALLEGRO_BITMAP* img_blue_s_bullet;
ALLEGRO_BITMAP* img_white_bullet;

// "rectangle" bitmaps
ALLEGRO_BITMAP* img_red_rectangle;
ALLEGRO_BITMAP* img_green_rectangle;
ALLEGRO_BITMAP* img_blue_rectangle;

// Bitmaps for tutorial
ALLEGRO_BITMAP* img_z_key;
ALLEGRO_BITMAP* img_x_key;
ALLEGRO_BITMAP* img_c_key;
ALLEGRO_BITMAP* img_z_key_p;
ALLEGRO_BITMAP* img_x_key_p;
ALLEGRO_BITMAP* img_c_key_p;
ALLEGRO_BITMAP* img_arrow_key;

	/* Structs */
// UI elements
struct menu_detail {
	char name[51];
	bool highlight;
};
typedef struct {
	int pos;
	int num;
	struct menu_detail item[5];

}MenuElement;
MenuElement menu;
MenuElement settings;
MenuElement s_select;
MenuElement pause;

//Songs
struct SongDetail{
	int id;
	char name[61];
	char author[51];
	int bpm;
	int length;
	float percent;
	int final_score;
	int full_score;
	char mark;
	int cleared;
	FILE* map;
	ALLEGRO_SAMPLE* sample;
	ALLEGRO_SAMPLE* pre_sample;
	ALLEGRO_SAMPLE_ID sample_id;
	ALLEGRO_SAMPLE_ID pre_sample_id;
	ALLEGRO_SAMPLE_INSTANCE* sample_ins;
};
typedef struct {
	int cur_song;
	int num;
	struct SongDetail id[3];
}Songs;
Songs songs;

// rhythm lines
#define MAX_LINES 13
typedef struct {
	ALLEGRO_COLOR color;
	int type;
	bool hidden;
	double r;
} RhythmLine;
RhythmLine r_line[MAX_LINES];

void rhythm_judge(int i, int type);

// Resolution Settings
const int ResSets[6][2] = { {2560, 1440}, {1920, 1080}, {1600, 900}, {1280, 720}, {1024, 576}, {800, 450} };
int Respos = 2; // defult

typedef struct {
	// The center coordinate of the image.
	float x, y;
	// The width and height of the object.
	float w, h;
	// The velocity in x, y axes.
	float vx, vy;
	// Should we draw this object on the screen.
	bool hidden;
	// The pointer to the object’s image.
	ALLEGRO_BITMAP* img;
	int hp;
	double last_coli;
	bool tint;
	double tint_t;
	int b_type;
	double angle;
} MovableObject;
void draw_movable_object(MovableObject obj);
void draw_movable_object_rotate(MovableObject obj);
#define MAX_ENEMY01 20
#define ENEMY01_HP 2
#define MAX_ENEMY02 10
#define ENEMY02_HP 4
#define MAX_ENEMY03 2
#define ENEMY03_HP 100
#define PLANE_HP 100
// [HACKATHON 2-2]
// TODO: Declare the max bullet count that will show on screen.
// You can try max 4 bullets here and see if you needed more.
// Uncomment and fill in the code below.
#define MAX_BULLET 50
#define MAX_E01_BULLET 200
#define MAX_E02_BULLET 100
#define MAX_E03_LAZER 3
#define MAX_WHITE_BULLET 310
#define BULLET_DESTROY_R 650
MovableObject plane;
MovableObject enemy01[MAX_ENEMY01];
MovableObject e01_bullets[MAX_E01_BULLET];
MovableObject enemy02[MAX_ENEMY02];
MovableObject e02_bullets[MAX_E02_BULLET];
MovableObject enemy03[MAX_ENEMY03];
MovableObject white_bullets[MAX_WHITE_BULLET];
MovableObject bullets[MAX_BULLET];

typedef struct {
	int x;
	int y;
	double last_t;
	double last_coli;
	int damage;
	bool hidden;
	int width;
	MovableObject* source;
}lazers;
lazers e03_lazers[MAX_E03_LAZER];

// [HACKATHON 2-4]
// TODO: Set up bullet shooting cool-down variables.
// 1) Declare your shooting cool-down time as constant. (0.2f will be nice)
// 2) Declare your last shoot timestamp.
// Uncomment and fill in the code below.
double last_shoot_timestamp;
double last_shoot_timestamp_e01;
double last_shoot_timestamp_e02;
double last_shoot_timestamp_e03;

/* Global variables for the game */
// graphics
const float MARGIN_RATIO = 0.2;
float SCALE_RATIO;
int res_changed = 0;
int last_scene;
int background_a = 0;
// game event loop
bool done = false;
// music and beat
#define MAX_BAR 801
char cur_map[MAX_BAR][8];
int total_beat;
double bpm_t;
int last_s_pos = 0;
double cur_time;
double beat_cur_time;
double last_beat_time;
int count_beat;
int count_bar;
int last_bar_1;
double line_speed;
int combo_count = 0;
int e01_last_bar;
int music_offset;
bool m_offset;
double last_beat_timer_tick;
bool beat_timer_flag = false;
double bpm_error;
// collisions
#define coli_cooldown 1.5
// for game pause
unsigned int pause_song_pos;
double pause_time_delta;
// for scene transition
bool fade_now = false;
#define FADE_SPEED 40
#define INTRO_TIME 2
double intro_count;
// Text animations (transparency)
int perfect_a = 0;
int good_a = 0;
int missed_a = 0;
int judge_text_v = 9;
int score_a = 255;
// Tutorial text animations
int tutor_text_delta = 0;
int tutor_text_v;
// "rectangle" animations
int red_rec_a = 0;
int blue_rec_a = 0;
int green_rec_a = 0;
// red tint animation
#define red_tint_time 0.5 
// SCORES
float cur_percent;
int cur_hit_score;
int cur_hit_score_buf;
int cur_final_score;
int cur_final_score_buf;
int cur_missed;
int cur_good;
int cur_perfect;
int cur_max_combo;
char cur_mark;
int energy;
int bullet_level;
bool end_status;
bool high_score;
#define ENEMY01_SCORE 100
#define ENEMY02_SCORE 200
#define ENEMY03_SCORE 1000

// bars
double song_prog_bar_v;
double song_prog_bar_pos;
float hp_bar_buffer;
float hp_bar_v = 0.5;
float energy_bar_buffer;
float energy_bar_v = 1;
int energy_a = 255;
unsigned char circle_r;
unsigned char circle_g;
// judging test DEBUG
int perfect_r;
int good_r;
int ignore_r;

/* Game Settings */
// Display (screen) width.(defult)
int SCREEN_W = 1600;
// Display (screen) height.(defult)
int SCREEN_H = 900;
// Audio Levels
int music_lvl = 10;
int sfx_lvl = 10;


/* Declare function prototypes. */

// Initialize allegro5 library
void allegro5_init(void);
// Initialize variables and resources.
// Allows the game to perform any initialization it needs before
// starting to run.
void game_init(void);
// Process events inside the event queue using an infinity loop.
void game_start_event_loop(void);
// Run game logic such as updating the world, checking for collision,
// switching scenes and so on.
// This is called when the game should update its logic.
void game_update(void);
// Draw to display.
// This is called when the game should draw itself.
void game_draw(void);
// Release resources.
// Free the pointers we allocated.
void game_destroy(void);
// Function to change from one scene to another.
void game_change_scene(int next_scene);
void scene_fade_out();
void scene_fade_in();

// Other functions
void object_hit(MovableObject* obj_p, int pos, int hp, double time);
void save_song_score();
void draw_white_bullets(int type);
int cal_obj_distance(MovableObject a, MovableObject b);
int cal_pnt_distance(int x1, int y1, int x2, int y2);
int cal_pnt_to_line(int a1, int a2, int b1, int b2, int x, int y);
// Load resized bitmap and check if failed.
ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h);
// [HACKATHON 3-2]
// TODO: Declare a function.
// Determines whether the point (px, py) is in rect (x, y, w, h).
// Uncomment the code below.
bool pnt_in_rect(int px, int py, int x, int y, int w, int h);
bool pnt_in_object(int px, int py, MovableObject obj, int offset);
bool pnt_in_object_circle(int px, int py, MovableObject obj, int r);
/* Event callbacks. */
void on_key_down(int keycode);
void on_mouse_down(int btn, int x, int y);

/* Declare function prototypes for debugging. */

// Display error message and exit the program, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// If the program crashes unexpectedly, you can inspect "log.txt" for
// further information.
void game_abort(const char* format, ...);
// Log events for later debugging, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// You can inspect "log.txt" for logs in the last run.
void game_log(const char* format, ...);
// Log using va_list.
void game_vlog(const char* format, va_list arg);

int main(int argc, char** argv) {
	// Set random seed for better random outcome.
	srand(time(NULL));
	allegro5_init();
	game_log("Allegro5 initialized");
	game_log("Game begin");
	// Initialize game variables.
	game_init();
	game_log("Game initialized");
	// Draw the first frame.
	game_draw();
	game_log("Game start event loop");
	// This call blocks until the game is finished.
	game_start_event_loop();
	game_log("Game end");
	game_destroy();
	return 0;
}

void allegro5_init(void) {
	if (!al_init())
		game_abort("failed to initialize allegro");

	// Initialize add-ons.
	if (!al_init_primitives_addon())
		game_abort("failed to initialize primitives add-on");
	if (!al_init_font_addon())
		game_abort("failed to initialize font add-on");
	if (!al_init_ttf_addon())
		game_abort("failed to initialize ttf add-on");
	if (!al_init_image_addon())
		game_abort("failed to initialize image add-on");
	if (!al_install_audio())
		game_abort("failed to initialize audio add-on");
	if (!al_init_acodec_addon())
		game_abort("failed to initialize audio codec add-on");
	if (!al_reserve_samples(RESERVE_SAMPLES))
		game_abort("failed to reserve samples");
	if (!al_install_keyboard())
		game_abort("failed to install keyboard");
	if (!al_install_mouse())
		game_abort("failed to install mouse");
	// TODO: Initialize other addons such as video, ...
	// open save files
	config = fopen("config.txt", "r");
	if (config == NULL)
		game_abort("failed to open config.txt");

	game_log("Config file is loaded");
	// read in "config" file
	if (fscanf(config, "%d", &Respos) != 1)
		game_abort("failed to read Resolution");
	if (fscanf(config, "%d", &music_lvl) != 1)
		game_abort("failed to read music_lvl");
	if (fscanf(config, "%d", &sfx_lvl) != 1)
		game_abort("failed to read sfx_lvl");
	if (fscanf(config, "%d", &perfect_r) != 1)
		game_abort("failed to read perfect_r");
	if (fscanf(config, "%d", &good_r) != 1)
		game_abort("failed to read good_r");
	if (fscanf(config, "%d", &ignore_r) != 1)
		game_abort("failed to read ignore_r");
	if (fscanf(config, "%d", &music_offset) != 1)
		game_abort("failed to read music offset");

	fclose(config);

	SCREEN_W = ResSets[Respos][0];
	SCREEN_H = ResSets[Respos][1];
	SCALE_RATIO = SCREEN_W * 0.000625;

	// Setup game display.
	game_display = al_create_display(SCREEN_W, SCREEN_H);
	if (!game_display)
		game_abort("failed to create display");
	al_set_window_title(game_display, "I2P(I)_2019 Final Project 108062103");

	// Setup update timer.
	game_update_timer = al_create_timer(1.0f / FPS);
	if (!game_update_timer)
		game_abort("failed to create timer");
	beat_timer = al_create_timer(1.0f / FPS);
	if (!beat_timer)
		game_abort("failed to create beat timer");

	// Setup event queue.
	game_event_queue = al_create_event_queue();
	if (!game_event_queue)
		game_abort("failed to create event queue");

	// Malloc mouse buttons state according to button counts.
	const unsigned m_buttons = al_get_mouse_num_buttons();
	game_log("There are total %u supported mouse buttons", m_buttons);
	// mouse_state[0] will not be used.
	mouse_state = malloc((m_buttons + 1) * sizeof(bool));
	memset(mouse_state, false, (m_buttons + 1) * sizeof(bool));

	// Register display, timer, keyboard, mouse events to the event queue.
	al_register_event_source(game_event_queue, al_get_display_event_source(game_display));
	al_register_event_source(game_event_queue, al_get_timer_event_source(game_update_timer));
	al_register_event_source(game_event_queue, al_get_timer_event_source(beat_timer));
	al_register_event_source(game_event_queue, al_get_keyboard_event_source());
	al_register_event_source(game_event_queue, al_get_mouse_event_source());
	// TODO: Register other event sources such as timer, video, ...

	// Start the timer to update and draw the game.
	al_start_timer(game_update_timer);
}

void game_init(void) {
	/* Shared resources*/
	font_pirulen_120 = al_load_font("pirulen.ttf", (int)120 * SCALE_RATIO, 0);
	if (!font_pirulen_120)
		game_abort("failed to load font: pirulen.ttf with size 120");
	font_pirulen_72 = al_load_font("pirulen.ttf", (int)72*SCALE_RATIO, 0);
	if (!font_pirulen_72)
		game_abort("failed to load font: pirulen.ttf with size 72");

	font_pirulen_40 = al_load_font("pirulen.ttf", (int)40*SCALE_RATIO, 0);
	if (!font_pirulen_40)
		game_abort("failed to load font: pirulen.ttf with size 40");
	font_pirulen_30 = al_load_font("pirulen.ttf", (int)30 * SCALE_RATIO, 0);
	if (!font_pirulen_30)
		game_abort("failed to load font: pirulen.ttf with size 30");
	font_calibri_35 = al_load_font("calibri.ttf", (int)35 * SCALE_RATIO, 0);
	if(!font_calibri_35)
		game_abort("failed to load font: calibri.ttf with size 35");
	font_freescript_40 = al_load_font("FREESCPT.TTF", (int)40 * SCALE_RATIO, 0);
	if (!font_freescript_40)
		game_abort("failed to load font: FREESCPT.TTF with size 30");

	// colors
	gray_middle = al_map_rgb(128, 128, 128);
	black = al_map_rgb(0, 0, 0);
	white = al_map_rgb(255, 255, 255);
	red = al_map_rgb(255, 77, 77);
	green = al_map_rgb(0, 230, 115);
	blue = al_map_rgb(102, 194, 255);
	
	// common varibles

	// UI sound samples
	ui_back01 = al_load_sample("ui back01.ogg");
	if (!ui_back01)
		game_abort("Failed to load sample: ui back01.ogg");
	ui_back02 = al_load_sample("ui back02.ogg");
	if (!ui_back02)
		game_abort("Failed to load sample: ui back02.ogg");
	ui_click = al_load_sample("ui click.ogg");
	if (!ui_click)
		game_abort("Failed to load sample: ui click.ogg");
	ui_enter01 = al_load_sample("ui enter01.ogg");
	if (!ui_enter01)
		game_abort("Failed to load sample: ui enter01.ogg");
	ui_enter02 = al_load_sample("ui enter02.ogg");
	if (!ui_enter02)
		game_abort("Failed to load sample: ui enter02.ogg");
	ui_slide = al_load_sample("ui slide.ogg");
	if (!ui_slide)
		game_abort("Failed to load sample: ui slide.ogg");

	// Results BGM
	clear_song = al_load_sample("Crab Rave (part).ogg");
	if (!clear_song)
		game_abort("Failed to load sample: Crab Rave (part).ogg");
	failed_song = al_load_sample("GTA5 failed sfx.ogg");
	if (!failed_song)
		game_abort("Failed to load sample: GTA5 failed sfx.ogg");

	// Load INTRO resources
	allegro_logo = load_bitmap_resized("allegro logo.png", 400 * SCALE_RATIO, 132 * SCALE_RATIO);
	if (!allegro_logo)
		game_abort("failed to load image: allegro logo.png");
	headphones_img = load_bitmap_resized("headphone logo.png", 256 * SCALE_RATIO, 256 * SCALE_RATIO);
	if (!headphones_img)
		game_abort("failed to load image: headphone logo.png");

	/* Initialize Songs */
	//num (amount of songs)
	songs.num = 3;
	//cur_song (current position)
	songs.cur_song = 0;
	// DETAILS for each songs
	//id
	for (int i = 0; i < songs.num; i++)
	{
		songs.id[i].id = i;
	}
	//name
	strcpy(songs.id[0].name, "Tutorial - Heart of The Mountain");
	strcpy(songs.id[1].name, "The Spectre [NCS Release]");
	strcpy(songs.id[2].name, "Blank Page 空白頁面");
	//author
	strcpy(songs.id[0].author, "Lena Raine");
	strcpy(songs.id[1].author, "Alan Walker");
	strcpy(songs.id[2].author, "Sweet John 甜約翰");
	//bpm
	songs.id[0].bpm = 100;
	songs.id[1].bpm = 128;
	songs.id[2].bpm = 144;
	//length
	songs.id[0].length = 318;
	songs.id[1].length = 492;
	songs.id[2].length = 595;
	//samples
	songs.id[0].sample = al_load_sample("Heart of The Mountain(part).ogg");
	if (!songs.id[0].sample)
		game_abort("failed to load song : Heart of The Mountain(part)");

	songs.id[1].sample = al_load_sample("The Spectre.ogg");
	if (!songs.id[1].sample)
		game_abort("failed to load song : The Spectre.ogg");

	songs.id[2].sample = al_load_sample("Blank Page.ogg");
	if (!songs.id[2].sample)
		game_abort("failed to load song : Blank Page.ogg");

	//pre_samples
	songs.id[0].pre_sample = al_load_sample("Heart of The Mountain(pre).ogg");
	if (!songs.id[0].pre_sample)
		game_abort("failed to load song : Heart of The Mountain(pre)");

	songs.id[1].pre_sample = al_load_sample("The Spectre (pre).ogg");
	if (!songs.id[1].pre_sample)
		game_abort("failed to load song : The Spectre (pre).ogg");

	songs.id[2].pre_sample = al_load_sample("Blank Page (pre).ogg");
	if (!songs.id[2].pre_sample)
		game_abort("failed to load song : Blank Page (pre).ogg");

	//sample instances
	songs.id[0].sample_ins = al_create_sample_instance(songs.id[0].sample);
	songs.id[1].sample_ins = al_create_sample_instance(songs.id[1].sample);
	songs.id[2].sample_ins = al_create_sample_instance(songs.id[2].sample);

	/* Read score save files */
	save = fopen("save.txt", "r");
	if (save == NULL)
		game_abort("failed to open file: save.txt");
	game_log("Save file is loaded");
	for(int i=0; i<songs.num; i++)
	{
		if (fscanf(save, "%c\n", &songs.id[i].mark) != 1)
			game_abort("falied to load: mark %d", i);
		if (fscanf(save, "%d\n", &songs.id[i].cleared) != 1)
			game_abort("falied to load: cleared %d", i);
		if (fscanf(save, "%d\n", &songs.id[i].final_score) != 1)
			game_abort("falied to load: score %d", i);
		if (fscanf(save, "%f\n", &songs.id[i].percent) != 1)
			game_abort("falied to load: precent %d", i);
	}
	fclose(save);

	/* Initialize UI Elements*/
	// num (number of items)
	menu.num = 3;
	settings.num = 4;
	s_select.num = 3;
	pause.num = 3;

	//name
	strcpy(menu.item[0].name, "Start");
	strcpy(menu.item[1].name, "Settings");
	strcpy(menu.item[2].name, "Quit");

	strcpy(settings.item[0].name, "Resolution");
	strcpy(settings.item[1].name, "Music volume");
	strcpy(settings.item[2].name, "SFX volume");
	strcpy(settings.item[3].name, "Music offset");


	strcpy(s_select.item[0].name, songs.id[0].name);
	strcpy(s_select.item[1].name, songs.id[1].name);
	strcpy(s_select.item[2].name, songs.id[2].name);

	strcpy(pause.item[0].name, "Resume");
	strcpy(pause.item[1].name, "Retry");
	strcpy(pause.item[2].name, "Give up");

	/* Menu Scene resources*/
	main_img_background = load_bitmap_resized("purple background.png", SCREEN_W, SCREEN_H);

	main_bgm = al_load_sample("Discovery.ogg");
	if (!main_bgm)
		game_abort("failed to load audio: S31-Night Prowler.ogg");

	// [HACKATHON 3-4]
	// TODO: Load settings images.
	// Don't forget to check their return values.
	// Uncomment and fill in the code below.
	img_settings = load_bitmap_resized("settings.png", (int)38 * SCALE_RATIO, (int)38 * SCALE_RATIO);
	if (!img_settings)
		game_abort("failed to load image: settings.png");
	img_settings2 = load_bitmap_resized("settings2.png", (int)38 * SCALE_RATIO, (int)38 * SCALE_RATIO);
	if (!img_settings2)
		game_abort("failed to load image: settings2.png");

	/* Start Scene resources*/
	start_img_background = load_bitmap_resized("gray background.png", SCREEN_W, SCREEN_H*2);

	start_img_plane = load_bitmap_resized("plane.png", (int)51*SCALE_RATIO, (int)51*SCALE_RATIO );
	if (!start_img_plane)
		game_abort("failed to load image: plane.png");

	img_enemy01 = load_bitmap_resized("enemy01.png", (int)41*SCALE_RATIO, (int)50*SCALE_RATIO);
	if (!img_enemy01)
		game_abort("failed to load image: enemy01.png");

	start_bgm = al_load_sample("mythica.ogg");
	if (!start_bgm)
		game_abort("failed to load audio: mythica.ogg");

	img_enemy02 = load_bitmap_resized("enemy02.png", (int)58 * SCALE_RATIO, (int)90 * SCALE_RATIO);
	if (!img_enemy02)
		game_abort("failed to load image: enemy02.png");

	img_enemy03 = load_bitmap_resized("enemy03.png", (int)200 * SCALE_RATIO, (int)200 * SCALE_RATIO);
	if (!img_enemy03)
		game_abort("failed to load image: enemy03.png");
	img_enemy03_a = load_bitmap_resized("enemy03_a.png", (int)200 * SCALE_RATIO, (int)200 * SCALE_RATIO);
	if (!img_enemy03_a)
		game_abort("failed to load image: enemy03_a.png");
	// [HACKATHON 2-5]
	// TODO: Initialize bullets.
	// 1) Search for a bullet image online and put it in your project.
	//    You can use the image we provided.
	// 2) Load it in by 'al_load_bitmap' or 'load_bitmap_resized'.
	// 3) If you use 'al_load_bitmap', don't forget to check its return value.
	// Uncomment and fill in the code below.
	img_bullet = load_bitmap_resized("image12.png", 17*SCALE_RATIO, 17*SCALE_RATIO);
	if (!img_bullet)
		game_abort("failed to load image: image12.png");

	img_red_bullet = load_bitmap_resized("red bullet.png", 16 * SCALE_RATIO, 16 * SCALE_RATIO);
	if (!img_red_bullet)
		game_abort("failed to load image: red bullet.png");

	img_green_bullet = load_bitmap_resized("green bullet.png", 16 * SCALE_RATIO, 16 * SCALE_RATIO);
	if (!img_green_bullet)
		game_abort("failed to load image: green bullet.png");

	img_blue_bullet = load_bitmap_resized("blue bullet.png", 16 * SCALE_RATIO, 16 * SCALE_RATIO);
	if (!img_blue_bullet)
		game_abort("failed to load image: blue bullet.png");

	img_red_s_bullet = load_bitmap_resized("red s bullet.png", 16 * SCALE_RATIO, 16 * SCALE_RATIO);
	if (!img_red_s_bullet)
		game_abort("failed to load image: red s bullet.png");

	img_green_s_bullet = load_bitmap_resized("green s bullet.png", 16 * SCALE_RATIO, 16 * SCALE_RATIO);
	if (!img_green_s_bullet)
		game_abort("failed to load image: green s bullet.png");

	img_blue_s_bullet = load_bitmap_resized("blue s bullet.png", 16 * SCALE_RATIO, 16 * SCALE_RATIO);
	if (!img_blue_s_bullet)
		game_abort("failed to load image: blue s bullet.png");
	
	img_white_bullet = load_bitmap_resized("white bullet.png", 16 * SCALE_RATIO, 16 * SCALE_RATIO);
	if (!img_white_bullet)
		game_abort("failed to load image: white bullet.png");

	// load "rectangle" bitmaps
	img_red_rectangle = load_bitmap_resized("red rectangle.png", 1280 * SCALE_RATIO, 900 * SCALE_RATIO);
	if (!img_red_rectangle)
		game_abort("failed to load image: red rectangle.png");

	img_blue_rectangle = load_bitmap_resized("blue rectangle.png", 1280 * SCALE_RATIO, 900 * SCALE_RATIO);
	if (!img_blue_rectangle)
		game_abort("failed to load image: blue rectangle.png");

	img_green_rectangle = load_bitmap_resized("green rectangle.png", 1280 * SCALE_RATIO, 900 * SCALE_RATIO);
	if (!img_green_rectangle)
		game_abort("failed to load image: green rectangle.png");

	// load tutorial bitmaps
	img_arrow_key = load_bitmap_resized("arrow_key.png", 280 * SCALE_RATIO, 194 * SCALE_RATIO);
	if (!img_arrow_key)
		game_abort("failed to load image: arrow_key.png");

	img_z_key = load_bitmap_resized("z_key.png", 100 * SCALE_RATIO, 100 * SCALE_RATIO);
	if (!img_z_key)
		game_abort("failed to load image: z_key.png");
	img_z_key_p = load_bitmap_resized("z_key_p.png", 100 * SCALE_RATIO, 100 * SCALE_RATIO);
	if (!img_z_key_p)
		game_abort("failed to load image: z_key_p.png");
	img_x_key = load_bitmap_resized("x_key.png", 100 * SCALE_RATIO, 100 * SCALE_RATIO);
	if (!img_x_key)
		game_abort("failed to load image: x_key.png");
	img_x_key_p = load_bitmap_resized("x_key_p.png", 100 * SCALE_RATIO, 100 * SCALE_RATIO);
	if (!img_x_key_p)
		game_abort("failed to load image: x_key_p.png");
	img_c_key = load_bitmap_resized("c_key.png", 100 * SCALE_RATIO, 100 * SCALE_RATIO);
	if (!img_c_key)
		game_abort("failed to load image: c_key.png");
	img_c_key_p = load_bitmap_resized("c_key_p.png", 100 * SCALE_RATIO, 100 * SCALE_RATIO);
	if (!img_c_key_p)
		game_abort("failed to load image: c_key_p.png");


	// Change to first scene.
	game_change_scene(SCENE_INTRO);
	intro_count = al_get_time();
}

void game_start_event_loop(void) {
	
	ALLEGRO_EVENT event;
	int redraws = 0;
	while (!done) {
		al_wait_for_event(game_event_queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			// Event for clicking the window close button.
			game_log("Window close button clicked");
			done = true;
		}
		else if (event.type == ALLEGRO_EVENT_TIMER) {
			// Event for redrawing the display.
			if (event.timer.source == game_update_timer)
				// The redraw timer has ticked.
				redraws++;
			else if (event.timer.source == beat_timer)
			{
				if (beat_timer_flag)
				{
					al_set_timer_speed(beat_timer, bpm_t - bpm_error);
					beat_timer_flag = false;
				}
				last_beat_timer_tick = al_get_time();
				int i;
				count_beat++;
				if (count_beat == 4)
				{
					count_bar++;
					count_beat = 0;
				}
				for (i = 0; ; i++)
				{
					if (r_line[i].hidden)
						break;
				}
				// spawn rhythm line
				if (i < MAX_LINES && cur_map[count_bar + 3][count_beat] != '-')
				{
					r_line[i].hidden = false;
					// assign color
					if (cur_map[count_bar + 3][count_beat] == '0')
					{
						r_line[i].type = 0;
						r_line[i].color = red;
					}
					else if (cur_map[count_bar + 3][count_beat] == '1')
					{
						r_line[i].type = 1;
						r_line[i].color = green;
					}
					else if (cur_map[count_bar + 3][count_beat] == '2')
					{
						r_line[i].type = 2;
						r_line[i].color = blue;
					}
					// assign radius
					r_line[i].r = 640 * SCALE_RATIO;
				}
			}
		}
		else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
			// Event for keyboard key down.
			game_log("Key with keycode %d down", event.keyboard.keycode);
			key_state[event.keyboard.keycode] = true;
			on_key_down(event.keyboard.keycode);
		}
		else if (event.type == ALLEGRO_EVENT_KEY_UP) {
			// Event for keyboard key up.
			game_log("Key with keycode %d up", event.keyboard.keycode);
			key_state[event.keyboard.keycode] = false;
		}
		else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			// Event for mouse key down.
			game_log("Mouse button %d down at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
			mouse_state[event.mouse.button] = true;
			on_mouse_down(event.mouse.button, event.mouse.x, event.mouse.y);
		}
		else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
			// Event for mouse key up.
			game_log("Mouse button %d up at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
			mouse_state[event.mouse.button] = false;
		}
		else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
			if (event.mouse.dx != 0 || event.mouse.dy != 0) {
				// Event for mouse move.
				/* This log has HUGE IMPACT on performance */
				//game_log("Mouse move to ( %d, %d)", event.mouse.x, event.mouse.y);
				mouse_x = event.mouse.x;
				mouse_y = event.mouse.y;
			}
			else if (event.mouse.dz != 0) {
				// Event for mouse scroll.
				game_log("Mouse scroll at (%d, %d) with delta %d", event.mouse.x, event.mouse.y, event.mouse.dz);
			}
		}
		// TODO: Process more events and call callbacks by adding more
		// entries inside Scene.

		// Redraw
		if (redraws > 0 && al_is_event_queue_empty(game_event_queue)) {
			 if (redraws > 1)
			 	game_log("%d frame(s) dropped", redraws - 1);
			// Update and draw the next frame.
			game_update();
			game_draw();
			redraws = 0;
		}
	}
}

void game_update(void) {
	// Game update for SCENE_INTRO
	if (active_scene == SCENE_INTRO)
	{
		if (al_get_time() - intro_count > INTRO_TIME)
		{
			intro_count = al_get_time();
			game_change_scene(SCENE_INTRO2);
		}
	}
	// Game update for SCENE_INTRO2
	if (active_scene == SCENE_INTRO2)
	{
		if (al_get_time() - intro_count > INTRO_TIME)
		{
			game_change_scene(SCENE_MENU);
		}
	}
	// Game Update for SCENE_MENU
	if (active_scene == SCENE_MENU)
	{
		for (int i = 0; i < menu.num; i++)
		{
			if (menu.pos == i)
				menu.item[i].highlight = true;
			else
				menu.item[i].highlight = false;
		}
	}

	// Game update for SCENE_PAUSE
	if (active_scene == SCENE_PAUSE)
	{
		for (int i = 0; i < pause.num; i++)
		{
			if (pause.pos == i)
				pause.item[i].highlight = true;
			else
				pause.item[i].highlight = false;
		}
	}

	// Game Update for SCENE_SETTINGS
	if (active_scene == SCENE_SETTINGS)
	{
		for (int i = 0; i < settings.num; i++)
		{
			if (settings.pos == i)
				settings.item[i].highlight = true;
			else
				settings.item[i].highlight = false;
		}
	}

	// Game Update for SCENE_S_SELECT
	if (active_scene == SCENE_S_SELECT)
	{
		for (int i = 0; i < s_select.num; i++)
		{
			if (s_select.pos == i)
				s_select.item[i].highlight = true;
			else
				s_select.item[i].highlight = false;
		}
		if (score_a <= 241)
			score_a += 14;
		else
			score_a = 255;
		//change song detect
		if (last_s_pos != s_select.pos)
		{
			al_stop_sample(&songs.id[last_s_pos].pre_sample_id);
			if (!al_play_sample(songs.id[s_select.pos].pre_sample, music_lvl*0.1, 0, 1,
				ALLEGRO_PLAYMODE_LOOP, &songs.id[s_select.pos].pre_sample_id))
			{
				game_log("failed to play sample %d", s_select.pos);
			}
			game_log("song changed from %d to %d", last_s_pos, s_select.pos);
			last_s_pos = s_select.pos;
		}
	}
	// Game Update for SCENE_RESULTS
	if (active_scene == SCENE_RESULTS)
	{
		// score animations
		if (cur_final_score < cur_final_score_buf)
			cur_final_score += 27;
		else if (cur_final_score >= cur_final_score_buf)
		{
			cur_final_score = cur_final_score_buf;
			al_stop_sample(&ui_click_id);
		}
			
		// bar animations
		if (cur_final_score * pow(songs.id[s_select.pos].full_score, -1) < 0.5)
		{
			circle_g = 25 + cur_final_score * pow(songs.id[s_select.pos].full_score, -1) * 215 * 2;
			circle_r = 235;
		}
		else
		{
			circle_r = 235 - (cur_final_score * pow(songs.id[s_select.pos].full_score, -1)-0.5) * 215 * 2;
			circle_g = 235;
		}
	}
	/* ========================================================================================================================== */
	// Game Update for　SCENE_GAME
	int i, j;
	if (active_scene == SCENE_GAME) {
		cur_time = al_get_time();
		/* Plane movement */
		plane.vx = plane.vy = 0;
		
		if (key_state[ALLEGRO_KEY_UP] || key_state[ALLEGRO_KEY_W])
			plane.vy -= 1*SCALE_RATIO;
		if (key_state[ALLEGRO_KEY_DOWN] || key_state[ALLEGRO_KEY_S])
			plane.vy += 1*SCALE_RATIO;
		if (key_state[ALLEGRO_KEY_LEFT] || key_state[ALLEGRO_KEY_A])
			plane.vx -= 1*SCALE_RATIO;
		if (key_state[ALLEGRO_KEY_RIGHT] || key_state[ALLEGRO_KEY_D])
			plane.vx += 1*SCALE_RATIO;
	
		// 0.71 is (1/sqrt(2)).
		plane.y += plane.vy * 8 * (plane.vx ? 0.72f : 1);
		plane.x += plane.vx * 8 * (plane.vy ? 0.72f : 1);

		// [HACKATHON 1-1]
		// TODO: Limit the plane's collision box inside the frame.
		//       (x, y axes can be separated.)
		// Uncomment and fill in the code below.
		/* Update Plane */
		if (plane.x > SCREEN_W - plane.w/2)
			plane.x = SCREEN_W - plane.w / 2;
		else if (plane.x < SCREEN_W*MARGIN_RATIO + plane.w/2)
			plane.x = SCREEN_W * MARGIN_RATIO + plane.w / 2;
		if (plane.y < plane.h/2)
			plane.y = plane.h / 2;
		else if (plane.y > SCREEN_H - plane.h / 2)
			plane.y = SCREEN_H - plane.h / 2;
		//enemy01 collision
		for (i = 0; i < MAX_ENEMY01; i++)
		{
			if (enemy01[i].hidden)
				continue;
			if (pnt_in_object(plane.x, plane.y, enemy01[i], 45*SCALE_RATIO))
			{
				if (cur_time - plane.last_coli > coli_cooldown)
				{
					enemy01[i].last_coli = cur_time;
					object_hit(enemy01, i, 2, cur_time);
					object_hit(&plane, 0, 6, cur_time);
					game_log("plane hit enemy01");
					plane.last_coli = cur_time;
				}
			}
		}
		//enemy02 collision
		for (i = 0; i < MAX_ENEMY02; i++)
		{
			if (enemy02[i].hidden)
				continue;
			if (pnt_in_object_circle(plane.x, plane.y, enemy02[i], 90*SCALE_RATIO))
			{
				if (cur_time - plane.last_coli > coli_cooldown)
				{
					enemy02[i].last_coli = cur_time;
					object_hit(enemy02, i, 2, cur_time);
					object_hit(&plane, 0, 6, cur_time);
					game_log("plane hit enemy02");
					plane.last_coli = cur_time;
				}
			}
		}
		// enemy 03 collision
		for (i = 0; i < MAX_ENEMY03; i++)
		{
			if (enemy03[i].hidden)
				continue;
			if (pnt_in_object_circle(plane.x, plane.y, enemy03[i], 120 * SCALE_RATIO))
			{
				if (cur_time - plane.last_coli > coli_cooldown)
				{
					enemy03[i].last_coli = cur_time;
					object_hit(enemy03, i, 2, cur_time);
					object_hit(&plane, 0, 10, cur_time);
					game_log("plane hit enemy03");
					plane.last_coli = cur_time;
				}
			}
		}
		// [HACKATHON 2-7]
		// TODO: Update bullet coordinates.
		// 1) For each bullets, if it's not hidden, update x, y
		// according to vx, vy.
		// 2) If the bullet is out of the screen, hide it.
		// Uncomment and fill in the code below.
		/* Update Bullets */
		for (i=0; i<MAX_BULLET; i++) 
		{
			if (bullets[i].hidden)
				continue;
			bullets[i].x += bullets[i].vx;
			bullets[i].y += bullets[i].vy;
			if (bullets[i].y < -(bullets[i].h/2))
				bullets[i].hidden = true;
			//enemy01 collision
			for (j = 0; j < MAX_ENEMY01; j++)
			{
				if (enemy01[j].hidden)
					continue;
				if (pnt_in_object(bullets[i].x, bullets[i].y, enemy01[j], 5))
				{
					bullets[i].hidden = true;
					object_hit(enemy01, j, 1, cur_time);
				}
			}
			//enemy02 collision
			for (j = 0; j < MAX_ENEMY02; j++)
			{
				if (enemy02[j].hidden)
					continue;
				if (pnt_in_object(bullets[i].x, bullets[i].y, enemy02[j], 6))
				{
					bullets[i].hidden = true;
					object_hit(enemy02, j, 1, cur_time);
				}
			}
			//enemy03 collision
			for (j = 0; j < MAX_ENEMY03; j++)
			{
				if (enemy03[j].hidden)
					continue;
				if (pnt_in_object(bullets[i].x, bullets[i].y, enemy03[j], 6))
				{
					bullets[i].hidden = true;
					object_hit(enemy03, j, 1, cur_time);
				}
			}
		}

		/* Update enemy01 bullets */
		for (i = 0; i < MAX_E01_BULLET; i++)
		{
			if (e01_bullets[i].hidden)
				continue;
			e01_bullets[i].y += e01_bullets[i].vy;
			if (e01_bullets[i].y > SCREEN_H + e01_bullets[i].h / 2)
				e01_bullets[i].hidden = true;
			// plane collision
			if (pnt_in_object(e01_bullets[i].x, e01_bullets[i].y, plane, 11*SCALE_RATIO))
			{
				e01_bullets[i].hidden = true;
				object_hit(&plane, 0, 4, cur_time);
			}
		}

		/* Update enemy02 bullets */
		for (i = 0; i < MAX_E02_BULLET; i++)
		{
			if (e02_bullets[i].hidden)
				continue;
			// corner collision
			e02_bullets[i].y += e02_bullets[i].vy;
			e02_bullets[i].x += e02_bullets[i].vx;
			if (e02_bullets[i].y > SCREEN_H + e02_bullets[i].h / 2)
				e02_bullets[i].hidden = true;
			else if (e02_bullets[i].y < e02_bullets[i].h / 2)
				e02_bullets[i].hidden = true;
			if (e02_bullets[i].x > SCREEN_W - e02_bullets[i].w / 2)
				e02_bullets[i].hidden = true;
			else if(e02_bullets[i].x < SCREEN_W*MARGIN_RATIO + e02_bullets[i].w / 2)
				e02_bullets[i].hidden = true;;
			// plane collision
			if (pnt_in_object(e02_bullets[i].x, e02_bullets[i].y, plane, 11*SCALE_RATIO))
			{
				e02_bullets[i].hidden = true;
				object_hit(&plane, 0, 4, cur_time);
			}
		}
		/* Update enemy03 lazers */
		for (i = 0; i < MAX_E03_LAZER; i++)
		{
			if (e03_lazers[i].hidden)
				continue;
			// count shoot time
			if (cur_time - e03_lazers[i].last_t >= bpm_t * 16 - 0.0079)
			{
				e03_lazers[i].source->b_type = 255;
				e03_lazers[i].hidden = true;
			}
			if(e03_lazers[i].source->hidden)
				e03_lazers[i].hidden = true;
			// update position
			e03_lazers[i].x = e03_lazers[i].source->x;
			e03_lazers[i].y = e03_lazers[i].source->y;
			// plane collision ouo
			// x
			if (plane.x < (e03_lazers[i].x + e03_lazers[i].width/2 + plane.h/2) && plane.x >(e03_lazers[i].x - e03_lazers[i].width/2 - plane.h/2))
			{
				if (cur_time - plane.last_coli > coli_cooldown)
				{
					object_hit(&plane, 0, e03_lazers[i].damage, cur_time);
					plane.last_coli = cur_time;
				}
			}
			// y
			else if (plane.y < (e03_lazers[i].y + e03_lazers[i].width/2 + plane.w/2) && plane.y >(e03_lazers[i].y - e03_lazers[i].width/2 - plane.w/2))
			{
				if (cur_time - plane.last_coli > coli_cooldown)
				{
					object_hit(&plane, 0, e03_lazers[i].damage, cur_time);
					plane.last_coli = cur_time;
				}
			}
			// 45 degree one
			int d = cal_pnt_to_line(e03_lazers[i].x, e03_lazers[i].y, e03_lazers[i].x + 2500, e03_lazers[i].y + 2500, plane.x, plane.y);
			if (d < e03_lazers[i].width/2 + 18 * SCALE_RATIO)
			{
				if (cur_time - plane.last_coli > coli_cooldown)
				{
					object_hit(&plane, 0, e03_lazers[i].damage, cur_time);
					plane.last_coli = cur_time;
				}
			}
			// -45 degree one
			d = cal_pnt_to_line(e03_lazers[i].x, e03_lazers[i].y, e03_lazers[i].x - 2500, e03_lazers[i].y + 2500, plane.x, plane.y);
			if (d < e03_lazers[i].width/2 + 18 * SCALE_RATIO)
			{
				if (cur_time - plane.last_coli > coli_cooldown)
				{
					object_hit(&plane, 0, e03_lazers[i].damage, cur_time);
					plane.last_coli = cur_time;
				}
			}
		}

		/* Update white bullets*/
		for (i = 0; i < MAX_WHITE_BULLET; i++)
		{
			if (white_bullets[i].hidden)
				continue;
			if(white_bullets[i].hp >= 5)
			{ 
				white_bullets[i].hp -= 5;
			}
			else
			{
				white_bullets[i].hp = 0;
				white_bullets[i].hidden = true;
			}
			
		}

		/* update rhythm lines */
		for (i = 0; i < MAX_LINES; i++)
		{
			if (r_line[i].hidden)
				continue;
			r_line[i].r -= line_speed;
			if (r_line[i].r <= 0)
			{
				r_line[i].hidden = true;
				missed_a = 255;
				combo_count = 0;
				red_rec_a = 255;
				cur_missed++;
				game_log("missed");
				energy -= 5;
			}
		}
		/* update enemy01 */
		if (count_bar >= e01_last_bar + 2)
		{
			e01_last_bar = count_bar;
			for (i = 0; i < MAX_ENEMY01; i++)
			{
				if (enemy01[i].vy > 0)
					enemy01[i].vy = 0;
				else
					enemy01[i].vy = 2*SCALE_RATIO;
			}
		}
		for (i = 0; i < MAX_ENEMY01; i++)
		{
			if (enemy01[i].hidden)
				continue;
			enemy01[i].y += enemy01[i].vy;
			enemy01[i].x += enemy01[i].vx;
			//screen boardar collision
			if (enemy01[i].x < SCREEN_W * MARGIN_RATIO + enemy01[i].w / 2)
			{
				enemy01[i].x = SCREEN_W * MARGIN_RATIO + enemy01[i].w / 2;
				enemy01[i].vx = 2*SCALE_RATIO;
			}
			else if (enemy01[i].x > SCREEN_W - enemy01[i].w / 2)
			{
				enemy01[i].x = SCREEN_W - enemy01[i].w / 2;
				enemy01[i].vx = -2*SCALE_RATIO;
			}
			if (enemy01[i].y > SCREEN_H + enemy01[i].h / 2)
			{
				enemy01[i].hidden = true;
			}
			// hp
			if (enemy01[i].hp <= 0)
			{
				enemy01[i].hidden = true;
				cur_hit_score_buf += ENEMY01_SCORE;
			}
		}

		/* Update enemy02 */
		for (i = 0; i < MAX_ENEMY02; i++)
		{
			if (enemy02[i].hidden)
				continue;
			enemy02[i].y += enemy02[i].vy;
			//screen boardar collision
			if (enemy02[i].y > SCREEN_H + enemy02[i].h / 2)
			{
				enemy02[i].hidden = true;
			}
			// hp
			if (enemy02[i].hp <= 0)
			{
				enemy02[i].hidden = true;
				cur_hit_score_buf += ENEMY02_SCORE;
			}
			// calculate angle
			enemy02[i].angle = atan2(plane.x - enemy02[i].x, plane.y - enemy02[i].y);
		}

		/* Update enemy03 */
		for (i = 0; i < MAX_ENEMY03; i++)
		{
			if (enemy03[i].hidden)
				continue;
			enemy03[i].y += enemy03[i].vy;
			enemy03[i].x += enemy03[i].vx;
			//screen boardar collision
			if (enemy03[i].x < SCREEN_W * MARGIN_RATIO + enemy03[i].w / 2)
			{
				enemy03[i].vx = 2 * SCALE_RATIO;
			}
			else if (enemy03[i].x > SCREEN_W - enemy03[i].w / 2)
			{
				enemy03[i].vx = -2 * SCALE_RATIO;
			}
			if (enemy03[i].y > SCREEN_H - enemy03[i].h / 2)
			{
				enemy03[i].vy = -1 * SCALE_RATIO;
			}
			else if (enemy03[i].y < 0 + enemy03[i].h / 2)
			{
				enemy03[i].vy = 1 * SCALE_RATIO;
			}
			// hp
			if (enemy03[i].hp <= 0)
			{
				enemy03[i].hidden = true;
				cur_hit_score_buf += ENEMY03_SCORE;
			}
			// animation
			if (enemy03[i].b_type > 0)
				enemy03[i].b_type -= 255 * pow((bpm_t * 16 * FPS), -1);

		}


		// [HACKATHON 2-8]
		// TODO: Shoot if key is down and cool-down is over.
		// 1) Get the time now using 'al_get_time'.
		// 2) If Space key is down in 'key_state' and the time
		//    between now and last shoot is not less that cool
		//    down time.
		// 3) Loop through the bullet array and find one that is hidden.
		//    (This part can be optimized.)
		// 4) The bullet will be found if your array is large enough.
		// 5) Set the last shoot time to now.
		// 6) Set hidden to false (recycle the bullet) and set its x, y to the
		//    front part of your plane.
		// Uncomment and fill in the code below.

		/* Shoot bullets */
		double now = al_get_time();
		//bullets for plane
		// level 0 : normal
		if (bullet_level == 0)
		{
			if (now - last_shoot_timestamp >= bpm_t * 2 - 0.0079)
			{
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 0;
				}

			}
		}
		// level 1 : 2x amount
		else if (bullet_level == 1)
		{
			if (now - last_shoot_timestamp >= bpm_t * 2 - 0.0079)
			{
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x - 10*SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 0;
				}
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x + 10 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 0;
				}
			}
		}
		// level 2 : speed up (1.5x shoot speed of lvl 1)
		else if (bullet_level == 2)
		{
			if (now - last_shoot_timestamp >= bpm_t * 1.33 - 0.0079)
			{
				// 01
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x - 10 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 0;
				}
				// 02
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x + 10 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 0;
				}
			}
		}
		// level 3 : spread+ (2 straight bullet + spread)
		else if (bullet_level == 3)
		{
			if (now - last_shoot_timestamp >= bpm_t * 2 - 0.0079)
			{
				// 01
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x + 10 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 0;
				}
				// 02
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x + 20 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 8 * SCALE_RATIO;
				}
				// 03
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x - 20 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = -8 * SCALE_RATIO;
				}
				// 04
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x - 10 *SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 0;
				}
			}
		}
		// level 4 : spread++ (1 straight bullet + spread and 1.5x speed)
		else if (bullet_level == 4)
		{
			if (now - last_shoot_timestamp >= bpm_t * 1.33 - 0.0079)
			{
				// 01
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 0;
				}
				// 02
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x + 20 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 8 * SCALE_RATIO;
				}
				// 03
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x - 20 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = -8 * SCALE_RATIO;
				}
			}
		}
		// level 5 : spread+++ XD (1.5x shoot speed of lvl 3)
		else if (bullet_level == 5)
		{
			if (now - last_shoot_timestamp >= bpm_t * 1.33 - 0.0079)
			{
				// 01
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x + 10 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 0;
				}
				// 02
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x + 20 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 8 * SCALE_RATIO;
				}
				// 03
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x - 20 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = -8 * SCALE_RATIO;
				}
				// 04
				for (i = 0; ; i++) {
					if (bullets[i].hidden)
						break;
				}
				if (i < MAX_BULLET) {
					last_shoot_timestamp = now;
					bullets[i].hidden = false;
					bullets[i].x = plane.x - 10 * SCALE_RATIO;
					bullets[i].y = plane.y - plane.h / 2;
					bullets[i].vx = 0;
				}
			}
		}


		/* Shoot enemy01 bullets */
		if (now - last_shoot_timestamp_e01 >= bpm_t * 4 - 0.0079)
		{
			//bullets for enemy01
			for (j = 0; j < MAX_ENEMY01; j++)
			{
				if (enemy01[j].hidden)
					continue;
				for (i = 0; ; i++)
				{
					if (e01_bullets[i].hidden)
						break;
				}
				if (i < MAX_E01_BULLET)
				{
					last_shoot_timestamp_e01 = now;
					e01_bullets[i].hidden = false;
					e01_bullets[i].x = enemy01[j].x;
					e01_bullets[i].y = enemy01[j].y;
					if (enemy01[j].b_type == 0)
					{
						e01_bullets[i].b_type = 0;
						e01_bullets[i].img = img_red_bullet;
					}
					else if (enemy01[j].b_type == 1)
					{
						e01_bullets[i].b_type = 1;
						e01_bullets[i].img = img_green_bullet;
					}
					else
					{
						e01_bullets[i].b_type = 2;
						e01_bullets[i].img = img_blue_bullet;
					}
				}
			}
		
		}

		/* Shoot enemy02 bullets */
		if (now - last_shoot_timestamp_e02 >= bpm_t * 8 - 0.0079)
		{
			//bullets for enemy02
			for (j = 0; j < MAX_ENEMY02; j++)
			{
				if (enemy02[j].hidden)
					continue;
				for (i = 0; ; i++)
				{
					if (e02_bullets[i].hidden)
						break;
				}
				if (i < MAX_E02_BULLET)
				{
					last_shoot_timestamp_e02 = now;
					e02_bullets[i].hidden = false;
					e02_bullets[i].x = enemy02[j].x;
					e02_bullets[i].y = enemy02[j].y;
					// calaulate the angle
					enemy02[j].angle = atan2(plane.x - enemy02[j].x, plane.y - enemy02[j].y);
					e02_bullets[i].vx = 4 * sin(enemy02[j].angle)*SCALE_RATIO;
					e02_bullets[i].vy = 4 * cos(enemy02[j].angle)*SCALE_RATIO;
					if (enemy01[j].b_type == 0)
					{
						e02_bullets[i].b_type = 0;
						e02_bullets[i].img = img_red_bullet;
					}
					else if (enemy01[j].b_type == 1)
					{
						e02_bullets[i].b_type = 1;
						e02_bullets[i].img = img_green_bullet;
					}
					else
					{
						e02_bullets[i].b_type = 2;
						e02_bullets[i].img = img_blue_bullet;
					}
				}
			}
		}

		/* Shoot enemy03 lazers */
		if (now - last_shoot_timestamp_e03 >= bpm_t * 32 - 0.0079)
		{
			//lazerss for enemy03
			for (j = 0; j < MAX_ENEMY03; j++)
			{
				if (enemy03[j].hidden)
					continue;
				for (i = 0; ; i++)
				{
					if (e03_lazers[i].hidden)
						break;
				}
				if (i < MAX_E03_LAZER)
				{
					last_shoot_timestamp_e03 = now;
					e03_lazers[i].hidden = false;
					e03_lazers[i].x = enemy03[j].x;
					e03_lazers[i].y = enemy03[j].y;
					e03_lazers[i].last_t = now;
					e03_lazers[i].source = &enemy03[j];
				}
				
			}
		}

		/* time the rhythm line */

		beat_cur_time = al_get_time();
		if (!m_offset)
		{
			beat_cur_time += ((double)music_offset)*0.001;
			m_offset = true;
		}
		/*double bpm_error = 0;
		if (s_select.pos == 0)
			bpm_error = 0.0075;
		else if (s_select.pos == 1)
			bpm_error = 0.0033;
		else if (s_select.pos == 2)
			bpm_error = 0.004446;*/
		/*if (beat_cur_time - last_beat_time >= bpm_t - bpm_error)
		{
			count_beat++;
			last_beat_time = beat_cur_time;
			if (count_beat == 4)
			{
				count_bar++;
				count_beat = 0;
			}
			for (i = 0; ; i++)
			{
				if (r_line[i].hidden)
					break;
			}
			// spawn rhythm line
			if (i < MAX_LINES && cur_map[count_bar+3][count_beat] != '-')
			{
				r_line[i].hidden = false;
				// assign color
				if (cur_map[count_bar + 3][count_beat] == '0')
				{
					r_line[i].type = 0;
					r_line[i].color = red;
				}
				else if (cur_map[count_bar + 3][count_beat] == '1')
				{
					r_line[i].type = 1;
					r_line[i].color = green;
				}
				else if (cur_map[count_bar + 3][count_beat] == '2')
				{
					r_line[i].type = 2;
					r_line[i].color = blue;
				}
				// assign radius
				r_line[i].r = 640 * SCALE_RATIO;
			}
		}*/
		/* "bar" dependent things */
		
		if (last_bar_1 < count_bar)
		{
			last_bar_1 = count_bar;
			// spawn enemy01
			if (cur_map[count_bar][5] == '1')
			{
				int amount = cur_map[count_bar][6] - '0';
				for (i = 0; (i < MAX_ENEMY01) && (amount > 0); i++)
				{
					if (!enemy01[i].hidden)
						continue;
					else
					{
						// random x for enemy01
						int r;
						int flag = 1;
						while (flag)
						{
							flag = 0;
							r = (rand() % MAX_ENEMY01 + 1) * (SCREEN_W *(1 - MARGIN_RATIO) / (MAX_ENEMY01 + 1)) + SCREEN_W * MARGIN_RATIO;
							for (j = 0; j < i; j++)
							{
								if (r == enemy01[j].x)
								{
									flag = 1;
									break;
								}
							}
							if (j == i)
							{
								enemy01[i].x = r;
							}
						}
						//random vx
						enemy01[i].vx = rand() % 2 ? 2 * SCALE_RATIO : -2 * SCALE_RATIO;
						//initial vy
						enemy01[i].vy = 0;
						//fixed y
						enemy01[i].y = 0 - enemy01[i].h / 2;
						enemy01[i].hidden = false;
						enemy01[i].hp = ENEMY01_HP;
						amount--;
					}
				}
			}

			// spawn enemy02
			if (cur_map[count_bar][5] == '2')
			{
				int amount = cur_map[count_bar][6] - '0';
				for (i = 0; (i < MAX_ENEMY02) && (amount > 0); i++)
				{
					if (!enemy02[i].hidden)
						continue;
					else
					{
						// random x for enemy02
						int r;
						int flag = 1;
						while (flag)
						{
							flag = 0;
							r = (rand() % 5 + 1) * (SCREEN_W *(1 - MARGIN_RATIO) / (5 + 1)) + SCREEN_W * MARGIN_RATIO;
							for (j = 0; j < i; j++)
							{
								if (r == enemy02[j].x && enemy02[j].y < 0)
								{
									flag = 1;
									break;
								}
							}
							if (j == i)
							{
								enemy02[i].x = r;
							}
						}
						//fixed y
						enemy02[i].y = 0 - enemy02[i].h / 2;
						enemy02[i].hidden = false;
						enemy02[i].hp = ENEMY02_HP;
						amount--;
					}
				}
			}
			// spawn enemy03
			if (cur_map[count_bar][5] == '3')
			{
				int amount = cur_map[count_bar][6] - '0';
				for (i = 0; (i < MAX_ENEMY03) && (amount > 0); i++)
				{
					if (!enemy03[i].hidden)
						continue;
					else
					{
						// random x for enemy03
						int r;
						int flag = 1;
						while (flag)
						{
							flag = 0;
							r = (rand() % 2 + 1) * (SCREEN_W *(1 - MARGIN_RATIO) / (2 + 1)) + SCREEN_W * MARGIN_RATIO;
							for (j = 0; j < i; j++)
							{
								if (r == enemy03[j].x)
								{
									flag = 1;
									break;
								}
							}
							if (j == i)
							{
								enemy03[i].x = r;
							}
						}
						//fixed y
						enemy03[i].y = 0 - enemy03[i].h / 2;
						//assingn vx according to spawn position
						enemy03[i].vx = enemy03[i].x > (SCREEN_W/2) ? 2 * SCALE_RATIO : -2 * SCALE_RATIO;
						enemy03[i].hidden = false;
						enemy03[i].hp = ENEMY03_HP;
						enemy03[i].b_type = 255;
						last_shoot_timestamp_e03 = cur_time;
						amount--;
					}
				}
			}
		}

		/* red tint counter */
		// plane
		if (plane.tint)
		{
			if (cur_time - plane.tint_t > red_tint_time)
			{
				plane.tint = false;
			}
		}
		// enemy01
		for (i = 0; i < MAX_ENEMY01; i++)
		{
			if (enemy01[i].hidden)
				continue;
			if (enemy01[i].tint)
			{
				if (cur_time - enemy01[i].tint_t > red_tint_time)
				{
					enemy01[i].tint = false;
				}
			}
		}
		// enemy02
		for (i = 0; i < MAX_ENEMY02; i++)
		{
			if (enemy02[i].hidden)
				continue;
			if (enemy02[i].tint)
			{
				if (cur_time - enemy02[i].tint_t > red_tint_time)
				{
					enemy02[i].tint = false;
				}
			}
		}
		// enemy03
		for (i = 0; i < MAX_ENEMY03; i++)
		{
			if (enemy03[i].hidden)
				continue;
			if (enemy03[i].tint)
			{
				if (cur_time - enemy03[i].tint_t > red_tint_time)
				{
					enemy03[i].tint = false;
				}
			}
		}

		/* Update Max combo */
		if (combo_count > cur_max_combo)
			cur_max_combo = combo_count;
		/* Update the "bars"*/
		// song progress bar
		song_prog_bar_pos += song_prog_bar_v;
		// hp bar
		if (hp_bar_buffer > plane.hp * 4)
		{
			hp_bar_buffer -= hp_bar_v;
		}
		// energy bar
		if (energy_bar_buffer > energy*4)
		{
			energy_bar_buffer -= energy_bar_v;
		}
		else if (energy_bar_buffer < energy*4)
		{
			energy_bar_buffer += energy_bar_v;
		}
		/* Update energy */
		if (energy < 0)
			energy = 0;
		if (energy > 100)
		{
			energy = 100;
		}
		if (energy_bar_buffer == 400)
		{
			//upgrade bullets
			if (bullet_level < 5)
			{
				bullet_level++;
				energy_a = 255; 
				energy = 0;
				energy_bar_buffer = 0;
			}
		}
		if (bullet_level == 5)
		{
			energy_a = 255;
			energy = 100;
			energy_bar_buffer = 400;
		}
		if (energy_a >= 3 && bullet_level < 5)
			energy_a -= 3;
		else if(bullet_level < 5)
			energy_a = 0;

		/* Update score animations */
		// hit_sore in game
		if (cur_hit_score_buf > cur_hit_score)
			cur_hit_score += 7;

		/* Update "rectangle" animations */
		if (red_rec_a >= 10)
			red_rec_a -= 10;
		else
			red_rec_a = 0;

		if (blue_rec_a >= 10)
			blue_rec_a -= 10;
		else
			blue_rec_a = 0;

		if (green_rec_a >= 10)
			green_rec_a -= 10;
		else
			green_rec_a = 0;
		/* Update Turotial animation */
		if (s_select.pos == 0)
		{
			tutor_text_delta += tutor_text_v;
		}

		/* Update background animationv */
		if (background_a < 2*SCREEN_H)
			background_a += 1;
		else
			background_a = 0;
		/* Song End Conditions */
		// clear
		if (count_bar >= songs.id[s_select.pos].length)
		{
			game_log("Song End with status CLEAR");
			end_status = true;
			game_change_scene(SCENE_RESULTS);
		}
		// failed
		else if (plane.hp <= 0)
		{
			game_log("Song End with status FAILED ");
			end_status = false;
			game_change_scene(SCENE_RESULTS);
		}
	}
}

void game_draw(void) {
	if (active_scene == SCENE_INTRO)
	{
		al_draw_bitmap(allegro_logo, SCREEN_W / 2 - 200 * SCALE_RATIO, SCREEN_H*0.7, 0);
		al_draw_text(font_pirulen_30, white, SCREEN_W / 2 - 200 * SCALE_RATIO, SCREEN_H*0.6, 
			ALLEGRO_ALIGN_LEFT, "Made With");
	}
	else if (active_scene == SCENE_INTRO2)
	{
		al_draw_bitmap(headphones_img, SCREEN_W / 2 - 128 * SCALE_RATIO, SCREEN_H*0.5 - 128 * SCALE_RATIO, 0);
		al_draw_text(font_pirulen_40, white, SCREEN_W / 2, SCREEN_H*0.7, 
			ALLEGRO_ALIGN_CENTER, "Use Headphones For the Best Experience");
	}
	else if (active_scene == SCENE_MENU) {
		al_clear_to_color(black);
		al_draw_bitmap(main_img_background, 0, 0, 0);
		al_draw_text(font_pirulen_72, white, SCREEN_W / 2, 40*SCALE_RATIO, ALLEGRO_ALIGN_CENTER, "Beat' em");
		al_draw_textf(font_pirulen_40, menu.item[0].highlight ? white : gray_middle, SCREEN_W / 2, SCREEN_H - 400 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "%s", menu.item[0].name);
		al_draw_textf(font_pirulen_40, menu.item[1].highlight ? white : gray_middle, SCREEN_W / 2, SCREEN_H - 300 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "%s", menu.item[1].name);
		al_draw_textf(font_pirulen_40, menu.item[2].highlight ? white : gray_middle, SCREEN_W / 2, SCREEN_H - 200 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "%s", menu.item[2].name);
		// [HACKATHON 3-5]
		// TODO: Draw settings images.
		// The settings icon should be located at (x, y, w, h) = (SCREEN_W - 48, 10, 38, 38).
		// Change its image according to your mouse position.
		if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W - (int)48 * SCALE_RATIO, (int)10 * SCALE_RATIO, (int)38 * SCALE_RATIO, (int)38 * SCALE_RATIO))
			al_draw_bitmap(img_settings2, SCREEN_W - (int)48 * SCALE_RATIO, (int)10 * SCALE_RATIO, (int)0 * SCALE_RATIO);
		else
			al_draw_bitmap(img_settings, SCREEN_W - (int)48 * SCALE_RATIO, (int)10 * SCALE_RATIO, (int)0 * SCALE_RATIO);
	}

	/* Game draw for SCENE_GAME =====================================================================================================*/
	else if (active_scene == SCENE_GAME) {
		int i;
		al_clear_to_color(black);
		//draw background
		al_draw_bitmap(start_img_background, 0, background_a, 0);
		al_draw_bitmap(start_img_background, 0, background_a - 2*SCREEN_H, 0);
		//draw the margin line
		al_draw_line(SCREEN_W * MARGIN_RATIO - 3 * SCALE_RATIO, 0, SCREEN_W * MARGIN_RATIO - 3 * SCALE_RATIO, SCREEN_H, gray_middle, 6 * SCALE_RATIO);
		//draw the "rectangles"
		if (red_rec_a > 0)
			al_draw_tinted_bitmap(img_red_rectangle, al_map_rgba(red_rec_a, red_rec_a, red_rec_a, red_rec_a), 320 * SCALE_RATIO, 0, 0);
		if(green_rec_a > 0)
			al_draw_tinted_bitmap(img_green_rectangle, al_map_rgba(green_rec_a, green_rec_a, green_rec_a, green_rec_a), 320 * SCALE_RATIO, 0, 0);
		if(blue_rec_a > 0)
			al_draw_tinted_bitmap(img_blue_rectangle, al_map_rgba(blue_rec_a, blue_rec_a, blue_rec_a, blue_rec_a), 320 * SCALE_RATIO, 0, 0);
		//draw tutorial texts
		if (s_select.pos == 0)
		{
			//draw arrow keys
			if (tutor_text_delta < 1200 * SCALE_RATIO)
			{
				al_draw_bitmap(img_arrow_key, SCREEN_W - 380 * SCALE_RATIO, -220 * SCALE_RATIO + tutor_text_delta * SCALE_RATIO, 0);
				al_draw_text(font_calibri_35, white, SCREEN_W - 240 * SCALE_RATIO, -270 * SCALE_RATIO + tutor_text_delta * SCALE_RATIO,
					ALLEGRO_ALIGN_CENTER, "Use arrow keys to move");
			}
			// z x c keys
			if (tutor_text_delta < 1500*SCALE_RATIO)
			{
				al_draw_text(font_calibri_35, white, 700 * SCALE_RATIO, -615 * SCALE_RATIO + tutor_text_delta * SCALE_RATIO,
					ALLEGRO_ALIGN_CENTER, "Press the according button when the circles align");
				al_draw_text(font_calibri_35, white, 700 * SCALE_RATIO, -575 * SCALE_RATIO + tutor_text_delta * SCALE_RATIO,
					ALLEGRO_ALIGN_CENTER, "Press perfectly to destory enemy bullets");
				// Z
				if (tutor_text_delta > 710 && tutor_text_delta < 735)
					al_draw_bitmap(img_z_key_p, 550 * SCALE_RATIO, -525 * SCALE_RATIO + tutor_text_delta * SCALE_RATIO, 0);
				else
					al_draw_bitmap(img_z_key, 550 * SCALE_RATIO, -525 * SCALE_RATIO + tutor_text_delta * SCALE_RATIO, 0);
				// X
				if (tutor_text_delta > 855 && tutor_text_delta < 880)
					al_draw_bitmap(img_x_key_p, 650 * SCALE_RATIO, -525 * SCALE_RATIO + tutor_text_delta * SCALE_RATIO, 0);
				else
					al_draw_bitmap(img_x_key, 650 * SCALE_RATIO, -525 * SCALE_RATIO + tutor_text_delta * SCALE_RATIO, 0);
				// C
				if (tutor_text_delta > 1140 && tutor_text_delta < 1165)
					al_draw_bitmap(img_c_key_p, 750 * SCALE_RATIO, -525 * SCALE_RATIO + tutor_text_delta * SCALE_RATIO, 0);
				else
					al_draw_bitmap(img_c_key, 750 * SCALE_RATIO, -525 * SCALE_RATIO + tutor_text_delta * SCALE_RATIO, 0);
			}
			// Good Luck !
			if (tutor_text_delta < 2000 * SCALE_RATIO)
			{
				al_draw_text(font_pirulen_30, red, SCREEN_W*(1 + MARGIN_RATIO) / 2, -800 * SCALE_RATIO + tutor_text_delta * SCALE_RATIO,
					ALLEGRO_ALIGN_CENTER, "Good Luck !");
			}
			// DEBUG Draw "tutor_text_delta"
			//al_draw_textf(font_calibri_35, white, SCREEN_W - 600, 500, 0, "%d", tutor_text_delta);
		}
		
		//draw the judging line
		al_draw_circle(plane.x, plane.y, 50 * SCALE_RATIO, gray_middle, 5*SCALE_RATIO);
		// draw the rhythm line
		// we use the method like drawing the bullets
		for (i = 0; i < MAX_LINES; i++)
		{
			if (!r_line[i].hidden)
			{
				al_draw_circle(plane.x, plane.y, r_line[i].r, r_line[i].color, 2*SCALE_RATIO);
			}
		}
		// DEBUG draw judging regeion
		/*al_draw_circle(plane.x, plane.y, (50+perfect_r), green, 5);
		al_draw_circle(plane.x, plane.y, (50-perfect_r), green, 5);
		al_draw_circle(plane.x, plane.y, (50+good_r), red, 5);
		al_draw_circle(plane.x, plane.y, (50-good_r), red, 5);
		al_draw_circle(plane.x, plane.y, ignore_r, white, 5);*/

		// draw "Level UP" text
		if (energy_a > 0 && bullet_level < 5)
		{
			al_draw_text(font_pirulen_40, al_map_rgba(255, 221, 0, energy_a),
				SCREEN_W*MARGIN_RATIO + SCREEN_W * (1 - MARGIN_RATIO) / 2, 50*SCALE_RATIO,
				ALLEGRO_ALIGN_CENTER, "Level UP !");
		}

		// [HACKATHON 2-9]
		// TODO: Draw all bullets in your bullet array.
		// Uncomment and fill in the code below.
		for (i=0; i<MAX_BULLET; i++)
			draw_movable_object(bullets[i]);
		// draw plane
		draw_movable_object(plane);
		// draw enemy01 bullets
		for (i = 0; i < MAX_E01_BULLET; i++)
			draw_movable_object(e01_bullets[i]);
		// draw enemy02 bullets
		for (i = 0; i < MAX_E02_BULLET; i++)
			draw_movable_object(e02_bullets[i]);
		// draw white bullets
		for (i = 0; i < MAX_WHITE_BULLET; i++)
		{
			if (white_bullets[i].hidden)
				continue;
			al_draw_tinted_bitmap(white_bullets[i].img, al_map_rgb(white_bullets[i].hp, white_bullets[i].hp, white_bullets[i].hp),
				round(white_bullets[i].x - white_bullets[i].w / 2), round(white_bullets[i].y - white_bullets[i].h / 2), 0);
		}
		// draw enemy03 lazers
		for (i = 0; i < MAX_E03_LAZER; i++)
		{
			if (e03_lazers[i].hidden)
				continue;
			// x
			al_draw_line(SCREEN_W*MARGIN_RATIO, e03_lazers[i].y, SCREEN_W, e03_lazers[i].y, al_map_rgb(255, 0, 0), e03_lazers[i].width);
			al_draw_line(SCREEN_W*MARGIN_RATIO, e03_lazers[i].y, SCREEN_W, e03_lazers[i].y, al_map_rgb(255, 142, 142), e03_lazers[i].width - 6 * SCALE_RATIO);
			al_draw_line(SCREEN_W*MARGIN_RATIO, e03_lazers[i].y, SCREEN_W, e03_lazers[i].y, white, e03_lazers[i].width - 12 * SCALE_RATIO);
			// y
			al_draw_line(e03_lazers[i].x, 0, e03_lazers[i].x, SCREEN_H, al_map_rgb(255, 0, 0), e03_lazers[i].width);
			al_draw_line(e03_lazers[i].x, 0, e03_lazers[i].x, SCREEN_H, al_map_rgb(255, 142, 142), e03_lazers[i].width - 6 * SCALE_RATIO);
			al_draw_line(e03_lazers[i].x, 0, e03_lazers[i].x, SCREEN_H, white, e03_lazers[i].width - 12 * SCALE_RATIO);
			// 45 degree
			al_draw_line(e03_lazers[i].x - 2500, e03_lazers[i].y - 2500, e03_lazers[i].x + 2500, e03_lazers[i].y + 2500, al_map_rgb(255, 0, 0), e03_lazers[i].width);
			al_draw_line(e03_lazers[i].x - 2500, e03_lazers[i].y - 2500, e03_lazers[i].x + 2500, e03_lazers[i].y + 2500, al_map_rgb(255, 142, 142), e03_lazers[i].width - 6 * SCALE_RATIO);
			al_draw_line(e03_lazers[i].x - 2500, e03_lazers[i].y - 2500, e03_lazers[i].x + 2500, e03_lazers[i].y + 2500, white, e03_lazers[i].width - 12 * SCALE_RATIO);
			// -45 degree
			al_draw_line(e03_lazers[i].x - 2500, e03_lazers[i].y + 2500, e03_lazers[i].x + 2500, e03_lazers[i].y - 2500, al_map_rgb(255, 0, 0), e03_lazers[i].width);
			al_draw_line(e03_lazers[i].x - 2500, e03_lazers[i].y + 2500, e03_lazers[i].x + 2500, e03_lazers[i].y - 2500, al_map_rgb(255, 142, 142), e03_lazers[i].width - 6 * SCALE_RATIO);
			al_draw_line(e03_lazers[i].x - 2500, e03_lazers[i].y + 2500, e03_lazers[i].x + 2500, e03_lazers[i].y - 2500, white, e03_lazers[i].width - 12 * SCALE_RATIO);
		}
		// draw enemy01
		for (i = 0; i < MAX_ENEMY01; i++)
			draw_movable_object(enemy01[i]);
		// draw enemy02
		for (i = 0; i < MAX_ENEMY02; i++)
			draw_movable_object_rotate(enemy02[i]);
		// draw enemy03
		for (i = 0; i < MAX_ENEMY03; i++)
		{
			draw_movable_object(enemy03[i]);
			// draw enemy03 animation (!! here we use "b_type" to store data for animation !!)
			if (enemy03[i].hidden)
				continue;
			al_draw_tinted_bitmap(img_enemy03_a, al_map_rgb(255, enemy03[i].b_type, enemy03[i].b_type), round(enemy03[i].x - enemy03[i].w / 2), round(enemy03[i].y - enemy03[i].h / 2), 0);
		}



		/* draw side UI */
		// draw the side pannel
		al_draw_filled_rectangle(0, 0, SCREEN_W * MARGIN_RATIO - 5 * SCALE_RATIO, SCREEN_H, black);

		// draw hp bar
		al_draw_filled_rectangle(SCREEN_W*MARGIN_RATIO - 60 * SCALE_RATIO, SCREEN_H - 720 * SCALE_RATIO,
			SCREEN_W*MARGIN_RATIO - 100 * SCALE_RATIO, SCREEN_H - (320 + (int)hp_bar_buffer)* SCALE_RATIO, red);
		al_draw_filled_rectangle(SCREEN_W*MARGIN_RATIO - 60 * SCALE_RATIO, SCREEN_H - 320 * SCALE_RATIO,
			SCREEN_W*MARGIN_RATIO - 100 * SCALE_RATIO, SCREEN_H - (320 + (int)hp_bar_buffer)* SCALE_RATIO, green);
		al_draw_rectangle(SCREEN_W*MARGIN_RATIO - 60 * SCALE_RATIO, SCREEN_H - 720 * SCALE_RATIO,
			SCREEN_W*MARGIN_RATIO - 100 * SCALE_RATIO, SCREEN_H - 320 * SCALE_RATIO, white, 3);
		al_draw_text(font_calibri_35, white, SCREEN_W*MARGIN_RATIO - 80 * SCALE_RATIO, 625 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "HP");
		// draw energy bar
		al_draw_filled_rectangle(SCREEN_W*MARGIN_RATIO - 210 * SCALE_RATIO, SCREEN_H - 720 * SCALE_RATIO,
			SCREEN_W*MARGIN_RATIO - 250 * SCALE_RATIO, SCREEN_H - (320 + (int)energy_bar_buffer)* SCALE_RATIO, al_map_rgb(6, 61, 116));
		al_draw_filled_rectangle(SCREEN_W*MARGIN_RATIO - 210 * SCALE_RATIO, SCREEN_H - 320 * SCALE_RATIO,
			SCREEN_W*MARGIN_RATIO - 250 * SCALE_RATIO, SCREEN_H - (320 + (int)energy_bar_buffer)* SCALE_RATIO, blue);
		al_draw_rectangle(SCREEN_W*MARGIN_RATIO - 210 * SCALE_RATIO, SCREEN_H - 720 * SCALE_RATIO,
			SCREEN_W*MARGIN_RATIO - 250 * SCALE_RATIO, SCREEN_H - 320 * SCALE_RATIO, white, 3);
		if (energy_a > 0)
			al_draw_filled_rectangle(SCREEN_W*MARGIN_RATIO - 210 * SCALE_RATIO, SCREEN_H - 720 * SCALE_RATIO,
				SCREEN_W*MARGIN_RATIO - 250 * SCALE_RATIO, SCREEN_H - 320 * SCALE_RATIO, al_map_rgba(255, 221, 0, energy_a));
		if (bullet_level == 5)
		{
			al_draw_text(font_pirulen_30, al_map_rgb(255, 221, 0), SCREEN_W * MARGIN_RATIO *0.1, 130 * SCALE_RATIO,
				ALLEGRO_ALIGN_LEFT, "Level FULL !");
		}
		al_draw_text(font_calibri_35, white, SCREEN_W*MARGIN_RATIO - 230 * SCALE_RATIO, 625 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "Energy");
		//draw music-related stats
		al_draw_textf(font_pirulen_30, white, SCREEN_W * MARGIN_RATIO *0.1, 65 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "BAR %d", count_bar);
		al_draw_textf(font_pirulen_30, white, SCREEN_W * MARGIN_RATIO *0.1, 95 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "Beat %d", count_beat);
		al_draw_textf(font_pirulen_30, white, SCREEN_W * MARGIN_RATIO *0.1, SCREEN_H - 160 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%d Combo", combo_count);
		al_draw_textf(font_pirulen_30, white, SCREEN_W * MARGIN_RATIO *0.1, SCREEN_H - 210 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%.2f %%", cur_percent);
		//draw score
		al_draw_textf(font_pirulen_40, white, SCREEN_W * MARGIN_RATIO *0.1, 15 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%07d", cur_hit_score);
		//draw the song progress bar
		al_draw_filled_rectangle(SCREEN_W * MARGIN_RATIO - 10 * SCALE_RATIO, SCREEN_H,
			SCREEN_W * MARGIN_RATIO, SCREEN_H - song_prog_bar_pos, al_map_rgb(255, 204, 0));
		if (perfect_a >= 0)
		{
			al_draw_text(font_pirulen_40, al_map_rgba(0, 230, 115, perfect_a), SCREEN_W * MARGIN_RATIO *0.1, SCREEN_H - 80 * SCALE_RATIO,
				ALLEGRO_ALIGN_LEFT, "Perfect");
			perfect_a -= judge_text_v;
		}
		if (good_a >= 0)
		{
			al_draw_text(font_pirulen_40, al_map_rgba(102, 194, 255, good_a), SCREEN_W * MARGIN_RATIO *0.1, SCREEN_H - 80 * SCALE_RATIO,
				ALLEGRO_ALIGN_LEFT, "Good");
			good_a -= judge_text_v;
		}
		if (missed_a >= 0)
		{
			al_draw_text(font_pirulen_40, al_map_rgba(255, 77, 77, missed_a), SCREEN_W * MARGIN_RATIO *0.1, SCREEN_H - 80 * SCALE_RATIO,
				ALLEGRO_ALIGN_LEFT, "Missed");
			missed_a -= judge_text_v;
		}
		
	}
	/* =========================================================================================================================== */

	// [HACKATHON 3-9]
	// TODO: If active_scene is SCENE_SETTINGS.
	// Draw anything you want, or simply clear the display.
	else if (active_scene == SCENE_SETTINGS) 
	{
		al_clear_to_color(black);
		al_draw_bitmap(main_img_background, 0, 0, 0);
		al_draw_text(font_pirulen_72, white, SCREEN_W / 2, 30 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "SETTINGS" );
		al_draw_textf(font_pirulen_40, settings.item[0].highlight ? white : gray_middle, SCREEN_W*0.26, 200 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%s : < %d x %d >", settings.item[0].name, ResSets[Respos][0], ResSets[Respos][1]);
		al_draw_textf(font_pirulen_40, settings.item[1].highlight ? white : gray_middle, SCREEN_W*0.26, 300 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%s : < %d >", settings.item[1].name, music_lvl);
		al_draw_textf(font_pirulen_40, settings.item[2].highlight ? white : gray_middle, SCREEN_W*0.26, 400 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%s : < %d >", settings.item[2].name, sfx_lvl);
		al_draw_textf(font_pirulen_40, settings.item[3].highlight ? white : gray_middle, SCREEN_W*0.26, 500 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%s : < %d > ms", settings.item[3].name, music_offset);
		if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W - (int)48 * SCALE_RATIO, (int)10 * SCALE_RATIO, (int)38 * SCALE_RATIO, (int)38 * SCALE_RATIO))
			al_draw_bitmap(img_settings2, SCREEN_W - (int)48 * SCALE_RATIO, (int)10 * SCALE_RATIO, 0);
		else
			al_draw_bitmap(img_settings, SCREEN_W - (int)48 * SCALE_RATIO, (int)10 * SCALE_RATIO, 0);
		if (res_changed == 1)
			al_draw_text(font_pirulen_40, red, SCREEN_W / 2, SCREEN_H*0.9,
				ALLEGRO_ALIGN_CENTER, "Please Restart The Game to Apply New Settings");
	}
	// We draw things for SCENE_PAUSE
	else if (active_scene == SCENE_PAUSE)
	{
		al_clear_to_color(black);
		al_draw_text(font_pirulen_72, white, SCREEN_W / 2, (int)30 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "PAUSED");
		al_draw_textf(font_pirulen_40, pause.item[0].highlight ? white : gray_middle, SCREEN_W / 2, 200 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "%s", pause.item[0].name);
		al_draw_textf(font_pirulen_40, pause.item[1].highlight ? white : gray_middle, SCREEN_W / 2, 300 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "%s", pause.item[1].name);
		al_draw_textf(font_pirulen_40, pause.item[2].highlight ? white : gray_middle, SCREEN_W / 2, 400 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "%s", pause.item[2].name);
		al_draw_textf(font_pirulen_40, pause.item[3].highlight ? white : gray_middle, SCREEN_W / 2, 500 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "%s", pause.item[3].name);
	}
	// Draw things for SCENE_S_SELECT
	else if (active_scene == SCENE_S_SELECT)
	{
		al_clear_to_color(black);
		al_draw_bitmap(main_img_background, 0, 0, 0);
		al_draw_text(font_pirulen_72, white, SCREEN_W / 2, (int)30 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "Song Select");
		// draw songs
		al_draw_textf(font_pirulen_40, s_select.item[0].highlight ? white : gray_middle, 500 * SCALE_RATIO, 200 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%s", s_select.item[0].name);
		al_draw_textf(font_freescript_40, s_select.item[0].highlight ? white : gray_middle, 500 * SCALE_RATIO, 250 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "  by %s", songs.id[0].author);

		al_draw_textf(font_pirulen_40, s_select.item[1].highlight ? white : gray_middle, 500 * SCALE_RATIO, 320 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%s", s_select.item[1].name);
		al_draw_textf(font_freescript_40, s_select.item[1].highlight ? white : gray_middle, 500 * SCALE_RATIO, 370 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "  by %s", songs.id[1].author);

		al_draw_textf(font_pirulen_40, s_select.item[2].highlight ? white : gray_middle, 500 * SCALE_RATIO, 440 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%s", s_select.item[2].name);
		al_draw_textf(font_freescript_40, s_select.item[2].highlight ? white : gray_middle, 500 * SCALE_RATIO, 490 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "  by %s", songs.id[2].author);

		// draw scores
		al_draw_textf(font_pirulen_72, al_map_rgba(score_a, score_a, score_a, score_a), 250 * SCALE_RATIO, 210 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "%c", songs.id[s_select.pos].mark);
		al_draw_textf(font_pirulen_40, al_map_rgba(score_a, score_a, score_a, score_a), 250 * SCALE_RATIO, 400 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "%d", songs.id[s_select.pos].final_score);
		al_draw_textf(font_pirulen_30, al_map_rgba(score_a, score_a, score_a, score_a), 250 * SCALE_RATIO, 470 * SCALE_RATIO,
			ALLEGRO_ALIGN_CENTER, "%.2f %%", songs.id[s_select.pos].percent);
		if (songs.id[s_select.pos].cleared && score_a == 255)
			al_draw_text(font_pirulen_30, al_map_rgba(0, 230, 115, score_a), 250 * SCALE_RATIO, 340 * SCALE_RATIO, ALLEGRO_ALIGN_CENTER, "Cleared");
		//al_draw_filled_rectangle(30 * SCALE_RATIO, 200 * SCALE_RATIO, 470 * SCALE_RATIO, 500 * SCALE_RATIO, al_map_rgba(0, 0, 0, score_a));
		
	}
	// Draw things for SCENE_RESULTS
	else if (active_scene == SCENE_RESULTS)
	{
		al_clear_to_color(black);
		al_draw_bitmap(main_img_background, 0, 0, 0);
		al_draw_text(font_pirulen_72, white, SCREEN_W / 2, (int)30 * SCALE_RATIO,
			ALLEGRO_ALIGN_RIGHT, "Song ");
		if (end_status)
		{
			al_draw_text(font_pirulen_72, green, SCREEN_W / 2, (int)30 * SCALE_RATIO,
				ALLEGRO_ALIGN_LEFT, " CLEARED");
		}
		else
		{
			al_draw_text(font_pirulen_72, red, SCREEN_W / 2, (int)30 * SCALE_RATIO,
				ALLEGRO_ALIGN_LEFT, " FAILED");
		}
		// draw song and author name
		al_draw_textf(font_pirulen_40, white, 500 * SCALE_RATIO, 180 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%s", songs.id[s_select.pos].name);
		al_draw_textf(font_pirulen_30, white, 500 * SCALE_RATIO, 250 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%s", songs.id[s_select.pos].author);
		// draw judging stats
		al_draw_text(font_pirulen_40, green, 390 * SCALE_RATIO, 610 * SCALE_RATIO, ALLEGRO_ALIGN_RIGHT, "Perfect");
		al_draw_text(font_pirulen_40, blue, 390 * SCALE_RATIO, 670 * SCALE_RATIO, ALLEGRO_ALIGN_RIGHT, "Good");
		al_draw_text(font_pirulen_40, red, 390 * SCALE_RATIO, 730 * SCALE_RATIO, ALLEGRO_ALIGN_RIGHT, "Missed");
		al_draw_text(font_pirulen_40, al_map_rgb(255, 221, 0), 390 * SCALE_RATIO, 790 * SCALE_RATIO, ALLEGRO_ALIGN_RIGHT, "Max Combo");

		al_draw_textf(font_pirulen_40, white, 420 * SCALE_RATIO, 610 * SCALE_RATIO, ALLEGRO_ALIGN_LEFT, "%d", cur_perfect);
		al_draw_textf(font_pirulen_40, white, 420 * SCALE_RATIO, 670 * SCALE_RATIO, ALLEGRO_ALIGN_LEFT, "%d", cur_good);
		al_draw_textf(font_pirulen_40, white, 420 * SCALE_RATIO, 730 * SCALE_RATIO, ALLEGRO_ALIGN_LEFT, "%d", cur_missed);
		al_draw_textf(font_pirulen_40, white, 420 * SCALE_RATIO, 790 * SCALE_RATIO, ALLEGRO_ALIGN_LEFT, "%d", cur_max_combo);
		// draw scores
		al_draw_textf(font_pirulen_40, white, 500 * SCALE_RATIO, 400 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%d X %.2f %%", cur_hit_score, cur_percent);
		al_draw_textf(font_pirulen_72, white, 500 * SCALE_RATIO, 480 * SCALE_RATIO,
			ALLEGRO_ALIGN_LEFT, "%07d", cur_final_score);
		if (high_score && cur_final_score == cur_final_score_buf)
			al_draw_text(font_pirulen_40, al_map_rgb(255, 221, 0), 1050*SCALE_RATIO, 495*SCALE_RATIO, ALLEGRO_ALIGN_LEFT, "New High Score !");
		// draw circle bar
		al_draw_circle(250 * SCALE_RATIO , 360 * SCALE_RATIO, 170 * SCALE_RATIO, white, 4 * SCALE_RATIO);
		al_draw_circle(250 * SCALE_RATIO, 360 * SCALE_RATIO, 130 * SCALE_RATIO, white, 4 * SCALE_RATIO);
		al_draw_arc(250 * SCALE_RATIO, 360 * SCALE_RATIO, 150 * SCALE_RATIO,
			-ALLEGRO_PI / 2, 2 * ALLEGRO_PI*cur_final_score*pow(songs.id[s_select.pos].full_score, -1), al_map_rgb(circle_r, circle_g, 10), 36 * SCALE_RATIO);
		if (cur_final_score == cur_final_score_buf)
		{
			al_draw_textf(font_pirulen_120, al_map_rgb(circle_r, circle_g, 10), 250 * SCALE_RATIO, 290 * SCALE_RATIO,
				ALLEGRO_ALIGN_CENTRE, "%c", cur_mark);
		}
	}
	if (fade_now){ }
	else
		al_flip_display();
}


void game_destroy(void) {
	// Save things into config.txt
	config = fopen("config.txt", "w");
	if (config == NULL)

		game_abort("failed to open config (for saves).txt");
	fprintf(config, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n", Respos, music_lvl, sfx_lvl, perfect_r, good_r, ignore_r, music_offset);
	fclose(config);

	// Destroy everything you have created.
	// Free the memories allocated by malloc or allegro functions.
	// Destroy shared resources.
	al_destroy_font(font_pirulen_72);
	al_destroy_font(font_pirulen_40);
	al_destroy_font(font_pirulen_120);
	al_destroy_font(font_pirulen_30);
	al_destroy_font(font_calibri_35);
	al_destroy_font(font_freescript_40);

	/* Menu Scene resources*/
	al_destroy_bitmap(main_img_background);
	al_destroy_sample(main_bgm);
	// [HACKATHON 3-6]
	// TODO: Destroy the 2 settings images.
	// Uncomment and fill in the code below.
	al_destroy_bitmap(img_settings);
	al_destroy_bitmap(img_settings2);

	/* Start Scene resources*/
	al_destroy_bitmap(start_img_background);
	al_destroy_bitmap(start_img_plane);
	al_destroy_bitmap(img_enemy01);
	al_destroy_sample(start_bgm);
	al_destroy_bitmap(img_enemy02);
	al_destroy_bitmap(img_enemy03);
	al_destroy_bitmap(img_enemy03_a);
	// [HACKATHON 2-10]
	// TODO: Destroy your bullet image.
	// Uncomment and fill in the code below.
	al_destroy_bitmap(img_bullet);
	al_destroy_bitmap(img_red_bullet);
	al_destroy_bitmap(img_green_bullet);
	al_destroy_bitmap(img_blue_bullet);
	al_destroy_bitmap(img_red_s_bullet);
	al_destroy_bitmap(img_green_s_bullet);
	al_destroy_bitmap(img_blue_s_bullet);
	al_destroy_bitmap(img_white_bullet);

	al_destroy_timer(game_update_timer);
	al_destroy_timer(beat_timer);
	al_destroy_event_queue(game_event_queue);
	al_destroy_display(game_display);
	// destroy ui samples
	al_destroy_sample(ui_click);
	al_destroy_sample(ui_slide);
	al_destroy_sample(ui_enter01);
	al_destroy_sample(ui_enter02);
	al_destroy_sample(ui_back01);
	al_destroy_sample(ui_back02);
	// destroy Results BGM
	al_destroy_sample(clear_song);
	al_destroy_sample(failed_song);
	// destroy Intro recources 
	al_destroy_bitmap(allegro_logo);
	al_destroy_bitmap(headphones_img);
	
	// destroy "rectangle" bitmaps
	al_destroy_bitmap(img_red_rectangle);
	al_destroy_bitmap(img_green_rectangle);
	al_destroy_bitmap(img_blue_rectangle);
	// destroy Bitmaps for tutorial
	al_destroy_bitmap(img_z_key);
	al_destroy_bitmap(img_x_key);
	al_destroy_bitmap(img_c_key);
	al_destroy_bitmap(img_z_key_p);
	al_destroy_bitmap(img_x_key_p);
	al_destroy_bitmap(img_c_key_p);
	al_destroy_bitmap(img_arrow_key);
	// destroy song samples
	for (int i = 0; i < songs.num; i++)
	{
		al_destroy_sample(songs.id[i].sample);
		al_destroy_sample(songs.id[i].pre_sample);
		al_destroy_sample_instance(songs.id[i].sample_ins);
	}


	free(mouse_state);
}

void game_change_scene(int next_scene) {
	if (next_scene == SCENE_PAUSE)
	{
		pause.pos = 0;
		pause_song_pos = al_get_sample_instance_position(songs.id[s_select.pos].sample_ins);
		al_set_sample_instance_playing(songs.id[s_select.pos].sample_ins, false);
		pause_time_delta = cur_time - last_beat_timer_tick;
		al_stop_timer(beat_timer);
	}
	/* fade out started */
	// time sensitive things up there
	scene_fade_out();
	last_scene = active_scene;
	active_scene = next_scene;
	game_log("Change scene from %d to %d", last_scene, next_scene);
	// TODO: Destroy resources initialized when creating scene.
	if (last_scene == SCENE_MENU) {
		al_stop_sample(&main_bgm_id);
		game_log("stop audio (MENU bgm)");
	}
	else if (last_scene == SCENE_GAME) {
		al_stop_sample(&start_bgm_id);
		game_log("stop audio (GAME bgm)");
	}
	
	// TODO: Allocate resources before entering scene.
	if (active_scene == SCENE_MENU) {
		if (!al_play_sample(main_bgm, music_lvl*0.1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &main_bgm_id))
			game_abort("failed to play audio (bgm)");
	}
	if (active_scene == SCENE_GAME && last_scene != SCENE_PAUSE) {
		int i, j;
		// reset cur_map
		for (i = 0; i < MAX_BAR; i++)
		{
			for (j = 0; j < 7; j++)
			{
				cur_map[i][j] = '\0';
			}
		}

		//load song data
		if (s_select.pos == 0)
			songs.id[s_select.pos].map = fopen("map_0.txt", "r");
		else if(s_select.pos == 1)
			songs.id[s_select.pos].map = fopen("map_1.txt", "r");
		else if (s_select.pos == 2)
			songs.id[s_select.pos].map = fopen("map_2.txt", "r");


		if (!songs.id[s_select.pos].map)
			game_abort("failed to load map_%d", s_select.pos);
		for (i = 0; i < MAX_BAR; i++)
		{
			fscanf(songs.id[s_select.pos].map, "%s", cur_map[i]);
		}
		fclose(songs.id[s_select.pos].map);
		bpm_t = pow(songs.id[s_select.pos].bpm, -1) * 60 * 0.25; //  60*1/bpm/4
		line_speed = 590 * SCALE_RATIO * pow(FPS, -1) * pow(bpm_t*12, -1); //    r_max * (  (1/60) / bpm_t*4 )
		bpm_error = 0;
		if (s_select.pos == 0)
			bpm_error = 0;
		else if (s_select.pos == 1)
			bpm_error = 0;
		else if (s_select.pos == 2)
			bpm_error = 0.00002;
		al_set_timer_speed(beat_timer, bpm_t - bpm_error - music_offset*0.001);
		beat_timer_flag = true;
		count_beat = 0;
		count_bar = 0;
		last_bar_1 = 0;
		song_prog_bar_v = SCREEN_H / ((bpm_t * 4)*songs.id[s_select.pos].length) * pow(FPS, -1);
		game_log("song bar v : %f", song_prog_bar_v);
		game_log("BPM_T : %lf", bpm_t);
		song_prog_bar_pos = 0;
		total_beat = 0;
		// count total beat
		for (i = 0; i < MAX_BAR; i++)
		{
			for (j = 0; j < 4; j++)
			{
				if (isdigit(cur_map[i][j]))
					total_beat++;
			}
		}
		game_log("Total beat : %d", total_beat);
		// count full score
		songs.id[s_select.pos].full_score = 0;
		for (i = 0; i < MAX_BAR; i++)
		{
			if (isdigit(cur_map[i][5]) && isdigit(cur_map[i][6]))
			{
				if(cur_map[i][5] == '1')
					songs.id[s_select.pos].full_score += ENEMY01_SCORE * (cur_map[i][6] - '0');
				else if (cur_map[i][5] == '2')
					songs.id[s_select.pos].full_score += ENEMY02_SCORE * (cur_map[i][6] - '0');
				else if (cur_map[i][5] == '3')
					songs.id[s_select.pos].full_score += ENEMY03_SCORE * (cur_map[i][6] - '0');
			}
		}
		game_log("Full score : %d", songs.id[s_select.pos].full_score);

		// Tutorial text
		if (s_select.pos == 0)
		{
			tutor_text_delta = 0;
			tutor_text_v = 1 * SCALE_RATIO;
		}
		// initialize scores
		cur_final_score = 0;
		cur_hit_score = 0;
		cur_hit_score_buf = 0;
		cur_percent = 0;
		cur_mark = 'F';
		cur_max_combo = 0;
		cur_perfect = 0;
		cur_good = 0;
		cur_missed = 0;
		energy = 0;
		energy_a = 0;
		bullet_level = 0;
		if (s_select.pos == 1)
			bullet_level = 1;
		else if (s_select.pos == 2)
			bullet_level = 1;
		high_score = false;
		m_offset = false;

		int background_a = 0;

		// initialize values for the plane
		plane.img = start_img_plane;
		plane.x = SCREEN_W*(1-MARGIN_RATIO)/2 + SCREEN_W*MARGIN_RATIO;
		plane.y = SCREEN_H * 0.9;
		plane.w = al_get_bitmap_width(plane.img);
		plane.h = al_get_bitmap_height(plane.img);
		plane.hp = PLANE_HP;
		hp_bar_buffer = PLANE_HP * 4;
		energy_bar_buffer = 0;
		// initialize enemy01
		e01_last_bar = 0;
		for (i = 0; i < MAX_ENEMY01; i++) 
		{
			enemy01[i].img = img_enemy01;
			enemy01[i].w = al_get_bitmap_width(img_enemy01);
			enemy01[i].h = al_get_bitmap_height(img_enemy01);
			enemy01[i].vy = 2*SCALE_RATIO;
			enemy01[i].vx = (rand() % 2) ? 2*SCALE_RATIO : -2*SCALE_RATIO;
			enemy01[i].hidden = true;
			// random type
			int type = rand() % 3;
			if (type == 0)
				enemy01[i].b_type = 0;
			else if (type == 1)
				enemy01[i].b_type = 1;
			else
				enemy01[i].b_type = 2;
		}
		// initialize enemy02
		for (i = 0; i < MAX_ENEMY02; i++) 
		{
			enemy02[i].img = img_enemy02;
			enemy02[i].w = al_get_bitmap_width(img_enemy02);
			enemy02[i].h = al_get_bitmap_height(img_enemy02);
			enemy02[i].vy = 1 * SCALE_RATIO;
			enemy02[i].vx = 0;
			enemy02[i].hidden = true;
			// random type
			int type = rand() % 3;
			if (type == 0)
				enemy02[i].b_type = 0;
			else if (type == 1)
				enemy02[i].b_type = 1;
			else
				enemy02[i].b_type = 2;
		}
		// initialize enemy03
		for (i = 0; i < MAX_ENEMY03; i++) 
		{
			enemy03[i].img = img_enemy03;
			enemy03[i].w = al_get_bitmap_width(img_enemy03);
			enemy03[i].h = al_get_bitmap_height(img_enemy03);
			enemy03[i].vy = 1 * SCALE_RATIO;
			enemy03[i].vx = 0;
			enemy03[i].hidden = true;
			
		}
		// [HACKATHON 2-6]
		// TODO: Initialize bullets.
		// For each bullets in array, set their w and h to the size of
		// the image, and set their img to bullet image, hidden to true,
		// (vx, vy) to (0, -3).
		// Uncomment and fill in the code below.
		for (i = 0; i < MAX_BULLET; i++)
		{
			bullets[i].w = al_get_bitmap_width(img_bullet);
			bullets[i].h = al_get_bitmap_height(img_bullet);
			bullets[i].img = img_bullet;
			bullets[i].vx = 0;
			bullets[i].vy = -20*SCALE_RATIO;
			bullets[i].hidden = true;
		}
		// Initialize enemy01 bullets
		for (i = 0; i < MAX_E01_BULLET; i++)
		{
			e01_bullets[i].vx = 0;
			e01_bullets[i].vy = 4*SCALE_RATIO;
			e01_bullets[i].w = al_get_bitmap_width(img_red_bullet);
			e01_bullets[i].h = al_get_bitmap_height(img_red_bullet);
			e01_bullets[i].hidden = true;
		}
		// Initialize enemy02 bullets
		for (i = 0; i < MAX_E02_BULLET; i++)
		{
			e02_bullets[i].w = al_get_bitmap_width(img_red_bullet);
			e02_bullets[i].h = al_get_bitmap_height(img_red_bullet);
			e02_bullets[i].hidden = true;
		}
		// Initialize enemy03 lazers
		for (i = 0; i < MAX_E03_LAZER; i++)
		{
			e03_lazers[i].hidden = true;
			e03_lazers[i].damage = 10;
			e03_lazers[i].width = 16*SCALE_RATIO;
		}
		// Initialize white bullets
		for (i = 0; i < MAX_WHITE_BULLET; i++)
		{
			white_bullets[i].w = al_get_bitmap_width(img_white_bullet);
			white_bullets[i].h = al_get_bitmap_height(img_white_bullet);
			white_bullets[i].img = img_white_bullet;
			white_bullets[i].hp = 0;
			white_bullets[i].hidden = true;
		}
		// initialize rhythm lines
		for (i = 0; i < MAX_LINES; i++)
		{
			r_line[i].hidden = true;
			r_line[i].r = 640 * SCALE_RATIO;
		}
		// clear combo_count
		combo_count = 0;

		// play the song for the level
		al_attach_sample_instance_to_mixer(songs.id[s_select.pos].sample_ins, al_get_default_mixer());
		al_set_sample_instance_gain(songs.id[s_select.pos].sample_ins, music_lvl * 0.1);
		al_set_sample_instance_playing(songs.id[s_select.pos].sample_ins, true);
		// start beat timer
		al_start_timer(beat_timer);
	}
	
	if (active_scene == SCENE_S_SELECT)
	{
		al_stop_timer(beat_timer);
		// detach sample instance
		al_detach_sample_instance(songs.id[s_select.pos].sample_ins);
		if (al_play_sample(songs.id[s_select.pos].pre_sample, music_lvl*0.1, 0, 1,
			ALLEGRO_PLAYMODE_LOOP, &songs.id[s_select.pos].pre_sample_id))
		{
			game_log("failed to play song %d", s_select.pos);
		}
	}
	if (last_scene == SCENE_S_SELECT)
	{
		al_stop_sample(&songs.id[s_select.pos].pre_sample_id);
	}
	if (last_scene == SCENE_GAME && active_scene != SCENE_PAUSE)
	{
		al_set_sample_instance_playing(songs.id[s_select.pos].sample_ins, false);
		al_detach_sample_instance(songs.id[s_select.pos].sample_ins);
	}
	if (active_scene == SCENE_RESULTS)
	{
		al_stop_timer(beat_timer);
		// scores and saves
		cur_final_score_buf = cur_hit_score * cur_percent * 0.01;
		cur_final_score = 0;
		if (cur_final_score_buf > songs.id[s_select.pos].full_score * 0.95)
			cur_mark = 'S';
		else if (cur_final_score_buf > songs.id[s_select.pos].full_score * 0.8)
			cur_mark = 'A';
		else if (cur_final_score_buf > songs.id[s_select.pos].full_score * 0.7)
			cur_mark = 'B';
		else if (cur_final_score_buf > songs.id[s_select.pos].full_score * 0.5)
			cur_mark = 'C';
		else if (cur_final_score_buf > songs.id[s_select.pos].full_score * 0.3)
			cur_mark = 'D';
		else
			cur_mark = 'F';

		if (cur_final_score_buf > songs.id[s_select.pos].final_score)
		{
			high_score = true;
			songs.id[s_select.pos].final_score = cur_final_score_buf;
			songs.id[s_select.pos].percent = cur_percent;
			songs.id[s_select.pos].mark = cur_mark;
			songs.id[s_select.pos].cleared = end_status;
			save_song_score();
		}
		// sound effects
		if (end_status)
			al_play_sample(clear_song, music_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_LOOP, &clear_song_id);
		else
			al_play_sample(failed_song, music_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &failed_song_id);

		// score ticking sound effect
		al_play_sample(ui_click, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_LOOP, &ui_click_id);
	}
	/* fade in started */
	// time sensitive things down there
	scene_fade_in();
	if (active_scene == SCENE_GAME && last_scene == SCENE_PAUSE)
	{
		al_set_sample_instance_position(songs.id[s_select.pos].sample_ins, pause_song_pos);
		al_set_sample_instance_playing(songs.id[s_select.pos].sample_ins, true);
		//last_beat_time = al_get_time() - pause_time_delta;
		al_set_timer_speed(beat_timer, pause_time_delta);
		al_start_timer(beat_timer);
		beat_timer_flag = true;
	}
}

void scene_fade_out()
{
	int a = 0;
	fade_now = true;
	while (a <= 255)
	{
		game_draw();
		al_draw_filled_rectangle(0, 0, SCREEN_W, SCREEN_H, al_map_rgba(0, 0, 0, a));
		al_flip_display();
		a += FADE_SPEED;
		al_rest(0.016667);
	}
}

void scene_fade_in()
{
	int a = 255;
	while (a >= 0)
	{
		game_draw();
		al_draw_filled_rectangle(0, 0, SCREEN_W, SCREEN_H, al_map_rgba(0, 0, 0, a));
		al_flip_display();
		a -= FADE_SPEED;
		al_rest(0.016667);
	}
	fade_now = false;
}

void on_key_down(int keycode) {
	//SCENE_INTRO
	if (active_scene == SCENE_INTRO)
	{
		if (keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_ESCAPE)
		{
			intro_count = al_get_time();
			game_change_scene(SCENE_INTRO2);
		}
	}
	//SCENE_INTRO2
	else if (active_scene == SCENE_INTRO2)
	{
		if (keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_ESCAPE)
		{
			game_change_scene(SCENE_MENU);
		}
	}
	//SCENE_MENU
	else if (active_scene == SCENE_MENU) {
		if (keycode == ALLEGRO_KEY_ENTER)
		{
			al_play_sample(ui_enter01, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_enter01_id);
			if (menu.pos == 0)
			{
				game_change_scene(SCENE_S_SELECT);
			}
			else if (menu.pos == 1)
				game_change_scene(SCENE_SETTINGS);
			else if (menu.pos == 2)
				done = true;
		}
		else if (keycode == ALLEGRO_KEY_UP)
		{
			al_play_sample(ui_click, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_click_id);
			menu.pos = (menu.pos + menu.num - 1) % menu.num;
		}
		else if (keycode == ALLEGRO_KEY_DOWN)
		{
			al_play_sample(ui_click, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_click_id);
			menu.pos = (menu.pos + 1) % menu.num;
		}
	}
	//SCENE_GAME
	else if (active_scene == SCENE_GAME)
	{
		int i;
		if (keycode == ALLEGRO_KEY_ESCAPE)
			game_change_scene(SCENE_PAUSE);
		// judgement section
		else if (keycode == ALLEGRO_KEY_Z  || keycode == ALLEGRO_KEY_X || keycode == ALLEGRO_KEY_C)
		{
			//find the nearest circle
			double small = 600 * SCALE_RATIO;
			int flag = 0;
			int small_i;
			for (i = 0; i < MAX_LINES; i++)
			{
				if (r_line[i].hidden)
					continue;
				if (r_line[i].r < small)
				{
					flag = 1;
					small = r_line[i].r;
					small_i = i;
				}
			}
			if (flag)
			{
				if (r_line[small_i].r < ignore_r*SCALE_RATIO)
				{
					if (keycode == ALLEGRO_KEY_Z && r_line[small_i].type == 0)
					{
						rhythm_judge(small_i, r_line[small_i].type);
					}
					else if (keycode == ALLEGRO_KEY_X && r_line[small_i].type == 1)
					{
						rhythm_judge(small_i, r_line[small_i].type);
					}
					else if (keycode == ALLEGRO_KEY_C && r_line[small_i].type == 2)
					{
						rhythm_judge(small_i, r_line[small_i].type);
					}
					else
					{
						missed_a = 255;
						combo_count = 0;
						red_rec_a = 255;
						cur_missed++;
						game_log("missed");
						energy -= 5;
					}
					r_line[small_i].hidden = true;
				}
			}
		}
	}
	//SCENE_PAUSE
	else if(active_scene == SCENE_PAUSE)
	{
		if (keycode == ALLEGRO_KEY_ESCAPE)
			game_change_scene(SCENE_GAME);
		else if (keycode == ALLEGRO_KEY_UP)
		{
			al_play_sample(ui_click, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_click_id);
			pause.pos = (pause.pos + pause.num - 1) % pause.num;
		}
		else if (keycode == ALLEGRO_KEY_DOWN)
		{
			al_play_sample(ui_click, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_click_id);
			pause.pos = (pause.pos + 1) % pause.num;
		}
		else if (keycode == ALLEGRO_KEY_ENTER)
		{
			if (pause.pos == 0)
				game_change_scene(SCENE_GAME);
			else if (pause.pos == 1)
			{
				active_scene = SCENE_S_SELECT;
				game_change_scene(SCENE_GAME);
			}
			else if (pause.pos == 2)
				game_change_scene(SCENE_S_SELECT);
		}
	}
	//SCENE_SETTINGS
	else if (active_scene == SCENE_SETTINGS)
	{
		if (keycode == ALLEGRO_KEY_UP)
		{
			al_play_sample(ui_click, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_click_id);
			settings.pos = (settings.pos + settings.num - 1) % settings.num;
		}
		else if (keycode == ALLEGRO_KEY_DOWN)
		{
			al_play_sample(ui_click, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_click_id);
			settings.pos = (settings.pos + 1) % settings.num;
		}
		else if (keycode == ALLEGRO_KEY_ESCAPE)
		{
			al_play_sample(ui_back01, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_back01_id);
			game_change_scene(SCENE_MENU);
		}
		else if (keycode == ALLEGRO_KEY_RIGHT)
		{
			al_play_sample(ui_slide, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_slide_id);
			if (settings.pos == 0 && Respos != 0)
			{
				res_changed = 1;
				Respos = (Respos + 5) % 6;
			}
			else if (settings.pos == 1 && music_lvl != 10)
			{
				music_lvl = (music_lvl + 1) % 11;
			}
			else if (settings.pos == 2 && sfx_lvl != 10)
			{
				sfx_lvl = (sfx_lvl + 1) % 11;
			}
			else if (settings.pos == 3 && music_offset != 100)
			{
				music_offset = (music_offset + 10) % 101;
			}
		}
		else if (keycode == ALLEGRO_KEY_LEFT)
		{
			al_play_sample(ui_slide, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_slide_id);
			if (settings.pos == 0 && Respos != 5)
			{
				res_changed = 1;
				Respos = (Respos + 1) % 6;
			}
			else if (settings.pos == 1 && music_lvl != 0)
			{
				music_lvl = (music_lvl + 10) % 11;
			}
			else if (settings.pos == 2 && sfx_lvl != 0)
			{
				sfx_lvl = (sfx_lvl + 10) % 11;
			}
			else if (settings.pos == 3 && music_offset != -100)
			{
				music_offset -= 10;
			}
		}

	}
	//SCENE_S_SELECT
	else if (active_scene == SCENE_S_SELECT)
	{
		if (keycode == ALLEGRO_KEY_ENTER)
		{
			al_play_sample(ui_enter02, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_enter02_id);
			if (s_select.pos == 0)
				game_change_scene(SCENE_GAME);
			else if (s_select.pos == 1)
				game_change_scene(SCENE_GAME);
			else if (s_select.pos == 2)
				game_change_scene(SCENE_GAME);
		}
		else if (keycode == ALLEGRO_KEY_UP)
		{
			al_play_sample(ui_click, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_click_id);
			s_select.pos = (s_select.pos + s_select.num - 1) % s_select.num;
			score_a = 0;
		}
		else if (keycode == ALLEGRO_KEY_DOWN)
		{
			al_play_sample(ui_click, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_click_id);
			s_select.pos = (s_select.pos + 1) % s_select.num;
			score_a = 0;
		}
		else if (keycode == ALLEGRO_KEY_ESCAPE)
		{
			al_play_sample(ui_back01, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_back01_id);
			al_stop_sample(&songs.id[s_select.pos].pre_sample_id);
			game_change_scene(SCENE_MENU);
		}
	}
	//SCENE_RESULTS
	else if (active_scene == SCENE_RESULTS)
	{
		if (keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_ESCAPE)
		{
			if (cur_final_score != cur_final_score_buf)
			{
				cur_final_score = cur_final_score_buf;
			}
			else
			{
				al_play_sample(ui_back02, sfx_lvl*0.1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &ui_back02_id);
				if (end_status)
					al_stop_sample(&clear_song_id);
				else
					al_stop_sample(&failed_song_id);
				game_change_scene(SCENE_S_SELECT);
			}
		}
	}
	
}

void on_mouse_down(int btn, int x, int y) {
	// [HACKATHON 3-8]
	// TODO: When settings clicked, switch to settings scene.
	// Uncomment and fill in the code below.
	if (active_scene == SCENE_MENU)
	{
		if (btn == 1) {
			if (pnt_in_rect(x, y, SCREEN_W-48, 10, 38, 38))
				game_change_scene(SCENE_SETTINGS);
		}
	}
	else if (active_scene == SCENE_SETTINGS)
	{
		if (btn == 1) {
			if (pnt_in_rect(x, y, SCREEN_W - 48, 10, 38, 38))
				game_change_scene(SCENE_MENU);
		}
	}
}
// Function to draw MovableObject with center x, y
void draw_movable_object(MovableObject obj) {
	if (obj.hidden)
		return;
	if (obj.tint)
		al_draw_tinted_bitmap(obj.img, al_map_rgb(255, 130, 130), round(obj.x - obj.w / 2), round(obj.y - obj.h / 2), 0);
	else
		al_draw_bitmap(obj.img, round(obj.x - obj.w / 2), round(obj.y - obj.h / 2), 0);
}
// Function to draw ROTATED MovableObject with center x, y
void draw_movable_object_rotate(MovableObject obj)
{
	if (obj.hidden)
		return;
	if (obj.tint)
		al_draw_tinted_rotated_bitmap(obj.img, al_map_rgb(255, 130, 130), round(obj.w/2), round(obj.w/2),
			round(obj.x), round(obj.y), -obj.angle, 0);
	else
		al_draw_rotated_bitmap(obj.img, round(obj.w / 2), round(obj.w / 2), round(obj.x), round(obj.y), -obj.angle, 0);
}
// destroy bullets and draw white bullets on top of them
void draw_white_bullets(int type)
{
	int i, j;
	// derstoy ENEMY01 bullets
	for (i = 0; i < MAX_E01_BULLET; i++)
	{
		if (e01_bullets[i].hidden)
			continue;
		if (e01_bullets[i].b_type != type)
			continue;
		
		if (cal_obj_distance(e01_bullets[i], plane) < BULLET_DESTROY_R*SCALE_RATIO)
		{
			e01_bullets[i].hidden = true;
			// show white bullets
			for (j = 0; ; j++)
			{
				if (white_bullets[j].hidden)
					break;
			}
			if (j < MAX_WHITE_BULLET)
			{
				white_bullets[j].hidden = false;
				white_bullets[j].x = e01_bullets[i].x;
				white_bullets[j].y = e01_bullets[i].y;
				white_bullets[j].hp = 255;
			}
		}
	}
	// destory ENEMY02 bullets
	for (i = 0; i < MAX_E02_BULLET; i++)
	{
		if (e02_bullets[i].hidden)
			continue;
		if (e02_bullets[i].b_type != type)
			continue;

		if (cal_obj_distance(e02_bullets[i], plane) < BULLET_DESTROY_R*SCALE_RATIO)
		{
			e02_bullets[i].hidden = true;
			// show white bullets
			for (j = 0; ; j++)
			{
				if (white_bullets[j].hidden)
					break;
			}
			if (j < MAX_WHITE_BULLET)
			{
				white_bullets[j].hidden = false;
				white_bullets[j].x = e02_bullets[i].x;
				white_bullets[j].y = e02_bullets[i].y;
				white_bullets[j].hp = 255;
			}
		}
	}
}

// Judge player input according to current circle radius
void rhythm_judge(int i, int type)
{
	// Perfect
	if (r_line[i].r < (50+perfect_r) * SCALE_RATIO && r_line[i].r > (50-perfect_r) * SCALE_RATIO)
	{
		perfect_a = 255;
		combo_count++;
		cur_percent += pow(total_beat, -1) * 100;
		cur_perfect++;
		game_log("perfect");
		draw_white_bullets(type);
		green_rec_a = 255;
		energy += 5;
	}
	// Good
	else if (r_line[i].r < (50+good_r) * SCALE_RATIO && r_line[i].r > (50-good_r) * SCALE_RATIO)
	{
		good_a = 255;
		combo_count++;
		cur_percent += pow(total_beat, -1) * 100 * 0.5;
		cur_good++;
		game_log("good");
		blue_rec_a = 255;
		energy += 2;
	}
	// Missed
	else
	{
		missed_a = 255;
		combo_count = 0;
		cur_missed++;
		game_log("missed");
		red_rec_a = 255;
		energy -= 5;
	}
}
//Function to save all song socres
void save_song_score()
{
	save = fopen("save.txt", "w");
	if (save == NULL)
		game_abort("failed to open file: save.txt");
	game_log("Save file is loaded");
	for (int i = 0; i < songs.num; i++)
	{
		if (fprintf(save, "%c\n", songs.id[i].mark) < 0)
			game_abort("failed to write: mark %d", i);
		if (fprintf(save, "%d\n", songs.id[i].cleared) < 0)
			game_abort("failed to write: cleared %d", i);
		if (fprintf(save, "%d\n", songs.id[i].final_score) < 0)
			game_abort("failed to write: score %d", i);
		if (fprintf(save, "%.2f\n", songs.id[i].percent) < 0)
			game_abort("failed to wirte: precent %d", i);
	}
	fclose(save);
	game_log("All progresses are saved");
}

ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h) {
	ALLEGRO_BITMAP* loaded_bmp = al_load_bitmap(filename);
	if (!loaded_bmp)
		game_abort("failed to load image: %s", filename);
	ALLEGRO_BITMAP *resized_bmp = al_create_bitmap(w, h);
	ALLEGRO_BITMAP *prev_target = al_get_target_bitmap();

	if (!resized_bmp)
		game_abort("failed to create bitmap when creating resized image: %s", filename);
	al_set_target_bitmap(resized_bmp);
	al_draw_scaled_bitmap(loaded_bmp, 0, 0,
		al_get_bitmap_width(loaded_bmp),
		al_get_bitmap_height(loaded_bmp),
		0, 0, w, h, 0);
	al_set_target_bitmap(prev_target);
	al_destroy_bitmap(loaded_bmp);

	game_log("resized image: %s", filename);

	return resized_bmp;
}

// [HACKATHON 3-3]
// TODO: Define bool pnt_in_rect(int px, int py, int x, int y, int w, int h)
// Uncomment and fill in the code below.
bool pnt_in_rect(int px, int py, int x, int y, int w, int h) 
{
	if (px >= x && px <= x + w && py >= y && py <= y + h)
		return true;
	else
		return false;
}
// Function to determine if a point is in a MovableObject by using rectangle collision
bool pnt_in_object(int px, int py, MovableObject obj, int offset)
{
	if (px >= obj.x - (obj.w+offset) / 2 && px <= obj.x + (obj.w + offset) / 2 
		&& py >= obj.y - (obj.h + offset) / 2 && py <= obj.y + (obj.h + offset) / 2)
		return true;
	else
		return false;
}
// Function to determine if a point is in a MovableObject by using circle collision
bool pnt_in_object_circle(int px, int py, MovableObject obj, int r)
{
	if (px >= obj.x - r && px <= obj.x + r && py >= obj.y - r && py <= obj.y + r)
		return true;
	else
		return false;
}
// Function to make hp reduction and tint intialization
void object_hit(MovableObject* obj_p, int pos, int hp, double time)
{
	(obj_p+pos)->hp -= hp;
	(obj_p+pos)->tint = true;
	(obj_p+pos)->tint_t = time;
}
// Function to calulate distance of two MovableObjects with int accuracy
int cal_obj_distance(MovableObject a, MovableObject b)
{
	return round(sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2)));
}
// Function to calulate distance of two points with int accuracy
int cal_pnt_distance(int x1, int y1, int x2, int y2)
{
	return round(sqrt(pow (x1 - x2, 2) + pow(y1 - y2, 2)));
}
// calculate distance between a straight line pasing (a1, a2), (b1, b2) and point (x, y) with int accuracy
int cal_pnt_to_line(int a1, int a2, int b1, int b2, int x, int y)
{
	int k = cal_pnt_distance(a1, a2, b1, b2);
	int t = cal_pnt_distance(x, y, b1, b2);
	int u = cal_pnt_distance(x, y, a1, a2);
	double s = (k + t + u)*pow(2, -1);
	return round(sqrt(s*(s - k)*(s - t)*(s - u)) *pow(k, -1) * 2);
}



// +=================================================================+
// | Code below is for debugging purpose, it's fine to remove it.    |
// | Deleting the code below and removing all calls to the functions |
// | doesn't affect the game.                                        |
// +=================================================================+

void game_abort(const char* format, ...) {
	va_list arg;
	va_start(arg, format);
	game_vlog(format, arg);
	va_end(arg);
	fprintf(stderr, "Fatal error occured, exiting after 5 secs");
	// Wait 5 secs before exiting.
	al_rest(5);
	// Force exit program.
	exit(1);
}

void game_log(const char* format, ...) {
#ifdef LOG_ENABLED
	va_list arg;
	va_start(arg, format);
	game_vlog(format, arg);
	va_end(arg);
#endif
}

void game_vlog(const char* format, va_list arg) {
#ifdef LOG_ENABLED
	static bool clear_file = true;
	vprintf(format, arg);
	printf("\n");
	// Write log to file for later debugging.
	FILE* pFile = fopen("log.txt", clear_file ? "w" : "a");
	if (pFile) {
		vfprintf(pFile, format, arg);
		fprintf(pFile, "\n");
		fclose(pFile);
	}
	clear_file = false;
#endif
}
