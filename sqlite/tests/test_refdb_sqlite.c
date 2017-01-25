#include "../sqlite_refdb_backend.h"
#include "../sqlite_odb_backend.h"

#include <git2/sys/repository.h>
#include <git2.h>

#include <stdio.h>


int main(int argc, char** argv) {
  git_refdb_backend* ref_backend = NULL;
  git_refdb* ref_db = NULL;
  const char* path = "test_repo";
  const char* file = "test_repo.db";
  int result = 0;
  int error = 0;
  git_repository* m_repo = NULL;

  // create fixtur
  git_libgit2_init();
  result = git_repository_init(&m_repo, path, 0 /* no bare */);
  result = git_refdb_new(&ref_db, m_repo);
  result = git_refdb_backend_sqlite(&ref_backend, m_repo, file);
  git_repository_set_refdb(m_repo, ref_db);



  // create reference to oid with "hello world!"

git_reference *ref = NULL;
git_oid test_oid0 = {};
git_oid_fromstrp(&test_oid0, "hello world!");
error = git_reference_create(&ref, m_repo,
      "refs/heads/direct",       /* name */
      &test_oid0,                      /* target */
      1,                      /* force? */
      NULL);                     /* the message for the reflog */


git_reference_iterator *iter = NULL;
error = git_reference_iterator_glob_new(&iter, m_repo, "refs/heads/*");
const char* name = NULL;
while (! (error = git_reference_next_name(&name, iter))) {
  printf("iterator at %s\n", name);
}

git_reference_iterator_free(iter);
  git_refdb_free(ref_db);

  return EXIT_SUCCESS;
}
