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
	VMB_EXT1
}

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
	bool key_state[512]; // NOTE: Just picked a large number here; how many do we really want?
	bool mouse_button_state[5]; // NOTE: Just picked 5 because I saw it done that way elsewhere...
	f32 mouse_x, mouse_y;

	f32 delta_time; // NOTE TODO: Make sure this is "well-defined"
};

