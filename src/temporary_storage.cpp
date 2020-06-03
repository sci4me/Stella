constexpr u64 TEMPORARY_STORAGE_SIZE = 1024 * 64;
constexpr u64 TEMPORARY_STORAGE_ALIGNMENT = 8;

// TODO: Make this thread-safe!

struct Temporary_Storage {
	static constexpr u64 SIZE = 1024 * 64;
	static constexpr u64 ALIGNMENT = 8;

	u8 data[TEMPORARY_STORAGE_SIZE];
	u64 used;
	u64 _mark;

	void* alloc(u64 x) {
		u64 n = (x + (TEMPORARY_STORAGE_ALIGNMENT - 1)) & ~(TEMPORARY_STORAGE_ALIGNMENT - 1);
		u64 new_used = used + n;
		
		if(new_used > TEMPORARY_STORAGE_SIZE) {
			// TODO ?
			assert(0);
			return 0;
		}

		void *result = (void*) (data + used);
		used = new_used;
		return result;
	}

	void clear() {
		used = 0;
	}

	void mark() {
		_mark = used;
	}

	void reset() {
		used = _mark;
	}
};

void* talloc(u64 n) { return g_inst->temp->alloc(n); }
void tclear() { return g_inst->temp->clear(); }
void tmark() { return g_inst->temp->mark(); }
void treset() { return g_inst->temp->reset(); }