#ifndef TEMPORARY_STORAGE_H
#define TEMPORARY_STORAGE_H

constexpr u64 TEMPORARY_STORAGE_SIZE = 1024 * 64;
constexpr u64 TEMPORARY_STORAGE_ALIGNMENT = 8;

void* talloc(u64 x);
void tclear();
void tmark();
void treset();

#endif