namespace prof {
	constexpr u32 MAX_DEBUG_EVENTS = 64 * 1024 * 64;
	constexpr u32 MAX_FRAME_PROFILES = 60;
}


#ifndef PROFILER_DISABLE

// TODO: Switch to rdtsc

// TODO: THREAD SAFETY!!@!!!!

// TODO: Use an arena for allocation Block_Profiles; 
// it's a simple linear-allocate and reset pattern, per-frame.

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


	struct Block_Profile {
		char *guid;
		char *name;
		char *file;
		u32 line;

		u32 count;
		u64 time_ns;
		u64 child_time_ns;

		Dynamic_Array<struct Block_Profile*> children;

		void init(char *guid, char *name, char *file, u32 line) {
			this->guid = guid;
			this->name = name;
			this->file = file;
			this->line = line;

			count = 0;
			time_ns = 0;
			child_time_ns = 0;

			children.init();
		}

		void deinit() {
			for(u32 i = 0; i < children.count; i++) {
				children[i]->deinit();
				mlc_free(children[i]);
			}

			children.deinit();
		}

		void calculate_child_time() {
			child_time_ns = 0;

			for(u32 i = 0; i < children.count; i++) {
				children[i]->calculate_child_time();
				child_time_ns += children[i]->time_ns;
			}
		}
	};

	struct Frame_Profile {
		u64 frame;
		Dynamic_Array<Block_Profile*> block_profiles;

		void init() {
			block_profiles.init();
		}

		void deinit() {
			block_profiles.deinit();
		}
	};


	struct Profiler {
		Static_Array<Debug_Event, MAX_DEBUG_EVENTS> frame_events;
		Frame_Profile frame_profiles[MAX_FRAME_PROFILES];
		u32 frame_profile_index = 0;
		u32 selected_frame_profile_index = 0;
		u64 frame_count = 0;


		void init() {
			for(u32 i = 0; i < MAX_FRAME_PROFILES; i++) frame_profiles[i].init();
		}

		void deinit() {
			for(u32 i = 0; i < MAX_FRAME_PROFILES; i++) frame_profiles[i].deinit();
		}
	};	

	void clear_frame_events() {
		g_inst->profiler->frame_events.clear();
	}

	void begin_frame() {
		Profiler *p = g_inst->profiler;
		p->frame_profile_index++;
		if(p->frame_profile_index >= MAX_FRAME_PROFILES) p->frame_profile_index = 0;
	}

	s32 _sort_block_profiles_comparator(Block_Profile* const& a, Block_Profile* const& b) {
		if(a->time_ns < b->time_ns) return  1;
		if(a->time_ns > b->time_ns) return -1;
		return 0;
	}

	void sort_block_profiles(Dynamic_Array<Block_Profile*>& bps) {
		for(u32 i = 0; i < bps.count; i++) {
			Block_Profile *bp = bps[i];
			sort_block_profiles(bp->children);
		}

		bps.qsort(_sort_block_profiles_comparator);
	}

	void end_frame() {
		Profiler *p = g_inst->profiler;

		// NOTE TODO BUG: We don't distinguish between calls to the same function from different callsites!!!!

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
		Frame_Profile& fp = p->frame_profiles[p->frame_profile_index];
		fp.frame = p->frame_count;

		for(u32 i = 0; i < fp.block_profiles.count; i++) {
			fp.block_profiles[i]->deinit();
			mlc_free(fp.block_profiles[i]);
		}
		fp.block_profiles.clear();

		for(u32 event_index = 0; event_index < p->frame_events.count; event_index++) {
			auto const& event = p->frame_events[event_index];

			switch(event.type) {
				case DEBUG_EVENT_BLOCK_START: {
					Block_Profile *this_block_profile;
					auto bpi = block_profiles.index_of(event.guid);
					if(bpi == -1) {
						this_block_profile = (Block_Profile*) mlc_malloc(sizeof(Block_Profile));
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

		for(u32 i = 0; i < fp.block_profiles.count; i++) {
			fp.block_profiles[i]->calculate_child_time();
		}

		sort_block_profiles(fp.block_profiles);

		block_profiles.deinit();
		block_profile_stack.deinit();

		p->selected_frame_profile_index = p->frame_profile_index;

		p->frame_count++;

		clear_frame_events();
	}

	char* format_ns(u64 ns) {
		f32 us = (f32)ns / 1000.0f;
		f32 ms = (f32)ns / 1000000.0f;

		if(ms > 1) {
			return tsprintf("%0.3f ms", ms);
		} else if(us > 1) {
			return tsprintf("%0.3f us", us);
		} else {
			return tsprintf("%llu ns", ns);
		}
	}

	void show_block_profile(Block_Profile *bp) {
		if(ImGui::TreeNode(bp->guid, "%s", bp->name)) {
			ImGui::Text("Count: %u", bp->count);

			ImGui::Columns(2);
				ImGui::Text("Time: %s", format_ns(bp->time_ns));
				ImGui::Text("Self: %s", format_ns(bp->time_ns - bp->child_time_ns));
				ImGui::Text("Child: %s", format_ns(bp->child_time_ns));
				ImGui::NextColumn();
				ImGui::Text("Time/Count: %s", format_ns(bp->time_ns / bp->count));
				ImGui::Text("Self/Count: %s", format_ns((bp->time_ns - bp->child_time_ns) / bp->count));
				ImGui::Text("Child/Count: %s", format_ns(bp->child_time_ns / bp->count));
			ImGui::Columns(1);

			for(u32 i = 0; i < bp->children.count; i++) {
				show_block_profile(bp->children[i]);
			}

			ImGui::TreePop();
		}
	}

	void show_frames() {
		Profiler *p = g_inst->profiler;

		// TODO: Figure out how to make this a proper
		// imgui component instead of just using ImGui::Dummy
		// Doing it the way we are now prevents scrolling from
		// working correctly (er, at all.)

		auto window_pos = ImGui::GetWindowPos();
		auto mouse_pos = ImGui::GetMousePos();
		auto dl = ImGui::GetWindowDrawList();

		constexpr f32 FRAME_WIDTH = 12.0f;
		constexpr f32 FRAME_HEIGHT = 40.0f;
		constexpr vec2 OFFSET = vec2(10.0f, 30.0f);

		vec2 min = vec2(window_pos) + OFFSET;
		vec2 max = min + vec2(FRAME_WIDTH * MAX_FRAME_PROFILES, 0) + vec2(0, FRAME_HEIGHT);

		ImGui::Dummy(max - min + vec2(10.0f, 10.0f));

		for(u32 i = 0; i < MAX_FRAME_PROFILES; i++) {
			vec2 s = min + vec2(i * FRAME_WIDTH, 0);
			vec2 e = s + vec2(FRAME_WIDTH, FRAME_HEIGHT);

			if(i == p->frame_profile_index) dl->AddRectFilled(s, e, 0xFF0000FF);
			else if(i == p->selected_frame_profile_index) dl->AddRectFilled(s, e, 0xFFFF0000);
			else if(i < p->frame_profile_index) dl->AddRectFilled(s, e, 0xFF00FF00);
			else if(i > p->frame_profile_index) dl->AddRectFilled(s, e, 0xFF00FFFF);

			if(mouse_pos.x >= s.x && mouse_pos.x < e.x && mouse_pos.y >= s.y && mouse_pos.y < e.y) {
				dl->AddRectFilled(s, e, 0x66FFFFFF);

				if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					p->selected_frame_profile_index = i;
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

			Profiler *p = g_inst->profiler;
			Frame_Profile& fp = p->frame_profiles[p->selected_frame_profile_index]; // NOTE: - 1 because we increment after write
			ImGui::Text("Frame: %llu", fp.frame);
			for(u32 i = 0; i < fp.block_profiles.count; i++) {
				show_block_profile(fp.block_profiles[i]);
			}
		}
		ImGui::End();
	}

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
			e.time_ns = nanotime();
			g_inst->profiler->frame_events.push(e);
		}

		~Timed_Block() {
			Debug_Event e;
			e.type = DEBUG_EVENT_BLOCK_END;
			e.guid = guid;
			e.name = name;
			e.file = file;
			e.line = line;
			e.time_ns = nanotime();
			g_inst->profiler->frame_events.push(e);
		}
	};
}


#define DEBUG_NAME__(a, b) a "|" #b
#define DEBUG_NAME_(a, b) DEBUG_NAME__(a, b)
#define DEBUG_NAME() DEBUG_NAME_(__FILE__, __LINE__)
#define TIMED_BLOCK_(name, file, line) prof::Timed_Block __timed_block_##LINE__((char*)DEBUG_NAME(), (char*)name, (char*)file, line);
#define TIMED_BLOCK(name) TIMED_BLOCK_(name, __FILE__, __LINE__)

// NOTE TODO: __PRETTY_FUNCTION__ is compiler-specific; we'll
// have to use uh.. __FUNCSIG__ on MSVC, I believe.
#define TIMED_FUNCTION() TIMED_BLOCK_(__PRETTY_FUNCTION__, __FILE__, __LINE__)

#else

namespace prof {
	void clear_frame_events() {}
	void begin_frame() {}
	void end_frame() {}
	void show() {}
}

#define TIMED_FUNCTION()

#endif