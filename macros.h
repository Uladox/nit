/* This file should not be required for any other headers */

#if defined(__GNUC__) || defined(__clang__)
# define likely(val)   __builtin_expect(!!(val), 1)
# define unlikely(val) __builtin_expect(!!(val), 0)
#else
# define likely(val)   (val)
# define unlikely(val) (val)
#endif

#define ARRAY_UNITS(array) (sizeof(array) / sizeof(*array))

#define QUOTE(...) #__VA_ARGS__

#define ANY_TYPE(VAL) ((void *) (VAL))
