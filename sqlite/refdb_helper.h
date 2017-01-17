#ifndef INCLUDE_git_sqlite_refdb_helper_h__
#define INCLUDE_git_sqlite_refdb_helper_h__
#include <stdio.h>
#include <ctype.h>

// check for _MSC_VER?:
// typedef unsigned char bool
// #define true 1
// #define false 0
#include <stdbool.h>

#define GITERR_CHECK_ALLOC(ptr) if (ptr == NULL) { return -1; }

// some stuff borrowed from src/buffer.h
#define GIT_BUF_INIT { git_buf__initbuf, 0, 0 }
const char * git_buf_cstr(const git_buf *buf)
{
	return buf->ptr;
}

size_t git_buf_len(const git_buf *buf)
{
	return buf->size;
}

bool git__isspace(int c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\r' || c == '\v');
}

// and src/buffer.c
void git_buf_rtrim(git_buf *buf)
{
	while (buf->size > 0) {
		if (!git__isspace(buf->ptr[buf->size - 1]))
			break;

		buf->size--;
	}

	if (buf->asize > buf->size)
		buf->ptr[buf->size] = '\0';
}

// and src/util.h



int git__prefixcmp(const char *str, const char *prefix)
{
	for (;;) {
		unsigned char p = *(prefix++), s;
		if (!p)
			return 0;
		if ((s = *(str++)) != p)
			return s - p;
	}
}

int git__suffixcmp(const char *str, const char *suffix)
{
	size_t a = strlen(str);
	size_t b = strlen(suffix);
	if (a < b)
		return -1;
	return strcmp(str + (a - b), suffix);
}


// and src/fnmatch.h and src/fnmatch.c
#define EOS	'\0'
#define RANGE_MATCH	1
#define RANGE_NOMATCH 0
#define RANGE_ERROR	(-1)

#define FNM_NOMATCH		1		/* Match failed. */
#define FNM_NOSYS		2		/* Function not supported (unused). */
#define	FNM_NORES		3		/* Out of resources */

#define FNM_NOESCAPE	0x01		/* Disable backslash escaping. */
#define FNM_PATHNAME	0x02		/* Slash must be matched by slash. */
#define FNM_PERIOD		0x04		/* Period must be matched by period. */
#define FNM_LEADING_DIR 0x08		/* Ignore /<tail> after Imatch. */
#define FNM_CASEFOLD	0x10		/* Case insensitive search. */

#define FNM_IGNORECASE	FNM_CASEFOLD
#define FNM_FILE_NAME	FNM_PATHNAME

int p_fnmatch(const char *pattern, const char *string, int flags)
{
  (void) pattern, (void) string, (void) flags;
		return 0;//p_fnmatchx(pattern, string, flags, 64);
}

static int rangematch(const char *pattern, char test, int flags, char **newp)
{
	int negate, ok;
	char c, c2;

	/*
	* A bracket expression starting with an unquoted circumflex
	* character produces unspecified results (IEEE 1003.2-1992,
	* 3.13.2). This implementation treats it like '!', for
	* consistency with the regular expression syntax.
	* J.T. Conklin (conklin@ngai.kaleida.com)
	*/
	if ((negate = (*pattern == '!' || *pattern == '^')) != 0)
		++pattern;

	if (flags & FNM_CASEFOLD)
		test = (char)tolower((unsigned char)test);

	/*
	* A right bracket shall lose its special meaning and represent
	* itself in a bracket expression if it occurs first in the list.
	* -- POSIX.2 2.8.3.2
	*/
	ok = 0;
	c = *pattern++;
	do {
		if (c == '\\' && !(flags & FNM_NOESCAPE))
			c = *pattern++;
		if (c == EOS)
			return (RANGE_ERROR);
		if (c == '/' && (flags & FNM_PATHNAME))
			return (RANGE_NOMATCH);
		if ((flags & FNM_CASEFOLD))
			c = (char)tolower((unsigned char)c);
		if (*pattern == '-'
			&& (c2 = *(pattern + 1)) != EOS && c2 != ']') {
			pattern += 2;
			if (c2 == '\\' && !(flags & FNM_NOESCAPE))
				c2 = *pattern++;
			if (c2 == EOS)
				return (RANGE_ERROR);
			if (flags & FNM_CASEFOLD)
				c2 = (char)tolower((unsigned char)c2);
			if (c <= test && test <= c2)
				ok = 1;
		}
		else if (c == test)
			ok = 1;
	} while ((c = *pattern++) != ']');

	*newp = (char *)pattern;
	return (ok == negate ? RANGE_NOMATCH : RANGE_MATCH);
}

