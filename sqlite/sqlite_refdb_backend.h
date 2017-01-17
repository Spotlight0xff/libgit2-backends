#ifndef SQLITE_REFDB_BACKEND_H
#define SQLITE_REFDB_BACKEND_H

#include <sqlite3.h>
#include <git2/sys/refdb_backend.h>

typedef struct sqlite_refdb_backend {
	git_refdb_backend parent;
	git_repository *repo;
	sqlite3 *db;
	sqlite3_stmt *st_read;
	sqlite3_stmt *st_read_all;
	sqlite3_stmt *st_write;
	sqlite3_stmt *st_delete;
} sqlite_refdb_backend;


extern int git_refdb_backend_sqlite(git_refdb_backend **backend_out, git_repository *repository, const char *sqlite_db);

#endif
