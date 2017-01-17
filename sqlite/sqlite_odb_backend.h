#ifndef SQLITE_ODB_BACKEND_H
#define SQLITE_ODB_BACKEND_H

#include <sqlite3.h>
#include <git2/sys/odb_backend.h>

typedef struct {
  git_odb_backend parent;
  sqlite3 *db;
  sqlite3_stmt *st_read;
  sqlite3_stmt *st_write;
  sqlite3_stmt *st_read_header;
  sqlite3_stmt *st_all;
} sqlite_odb_backend;


extern int git_odb_backend_sqlite(git_odb_backend **backend_out, const char *sqlite_db);

#endif
