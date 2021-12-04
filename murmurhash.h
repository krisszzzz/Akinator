#if !defined MURMURHASH_INCLUDED

#define MURMURHASH_INCLUDED
#include <stdlib.h>

#if defined DEBUG
typedef unsigned long long hash_t;

constexpr hash_t MurmurHash(const char* key, size_t data_size);

#endif

#endif
