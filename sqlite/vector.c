#include <assert.h>
#include "vector.h"

#define MIN_ALLOCSIZE 8
#define MAX(a,b) ((a) > (b) ? (a) : (b))

static inline size_t compute_new_size(git_vector *v)
{
  size_t new_size = v->_alloc_size;

  /* Use a resize factor of 1.5, which is quick to compute using integer
   * instructions and less than the golden ratio (1.618...) */
  if (new_size < MIN_ALLOCSIZE)
    new_size = MIN_ALLOCSIZE;
  else if (new_size <= (SIZE_MAX / 3) * 2)
    new_size += new_size / 2;
  else
    new_size = SIZE_MAX;

  return new_size;
}

static inline int resize_vector(git_vector *v, size_t new_size)
{
  void *new_contents;

  new_contents = git__reallocarray(v->contents, new_size, sizeof(void *));
  if (!new_contents)
    return -1;

  v->_alloc_size = new_size;
  v->contents = (void**)new_contents;

  return 0;
}

int git_vector_init(git_vector *v, size_t initial_size, git_vector_cmp cmp)
{
  assert(v);

  v->_alloc_size = 0;
  v->_cmp = cmp;
  v->length = 0;
  v->flags = GIT_VECTOR_SORTED;
  v->contents = NULL;

  return resize_vector(v, MAX(initial_size, MIN_ALLOCSIZE));
}

int git_vector_insert(git_vector *v, void *element)
{
  assert(v);

  if (v->length >= v->_alloc_size &&
      resize_vector(v, compute_new_size(v)) < 0)
    return -1;

  v->contents[v->length++] = element;

  git_vector_set_sorted(v, v->length <= 1);

  return 0;
}

void git_vector_free(git_vector *v)
{
  assert(v);

  free(v->contents);
  v->contents = NULL;

  v->length = 0;
  v->_alloc_size = 0;
}

void* git_vector_get(const git_vector *v, size_t position)
{
  return (position < v->length) ? v->contents[position] : NULL;
}
