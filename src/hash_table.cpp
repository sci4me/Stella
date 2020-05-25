u32 murmur3(void const *input, s32 len, u32 seed) {
	constexpr u32 C1 = 0xCC9E2D51;
	constexpr u32 C2 = 0x1B873593;

	u8 const *data = (u8 const*) input;
	s32 n_blocks = len / 4;

	u32 h = seed;

	u32 const *blocks = (u32 const*) (data + n_blocks * 4);
	for(s32 i = -n_blocks; i; i++) {
		u32 k = blocks[i];
		
		k *= C1;
		k = rotl32(k, 15);
		k *= C2;
		
		h ^= k;
		h = rotl32(h, 13);
		h = h * 5 + 0xE6546B64;
	}

	u8 const *tail = (u8 const*) (data + n_blocks * 4);
	u32 k = 0;
	switch(len & 3) {
		case 3: k ^= tail[2] << 16;
		case 2: k ^= tail[1] << 8;
		case 1: k ^= tail[0];
				k *= C1;
				k = rotl32(k, 15);
				k *= C2;
				h ^= k;
	}

	h ^= len;

	h ^= h >> 16;
	h *= 0x85EBCA6B;
	h ^= h >> 13;
	h *= 0xC2B2AE35;
	h ^= h >> 16;

	return h;
}


template<typename T>
u32 default_hash_fn(T const& v) {
	// NOTE: We're currently just using a fixed seed
	// theoretically we _could_ generate it randomly
	// at app-startup (or we could even be more granular
	// than that and make it unique to the hash table, but
	// meh, don't know that we need to.)
	// I just got this number off of random.org.
	//				- sci4me, 5/21/20
	constexpr u32 seed = 0xB23D66D5;
	return murmur3((void const *) &v, sizeof(T), seed);
}

template<typename T>
bool default_eq_fn(T const& a, T const& b) {
	return mlc_memcmp((void const*) &a, (void const*) &b, sizeof(T)) == 0;
}


constexpr f32 HASH_TABLE_LOAD_FACTOR_SHRINK_THRESHOLD = 0.1f;
constexpr f32 HASH_TABLE_LOAD_FACTOR_EXPAND_THRESHOLD = 0.7f;
constexpr u32 HASH_TABLE_DEFAULT_SIZE = 16;

// NOTE: Currently, we _require_ size to be a power of 2!
// Eventually, we _should_ switch to using sizes that are
// prime numbers. Probably.
// 				- sci4me, 5/21/20

template<typename K, typename V, u32 (*hash_fn)(K const&) = default_hash_fn, bool (*eq_fn)(K const&, K const&) = default_eq_fn>
struct Hash_Table {
	struct Slot {
		K key;
		V value;
		u32 hash;
	};

	u32 count;
	u32 size;
	u32 mask;
	Slot *slots;

	void init(u32 size = HASH_TABLE_DEFAULT_SIZE) {
		assert(size);
		assert((size & (size - 1)) == 0);
		
		this->count = 0;
		this->size = size;
		this->mask = size - 1;
		slots = (Slot*) mlc_malloc(size * sizeof(Slot));
		for(u32 i = 0; i < size; i++) slots[i].hash = 0;
	}

	void deinit() {
		mlc_free(slots);
	}

	void resize(u32 new_size) {
		assert(size);
		assert((size & (size - 1)) == 0);

		u32 old_count = count;
		u32 old_size = size;
		Slot *old_slots = slots;

		count = 0;
		size = new_size;
		mask = size - 1;
		slots = (Slot*) mlc_malloc(size * sizeof(Slot));
		for(u32 i = 0; i < size; i++) slots[i].hash = 0;

		for(u32 i = 0; i < old_size; i++) {
			if(old_slots[i].hash) {
				set(old_slots[i].key, old_slots[i].value);
			}
		}

		assert(count == old_count);

		mlc_free(old_slots);
	}

	void set(K _key, V _value) {
		if(load_factor() > HASH_TABLE_LOAD_FACTOR_EXPAND_THRESHOLD) {
			resize(size * 2);
		}

		K key = _key;
		V value = _value;
		u32 hash = hash_key(key);

		s32 i = hash & mask;
		s32 dist = 0;
		for(;;) {
			if(slots[i].hash == 0) {
				slots[i].key = key;
				slots[i].value = value;
				slots[i].hash = hash;
				count++;
				return;
			}

			s32 epd = (i + size - (slots[i].hash & mask)) & mask;
			if(epd < dist) {
				assert(slots[i].hash);

				K _k = slots[i].key;
				V _v = slots[i].value;
				u32 _h = slots[i].hash;

				slots[i].key = key;
				slots[i].value = value;
				slots[i].hash = hash;

				key = _k;
				value = _v;
				hash = _h;
				
				dist = epd;
			}

			i = (i + 1) & mask;
			dist++;
		}
	}

	V get(K key) {
		s32 i = index_of(key);
		if(i == -1) {
			V dummy;
			memset(&dummy, 0, sizeof(V)); // NOTE: not strictly necessary...
			return dummy;
		}
		return slots[i].value;
	}

	s32 index_of(K key) {
		u32 hash = hash_key(key);
		s32 i = hash & mask;
		u32 dist = 0;
		for(;;) {
			if(slots[i].hash == 0) {
				return -1;
			}

			s32 epd = (i + size - (hash & mask)) & mask;
			if(dist > epd) {
				return -1;
			}

			if(slots[i].hash == hash && eq_fn(slots[i].key, key)) {
				return i;
			}

			i = (i + 1) & mask;
			dist++;
		}
	}

	bool remove(K key) {
		s32 i = index_of(key);
		if(i == -1) return false;

		for(s32 j = 0; j < size - 1; j++) {
			s32 k = (i + 1) & mask;

			if(slots[k].hash == 0) break;
			
			s32 epd = (k + size - (slots[k].hash & mask)) & mask;
			if(epd == 0) break;

			mlc_memcpy(&slots[i], &slots[k], sizeof(Slot));

			i = (i + 1) & mask;
		}

		slots[i].hash = 0;
		count--;

		if(load_factor() < HASH_TABLE_LOAD_FACTOR_SHRINK_THRESHOLD) {
			resize(size / 2);
		}

		return true;
	}

	f32 load_factor() {
		return (f32) count / (f32) size;
	}

private:
	u32 hash_key(K key) {
		u32 h = hash_fn(key);
		// NOTE: a hash of 0 represents an empty slot
		if(h == 0) h |= 1;
		return h;
	}
};