/*____________________________________________________________________
|
| File: main.cpp
|
| Description: Main module in program.
|
| Functions:  Program_Get_User_Preferences
|             Program_Init
|							 Init_Graphics
|								Set_Mouse_Cursor
|             Program_Run
|							 Init_Render_State
|             Program_Free
|             Program_Immediate_Key_Handler
|
| (C) Copyright 2013 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#define _MAIN_

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>
#include "dp.h"
#include "..\Framework\win_support.h"
#include <rom8x8.h>

#include "main.h"
#include "position.h"
#include <ctime>
#include <stdlib.h>

/*___________________
|
| Type definitions
|__________________*/

typedef struct
{
	unsigned resolution;
	unsigned bitdepth;
} UserPreferences;

/*___________________
|
| Function Prototypes
|__________________*/

static int Init_Graphics(unsigned resolution, unsigned bitdepth, unsigned stencildepth, int *generate_keypress_events);
static void Set_Mouse_Cursor();
static void Init_Render_State();

/*___________________
|
| Constants
|__________________*/

#define MAX_VRAM_PAGES 2
#define GRAPHICS_RESOLUTION      \
	(                            \
		gxRESOLUTION_640x480 |   \
		gxRESOLUTION_800x600 |   \
		gxRESOLUTION_1024x768 |  \
		gxRESOLUTION_1152x864 |  \
		gxRESOLUTION_1280x960 |  \
		gxRESOLUTION_1400x1050 | \
		gxRESOLUTION_1440x1080 | \
		gxRESOLUTION_1600x1200 | \
		gxRESOLUTION_1152x720 |  \
		gxRESOLUTION_1280x800 |  \
		gxRESOLUTION_1440x900 |  \
		gxRESOLUTION_1680x1050 | \
		gxRESOLUTION_1920x1200 | \
		gxRESOLUTION_2048x1280 | \
		gxRESOLUTION_1280x720 |  \
		gxRESOLUTION_1600x900 |  \
		gxRESOLUTION_1920x1080 | \
		gxRESOLUTION_2560x1440)
#define GRAPHICS_STENCILDEPTH 0
#define GRAPHICS_BITDEPTH (gxBITDEPTH_24 | gxBITDEPTH_32)

#define AUTO_TRACKING 1
#define NO_AUTO_TRACKING 0

/*____________________________________________________________________
|
| Function: Program_Get_User_Preferences
|
| Input: Called from CMainFrame::Init
| Output: Allows program to popup dialog boxes, etc. to get any user
|   preferences such as screen resolution.  Returns preferences via a
|   pointer.  Returns true on success, else false to quit the program.
|___________________________________________________________________*/

int Program_Get_User_Preferences(void **preferences)
{

	static UserPreferences user_preferences;

	if (gxGetUserFormat(GRAPHICS_DRIVER, GRAPHICS_RESOLUTION, GRAPHICS_BITDEPTH, &user_preferences.resolution, &user_preferences.bitdepth))
	{
		*preferences = (void *)&user_preferences;
		return (1);
	}
	else
		return (0);
}

/*____________________________________________________________________
|
| Function: Program_Init
|
| Input: Called from CMainFrame::Start_Program_Thread()
| Output: Starts graphics mode.  Returns # of user pages available if
|       successful, else 0.
|___________________________________________________________________*/

int Program_Init(void *preferences, int *generate_keypress_events)
{
	UserPreferences *user_preferences = (UserPreferences *)preferences;
	int initialized = FALSE;

	if (user_preferences)
		initialized = Init_Graphics(user_preferences->resolution, user_preferences->bitdepth, GRAPHICS_STENCILDEPTH, generate_keypress_events);

	return (initialized);
}

/*____________________________________________________________________
|
| Function: Init_Graphics
|
| Input: Called from Program_Init()
| Output: Starts graphics mode.  Returns # of user pages available if
|       successful, else 0.
|___________________________________________________________________*/

static int Init_Graphics(unsigned resolution, unsigned bitdepth, unsigned stencildepth, int *generate_keypress_events)
{
	int num_pages;
	byte *font_data;
	unsigned font_size;

	/*____________________________________________________________________
	|
	| Init globals
	|___________________________________________________________________*/

	Pgm_num_pages = 0;
	Pgm_system_font = NULL;

	/*____________________________________________________________________
	|
	| Start graphics mode and event processing
	|___________________________________________________________________*/

	font_data = font_data_rom8x8;
	font_size = sizeof(font_data_rom8x8);

	// Start graphics mode
	num_pages = gxStartGraphics(resolution, bitdepth, stencildepth, MAX_VRAM_PAGES, GRAPHICS_DRIVER);
	if (num_pages == MAX_VRAM_PAGES)
	{
		// Init system, drawing fonts
		Pgm_system_font = gxLoadFontData(gxFONT_TYPE_GX, font_data, font_size);
		// Make system font the default drawing font
		gxSetFont(Pgm_system_font);

		// Start event processing
		evStartEvents(evTYPE_MOUSE_LEFT_PRESS | evTYPE_MOUSE_RIGHT_PRESS |
			evTYPE_MOUSE_LEFT_RELEASE | evTYPE_MOUSE_RIGHT_RELEASE |
			evTYPE_MOUSE_WHEEL_BACKWARD | evTYPE_MOUSE_WHEEL_FORWARD |
			//                   evTYPE_KEY_PRESS |
			evTYPE_RAW_KEY_PRESS | evTYPE_RAW_KEY_RELEASE,
			AUTO_TRACKING, EVENT_DRIVER);
		*generate_keypress_events = FALSE; // true if using evTYPE_KEY_PRESS in the above mask

		// Set a custom mouse cursor
		Set_Mouse_Cursor();

		// Set globals
		Pgm_num_pages = num_pages;
	}

	return (Pgm_num_pages);
}

/*____________________________________________________________________
|
| Function: Set_Mouse_Cursor
|
| Input: Called from Init_Graphics()
| Output: Sets default mouse cursor.
|___________________________________________________________________*/

static void Set_Mouse_Cursor()
{
	gxColor fc, bc;

	// Set cursor to a medium sized red arrow
	fc.r = 255;
	fc.g = 0;
	fc.b = 0;
	fc.a = 0;
	bc.r = 1;
	bc.g = 1;
	bc.b = 1;
	bc.a = 0;
	msSetCursor(msCURSOR_MEDIUM_ARROW, fc, bc);
}

void DeployObject(gx3dObject *obj, gx3dTexture tex, int x, int y, int z, gx3dColor ambient_color)
{
	gx3dMatrix m;
	gx3d_SetAmbientLight(ambient_color);

	gx3d_GetTranslateMatrix(&m, x, y, z);
	gx3d_SetObjectMatrix(obj, &m);
	gx3d_SetTexture(0, tex);
	gx3d_DrawObject(obj, 0);
}

void DeployObject(gx3dObject *obj, gx3dTexture tex, int x, int y, int z)
{
	gx3dMatrix m;

	gx3d_GetTranslateMatrix(&m, x, y, z);
	gx3d_SetObjectMatrix(obj, &m);
	gx3d_SetTexture(0, tex);
	gx3d_DrawObject(obj, 0);
}

double Distance(double x1, double y1, double x2, double y2) {
	return std::sqrt(std::pow((x2 - x1), 2) + std::pow((y2 - y1), 2));
}

/*____________________________________________________________________
|
| Function: Program_Run
|
| Input: Called from Program_Thread()
| Output: Runs program in the current video mode.  Begins with mouse
|   hidden.
|___________________________________________________________________*/

