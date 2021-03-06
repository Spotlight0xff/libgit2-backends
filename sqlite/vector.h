#ifndef INCLUDE_git_sqlite_vector_h__
#define INCLUDE_git_sqlite_vector_h__
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h> 
#include <stdbool.h>


typedef int(*git_vector_cmp)(const void *, const void *);

enum {
  GIT_VECTOR_SORTED = (1u << 0),
  GIT_VECTOR_FLAG_MAX = (1u << 1),
};

typedef struct git_vector {
  size_t _alloc_size;
  git_vector_cmp _cmp;
  void **contents;
  size_t length;
  uint32_t flags;
} git_vector;

#define GIT_VECTOR_INIT {0}
int git_vector_init(git_vector *v, size_t initial_size, git_vector_cmp cmp);
int git_vector_insert(git_vector *v, void *element);
void git_vector_free(git_vector *v);

void* git_vector_get(const git_vector *v, size_t position);

static inline bool git__multiply_sizet_overflow(size_t *out, size_t one, size_t two)
{
  if (one && SIZE_MAX / one < two)
    return true;
  *out = one * two;
  return false;
}

static inline void* git__reallocarray(void *ptr, size_t nelem, size_t elsize)
{
  size_t newsize;
  return git__multiply_sizet_overflow(&newsize, nelem, elsize) ?
    NULL : realloc(ptr, newsize);
}

#define git_vector_set_sorted(V,S) do { \
  (V)->flags = (S) ? ((V)->flags | GIT_VECTOR_SORTED) : \
  ((V)->flags & ~GIT_VECTOR_SORTED); } while (0)
#endif
