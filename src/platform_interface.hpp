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


// NOTE TODO: We don't want to include GLEW in here.
// However, if we don't, we'll have to create our own
// way of calling out to GL. Even if it's still through
// GLEW. Not sure what I want to do here yet, but, for
// now it's fine to just leave it as is.
//				- sci4me, 5/28/20
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
typedef u8 Virtual_Button;
enum Virtual_Button_ : Virtual_Button {
	VK_ESC,
	VK_A, VK_B, VK_C, VK_D, VK_E, VK_F, VK_G, VK_H, GK_I, VK_J, VK_K, VK_L, VK_M, 
	VK_N, VK_O, VK_P, VK_Q, VK_R, VK_S, VK_T, VK_U, VK_V, VK_W, VK_X, VK_Y, VK_Z,
	VK_0, VK_1, VK_2, VK_3, VK_4, VK_5, VK_6, VK_7, VK_8, VK_9,
	VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
	VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,
	VK_PAGE_UP, VK_PAGE_DOWN,
	VK_INSERT, VK_DELETE,
	VK_HOME, VK_END,
	VK_BACKSPACE,
	VK_ENTER, VK_KP_ENTER,
	VK_TAB, VK_SPACE,
	VK_CTRL_LEFT, VK_CTRL_RIGHT,
	VK_SHIFT_LEFT, VK_SHIFT_RIGHT,
	VK_ALT_LEFT, VK_ALT_RIGHT,
	VK_SUPER_LEFT, VK_SUPER_RIGHT,

	VMB_LEFT, VMB_MIDDLE, VMB_RIGHT,

	VB_COUNT,

	// NOTE: For platform layer use only!
	VB_INVALID
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
	BTN_FLAG_RESERVED4  = 128
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
// Currently the only member meant to be written by the game is `game_memory`.
// The game allocates its own struct which is used to store all of the game state,
// and stores a pointer to that struct in `game_memory`. This member is only read/written
// by the game code itself.
struct PlatformIO {
	s32 window_width;
	s32 window_height;
	bool window_just_resized;
	bool window_focused;

	u8 button_state[VB_COUNT];
	f32 mouse_x, mouse_y;
	f32 mouse_wheel_x, mouse_wheel_y; // TODO: Call these like, mouse_wheel_delta_x? 

	f32 delta_time;

	void *game_memory;

	inline bool is_button_down(Virtual_Button btn) { return (button_state[btn] & BTN_FLAG_DOWN) != 0; }
	inline bool was_button_pressed(Virtual_Button btn) { return (button_state[btn] & BTN_FLAG_PRESSED) != 0; }
	inline bool was_button_released(Virtual_Button btn) { return (button_state[btn] & BTN_FLAG_RELEASED) != 0; }
};


#define GAME_INIT(name) void name(PlatformIO *pio)
typedef GAME_INIT(Game_Init);

#define GAME_DEINIT(name) void name(PlatformIO *pio)
typedef GAME_DEINIT(Game_Deinit);

#define GAME_UPDATE_AND_RENDER(name) void name(PlatformIO *pio)
typedef GAME_UPDATE_AND_RENDER(Game_Update_And_Render);


#endif