void Program_Run()
{
	int quit;
	evEvent event;
	gx3dDriverInfo dinfo;
	gxColor color;
	char str[256];

	int gameState = 0; // 0 = Begin State // 1 = Play State // 2 = Help State 3 = End State

	gx3dMatrix m, m1, m2, m3, m4, m5;
	gx3dColor color3d_white = { 1, 1, 1, 0 };
	gx3dColor color3d_dim = { 0.1f, 0.1f, 0.1f };
	gx3dColor color3d_black = { 0, 0, 0, 0 };
	gx3dColor color3d_darkgray = { 0.3f, 0.3f, 0.3f, 0 };
	gx3dColor color3d_gray = { 0.6f, 0.6f, 0.6f, 0 };
	gx3dMaterialData material_default = {
		{1, 1, 1, 1}, // ambient color
		{1, 1, 1, 1}, // diffuse color
		{1, 1, 1, 1}, // specular color
		{0, 0, 0, 0}, // emissive color
		10			  // specular sharpness (0=disabled, 0.01=sharp, 10=diffused)
	};

	// // How to use C++ print outupt
	// string mystr;

	// mystr = "SUPER!";
	// char ss[256];
	// strcpy (ss, mystr.c_str());
	// debug_WriteFile (ss);

	/*____________________________________________________________________
	|
	| Print info about graphics driver to debug file.
	|___________________________________________________________________*/

	gx3d_GetDriverInfo(&dinfo);
	debug_WriteFile("_______________ Device Info ______________");
	sprintf(str, "max texture size: %dx%d", dinfo.max_texture_dx, dinfo.max_texture_dy);
	debug_WriteFile(str);
	sprintf(str, "max active lights: %d", dinfo.max_active_lights);
	debug_WriteFile(str);
	sprintf(str, "max user clip planes: %d", dinfo.max_user_clip_planes);
	debug_WriteFile(str);
	sprintf(str, "max simultaneous texture stages: %d", dinfo.max_simultaneous_texture_stages);
	debug_WriteFile(str);
	sprintf(str, "max texture stages: %d", dinfo.max_texture_stages);
	debug_WriteFile(str);
	sprintf(str, "max texture repeat: %d", dinfo.max_texture_repeat);
	debug_WriteFile(str);
	debug_WriteFile("__________________________________________");

	/*____________________________________________________________________
  |
  | Initialize the sound library
  |___________________________________________________________________*/

	snd_Init(22, 16, 2, 1, 1);
	snd_SetListenerDistanceFactorToFeet(snd_3D_APPLY_NOW);

	Sound s_crickets, s_song, s_footsteps, s_laser, s_twinkle;

	s_footsteps = snd_LoadSound("wav\\grass.wav", snd_CONTROL_VOLUME, 0);
	s_song = snd_LoadSound("wav\\night_music.wav", snd_CONTROL_VOLUME, 0);
	s_crickets = snd_LoadSound("wav\\cricket_chirp.wav", snd_CONTROL_3D, 0);
	s_laser = snd_LoadSound("wav\\laser.wav", snd_CONTROL_VOLUME, 0);
	s_twinkle = snd_LoadSound("wav\\twinkle.wav", snd_CONTROL_VOLUME, 0);

	/*____________________________________________________________________
	|
	| Initialize the graphics state
	|___________________________________________________________________*/

	// Set 2d graphics state
	Pgm_screen.xleft = 0;
	Pgm_screen.ytop = 0;
	Pgm_screen.xright = gxGetScreenWidth() - 1;
	Pgm_screen.ybottom = gxGetScreenHeight() - 1;
	gxSetWindow(&Pgm_screen);
	gxSetClip(&Pgm_screen);
	gxSetClipping(FALSE);

	// Set the 3D viewport
	gx3d_SetViewport(&Pgm_screen);
	// Init other 3D stuff
	Init_Render_State();

	/*____________________________________________________________________
	|
	| Init support routines
	|___________________________________________________________________*/

	gx3dVector heading, position;

	// Set starting camera position
	position.x = -200;
	position.y = 100;
	position.z = -4930;
	// Set starting camera view direction (heading)
	heading.x = -0.07; // {0,0,1} for cubic environment mapping to work correctly
	heading.y = -0.05;
	heading.z = 1;
	Position_Init(&position, &heading, RUN_SPEED);

	/*____________________________________________________________________
	|
	| Init 3D graphics
	|___________________________________________________________________*/

	// Set projection matrix
	float fov = 89; // degrees field of view
	float near_plane = 0.1f;
	float far_plane = 80000;
	gx3d_SetProjectionMatrix(fov, near_plane, far_plane);

	gx3d_SetFillMode(gx3d_FILL_MODE_GOURAUD_SHADED);

	// Clear the 3D viewport to all black
	color.r = 0;
	color.g = 0;
	color.b = 0;
	color.a = 0;

	/*____________________________________________________________________
	|
	| Load 3D models
	|___________________________________________________________________*/

	gx3dParticleSystem psys_glitter = Script_ParticleSystem_Create("glitter.gxps");

	// Load a 3D model

	gx3dObject *obj_ground;
	gx3d_ReadLWO2File("Objects\\ground.lwo", &obj_ground, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_ground = gx3d_InitTexture_File("Objects\\Images\\grass2.bmp", 0, 0);

	gx3dObject *obj_sky;
	gx3d_ReadLWO2File("Objects\\skydome.lwo", &obj_sky, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_sky = gx3d_InitTexture_File("Objects\\Images\\sky.bmp", 0, 0);

	gx3dObject *obj_title;
	gx3d_ReadLWO2File("Objects\\billboard_title.lwo", &obj_title, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_title = gx3d_InitTexture_File("Objects\\Images\\left.bmp", 0, 0);

	gx3dObject *obj_title2;
	gx3d_ReadLWO2File("Objects\\billboard_title_2.lwo", &obj_title2, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_title2 = gx3d_InitTexture_File("Objects\\Images\\right.bmp", 0, 0);

	gx3dObject *obj_cross;
	gx3d_ReadLWO2File("Objects\\billboard_cross.lwo", &obj_cross, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_cross = gx3d_InitTexture_File("Objects\\Images\\crosshair.bmp", "Objects\\Images\\crosshair_fa.bmp", 0);

	gx3dObject *obj_egg;
	gx3d_ReadLWO2File("Objects\\egg.lwo", &obj_egg, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_egg = gx3d_InitTexture_File("Objects\\Images\\egg.bmp", 0, 0);

	gx3dObject *obj_fence;
	gx3d_ReadLWO2File("Objects\\fence.lwo", &obj_fence, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_fence = gx3d_InitTexture_File("Objects\\Images\\wood.bmp", 0, 0);

	gx3dObject *obj_hill;
	gx3d_ReadLWO2File("Objects\\hill.lwo", &obj_hill, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_hill = gx3d_InitTexture_File("Objects\\Images\\hill.bmp", 0, 0);

	gx3dObject *obj_hay;
	gx3d_ReadLWO2File("Objects\\hay.lwo", &obj_hay, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_hay = gx3d_InitTexture_File("Objects\\Images\\hay.bmp", 0, 0);

	gx3dObject *obj_trashcan;
	gx3d_ReadLWO2File("Objects\\trashcan.lwo", &obj_trashcan, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_trashcan = gx3d_InitTexture_File("Objects\\Images\\trash.bmp", 0, 0);

	gx3dObject *obj_2d_egg;
	gx3d_ReadLWO2File("Objects\\billboard_egg.lwo", &obj_2d_egg, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_2d_egg = gx3d_InitTexture_File("Objects\\Images\\2d_egg.bmp", "Objects\\Images\\2d_egg_fa.bmp", 0);

	gx3dObject *obj_field;
	gx3d_ReadLWO2File("Objects\\field.lwo", &obj_field, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_field = gx3d_InitTexture_File("Objects\\Images\\field.bmp", "Objects\\Images\\field_fa.bmp", 0);

	gx3dObject *obj_poles;
	gx3d_ReadLWO2File("Objects\\poles.lwo", &obj_poles, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_poles = gx3d_InitTexture_File("Objects\\Images\\metal.bmp", 0, 0);

	gx3dObject *obj_fountain;
	gx3d_ReadLWO2File("Objects\\fountain.lwo", &obj_fountain, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_concrete = gx3d_InitTexture_File("Objects\\Images\\concrete.bmp", 0, 0);

	gx3dObject *obj_tree;
	gx3d_ReadLWO2File("Objects\\tree.lwo", &obj_tree, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_tree = gx3d_InitTexture_File("Objects\\Images\\tree.bmp", "Objects\\Images\\tree_fa.bmp", 0);

	gx3dObject *obj_windmill;
	gx3d_ReadLWO2File("Objects\\windmill.lwo", &obj_windmill, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3dTexture tex_windmill = gx3d_InitTexture_File("Objects\\Images\\windmill_texture.bmp", 0, 0);

	gx3dTexture tex_sign = gx3d_InitTexture_File("Objects\\Images\\sign_1.bmp", 0, 0);
	gx3dTexture tex_sign2 = gx3d_InitTexture_File("Objects\\Images\\sign_2.bmp", 0, 0);

	gx3dTexture tex_help = gx3d_InitTexture_File("Objects\\Images\\help_1.bmp", 0, 0);
	gx3dTexture tex_help2 = gx3d_InitTexture_File("Objects\\Images\\help_2.bmp", 0, 0);

	gx3dTexture tex_end = gx3d_InitTexture_File("Objects\\Images\\end_1.bmp", 0, 0);
	gx3dTexture tex_end2 = gx3d_InitTexture_File("Objects\\Images\\end_2.bmp", 0, 0);

	gx3dTexture tex_grass_field = gx3d_InitTexture_File("Objects\\Images\\grass_field.bmp", "Objects\\Images\\grass_field_fa.bmp", 0);

	gx3d_GetScaleMatrix(&m, 500, 200, 500);
	gx3d_TransformObject(obj_sky, &m);

	#define NUM_EGGS 12
    #define NUM_FIELD 35
    #define NUM_FIELD2 200
	#define NUM_TREES 15
	#define NUM_GRASS 100
	gx3dVector eggPosition[NUM_EGGS];
	bool eggDraw[NUM_EGGS];
	gx3dSphere eggSphere[NUM_EGGS];
	bool eggOnScreen[NUM_EGGS];
	static float eggSpeed[NUM_EGGS];
	int eggsCollected = 0;
	bool eggParticle[NUM_EGGS];
	bool helpScreen = false;
	int gameOver = 0;


	for (int i = 0; i < NUM_EGGS; i++) {
		eggDraw[i] = true;
		eggSpeed[i] = 0.2f;
		eggParticle[i] = false;
	}

	eggPosition[0].x = 555;
	eggPosition[0].y = 10;
	eggPosition[0].z = 1090;
	eggSpeed[0] = 0.1f;

	eggPosition[1].x = -1010;
	eggPosition[1].y = -5;
	eggPosition[1].z = -2000;
	eggSpeed[1] = 0.1f;

	eggPosition[2].x = -632;
	eggPosition[2].y = -5;
	eggPosition[2].z = -3278;
	eggSpeed[2] = 0.1f;

	eggPosition[3].x = -5650;
	eggPosition[3].y = -5;
	eggPosition[3].z = -3450;
	eggSpeed[3] = 0.1f;

	eggPosition[4].x = 4019;
	eggPosition[4].y = 20;
	eggPosition[4].z = 3568;
	eggSpeed[4] = 0.1f;

	eggPosition[5].x = 4328;
	eggPosition[5].y = 80;
	eggPosition[5].z = -1519;
	eggSpeed[5] = 0.1f;

	eggPosition[6].x = -5889;
	eggPosition[6].y = -5;
	eggPosition[6].z = 3263;
	eggSpeed[6] = 0.1f;

	eggPosition[7].x = -4113;
	eggPosition[7].y = -5;
	eggPosition[7].z = 4708;
	eggSpeed[7] = 0.1f;

	eggPosition[8].x = -1544;
	eggPosition[8].y = -5;
	eggPosition[8].z = 4921;
	eggSpeed[8] = 0.1f;

	eggPosition[9].x = 2972;
	eggPosition[9].y = 40;
	eggPosition[9].z = -2794;
	eggSpeed[9] = 0.1f;

	eggPosition[10].x = -5678;
	eggPosition[10].y = -5;
	eggPosition[10].z = -6182;
	eggSpeed[10] = 0.1f;

	eggPosition[11].x = -3742;
	eggPosition[11].y = -5;
	eggPosition[11].z = 4280;
	eggSpeed[11] = 0.1f;

	/*____________________________________________________________________
	|
	| create lights
	|___________________________________________________________________*/

	// This code needs to be outside the main game loop.  That was the problem in class.

	gx3dLight dir_light;
	gx3dLightData light_data;
	light_data.light_type = gx3d_LIGHT_TYPE_DIRECTION;
	light_data.direction.diffuse_color.r = 0.98;
	light_data.direction.diffuse_color.g = 0.37;
	light_data.direction.diffuse_color.b = 0.33;
	light_data.direction.diffuse_color.a = 0;
	light_data.direction.specular_color.r = 1;
	light_data.direction.specular_color.g = 1;
	light_data.direction.specular_color.b = 1;
	light_data.direction.specular_color.a = 0;
	light_data.direction.ambient_color.r = 0;
	light_data.direction.ambient_color.g = 0;
	light_data.direction.ambient_color.b = 0;
	light_data.direction.ambient_color.a = 0;
	light_data.direction.dst.x = -1;
	light_data.direction.dst.y = -1;
	light_data.direction.dst.z = 0;

	dir_light = gx3d_InitLight(&light_data);

	gx3dLight main_light;
	light_data.light_type = gx3d_LIGHT_TYPE_DIRECTION;
	light_data.direction.diffuse_color.r = 1;
	light_data.direction.diffuse_color.g = 1;
	light_data.direction.diffuse_color.b = 1;
	light_data.direction.diffuse_color.a = 0;
	light_data.direction.specular_color.r = 1;
	light_data.direction.specular_color.g = 1;
	light_data.direction.specular_color.b = 1;
	light_data.direction.specular_color.a = 0;
	light_data.direction.ambient_color.r = 0;
	light_data.direction.ambient_color.g = 0;
	light_data.direction.ambient_color.b = 0;
	light_data.direction.ambient_color.a = 0;
	light_data.direction.dst.x = -1;
	light_data.direction.dst.y = -1;
	light_data.direction.dst.z = 0;

	main_light = gx3d_InitLight(&light_data);

	gx3dLight point_light1;
	light_data.light_type = gx3d_LIGHT_TYPE_POINT;
	light_data.point.diffuse_color.r = 1; // red light
	light_data.point.diffuse_color.g = 1;
	light_data.point.diffuse_color.b = 1;
	light_data.point.diffuse_color.a = 0;
	light_data.point.specular_color.r = 1;
	light_data.point.specular_color.g = 1;
	light_data.point.specular_color.b = 1;
	light_data.point.specular_color.a = 0;
	light_data.point.ambient_color.r = 0; // ambient turned offf
	light_data.point.ambient_color.g = 0;
	light_data.point.ambient_color.b = 0;
	light_data.point.ambient_color.a = 0;
	light_data.point.src.x = 0;
	light_data.point.src.y = 25; // top of pine tree is 60' tall so this light be will just above that height
	light_data.point.src.z = 0;
	light_data.point.range = 300;
	light_data.point.constant_attenuation = 0;
	light_data.point.linear_attenuation = 0.1;
	light_data.point.quadratic_attenuation = 0;

	point_light1 = gx3d_InitLight(&light_data);

	gx3dVector light_position = { 10, 20, 0 }, xlight_position;
	float angle = 0;

	/*____________________________________________________________________
	|
	| Flush input queue
	|___________________________________________________________________*/

	int move_x, move_y; // mouse movement counters

	// Flush input queue
	evFlushEvents();
	// Zero mouse movement counters
	msGetMouseMovement(&move_x, &move_y); // call this here so the next call will get movement that has occurred since it was called here
	// Hide mouse cursor
	msHideMouse();

	/*____________________________________________________________________
	|
	| Main game loop
	|___________________________________________________________________*/

	boolean walking = false;
	int pickupDistance = 350;

	snd_PlaySound(s_song, 1);
	snd_SetSoundVolume(s_song, 70);

	snd_SetSoundMode(s_crickets, snd_3D_MODE_ORIGIN_RELATIVE, snd_3D_APPLY_NOW);
	snd_SetSoundPosition(s_crickets, 0, 0, 0, snd_3D_APPLY_NOW);
	snd_SetSoundMinDistance(s_crickets, 0, snd_3D_APPLY_NOW);
	snd_SetSoundMaxDistance(s_crickets, 100, snd_3D_APPLY_NOW);
	snd_PlaySound(s_crickets, 1);

	// Variables
	unsigned elapsed_time, last_time, new_time;
	bool force_update;
	unsigned cmd_move;

	// Init loop variables
	cmd_move = 0;
	last_time = 0;
	force_update = false;

	int lightMode = 0; // 0 = ambient, 1 = directional, 2 = point

	int field_x[NUM_FIELD], field_z[NUM_FIELD], field_y[NUM_FIELD];

	for (int i = 0; i < NUM_FIELD; i++) {
		field_x[i] = (rand() % 1800 - 2700);
		field_y[i] = 30;
		field_z[i] = (rand() % 1600 - 1900);
	}

	int field2_x[NUM_FIELD2], field2_z[NUM_FIELD2], field2_y[NUM_FIELD2];

	for (int i = 0; i < NUM_FIELD2; i++) {
		field2_x[i] = (rand() % 4000 - 7400);
		field2_y[i] = 30;
		field2_z[i] = (rand() % 3000 + 6000);
	}

	int tree_x[NUM_TREES], tree_y[NUM_TREES], tree_z[NUM_TREES];

	for (int i = 0; i < NUM_TREES; i++) {
		tree_x[i] = (rand() % 2500 + 2000);
		tree_y[i] = 5;
		tree_z[i] = (rand() % 4000 - 1800);
	}

	int tree2_x[NUM_TREES], tree2_y[NUM_TREES], tree2_z[NUM_TREES];

	for (int i = 0; i < NUM_TREES; i++) {
		tree2_x[i] = (rand() % 2499 + 600);
		tree2_y[i] = 5;
		tree2_z[i] = (rand() % 1999 - 3700);
	}

	int grass_x[NUM_GRASS], grass_y[NUM_GRASS], grass_z[NUM_GRASS];

	for (int i = 0; i < NUM_GRASS; i++) {
		grass_x[i] = (rand() % 2499 - 4000);
		grass_y[i] = -19;
		grass_z[i] = (rand() % 1999 - 3000);
	}

	int grass2_x[NUM_GRASS], grass2_y[NUM_GRASS], grass2_z[NUM_GRASS];

	for (int i = 0; i < NUM_GRASS; i++) {
		grass2_x[i] = (rand() % 1500 - 2900);
		grass2_y[i] = -19;
		grass2_z[i] = (rand() % 1599 - 4900);
	}

	int grass3_x[50], grass3_y[50], grass3_z[50];

	for (int i = 0; i < 50; i++) {
		grass3_x[i] = (rand() % 1800 - 2000);
		grass3_y[i] = -19;
		grass3_z[i] = (rand() % 800 - 5000);
	}

	int grass4_x[NUM_GRASS], grass4_y[NUM_GRASS], grass4_z[NUM_GRASS];

	for (int i = 0; i < NUM_GRASS; i++) {
		grass4_x[i] = (rand() % 1750 + 75);
		grass4_y[i] = -19;
		grass4_z[i] = (rand() % 1999 - 4100);
	}

	int grass5_x[NUM_GRASS], grass5_y[NUM_GRASS], grass5_z[NUM_GRASS];

	for (int i = 0; i < NUM_GRASS; i++) {
		grass5_x[i] = (rand() % 2499 + 2050);
		grass5_y[i] = -19;
		grass5_z[i] = (rand() % 1999 - 4100);
	}

	int grass6_x[NUM_GRASS], grass6_y[NUM_GRASS], grass6_z[NUM_GRASS];

	for (int i = 0; i < NUM_GRASS; i++) {
		grass6_x[i] = (rand() % 1700 - 2600);
		grass6_y[i] = -19;
		grass6_z[i] = (rand() % 1999 - 1675);
	}

	int grass7_x[NUM_GRASS], grass7_y[NUM_GRASS], grass7_z[NUM_GRASS];

	for (int i = 0; i < NUM_GRASS; i++) {
		grass7_x[i] = (rand() % 1550 + 2900);
		grass7_y[i] = -19;
		grass7_z[i] = (rand() % 5500 - 2800);
	}

	int grass8_x[50], grass8_y[50], grass8_z[50];

	for (int i = 0; i < 50; i++) {
		grass8_x[i] = (rand() % 1850 + 500);
		grass8_y[i] = -19;
		grass8_z[i] = (rand() % 800 + 2175);
	}

	bool fastMovement = false;

	// Game loop
	for (quit = FALSE; NOT quit;)
	{
		/*____________________________________________________________________
		|
		| Update clock
		|___________________________________________________________________*/

		// Get the current time (# milliseconds since the program started)
		new_time = timeGetTime();
		// Compute the elapsed time (in milliseconds) since the last time through this loop
		if (last_time == 0)
			elapsed_time = 0;
		else
			elapsed_time = new_time - last_time;
		last_time = new_time;

		static float currTime = 0.0f;

		/*____________________________________________________________________
		|
		| Process user input
		|___________________________________________________________________*/

		gxRelation relation;
		gx3dRay viewVector;
		viewVector.origin = position;
		viewVector.direction = heading;

		// Any event ready?
		if (evGetEvent(&event))
		{
			// key press?
			if (event.type == evTYPE_RAW_KEY_PRESS)
			{
				// If ESC pressed, exit the program
				if (event.keycode == evKY_ESC)
					quit = TRUE;
				if (event.keycode == evKY_ENTER && gameState == 0)
					gameState = 1;
				if (event.keycode == evKY_F2) {
					sprintf(str, "\n Player X: %.2f, \n Player Y: %.2f \n Player Z: %.2f \n", position.x, position.y, position.z);
					debug_WriteFile(str);
				}
				if (event.keycode == evKY_F1) {
					helpScreen = !helpScreen;

				}
				else
				{
					if (event.keycode == 'w')
						cmd_move |= POSITION_MOVE_FORWARD;
					else if (event.keycode == 's')
						cmd_move |= POSITION_MOVE_BACK;
					else if (event.keycode == 'a')
						cmd_move |= POSITION_MOVE_LEFT;
					else if (event.keycode == 'd')
						cmd_move |= POSITION_MOVE_RIGHT;
					else if (event.keycode == evKY_F1)
					{
						lightMode++;
						if (lightMode == 3)
							lightMode = 0;
					}
					else if (event.keycode == evKY_SHIFT)
						fastMovement = true;
				}
			}
			// key release?
			else if (event.type == evTYPE_RAW_KEY_RELEASE)
			{
				if (event.keycode == 'w')
					cmd_move &= ~(POSITION_MOVE_FORWARD);
				else if (event.keycode == 's')
					cmd_move &= ~(POSITION_MOVE_BACK);
				else if (event.keycode == 'a')
					cmd_move &= ~(POSITION_MOVE_LEFT);
				else if (event.keycode == 'd')
					cmd_move &= ~(POSITION_MOVE_RIGHT);
				else if (event.keycode == evKY_SHIFT)
					fastMovement = false;
			}

			if (gameState == 1) {

				

				if (event.type == evTYPE_MOUSE_LEFT_PRESS) {
					/* if (!snd_IsPlaying(s_laser)) {
						snd_PlaySound(s_laser, 0);
					} */

					
					// bool egg_hit = false;

					
					
					for (int i = 0; i < NUM_EGGS /* && !egg_hit */; i++)
					{
						if (eggDraw[i]) {
							if (eggOnScreen[i])
							{
								if (Distance(position.x, position.z, eggPosition[i].x, eggPosition[i].z) > pickupDistance) continue;

								gxRelation rel = gx3d_Relation_Ray_Sphere(&viewVector, &eggSphere[i]);
								if (rel == gxRELATION_INTERSECT)
								{
									eggDraw[i] = false;
									eggParticle[i] = true;
									currTime = timeGetTime() / 1000;
									gameOver++;

									snd_SetSoundVolume(s_twinkle, 140);

									if (snd_IsPlaying(s_twinkle))
										snd_PlaySound(s_twinkle, 0);
									snd_PlaySound(s_twinkle, 0);
								}
							}
						}
					}
				}

				if (gameOver == NUM_EGGS) {
					gameState = 3;
			}

			}
			if (cmd_move != 0 && gameState == 1 && !helpScreen)
			{
				walking = true;
				if (!snd_IsPlaying(s_footsteps))
					snd_PlaySound(s_footsteps, 1);
				snd_SetSoundVolume(s_footsteps, 75);
			}
			else
			{
				walking = false;
				snd_StopSound(s_footsteps);
			}
		}
		// Check for camera movement (via mouse)
		msGetMouseMovement(&move_x, &move_y);

		/*____________________________________________________________________
		|
		| Update camera view
		|___________________________________________________________________*/

		if (fastMovement)
			Position_Set_Speed(RUN_SPEED * 78);
		else
			Position_Set_Speed(RUN_SPEED * 53);

		if (gameState != 1)
		{
			move_y = 0;
			move_x = 0;
			cmd_move = 0;
		}

		bool position_changed, camera_changed;
		Position_Update(elapsed_time, cmd_move, move_y, move_x, force_update,
			&position_changed, &camera_changed, &position, &heading);
		snd_SetListenerPosition(position.x, position.y, position.z, snd_3D_APPLY_NOW);
		snd_SetListenerOrientation(heading.x, heading.y, heading.z, 0, 1, 0, snd_3D_APPLY_NOW);

		/*____________________________________________________________________
		|
		| Draw 3D graphics
		|___________________________________________________________________*/

		// Render the screen
		gx3d_ClearViewport(gx3d_CLEAR_SURFACE | gx3d_CLEAR_ZBUFFER, color, gx3d_MAX_ZBUFFER_VALUE, 0);
		// Start rendering in 3D
		if (gx3d_BeginRender())
		{
			// Set the default material
			gx3d_SetMaterial(&material_default);

			bool cross = true;
			gx3dMatrix view_save;
			gx3dVector tfrom, tto, twup;

			gx3d_SetAmbientLight(color3d_white);
			gx3d_DisableLight(dir_light);


			if (gameState != 3 && helpScreen == true) {
				// Draw Help Screen
				if (helpScreen) {

					cross = false;

					move_y = 0;
					move_x = 0;
					cmd_move = 0;

					// Save current view matrix
					view_save;
					gx3d_GetViewMatrix(&view_save);

					// Set new view matrix
					tfrom = { 0, 0, 1 }, tto = { 0, 0, 0 }, twup = { 0, 1, 0 };
					gx3d_CameraSetPosition(&tfrom, &tto, &twup, gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED);
					gx3d_CameraSetViewMatrix();

					gx3d_GetTranslateMatrix(&m, 14.25, -15.6, -36);
					gx3d_SetObjectMatrix(obj_title, &m);
					gx3d_SetTexture(0, tex_help);
					gx3d_DrawObject(obj_title, 0);

					gx3d_GetTranslateMatrix(&m, -15.75, -15.6, -36);
					gx3d_SetObjectMatrix(obj_title2, &m);
					gx3d_SetTexture(0, tex_help2);
					gx3d_DrawObject(obj_title2, 0);

					// Restore View Matrix
					gx3d_SetViewMatrix(&view_save);

					// Stop rendering
					gx3d_EndRender();
				}
			}

			if (gameState == 0)
			{

				// Draw Title Screen
				{
					cross = false;

					// Save current view matrix
					view_save;
					gx3d_GetViewMatrix(&view_save);

					// Set new view matrix
					tfrom = { 0, 0, 1 }, tto = { 0, 0, 0 }, twup = { 0, 1, 0 };
					gx3d_CameraSetPosition(&tfrom, &tto, &twup, gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED);
					gx3d_CameraSetViewMatrix();

					// Draw Title Screen
					gx3d_GetTranslateMatrix(&m, 15.25, -14.3, -37);
					gx3d_SetObjectMatrix(obj_title, &m);
					gx3d_SetTexture(0, tex_title);
					gx3d_DrawObject(obj_title, 0);

					gx3d_GetTranslateMatrix(&m, -14.7, -14.3, -37);
					gx3d_SetObjectMatrix(obj_title2, &m);
					gx3d_SetTexture(0, tex_title2);
					gx3d_DrawObject(obj_title2, 0);

					// Restore View Matrix
					gx3d_SetViewMatrix(&view_save);

					// Stop rendering
					gx3d_EndRender();
				}
			}

			if (gameState == 1)
			{

				// Draw skydome
				{
					gx3d_SetAmbientLight(color3d_gray);
					gx3d_EnableLight(dir_light);
					gx3d_GetTranslateMatrix(&m, 0, 0, 0);
					gx3d_SetObjectMatrix(obj_sky, &m);
					gx3d_SetTexture(0, tex_sky);
					gx3d_DrawObject(obj_sky, 0);
				}

				gx3d_DisableLight(dir_light);
				gx3d_SetAmbientLight(color3d_white);
				gx3d_EnableLight(main_light);

				// Draw ground
				{
					gx3d_GetTranslateMatrix(&m, 0, 0, 0);
					gx3d_SetObjectMatrix(obj_ground, &m);
					gx3d_SetTexture(0, tex_ground);
					gx3d_DrawObject(obj_ground, 0);
				}

				// Draw field
				{
					for (int i = 0; i < NUM_FIELD; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 50, 50, 50);
						gx3d_GetRotateYMatrix(&m2, 50);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_GetTranslateMatrix(&m3, field_x[i], field_y[i], field_z[i]);
						gx3d_MultiplyMatrix(&m, &m3, &m);
						gx3d_SetObjectMatrix(obj_field, &m);
						gx3d_SetTexture(0, tex_field);
						gx3d_DrawObject(obj_field, 0);


						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}
				}

				// Draw grass field
				{
					for (int i = 0; i < NUM_GRASS; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 10, 10, 10);
						gx3d_GetRotateYMatrix(&m2, 140);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_GetTranslateMatrix(&m3, grass_x[i], grass_y[i], grass_z[i]);
						gx3d_MultiplyMatrix(&m, &m3, &m);
						gx3d_SetObjectMatrix(obj_field, &m);
						gx3d_SetTexture(0, tex_grass_field);
						gx3d_DrawObject(obj_field, 0);


						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}

					for (int i = 0; i < NUM_GRASS; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 10, 10, 10);
						gx3d_GetRotateYMatrix(&m2, 140);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_GetTranslateMatrix(&m3, grass2_x[i], grass2_y[i], grass2_z[i]);
						gx3d_MultiplyMatrix(&m, &m3, &m);
						gx3d_SetObjectMatrix(obj_field, &m);
						gx3d_SetTexture(0, tex_grass_field);
						gx3d_DrawObject(obj_field, 0);


						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}

					for (int i = 0; i < 50; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 10, 10, 10);
						gx3d_GetRotateYMatrix(&m2, 140);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_GetTranslateMatrix(&m3, grass3_x[i], grass3_y[i], grass3_z[i]);
						gx3d_MultiplyMatrix(&m, &m3, &m);
						gx3d_SetObjectMatrix(obj_field, &m);
						gx3d_SetTexture(0, tex_grass_field);
						gx3d_DrawObject(obj_field, 0);


						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}

					for (int i = 0; i < NUM_GRASS; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 10, 10, 10);
						gx3d_GetRotateYMatrix(&m2, 140);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_GetTranslateMatrix(&m3, grass4_x[i], grass4_y[i], grass4_z[i]);
						gx3d_MultiplyMatrix(&m, &m3, &m);
						gx3d_SetObjectMatrix(obj_field, &m);
						gx3d_SetTexture(0, tex_grass_field);
						gx3d_DrawObject(obj_field, 0);


						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}

					for (int i = 0; i < NUM_GRASS; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 10, 10, 10);
						gx3d_GetRotateYMatrix(&m2, 140);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_GetTranslateMatrix(&m3, grass5_x[i], grass5_y[i], grass5_z[i]);
						gx3d_MultiplyMatrix(&m, &m3, &m);
						gx3d_SetObjectMatrix(obj_field, &m);
						gx3d_SetTexture(0, tex_grass_field);
						gx3d_DrawObject(obj_field, 0);


						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}

					for (int i = 0; i < NUM_GRASS; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 10, 10, 10);
						gx3d_GetRotateYMatrix(&m2, 140);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_GetTranslateMatrix(&m3, grass6_x[i], grass6_y[i], grass6_z[i]);
						gx3d_MultiplyMatrix(&m, &m3, &m);
						gx3d_SetObjectMatrix(obj_field, &m);
						gx3d_SetTexture(0, tex_grass_field);
						gx3d_DrawObject(obj_field, 0);


						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}

					for (int i = 0; i < NUM_GRASS; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 10, 10, 10);
						gx3d_GetRotateYMatrix(&m2, 140);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_GetTranslateMatrix(&m3, grass7_x[i], grass7_y[i], grass7_z[i]);
						gx3d_MultiplyMatrix(&m, &m3, &m);
						gx3d_SetObjectMatrix(obj_field, &m);
						gx3d_SetTexture(0, tex_grass_field);
						gx3d_DrawObject(obj_field, 0);


						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}

					for (int i = 0; i < 50; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 10, 10, 10);
						gx3d_GetRotateYMatrix(&m2, 140);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_GetTranslateMatrix(&m3, grass8_x[i], grass8_y[i], grass8_z[i]);
						gx3d_MultiplyMatrix(&m, &m3, &m);
						gx3d_SetObjectMatrix(obj_field, &m);
						gx3d_SetTexture(0, tex_grass_field);
						gx3d_DrawObject(obj_field, 0);


						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}
				}

				// Draw windmill field
				{
					for (int i = 0; i < NUM_FIELD2; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 50, 50, 50);
						gx3d_GetRotateYMatrix(&m2, -40);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_GetTranslateMatrix(&m3, field2_x[i], field2_y[i], field2_z[i]);
						gx3d_MultiplyMatrix(&m, &m3, &m);
						gx3d_SetObjectMatrix(obj_field, &m);
						gx3d_SetTexture(0, tex_field);
						gx3d_DrawObject(obj_field, 0);


						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}
				}

				// Draw trees
				{
					for (int i = 0; i < NUM_TREES; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 20, 20, 20);
						gx3d_GetTranslateMatrix(&m2, tree_x[i], tree_y[i], tree_z[i]);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_SetObjectMatrix(obj_tree, &m);
						gx3d_SetTexture(0, tex_tree);
						gx3d_DrawObject(obj_tree, 0);

						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}

					for (int i = 0; i < NUM_TREES; i++) {
						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetScaleMatrix(&m1, 20, 20, 20);
						gx3d_GetTranslateMatrix(&m2, tree2_x[i], tree2_y[i], tree2_z[i]);
						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_SetObjectMatrix(obj_tree, &m);
						gx3d_SetTexture(0, tex_tree);
						gx3d_DrawObject(obj_tree, 0);

						gx3d_DisableAlphaBlending();
						gx3d_DisableAlphaTesting();
					}
				}

				// Draw haybales
				{
					gx3d_GetScaleMatrix(&m1, 5, 5, 5);
					gx3d_GetRotateYMatrix(&m2, 130);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, -153, -19, 5084);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_hay, &m);
					gx3d_SetTexture(0, tex_hay);
					gx3d_DrawObject(obj_hay, 0);

					gx3d_GetScaleMatrix(&m1, 5, 5, 5);
					gx3d_GetRotateYMatrix(&m2, 130);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, 288, -19, 5342);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_hay, &m);
					gx3d_SetTexture(0, tex_hay);
					gx3d_DrawObject(obj_hay, 0);

					gx3d_GetScaleMatrix(&m1, 5, 5, 5);
					gx3d_GetRotateYMatrix(&m2, 130);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, 850, -19, 5495);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_hay, &m);
					gx3d_SetTexture(0, tex_hay);
					gx3d_DrawObject(obj_hay, 0);

					gx3d_GetScaleMatrix(&m1, 5, 5, 5);
					gx3d_GetRotateYMatrix(&m2, 130);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, 1675, -19, 5660);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_hay, &m);
					gx3d_SetTexture(0, tex_hay);
					gx3d_DrawObject(obj_hay, 0);

					gx3d_GetScaleMatrix(&m1, 5, 5, 5);
					gx3d_GetRotateYMatrix(&m2, 130);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, 2305, -19, 5464);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_hay, &m);
					gx3d_SetTexture(0, tex_hay);
					gx3d_DrawObject(obj_hay, 0);

					gx3d_GetScaleMatrix(&m1, 5, 5, 5);
					gx3d_GetRotateYMatrix(&m2, 130);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, 2525, -19, 4842);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_hay, &m);
					gx3d_SetTexture(0, tex_hay);
					gx3d_DrawObject(obj_hay, 0);

					gx3d_GetScaleMatrix(&m1, 5, 5, 5);
					gx3d_GetRotateYMatrix(&m2, 130);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, 3219, -19, 4503);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_hay, &m);
					gx3d_SetTexture(0, tex_hay);
					gx3d_DrawObject(obj_hay, 0);

					gx3d_GetScaleMatrix(&m1, 5, 5, 5);
					gx3d_GetRotateYMatrix(&m2, 130);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, 3711, -19, 4088);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_hay, &m);
					gx3d_SetTexture(0, tex_hay);
					gx3d_DrawObject(obj_hay, 0);

					gx3d_GetScaleMatrix(&m1, 5, 5, 5);
					gx3d_GetRotateYMatrix(&m2, 130);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, 3956, -19, 3469);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_hay, &m);
					gx3d_SetTexture(0, tex_hay);
					gx3d_DrawObject(obj_hay, 0);
				}

				// Draw trashcan
				{
					gx3d_GetScaleMatrix(&m1, 5, 5, 5);
					gx3d_GetTranslateMatrix(&m2, -1010, -19, -2000);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_trashcan, &m);
					gx3d_SetTexture(0, tex_trashcan);
					gx3d_DrawObject(obj_trashcan, 0);
				}

				// Draw fountain
				{
					gx3d_GetScaleMatrix(&m1, 20, 20, 20);
					gx3d_GetTranslateMatrix(&m2, 500, -19, 800);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fountain, &m);
					gx3d_SetTexture(0, tex_concrete);
					gx3d_DrawObject(obj_fountain, 0);
				}

				// Draw Windmill
				{
					gx3d_GetScaleMatrix(&m1, 23, 23, 23);
					gx3d_GetRotateYMatrix(&m2, 210);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, -5800, -19, 3200);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_windmill, &m);
					gx3d_SetTexture(0, tex_windmill);
					gx3d_DrawObject(obj_windmill, 0);

				}

				// Draw sign
				{
					// Left
					gx3d_GetScaleMatrix(&m1, 9, 9, 9);
					gx3d_GetRotateYMatrix(&m2, 155);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, -770, 270, -3280);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_title, &m);
					gx3d_SetTexture(0, tex_sign);
					gx3d_DrawObject(obj_title, 0);

					// Right
					gx3d_GetScaleMatrix(&m1, 9, 9, 9);
					gx3d_GetRotateYMatrix(&m2, 155);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetTranslateMatrix(&m3, -525, 270, -3166);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_title2, &m);
					gx3d_SetTexture(0, tex_sign2);
					gx3d_DrawObject(obj_title2, 0);
					
					// Draw poles
					gx3d_GetScaleMatrix(&m1, 15, 15, 15);
					gx3d_GetTranslateMatrix(&m2, -2000, -110, -2700);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_GetRotateYMatrix(&m3, -25);
					gx3d_MultiplyMatrix(&m, &m3, &m);
					gx3d_SetObjectMatrix(obj_poles, &m);
					gx3d_SetTexture(0, tex_poles);
					gx3d_DrawObject(obj_poles, 0);
				}

				// Draw fence
				{
					// First is front left of player
					gx3d_GetRotateYMatrix(&m1, -24);
					gx3d_GetTranslateMatrix(&m2, -650, -5, -4460);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					// The rest are going clockwise from first
					gx3d_GetRotateYMatrix(&m1, -24);
					gx3d_GetTranslateMatrix(&m2, -999, -5, -4618);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -10);
					gx3d_GetTranslateMatrix(&m2, -1358, -5, -4728);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 0);
					gx3d_GetTranslateMatrix(&m2, -1740, -5, -4760);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 15);
					gx3d_GetTranslateMatrix(&m2, -2115, -5, -4710);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 27);
					gx3d_GetTranslateMatrix(&m2, -2470, -5, -4575);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 27);
					gx3d_GetTranslateMatrix(&m2, -2813, -5, -4402);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -24);
					gx3d_GetTranslateMatrix(&m2, -3165, -5, -4390);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -35);
					gx3d_GetTranslateMatrix(&m2, -3500, -5, -4576);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -38);
					gx3d_GetTranslateMatrix(&m2, -3807, -5, -4802);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -27);
					gx3d_GetTranslateMatrix(&m2, -4127, -5, -5007);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -12);
					gx3d_GetTranslateMatrix(&m2, -4483, -5, -5132);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 13);
					gx3d_GetTranslateMatrix(&m2, -4854, -5, -5130);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 19);
					gx3d_GetTranslateMatrix(&m2, -5220, -5, -5026);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 35);
					gx3d_GetTranslateMatrix(&m2, -5557, -5, -4855);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 40);
					gx3d_GetTranslateMatrix(&m2, -5859, -5, -4623);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 51);
					gx3d_GetTranslateMatrix(&m2, -6125, -5, -4351);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 73);
					gx3d_GetTranslateMatrix(&m2, -6300, -5, -4020);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 87);
					gx3d_GetTranslateMatrix(&m2, -6365, -5, -3650);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 110);
					gx3d_GetTranslateMatrix(&m2, -6310, -5, -3280);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 130);
					gx3d_GetTranslateMatrix(&m2, -6122, -5, -2956);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 135.5);
					gx3d_GetTranslateMatrix(&m2, -5863, -5, -2674);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 135.5);
					gx3d_GetTranslateMatrix(&m2, -5588, -5, -2407);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 135.5);
					gx3d_GetTranslateMatrix(&m2, -5313, -5, -2140);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 135.5);
					gx3d_GetTranslateMatrix(&m2, -5038, -5, -1873);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 135.5);
					gx3d_GetTranslateMatrix(&m2, -4763, -5, -1606);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 125);
					gx3d_GetTranslateMatrix(&m2, -4517, -5, -1314);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 105);
					gx3d_GetTranslateMatrix(&m2, -4355, -5, -970);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 85);
					gx3d_GetTranslateMatrix(&m2, -4320, -5, -593);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 85);
					gx3d_GetTranslateMatrix(&m2, -4351, -5, -212);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 115);
					gx3d_GetTranslateMatrix(&m2, -4285, -5, 150);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 135);
					gx3d_GetTranslateMatrix(&m2, -4070, -5, 455);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 157);
					gx3d_GetTranslateMatrix(&m2, -3761, -5, 664);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 157);
					gx3d_GetTranslateMatrix(&m2, -3410, -5, 813);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 157);
					gx3d_GetTranslateMatrix(&m2, -2357, -5, 1260);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 157);
					gx3d_GetTranslateMatrix(&m2, -2006, -5, 1409);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, -1700, -5, 1627);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, -1444, -5, 1911);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, -1188, -5, 2195);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, -932, -5, 2479);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, -676, -5, 2763);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, -420, -5, 3047);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, -164, -5, 3331);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, 92, -5, 3615);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, 348, -5, 3899);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, 604, -5, 4183);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, 860, -5, 4467);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, 1116, -5, 4751);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 132);
					gx3d_GetTranslateMatrix(&m2, 1372, -5, 5035);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 1640, -5, 5060);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 1925, -5, 4805);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 2210, -5, 4550);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 2495, -5, 4295);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 2780, -5, 4040);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 3065, -5, 3785);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 3350, -5, 3530);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 3635, -5, 3275);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 3920, -5, 3020);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 4205, -5, 2765);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 4490, -5, 2510);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 42);
					gx3d_GetTranslateMatrix(&m2, 4775, -5, 2255);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4875, -5, 1935);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4815, -5, 1557);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4755, -5, 1179);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4695, -5, 801);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4635, -5, 423);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4575, -5, 45);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4515, -5, -333);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4455, -5, -711);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4395, -5, -1089);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4335, -5, -1467);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4275, -5, -1845);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4215, -5, -2223);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4155, -5, -2601);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4095, -5, -2979);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 4035, -5, -3357);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 3975, -5, -3735);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -81);
					gx3d_GetTranslateMatrix(&m2, 3915, -5, -4113);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 12);
					gx3d_GetTranslateMatrix(&m2, 3705, -5, -4257);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 12);
					gx3d_GetTranslateMatrix(&m2, 3331, -5, -4179);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 12);
					gx3d_GetTranslateMatrix(&m2, 2957, -5, -4101);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 12);
					gx3d_GetTranslateMatrix(&m2, 2583, -5, -4023);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, 12);
					gx3d_GetTranslateMatrix(&m2, 2209, -5, -3945);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -4.5);
					gx3d_GetTranslateMatrix(&m2, 1830, -5, -3920);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -4.5);
					gx3d_GetTranslateMatrix(&m2, 1449, -5, -3950);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -4.5);
					gx3d_GetTranslateMatrix(&m2, 1068, -5, -3980);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -4.5);
					gx3d_GetTranslateMatrix(&m2, 687, -5, -4010);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);

					gx3d_GetRotateYMatrix(&m1, -18);
					gx3d_GetTranslateMatrix(&m2, 313, -5, -4084);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_fence, &m);
					gx3d_SetTexture(0, tex_fence);
					gx3d_DrawObject(obj_fence, 0);
					// that took way too long :(
				}

				// Draw hills
				{
					gx3d_SetAmbientLight(color3d_white);

					// Draw hill
					gx3d_GetTranslateMatrix(&m, 0, 0, -9000);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill2
					gx3d_GetTranslateMatrix(&m, -3800, 0, -9000);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill3
					gx3d_GetTranslateMatrix(&m, 4000, 0, -8000);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill4
					gx3d_GetRotateYMatrix(&m1, 90);
					gx3d_GetTranslateMatrix(&m2, -9000, 0, -7000);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill5
					gx3d_GetRotateYMatrix(&m1, 90);
					gx3d_GetTranslateMatrix(&m2, 8000, 0, -7000);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill6
					gx3d_GetRotateYMatrix(&m1, 10);
					gx3d_GetTranslateMatrix(&m2, -9900, 0, -4500);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill7
					gx3d_GetRotateYMatrix(&m1, -90);
					gx3d_GetTranslateMatrix(&m2, 8000, 0, -5000);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill8
					gx3d_GetRotateYMatrix(&m1, 90);
					gx3d_GetTranslateMatrix(&m2, -8000, 0, 0);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill9
					gx3d_GetRotateYMatrix(&m1, -90);
					gx3d_GetTranslateMatrix(&m2, 8000, 0, -485);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill10
					gx3d_GetRotateYMatrix(&m1, 90);
					gx3d_GetTranslateMatrix(&m2, -10000, 0, 4500);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill11
					gx3d_GetRotateYMatrix(&m1, -90);
					gx3d_GetTranslateMatrix(&m2, 8000, 0, 4000);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill12
					gx3d_GetRotateYMatrix(&m1, 90);
					gx3d_GetTranslateMatrix(&m2, -5500, 0, 9000);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill13
					gx3d_GetRotateYMatrix(&m1, -90);
					gx3d_GetTranslateMatrix(&m2, 5000, 0, 9000);
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill14
					gx3d_GetTranslateMatrix(&m, -700, 0, 8900);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill15
					gx3d_GetTranslateMatrix(&m, -3000, 0, 8950);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill16
					gx3d_GetTranslateMatrix(&m, 1700, 0, 8900);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill17
					gx3d_GetTranslateMatrix(&m, 5845, 0, 6300);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);

					// Draw hill18
					gx3d_GetTranslateMatrix(&m, -9235, 0, 8160);
					gx3d_SetObjectMatrix(obj_hill, &m);
					gx3d_SetTexture(0, tex_hill);
					gx3d_DrawObject(obj_hill, 0);
				}

				gx3d_SetAmbientLight(color3d_white);
				gx3d_DisableLight(main_light);

				// Draw eggs
				{
					for (int i = 0; i < NUM_EGGS; i++) {
						static gx3dVector lerpLocation = eggPosition[i];
						if (eggDraw[i]) {
							eggSphere[i] = obj_egg->bound_sphere;
							eggSphere[i].center.x = eggPosition[i].x;
							eggSphere[i].center.y = lerpLocation.y + 15;
							eggSphere[i].center.z = eggPosition[i].z;
							eggSphere[i].radius *= 2;

							eggOnScreen[i] = false;
							relation = gx3d_Relation_Sphere_Frustum(&eggSphere[i]);

							if (relation != gxRELATION_OUTSIDE) {
								gx3dVector newLocation = eggPosition[i];
								newLocation.y += 20;
								float timeScale = (float)new_time / 1000;
								float alpha = std::sin(timeScale) * 0.5 + 0.5;
								gx3d_LerpVector(&eggPosition[i], &newLocation, alpha, &lerpLocation);

								eggOnScreen[i] = true;
								gx3d_GetScaleMatrix(&m1, 2.0f, 2.0f, 2.0f);
								gx3d_GetRotateYMatrix(&m2, timeScale * 100);
								gx3d_MultiplyMatrix(&m1, &m2, &m);
								gx3d_GetRotateXMatrix(&m3, -15);
								gx3d_MultiplyMatrix(&m, &m3, &m);
								gx3d_GetTranslateMatrix(&m4, eggPosition[i].x, lerpLocation.y, eggPosition[i].z);
								gx3d_MultiplyMatrix(&m, &m4, &m);
								gx3d_SetObjectMatrix(obj_egg, &m);
								gx3d_SetTexture(0, tex_egg);
								gx3d_DrawObject(obj_egg, 0);
							}
							
						}
						if (eggParticle[i]) {
							float elapsedParticle_time = timeGetTime() / 1000;
							elapsedParticle_time -= currTime;

							gx3d_EnableAlphaBlending();
							gx3d_GetScaleMatrix(&m1, 5.0f, 5.0f, 5.0f);
							gx3d_GetTranslateMatrix(&m2, eggPosition[i].x, lerpLocation.y + 25,  eggPosition[i].z);
							gx3d_MultiplyMatrix(&m1, &m2, &m);
							gx3d_SetParticleSystemMatrix(psys_glitter, &m);
							gx3d_UpdateParticleSystem(psys_glitter, elapsed_time);
							
						
							if (elapsedParticle_time > 1.0f)
								eggParticle[i] = false;
							gx3d_DrawParticleSystem(psys_glitter, &heading, false);
						}
						gx3d_DisableAlphaBlending();
					}
				}

				// Enable alpha blending (since the model to be drawn uses an alpha-blended texture)
				gx3d_EnableAlphaBlending();
				gx3d_EnableAlphaTesting(128);

				// Draw HUD
				{
					if (!helpScreen) {
						// Save current view matrix
						view_save;
						gx3d_GetViewMatrix(&view_save);

						// Set new view matrix
						tfrom = { 0, 0, 1 }, tto = { 0, 0, 0 }, twup = { 0, 1, 0 };
						gx3d_CameraSetPosition(&tfrom, &tto, &twup, gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED);
						gx3d_CameraSetViewMatrix();

						gx3d_EnableAlphaBlending();
						gx3d_EnableAlphaTesting(128);

						gx3d_GetTranslateMatrix(&m, 0, 0, -0.01);
						gx3d_SetObjectMatrix(obj_cross, &m);
						gx3d_SetTexture(0, tex_cross);

						// Check if crosshair should be drawn
						if (cross)
							gx3d_DrawObject(obj_cross, 0);

						for (int i = 0; i < NUM_EGGS; i++) {
							if (!eggDraw[i]) {
								eggsCollected++;
								gx3d_GetTranslateMatrix(&m, (-0.8 * (float)eggsCollected) + 6.8, 2.5, -0.01);
								gx3d_SetObjectMatrix(obj_2d_egg, &m);
								gx3d_SetTexture(0, tex_2d_egg);
								gx3d_DrawObject(obj_2d_egg, 0);
							}
						}
						eggsCollected = 0;
					}
				}

			}

			if (gameState == 3) {
				// Draw Game Over Screen
				{
					cross = false;

					// Save current view matrix
					view_save;
					gx3d_GetViewMatrix(&view_save);

					// Set new view matrix
					tfrom = { 0, 0, 1 }, tto = { 0, 0, 0 }, twup = { 0, 1, 0 };
					gx3d_CameraSetPosition(&tfrom, &tto, &twup, gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED);
					gx3d_CameraSetViewMatrix();

					// Draw Title Screen
					gx3d_GetTranslateMatrix(&m, 15.25, -14.3, -37);
					gx3d_SetObjectMatrix(obj_title, &m);
					gx3d_SetTexture(0, tex_end);
					gx3d_DrawObject(obj_title, 0);

					gx3d_GetTranslateMatrix(&m, -14.7, -14.3, -37);
					gx3d_SetObjectMatrix(obj_title2, &m);
					gx3d_SetTexture(0, tex_end2);
					gx3d_DrawObject(obj_title2, 0);

					// Restore View Matrix
					gx3d_SetViewMatrix(&view_save);

					// Stop rendering
					gx3d_EndRender();
				}
			}

			gx3d_DisableAlphaBlending();
			gx3d_DisableAlphaTesting();

			// Restore View Matrix
			gx3d_SetViewMatrix(&view_save);

			// Stop rendering
			gx3d_EndRender();

			// Page flip (so user can see it)
			gxFlipVisualActivePages(FALSE);
		}
	}

	/*____________________________________________________________________
	|
	| Free stuff and exit
	|___________________________________________________________________*/

	snd_StopSound(s_song);
	snd_StopSound(s_crickets);
	snd_Free();
	gx3d_FreeParticleSystem(psys_glitter);
}

