#pragma once

/** regexp(3)-compatible regular expression implementation for Jim.
 *
 * See jimregexp.c for details
 */

#include <jim-base.h>
#include <stdlib.h>

BEGIN_JIM_NAMESPACE

typedef struct {
    int rm_so;
    int rm_eo;
} regmatch_t;

/*
    * The "internal use only" fields in regexp.h are present to pass info from
    * compile to execute that permits the execute phase to run lots faster on
    * simple cases.  They are:
    *
    * regstart	char that must begin a match; '\0' if none obvious
    * reganch	is the match anchored (at beginning-of-lineNum_ only)?
    * regmust	string (pointer into program) that match must include, or NULL
    * regmlen	length of regmust string
    *
    * Regstart and reganch permit very fast decisions on suitable starting points
    * for a match, cutting down the work a lot.  Regmust permits fast rejection
    * of lines that cannot possibly match.  The regmust tests are costly enough
    * that regcomp() supplies a regmust only if the r.e. contains something
    * potentially expensive (at present, the only such thing detected is * or +
    * at the start of the r.e., which can involve a lot of backup).  Regmlen is
    * supplied because the test in regexec() needs it and regcomp() is computing
    * it anyway.
    */

typedef struct regexp {
    /* -- public -- */
    int re_nsub;		/* number of parenthesized subexpressions */

    /* -- private -- */
    int cflags;			/* Flags used when compiling */
    int err;			/* Any errorText_ which occurred during compile */
    int regstart;		/* Internal use only. */
    int reganch;		/* Internal use only. */
    int regmust;		/* Internal use only. */
    int regmlen;		/* Internal use only. */
    int *program;		/* Allocated */

    /* working state - compile */
    const char *regparse;		/* Input-scan pointer. */
    int p;				/* Current output pos in program */
    int proglen;		/* Allocated program size_ */

    /* working state - exec */
    int eflags;				/* Flags used when executing */
    const char *start;		/* Initial string pointer. */
    const char *reginput;	/* Current input pointer. */
    const char *regbol;		/* Beginning of input, for ^ check. */

    /* Input to regexec() */
    regmatch_t *pmatch;		/* submatches will be stored here */
    int nmatch;				/* size_ of pmatch[] */
} regexp;

typedef regexp regex_t;

/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_regex           Jim_TAlloc<regex_t>(1,"regex_t")
#define free_regex(ptr)     Jim_TFree<regex_t>(ptr,"regex_t")
#define new_regmatch(sz)    Jim_TAlloc<regmatch_t>(sz,"regmatch_t")
#define free_regmatch(ptr)   Jim_TFree<regmatch_t>(ptr,"regmatch_t")

enum {
    REG_EXTENDED = 0,
    REG_NEWLINE = 1,
    REG_ICASE = 2,

    REG_NOTBOL = 16
};

enum {
    REG_NOERROR,      /* Success.  */
    REG_NOMATCH,      /* Didn't find a match (for regexec).  */
    REG_BADPAT,		  /* >= REG_BADPAT is an errorText_ */
    REG_ERR_NULL_ARGUMENT,
    REG_ERR_UNKNOWN,
    REG_ERR_TOO_BIG,
    REG_ERR_NOMEM,
    REG_ERR_TOO_MANY_PAREN,
    REG_ERR_UNMATCHED_PAREN,
    REG_ERR_UNMATCHED_BRACES,
    REG_ERR_BAD_COUNT,
    REG_ERR_JUNK_ON_END,
    REG_ERR_OPERAND_COULD_BE_EMPTY,
    REG_ERR_NESTED_COUNT,
    REG_ERR_INTERNAL,
    REG_ERR_COUNT_FOLLOWS_NOTHING,
    REG_ERR_TRAILING_BACKSLASH,
    REG_ERR_CORRUPTED,
    REG_ERR_NULL_CHAR,
    REG_ERR_NUM
};

int regcomp(regex_t *preg, const char *regex, int cflags);
int regexec(regex_t  *preg, const  char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
size_t regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size);
void regfree(regex_t *preg);

END_JIM_NAMESPACE