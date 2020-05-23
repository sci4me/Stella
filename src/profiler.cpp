// NOTE: We may want to switch to rdtsc;
// things like clock_gettime are likely
// much more expensive?

// NOTE: 1. this is not cross-platform
//       2. we should (?) put it elsewhere
u64 get_time_ns() {
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	return t.tv_sec * 1000000000LLU + t.tv_nsec;
}

namespace prof {
	enum Debug_Event_Type {
		DEBUG_EVENT_BLOCK_START,
		DEBUG_EVENT_BLOCK_END
	};

	struct Debug_Event {
		Debug_Event_Type type;

		char *guid;
		char *name;
		char *file;
		u32 line;
		
		union {
			u64 time_ns;
		};
	};


	constexpr u32 MAX_DEBUG_EVENTS = 64 * 1024 * 64;
	Static_Array<Debug_Event, MAX_DEBUG_EVENTS> frame_events;


	struct Timed_Block {
		char *guid;
		char *name;
		char *file;
		u32 line;

		Timed_Block(char *guid, char *name, char *file, u32 line) {
			this->guid = guid;
			this->name = name;
			this->file = file;
			this->line = line;

			Debug_Event e;
			e.type = DEBUG_EVENT_BLOCK_START;
			e.guid = guid;
			e.name = name;
			e.file = file;
			e.line = line;
			e.time_ns = get_time_ns();
			frame_events.push(e);
		}

		~Timed_Block() {
			Debug_Event e;
			e.type = DEBUG_EVENT_BLOCK_END;
			e.guid = guid;
			e.name = name;
			e.file = file;
			e.line = line;
			e.time_ns = get_time_ns();
			frame_events.push(e);
		}
	};


	struct Block_Profile {
		char *guid;
		char *name;
		char *file;
		u32 line;

		u32 count;
		u64 time_ns;

		Dynamic_Array<struct Block_Profile*> children;

		void init(char *guid, char *name, char *file, u32 line) {
			this->guid = guid;
			this->name = name;
			this->file = file;
			this->line = line;

			count = 0;
			time_ns = 0;

			children.init();
		}

		void deinit() {
			for(u32 i = 0; i < children.count; i++) {
				children[i]->deinit();
				free(children[i]);
			}

			children.deinit();
		}
	};

	struct Frame_Profile {
		Dynamic_Array<Block_Profile*> block_profiles;

		void init() {
			block_profiles.init();
		}

		void deinit() {
			block_profiles.deinit();
		}
	};


	constexpr u32 MAX_FRAME_PROFILES = 60;
	Frame_Profile frame_profiles[MAX_FRAME_PROFILES];
	u32 frame_profile_index = 0;
	u32 selected_frame_profile_index = 0;


	void init() {
		for(u32 i = 0; i < MAX_FRAME_PROFILES; i++) frame_profiles[i].init();
	}

	void deinit() {
		for(u32 i = 0; i < MAX_FRAME_PROFILES; i++) frame_profiles[i].deinit();
	}

	void begin_frame() {
		frame_profile_index++;
		if(frame_profile_index >= MAX_FRAME_PROFILES) frame_profile_index = 0;
	}

	void end_frame() {
		// NOTE: We are using char* as an 8-byte value here!
		// This is _not_ a "string hash table"!
		// This is _only_ okay because we know that these
		// are actually compile-time constant strings, always!
		// Be careful!
		//					- sci4me, 5/22/20
		Hash_Table<char*, Block_Profile*> block_profiles;
		block_profiles.init();

		Dynamic_Array<Block_Profile*> block_profile_stack;
		block_profile_stack.init();

		Block_Profile *current_block_profile = nullptr;

		// TODO: sort blocks & block_profiles (not the Hash_Table obviously!)
		Frame_Profile& fp = frame_profiles[frame_profile_index];

		for(u32 i = 0; i < fp.block_profiles.count; i++) {
			fp.block_profiles[i]->deinit();
			free(fp.block_profiles[i]);
		}
		fp.block_profiles.clear();

		for(u32 event_index = 0; event_index < frame_events.count; event_index++) {
			auto const& event = frame_events[event_index];

			switch(event.type) {
				case DEBUG_EVENT_BLOCK_START: {
					Block_Profile *this_block_profile;
					auto bpi = block_profiles.index_of(event.guid);
					if(bpi == -1) {
						this_block_profile = (Block_Profile*) malloc(sizeof(Block_Profile));
						this_block_profile->init(event.guid, event.name, event.file, event.line);

						if(current_block_profile) {
							current_block_profile->children.push(this_block_profile);
						} else {
							fp.block_profiles.push(this_block_profile);
						}

						block_profiles.set(event.guid, this_block_profile);
					} else {
						this_block_profile = block_profiles.slots[bpi].value;
					}

					block_profile_stack.push(current_block_profile);
					current_block_profile = this_block_profile;

					current_block_profile->time_ns -= event.time_ns;
					break;
				}
				case DEBUG_EVENT_BLOCK_END: {
					current_block_profile->time_ns += event.time_ns;
					current_block_profile->count++;

					current_block_profile = block_profile_stack.pop();
					break;
				}
				default:
					assert(0);
					break;
			}
		}

		assert(block_profile_stack.count == 0);
		assert(current_block_profile == nullptr);

		block_profiles.deinit();
		block_profile_stack.deinit();

		frame_events.clear();

		selected_frame_profile_index = frame_profile_index;
	}

