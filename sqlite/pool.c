//source from util.c and pool.c
#include "pool.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>



struct git_pool_page {
  git_pool_page *next;
  uint32_t size;
  uint32_t avail;
  GIT_ALIGN(char data[1], 8);
};

uint32_t git_pool__system_page_size(void)
{
  static uint32_t size = 0;

  if (!size) {
    size_t page_size;
    //if (git__page_size(&page_size) < 0)
    page_size = 4096;
    /* allow space for malloc overhead */
    size = page_size - (2 * sizeof(void *)) - sizeof(git_pool_page);
  }

  return size;
}

static inline void* git__crtdbg__malloc(size_t len, const char *file, int line)
{
  (void) file, (void) line;
  //void *ptr = _malloc_dbg(len, _NORMAL_BLOCK, file, line);
  // in order to work on linux:
  void *ptr = malloc(len);
  //if (!ptr) giterr_set_oom();
  return ptr;
}

static void *pool_alloc_page(git_pool *pool, uint32_t size)
{
  git_pool_page *page;
  const uint32_t new_page_size = (size <= pool->page_size) ? pool->page_size : size;
  size_t alloc_size;

  if (GIT_ADD_SIZET_OVERFLOW(&alloc_size, new_page_size, sizeof(git_pool_page)) ||
      !(page = (git_pool_page*)git__malloc(alloc_size)))
    return NULL;

  page->size = new_page_size;
  page->avail = new_page_size - size;
  page->next = pool->pages;

  pool->pages = page;

  return page->data;
}

static void *pool_alloc(git_pool *pool, uint32_t size)
{
  git_pool_page *page = pool->pages;
  void *ptr = NULL;

  if (!page || page->avail < size)
    return pool_alloc_page(pool, size);

  ptr = &page->data[page->size - page->avail];
  page->avail -= size;

  return ptr;
}

static uint32_t alloc_size(git_pool *pool, uint32_t count)
{
  const uint32_t align = sizeof(void *) - 1;

  if (pool->item_size > 1) {
    const uint32_t item_size = (pool->item_size + align) & ~align;
    return item_size * count;
  }

  return (count + align) & ~align;
}

void *git_pool_malloc(git_pool *pool, uint32_t items)
{
  return pool_alloc(pool, alloc_size(pool, items));
}

char *git_pool_strndup(git_pool *pool, const char *str, size_t n)
{
  char *ptr = NULL;

  assert(pool && str && pool->item_size == sizeof(char));

  if ((uint32_t)(n + 1) < n)
    return NULL;

  if ((ptr = (char*)git_pool_malloc(pool, (uint32_t)(n + 1))) != NULL) {
    memcpy(ptr, str, n);
    ptr[n] = '\0';
  }

  return ptr;
}
//Pool header functions

void git_pool_init(git_pool *pool, uint32_t item_size)
{
  assert(pool);
  assert(item_size >= 1);

  memset(pool, 0, sizeof(git_pool));
  pool->item_size = item_size;
  pool->page_size = git_pool__system_page_size();
}

char *git_pool_strdup(git_pool *pool, const char *str)
{
  assert(pool && str && pool->item_size == sizeof(char));
  return git_pool_strndup(pool, str, strlen(str));
}

void git_pool_clear(git_pool *pool)
{
  git_pool_page *scan, *next;

  for (scan = pool->pages; scan != NULL; scan = next) {
    next = scan->next;
    free(scan);
  }

  pool->pages = NULL;
}
