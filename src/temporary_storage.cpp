#include "temporary_storage.hpp"

char temporary_storage_buffer[TEMPORARY_STORAGE_SIZE];
u64 temporary_storage_used = 0;
u64 temporary_storage_mark = 0;

void* talloc(u64 x) {
	u64 n = (x + (TEMPORARY_STORAGE_ALIGNMENT - 1)) & ~(TEMPORARY_STORAGE_ALIGNMENT - 1);
	u64 new_used = temporary_storage_used + n;
	
	if(new_used > TEMPORARY_STORAGE_SIZE) {
		// TODO ?
		return 0;
	}

	void *result = (void*) (temporary_storage_buffer + temporary_storage_used);
	temporary_storage_used = new_used;
	return result;
}

void tclear() {
	temporary_storage_used = 0;
}

void tmark() {
	temporary_storage_mark = temporary_storage_used;
}

void treset() {
	temporary_storage_used = temporary_storage_mark;
}