/*____________________________________________________________________
|
| Function: Init_Render_State
|
| Input: Called from Program_Run()
| Output: Initializes general 3D render state.
|___________________________________________________________________*/

static void Init_Render_State()
{
	// Enable zbuffering
	gx3d_EnableZBuffer();

	// Enable lighting
	gx3d_EnableLighting();

	// Set the default alpha blend factor
	gx3d_SetAlphaBlendFactor(gx3d_ALPHABLENDFACTOR_SRCALPHA, gx3d_ALPHABLENDFACTOR_INVSRCALPHA);

	// Init texture addressing mode - wrap in both u and v dimensions
	gx3d_SetTextureAddressingMode(0, gx3d_TEXTURE_DIMENSION_U | gx3d_TEXTURE_DIMENSION_V, gx3d_TEXTURE_ADDRESSMODE_WRAP);
	gx3d_SetTextureAddressingMode(1, gx3d_TEXTURE_DIMENSION_U | gx3d_TEXTURE_DIMENSION_V, gx3d_TEXTURE_ADDRESSMODE_WRAP);
	// Texture stage 0 default blend operator and arguments
	gx3d_SetTextureColorOp(0, gx3d_TEXTURE_COLOROP_MODULATE, gx3d_TEXTURE_ARG_TEXTURE, gx3d_TEXTURE_ARG_CURRENT);
	gx3d_SetTextureAlphaOp(0, gx3d_TEXTURE_ALPHAOP_SELECTARG1, gx3d_TEXTURE_ARG_TEXTURE, 0);
	// Texture stage 1 is off by default
	gx3d_SetTextureColorOp(1, gx3d_TEXTURE_COLOROP_DISABLE, 0, 0);
	gx3d_SetTextureAlphaOp(1, gx3d_TEXTURE_ALPHAOP_DISABLE, 0, 0);

	// Set default texture coordinates
	gx3d_SetTextureCoordinates(0, gx3d_TEXCOORD_SET0);
	gx3d_SetTextureCoordinates(1, gx3d_TEXCOORD_SET1);

	// Enable trilinear texture filtering
	gx3d_SetTextureFiltering(0, gx3d_TEXTURE_FILTERTYPE_TRILINEAR, 0);
	gx3d_SetTextureFiltering(1, gx3d_TEXTURE_FILTERTYPE_TRILINEAR, 0);
}

/*____________________________________________________________________
|
| Function: Program_Free
|
| Input: Called from CMainFrame::OnClose()
| Output: Exits graphics mode.
|___________________________________________________________________*/

void Program_Free()
{
	// Stop event processing
	evStopEvents();
	// Return to text mode
	if (Pgm_system_font)
		gxFreeFont(Pgm_system_font);
	gxStopGraphics();
}
