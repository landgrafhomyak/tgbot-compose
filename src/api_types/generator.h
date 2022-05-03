#ifndef API_TYPES_GENERATOR_H
# define API_TYPES_GENERATOR_H

# include <stdint.h>
//# include <Python.h>

static inline uint64_t hash_string(const char *key)
{
    uint64_t h = 525201411107845655ull;
    for (; *key; key++)
    {
        h ^= *key;
        h *= 0x5bd1e9955bd1e995;
        h ^= h >> 47;
    }
    return h;
}

#endif /* API_TYPES_GENERATOR_H */
