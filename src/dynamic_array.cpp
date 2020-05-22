constexpr u32 DYNAMIC_ARRAY_DEFAULT_SIZE = 16;

template<typename T>
struct Dynamic_Array {
	u32 count;
	u32 size;
	T *data;

	void init(u32 size = DYNAMIC_ARRAY_DEFAULT_SIZE) {
		this->count = 0;
		this->size = size;
		this->data = (T*) malloc(size * sizeof(T));
	}

	void deinit() {
		free(data);
	}

	void resize(u32 size) {
		if(size < DYNAMIC_ARRAY_DEFAULT_SIZE) return;

		assert(size >= count);
		
		data = (T*) realloc(data, size * sizeof(T));
		assert(data);
		
		this->size = size;
	}

	void clear() {
		count = 0;
	}

	void push(T v) {
		if(count == size) {
			resize(size * 2);
		}

		data[count++] = v;
	}

	T pop() {
		assert(count > 0);
		return data[--count];
	}

	void ordered_remove(u32 i) {
		assert(i < count);
		for(u32 j = i; j < count - 1; j++) {
			data[j] = data[j + 1];
		}
		count--;
	}

	void unordered_remove(u32 i) {
		assert(i < count);
		data[i] = data[count - 1];
		count--;
	}

	T& operator[](s32 x) { return data[x]; }
    T const& operator[](s32 x) const { return data[x]; }
};