static int p_fnmatchx(const char *pattern, const char *string, int flags, size_t recurs)
{
		const char *stringstart;
		char *newp;
		char c, test;
		int recurs_flags = flags & ~FNM_PERIOD;

		if (recurs-- == 0)
				return FNM_NORES;

		for (stringstart = string;;)
				switch (c = *pattern++) {
				case EOS:
						if ((flags & FNM_LEADING_DIR) && *string == '/')
								return (0);
						return (*string == EOS ? 0 : FNM_NOMATCH);
				case '?':
						if (*string == EOS)
								return (FNM_NOMATCH);
						if (*string == '/' && (flags & FNM_PATHNAME))
								return (FNM_NOMATCH);
						if (*string == '.' && (flags & FNM_PERIOD) &&
							(string == stringstart ||
							((flags & FNM_PATHNAME) && *(string - 1) == '/')))
								return (FNM_NOMATCH);
						++string;
						break;
				case '*':
						c = *pattern;

						/* Let '**' override PATHNAME match for this segment.
						 * It will be restored if/when we recurse below.
						 */
						if (c == '*') {
							c = *++pattern;
							/* star-star-slash is at the end, match by default */
							if (c == EOS)
								return 0;
							/* Double-star must be at end or between slashes */
							if (c != '/')
								return (FNM_NOMATCH);

							c = *++pattern;
							do {
								int e = p_fnmatchx(pattern, string, recurs_flags, recurs);
								if (e != FNM_NOMATCH)
									return e;
								string = strchr(string, '/');
							} while (string++);

							/* If we get here, we didn't find a match */
							return FNM_NOMATCH;
						}

						if (*string == '.' && (flags & FNM_PERIOD) &&
							(string == stringstart ||
							((flags & FNM_PATHNAME) && *(string - 1) == '/')))
								return (FNM_NOMATCH);

						/* Optimize for pattern with * at end or before /. */
						if (c == EOS) {
								if (flags & FNM_PATHNAME)
										return ((flags & FNM_LEADING_DIR) ||
											strchr(string, '/') == NULL ?
											0 : FNM_NOMATCH);
								else
										return (0);
						} else if (c == '/' && (flags & FNM_PATHNAME)) {
								if ((string = strchr(string, '/')) == NULL)
										return (FNM_NOMATCH);
								break;
						}

						/* General case, use recursion. */
						while ((test = *string) != EOS) {
								int e;

								e = p_fnmatchx(pattern, string, recurs_flags, recurs);
								if (e != FNM_NOMATCH)
										return e;
								if (test == '/' && (flags & FNM_PATHNAME))
										break;
								++string;
						}
						return (FNM_NOMATCH);
				case '[':
						if (*string == EOS)
								return (FNM_NOMATCH);
						if (*string == '/' && (flags & FNM_PATHNAME))
								return (FNM_NOMATCH);
						if (*string == '.' && (flags & FNM_PERIOD) &&
							(string == stringstart ||
							((flags & FNM_PATHNAME) && *(string - 1) == '/')))
								return (FNM_NOMATCH);

						switch (rangematch(pattern, *string, flags, &newp)) {
						case RANGE_ERROR:
								/* not a good range, treat as normal text */
								goto normal;
						case RANGE_MATCH:
								pattern = newp;
								break;
						case RANGE_NOMATCH:
								return (FNM_NOMATCH);
						}
						++string;
						break;
				case '\\':
						if (!(flags & FNM_NOESCAPE)) {
								if ((c = *pattern++) == EOS) {
										c = '\\';
										--pattern;
								}
						}
						/* FALLTHROUGH */
				default:
				normal:
						if (c != *string && !((flags & FNM_CASEFOLD) &&
									(tolower((unsigned char)c) ==
									tolower((unsigned char)*string))))
								return (FNM_NOMATCH);
						++string;
						break;
				}
		/* NOTREACHED */
}

#endif
