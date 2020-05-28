// NOTE: Currently this depends on our standard numeric types
// being defined before this file is included.

// NOTE: We explicitly do the ifdef version of pragma once here
// whereas most of the code never does any such thing;
// This isn't strictly necessary at the time of writing this note,
// but it likely will be if I can ever figure out how to structure
// the code such that I can either do dynamic loading or
// link it all together statically when doing a relase build.
//					- sci4me, 5/28/20

#ifndef PLATFORM_INTERFACE_H
#define PLATFORM_INTERFACE_H


#include "types.hpp"


#define GLEW_STATIC
#define GLEW_NO_GLU
#include "GL/glew.h"


#define GL_MAJOR 4
#define GL_MINOR 4
#define APP_NAME "Stella"



// TODO: Remove these defines!
#define KEYCODE_W           25
#define KEYCODE_A           38
#define KEYCODE_S           39
#define KEYCODE_D           40
#define KEYCODE_Q           24
#define KEYCODE_E           26
#define KEYCODE_UP          111
#define KEYCODE_DOWN        116
#define KEYCODE_LEFT        113
#define KEYCODE_RIGHT       114
#define KEYCODE_ESCAPE      9
#define KEYCODE_ENTER       36
#define KEYCODE_SPACE       65
#define KEYCODE_P           33
#define KEYCODE_L           46
#define KEYCODE_C 			54
#define KEYCODE_SHIFT_L     50
#define KEYCODE_SHIFT_R     62
#define KEYCODE_CTRL_L      37
#define KEYCODE_CTRL_R      105
#define KEYCODE_ALT_L       64
#define KEYCODE_ALT_R       108
#define KEYCODE_SUPER       133
#define KEYCODE_PLUS        21
#define KEYCODE_MINUS       20
#define KEYCODE_F1          67
#define KEYCODE_F2          68
#define KEYCODE_F3          69
#define KEYCODE_F4          70
#define KEYCODE_F10         76
#define KEYCODE_F11         95
#define KEYCODE_F12         96
#define KEYCODE_ESC 		9



// NOTE: Virtual_Key is our "virtual" representation of all the different
// keyboard keys that we want to be able to keep track of, etc.
// When indexing the `key_state` array in PlatformIO, use these as the indices.
// TODO: Make this more complete!
enum Virtual_Key {
	VK_ESC,

	VK_A, VK_B, VK_C, VK_D, VK_E, VK_F, VK_G, VK_H, GK_I, VK_J, VK_K, VK_L, VK_M, 
	VK_N, VK_O, VK_P, VK_Q, VK_R, VK_S, VK_T, VK_U, VK_V, VK_W, VK_X, VK_Y, VK_Z,

	VK_0, VK_1, VK_2, VK_3, VK_4, VK_5, VK_6, VK_7, VK_8, VK_9,

	VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12
};

// NOTE: Same as above.
enum Virtual_Mouse_Button {
	VMB_LEFT,
	VMB_MIDDLE,
	VMB_RIGHT,
	VMB_EXT0,
	VMB_EXT1,

	VMB_COUNT
};

// NOTE: The PlatformIO struct is the central interface point
// between the game and the platform layer. It is intended to be a
// 100% cross-platform interface that any platform layer can implement
// in order to be able to run the game on top of it.
//
// It contains members that are intended to be written to and/or read from
// in certain context. For example, the game should never write to `key_state`,
// or any of the other user input members.
//
// Currently there are no members intended to be written by the game, but
// there probably will be eventually™.
struct PlatformIO {
	s32 window_width;
	s32 window_height;
	bool window_just_resized;

	bool key_state[512]; // NOTE: Just picked a large number here; how many do we really want?
	bool mouse_button_state[VMB_COUNT];
	f32 mouse_x, mouse_y;

	f32 delta_time; // NOTE TODO: Make sure this is "well-defined"

	void *game_memory;
};


#define GAME_INIT(name) void name(PlatformIO *pio)
typedef GAME_INIT(Game_Init);

#define GAME_DEINIT(name) void name(PlatformIO *pio)
typedef GAME_DEINIT(Game_Deinit);

#define GAME_UPDATE_AND_RENDER(name) void name(PlatformIO *pio)
typedef GAME_UPDATE_AND_RENDER(Game_Update_And_Render);


#endif