	char* format_ns(u64 ns) {
		f32 us = (f32)ns / 1000.0f;
		f32 ms = (f32)ns / 1000000.0f;

		if(ms > 1) {
			u64 len = snprintf(nullptr, 0, "%0.3f ms", ms);
			char *r = (char*) talloc(len + 1);
			snprintf(r, len + 1, "%0.3f ms", ms);
			return r;
		} else if(us > 1) {
			u64 len = snprintf(nullptr, 0, "%0.3f us", us);
			char *r = (char*) talloc(len + 1);
			snprintf(r, len + 1, "%0.3f us", us);
			return r;
		} else {
			u64 len = snprintf(nullptr, 0, "%llu ns", ns);
			char *r = (char*) talloc(len + 1);
			snprintf(r, len + 1, "%llu ns", ns);
			return r;
		}
	}

	void show_block_profile(Block_Profile *bp) {
		if(ImGui::TreeNode(bp->guid, "%s", bp->name)) {
			ImGui::Text("Count: %u", bp->count);
			ImGui::Text("Time: %s", format_ns(bp->time_ns));

			for(u32 i = 0; i < bp->children.count; i++) {
				show_block_profile(bp->children[i]);
			}

			ImGui::TreePop();
		}
	}

	void show_frames() {
		auto window_pos = ImGui::GetWindowPos();
		auto mouse_pos = ImGui::GetMousePos();
		auto dl = ImGui::GetWindowDrawList();

		constexpr f32 FRAME_WIDTH = 12.0f;
		constexpr f32 FRAME_HEIGHT = 40.0f;
		constexpr vec2 OFFSET = vec2(10.0f, 30.0f);

		vec2 min = window_pos + OFFSET;
		vec2 max = min + vec2(FRAME_WIDTH * MAX_FRAME_PROFILES, 0) + vec2(0, FRAME_HEIGHT);

		ImGui::Dummy(max - min + vec2(10.0f, 10.0f));

		for(u32 i = 0; i < MAX_FRAME_PROFILES; i++) {
			vec2 s = min + vec2(i * FRAME_WIDTH, 0);
			vec2 e = s + vec2(FRAME_WIDTH, FRAME_HEIGHT);

			if(i == frame_profile_index) dl->AddRectFilled(s, e, 0xFF0000FF);
			else if(i == selected_frame_profile_index) dl->AddRectFilled(s, e, 0xFFFF0000);

			if(mouse_pos.x >= s.x && mouse_pos.x < e.x && mouse_pos.y >= s.y && mouse_pos.y < e.y) {
				dl->AddRectFilled(s, e, 0x66FFFFFF);

				if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					selected_frame_profile_index = i;
				}
			}

			if(i == 0) continue;

			vec2 ls = min + vec2(i * FRAME_WIDTH, 0);
			vec2 le = s + vec2(0, FRAME_HEIGHT - 1);
			dl->AddLine(ls, le, 0xFFFFFFFF);
		}

		dl->AddRect(min, max, 0xFFFFFFFF);
	}

	void show() {
		if(ImGui::Begin("Profiler", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav)) {
			show_frames();

			ImGui::Separator();

			Frame_Profile& fp = frame_profiles[selected_frame_profile_index]; // NOTE: - 1 because we increment after write
			for(u32 i = 0; i < fp.block_profiles.count; i++) {
				show_block_profile(fp.block_profiles[i]);
			}
		}
		ImGui::End();
	}
}


#define DEBUG_NAME__(a, b) a "|" #b
#define DEBUG_NAME_(a, b) DEBUG_NAME__(a, b)
#define DEBUG_NAME() DEBUG_NAME_(__FILE__, __LINE__)
#define TIMED_BLOCK_(name, file, line) prof::Timed_Block __timed_block_##LINE__((char*)DEBUG_NAME(), (char*)name, (char*)file, line);
#define TIMED_BLOCK(name) TIMED_BLOCK_(name, __FILE__, __LINE__)

// NOTE TODO: __PRETTY_FUNCTION__ is compiler-specific; we'll
// have to use uh.. __FUNCSIG__ on MSVC, I believe.
#define TIMED_FUNCTION() TIMED_BLOCK_(__PRETTY_FUNCTION__, __FILE__, __LINE__)