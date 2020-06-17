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


#include "common.hpp"
#include "opengl.hpp"


#if defined(STELLA_STATIC) && defined(STELLA_DYNAMIC)
#error "STELLA_STATIC may not be defined allongside STELLA_DYNAMIC; pick one or the other"
#elif !defined(STELLA_STATIC) && !defined(STELLA_DYNAMIC)
#error "STELLA_DYNAMIC or STELLA_STATIC must be defined!"
#endif


// NOTE TODO: It feels weird to have these
// be defined in here.......
#define GL_MAJOR 4
#define GL_MINOR 4
#define APP_NAME "Stella"

#define GL_DEBUG


// TODO: Instead of having malloc, calloc, realloc, and free,
// we just want to have alloc and free.
#define _PLATFORM_API_FUNCTIONS(X) 								  \
	X(mlc_alloc, 			void*, 		(u64)					) \
	X(mlc_free, 			void, 		(void*)					) \
	X(mlc_fwrite, 			void, 		(s32, char const*)		) \
	X(mlc_exit, 			void,	 	(s32)					) \
	X(nanotime, 			u64, 		()						) \
	X(read_entire_file, 	Buffer, 	(char const*)			)

#ifdef STELLA_DYNAMIC
extern "C" {
	#define _X(ident, ret, params) typedef ret ident##_fn params;
	_PLATFORM_API_FUNCTIONS(_X)
	#undef _X
}

#define _X_PLATFORM_API_FUNCTIONS(ident, ret, params) ident##_fn *ident;
#define PLATFORM_API_FUNCTIONS _PLATFORM_API_FUNCTIONS(_X_PLATFORM_API_FUNCTIONS)

struct PlatformAPI {
	PLATFORM_API_FUNCTIONS
};
#else
#define _X_PLATFORM_API_FUNCTIONS(ident, ret, params) extern "C" ret ident params;
#define PLATFORM_API_FUNCTIONS _PLATFORM_API_FUNCTIONS(_X_PLATFORM_API_FUNCTIONS)
#endif


// NOTE: Virtual_Key is our "virtual" representation of all the different
// keyboard keys that we want to be able to keep track of, etc.
// When indexing the `key_state` array in PlatformIO, use these as the indices.
// TODO: Make this more complete!
typedef u8 Virtual_Button;
enum Virtual_Button_ : Virtual_Button {
	VB_ESC,
	VB_A, VB_B, VB_C, VB_D, VB_E, VB_F, VB_G, VB_H, GK_I, VB_J, VB_K, VB_L, VB_M, 
	VB_N, VB_O, VB_P, VB_Q, VB_R, VB_S, VB_T, VB_U, VB_V, VB_W, VB_X, VB_Y, VB_Z,
	VB_0, VB_1, VB_2, VB_3, VB_4, VB_5, VB_6, VB_7, VB_8, VB_9,
	VB_F1, VB_F2, VB_F3, VB_F4, VB_F5, VB_F6, VB_F7, VB_F8, VB_F9, VB_F10, VB_F11, VB_F12,
	VB_LEFT, VB_RIGHT, VB_UP, VB_DOWN,
	VB_PAGE_UP, VB_PAGE_DOWN,
	VB_INSERT, VB_DELETE,
	VB_HOME, VB_END,
	VB_BACKSPACE,
	VB_ENTER, VB_KP_ENTER,
	VB_TAB, VB_SPACE,
	VB_CTRL_LEFT, VB_CTRL_RIGHT,
	VB_SHIFT_LEFT, VB_SHIFT_RIGHT,
	VB_ALT_LEFT, VB_ALT_RIGHT,
	VB_SUPER_LEFT, VB_SUPER_RIGHT,

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
	#ifdef STELLA_DYNAMIC
	PlatformAPI api;
	#endif

	s32 window_width;
	s32 window_height;
	bool window_just_resized;
	bool window_focused;

	u8 button_state[VB_COUNT];
	f32 mouse_x, mouse_y;
	f32 mouse_wheel_x, mouse_wheel_y; // TODO: Call these like, mouse_wheel_delta_x? 

	f32 delta_time;

	void *game_memory;

	OpenGL gl;

	// TODO: remove the `_button`?
	inline bool is_button_down(Virtual_Button btn) { return (button_state[btn] & BTN_FLAG_DOWN) != 0; }
	inline bool was_button_pressed(Virtual_Button btn) { return (button_state[btn] & BTN_FLAG_PRESSED) != 0; }
	inline bool was_button_released(Virtual_Button btn) { return (button_state[btn] & BTN_FLAG_RELEASED) != 0; }
};


#ifdef STELLA_DYNAMIC
#define GAME_ATTACH(name) void name(PlatformIO *pio, bool reload)
typedef GAME_ATTACH(Game_Attach);
#endif

#define GAME_INIT(name) void name(PlatformIO *pio)
typedef GAME_INIT(Game_Init);

#define GAME_DEINIT(name) void name(PlatformIO *pio)
typedef GAME_DEINIT(Game_Deinit);

#define GAME_UPDATE_AND_RENDER(name) void name(PlatformIO *pio)
typedef GAME_UPDATE_AND_RENDER(Game_Update_And_Render);


#endif