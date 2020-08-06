#define ALLOCATOR_ALLOC_FN(name) void* name(u64 n)
typedef ALLOCATOR_ALLOC_FN(Alloc_Fn);

#define ALLOCATOR_FREE_FN(name) void name(void* p)
typedef ALLOCATOR_FREE_FN(Free_Fn);


struct Allocator {
	Alloc_Fn *alloc;
	Free_Fn *free;
};


static Allocator _allocator;

void init_allocator(Alloc_Fn *alloc, Free_Fn *free) {
	_allocator.alloc = alloc;
	_allocator.free = free;
}


// TODO: Yuck
extern "C" void* mlc_memset(void*, s32, u64);

struct Arena {
	u8 *data;
	u64 used;
	u64 size;

	void init(u64 size) {
		data = (u8*) _allocator.alloc(size);
		used = 0;
		this->size = size;
	}

	void deinit() {
		_allocator.free(data);
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