constexpr u64 TEMPORARY_STORAGE_SIZE = 1024 * 64;
constexpr u64 TEMPORARY_STORAGE_ALIGNMENT = 8;

char temporary_storage_buffer[TEMPORARY_STORAGE_SIZE];
u64 temporary_storage_used = 0;

void* talloc(u64 x) {
	u64 n = (x + (TEMPORARY_STORAGE_ALIGNMENT - 1)) & ~(TEMPORARY_STORAGE_ALIGNMENT - 1);
	u64 new_used = temporary_storage_used + n;
	
	if(new_used > TEMPORARY_STORAGE_SIZE) {
		// TODO ?
		tprintf("Failed talloc for %llu bytes! :(\n", x);
		return 0;
	}

	void *result = (void*) (temporary_storage_buffer + temporary_storage_used);
	temporary_storage_used = new_used;
	return result;
}

void treset() {
	temporary_storage_used = 0;
}