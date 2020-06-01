constexpr u32 DYNAMIC_ARRAY_DEFAULT_SIZE = 16;

template<typename T>
struct Dynamic_Array {
	u32 count;
	u32 size;
	T *data;

	void init(u32 size = DYNAMIC_ARRAY_DEFAULT_SIZE) {
		this->count = 0;
		this->size = size;
		this->data = (T*) mlc_malloc(size * sizeof(T));
	}

	void deinit() {
		mlc_free(data);
	}

	void resize(u32 size) {
		if(size < DYNAMIC_ARRAY_DEFAULT_SIZE) return;

		assert(size >= count);
		
		data = (T*) mlc_realloc(data, size * sizeof(T));
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

    // TODO: Generalize this so we can use it on Static_Array, and,
    // ideally, just with "plain-old-C-arrays", if you will.
    // i.e. mlc_qsort, which, currently, appears to be 100% borked.
    void qsort(s32 (*compar)(T const&, T const&)) {
    	if(count <= 1) return;
    	_qsort(0, count - 1, compar);
    }

    void swap(s32 i, s32 j) {
    	const u64 N = sizeof(T);
    	void *temp = talloc(N);
    	mlc_memcpy(temp, &data[i], N);
    	mlc_memcpy(&data[i], &data[j], N);
    	mlc_memcpy(&data[j], temp, N);
    }

private:
	s32 _qsort_partition(s32 l, s32 h, s32 (*compar)(T const&, T const&)) {
		T const& pivot = data[h];
		s32 i = l - 1;

		for(s32 j = l; j < h; j++) {
			T const& b = data[j];
			if(compar(b, pivot) <= 0) {
				i++;
				swap(i, j);
			}
		}

		swap(i + 1, h);
		return i + 1;
	}

	void _qsort(s32 p, s32 r, s32 (*compar)(T const&, T const&)) {
		if(p < r) {
			s32 q = _qsort_partition(p, r, compar);
			_qsort(p, q - 1, compar);
			_qsort(q + 1, r, compar);
		}
	}
};