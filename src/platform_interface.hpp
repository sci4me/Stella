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

typedef u8 Button_Flags;
enum Button_Flags_ : Button_Flags {
	BTN_FLAG_NONE		= 0,

	BTN_FLAG_DOWN		= 1,
	BTN_FLAG_PRESSED 	= 2,
	BTN_FLAG_RELEASED 	= 4,

	BTN_FLAG_RESERVED0	= 8,
	BTN_FLAG_RESERVED1	= 16,
	BTN_FLAG_RESERVED2	= 32,
	BTN_FLAG_RESERVED3	= 64,

	// NOTE: Meant for internal use only!
	BTN_FLAG_LAST_DOWN  = 128
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

	u8 key_state[512]; // NOTE: Just picked a large number here; how many do we really want?
	u8 mouse_button_state[VMB_COUNT];
	f32 mouse_x, mouse_y;
	f32 mouse_wheel_x, mouse_wheel_y;

	f32 delta_time; // NOTE TODO: Make sure this is "well-defined"

	void *game_memory;

	inline bool is_key_down(Virtual_Key key) { return (key_state[key] & BTN_FLAG_DOWN) != 0; }
	inline bool is_key_pressed(Virtual_Key key) { return (key_state[key] & BTN_FLAG_PRESSED) != 0; }
	inline bool is_key_released(Virtual_Key key) { return (key_state[key] & BTN_FLAG_RELEASED) != 0; }
	inline bool is_mouse_button_down(Virtual_Mouse_Button key) { return (mouse_button_state[key] & BTN_FLAG_DOWN) != 0; }
	inline bool is_mouse_button_pressed(Virtual_Mouse_Button key) { return (mouse_button_state[key] & BTN_FLAG_PRESSED) != 0; }
	inline bool is_mouse_button_released(Virtual_Mouse_Button key) { return (mouse_button_state[key] & BTN_FLAG_RELEASED) != 0; }
};


#define GAME_INIT(name) void name(PlatformIO *pio)
typedef GAME_INIT(Game_Init);

#define GAME_DEINIT(name) void name(PlatformIO *pio)
typedef GAME_DEINIT(Game_Deinit);

#define GAME_UPDATE_AND_RENDER(name) void name(PlatformIO *pio)
typedef GAME_UPDATE_AND_RENDER(Game_Update_And_Render);


#endif