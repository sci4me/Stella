struct Arena {
	u8 *data;
	u64 used;
	u64 size;

	void init(u64 size) {
		data = (u8*) mlc_alloc(size);
		used = 0;
		this->size = size;
	}

	void deinit() {
		mlc_free(data);
	}

	void* alloc(u64 x, u64 alignment = 8) {
		u64 n = (x + (alignment - 1)) & ~(alignment - 1);
		u64 new_used = used + n;

		if(new_used > size) {
			// TODO ?
			assert(0);
			return 0;
		}

		void *result = (void*) (data + used);
		used = new_used;
		return result;
	}

	template<typename T>
	T* alloc_new() {
		void *p = alloc(sizeof(T), alignof(T));
		return new(p) T;
	}

	void clear() {
		mlc_memset(data, 0, size);
		used = 0;
	}
};