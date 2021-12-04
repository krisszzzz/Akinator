#define DEBUG
#include "log.h"
//#include "murmurhash.h"

typedef unsigned long long hash_t;

inline hash_t MurmurHash(const char* key, size_t data_size)
{
  const hash_t          first_hash_coefficient  = 0x5bd1e993;
  const hash_t          seed                    = 0;
  const int             binary_offset           = 24;
  hash_t                hash                    = seed ^ data_size;
  const unsigned char*  data                    = (const unsigned char *)key;
  hash_t                second_hash_coefficient = 0;

  while (data_size >= 4)
  {
      second_hash_coefficient  = data[0];
      second_hash_coefficient |= data[1] << 8;
      second_hash_coefficient |= data[2] << 16;
      second_hash_coefficient |= data[3] << 24;

      second_hash_coefficient *= first_hash_coefficient;
      second_hash_coefficient ^= second_hash_coefficient >> binary_offset;
      second_hash_coefficient *= first_hash_coefficient;

      second_hash_coefficient *= first_hash_coefficient;
      hash                    ^= second_hash_coefficient;


      data      += 4;
      data_size -= 4;
  }

  switch (data_size)
  {
    case 3: {                     /// -Wno-implicit-fallthrough
      hash ^= (data[2] << 16);    /// We use fallthrough, to mix hash
    }
    case 2: {
      hash ^= (data[1] << 8);
    }
    case 1: {
      hash ^= data[0];
      hash *= first_hash_coefficient;
    }
    case 0: {
      break;
    }
    default: {
      ErrorPrint("Inccorect data_size");
      return -1;
    }

  };

  hash ^= (hash >> 13);
  hash *= first_hash_coefficient;
  hash ^= (hash >> 15);

  return hash;
  return 0;
}
