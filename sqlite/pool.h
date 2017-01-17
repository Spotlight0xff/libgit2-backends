/*
 * This code is part of libgit2 src/pool.h
 */

#ifndef INCLUDE_git_sqlite_pool_h__
#define INCLUDE_git_sqlite_pool_h__
#include <limits.h>
#include <stdint.h>
#include "vector.h"

typedef struct git_pool_page git_pool_page;

/**
 * Chunked allocator.
 *
 * A `git_pool` can be used when you want to cheaply allocate
 * multiple items of the same type and are willing to free them
 * all together with a single call.  The two most common cases
 * are a set of fixed size items (such as lots of OIDs) or a
 * bunch of strings.
 *
 * Internally, a `git_pool` allocates pages of memory and then
 * deals out blocks from the trailing unused portion of each page.
 * The pages guarantee that the number of actual allocations done
 * will be much smaller than the number of items needed.
 *
 * For examples of how to set up a `git_pool` see `git_pool_init`.
 */
typedef struct {
  git_pool_page *pages; /* allocated pages */
  uint32_t item_size;  /* size of single alloc unit in bytes */
  uint32_t page_size;  /* size of page in bytes */
} git_pool;

#define GIT_POOL_INIT { NULL, 0, 0 }

/**
 * Initialize a pool.
 *
 * To allocation strings, use like this:
 *
 *     git_pool_init(&string_pool, 1);
 *     my_string = git_pool_strdup(&string_pool, your_string);
 *
 * To allocate items of fixed size, use like this:
 *
 *     git_pool_init(&pool, sizeof(item));
 *     my_item = git_pool_malloc(&pool, 1);
 *
 * Of course, you can use this in other ways, but those are the
 * two most common patterns.
 */
extern void git_pool_init(git_pool *pool, uint32_t item_size);

/**
 * Allocate space and duplicate string data into it.
 *
 * This is allowed only for pools with item_size == sizeof(char)
 */
extern char *git_pool_strdup(git_pool *pool, const char *str);

/**
 * Free all items in pool
 */
extern void git_pool_clear(git_pool *pool);

#if defined(__GNUC__)
#	define GIT_ALIGN(x, size) x __attribute__((aligned(size)))
#elif defined(_MSC_VER)
#	define GIT_ALIGN(x, size) __declspec(align(size)) x
#else
#	define GIT_ALIGN(x, size) x
#endif

/** Support for gcc/clang __has_builtin intrinsic */
#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

#if (SIZE_MAX == UINT_MAX) && __has_builtin(__builtin_uadd_overflow)
# define git__add_sizet_overflow(out, one, two) \
  __builtin_uadd_overflow(one, two, out)
# define git__multiply_sizet_overflow(out, one, two) \
  __builtin_umul_overflow(one, two, out)
#elif (SIZE_MAX == ULONG_MAX) && __has_builtin(__builtin_uaddl_overflow)
# define git__add_sizet_overflow(out, one, two) \
  __builtin_uaddl_overflow(one, two, out)
# define git__multiply_sizet_overflow(out, one, two) \
  __builtin_umull_overflow(one, two, out)
#else
/**
 * Sets `one + two` into `out`, unless the arithmetic would overflow.
 * @return true if the result fits in a `size_t`, false on overflow.
 */
inline bool git__add_sizet_overflow(size_t *out, size_t one, size_t two)
{
  if (SIZE_MAX - one < two)
    return true;
  *out = one + two;
  return false;
}

#endif
#define git__malloc(len) git__crtdbg__malloc(len, __FILE__, __LINE__)
#define GIT_ADD_SIZET_OVERFLOW(out, one, two) (git__add_sizet_overflow(out, one, two) ? -1 : 0)
#endif
