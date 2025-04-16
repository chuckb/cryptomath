/*
 * Copyright (c) 2025 Charles Benedict, Jr.
 * See LICENSE.md for licensing information.
 * This copyright notice must be retained in its entirety.
 * The LICENSE.md file must be retained and must be included with any distribution of this file.
 */

#include <string.h>
#include <stdlib.h>
#include "cypto_get_types.h"
#include "cryptomath2.h"
/*
** Eponymous virtual table module: "crypto_types"
** Presents a table with two columns:
**   symbol TEXT  (e.g., "BTC", "ETH", ...)
**   name TEXT   (name of the crypto)
**
** Usage in SQL:
**   SELECT symbol, name FROM crypto_types();
*/

// Forward declarations
static int cryptoTypesConnect(sqlite3 *db, void *pAux,
                              int argc, const char *const*argv,
                              sqlite3_vtab **ppVtab, char **pzErr);
static int cryptoTypesDisconnect(sqlite3_vtab *pVtab);
static int cryptoTypesBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info *pIdxInfo);
static int cryptoTypesOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor);
static int cryptoTypesClose(sqlite3_vtab_cursor *cur);
static int cryptoTypesFilter(sqlite3_vtab_cursor *pCursor, int idxNum,
                             const char *idxStr, int argc, sqlite3_value **argv);
static int cryptoTypesNext(sqlite3_vtab_cursor *pCursor);
static int cryptoTypesEof(sqlite3_vtab_cursor *pCursor);
static int cryptoTypesColumn(sqlite3_vtab_cursor *pCursor,
                             sqlite3_context *ctx, int i);
static int cryptoTypesRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid);

#define UNUSED(x) (void)(x)

/* A simple structure for the virtual table (no real data needed). */
typedef struct {
  sqlite3_vtab base;  /* Base class.  Must be first. */
  /* You could store additional fields for state here. */
} cryptoTypesVtab;

/* Cursor structure - tracks our current row index. */
typedef struct {
  sqlite3_vtab_cursor base;  /* Base class. Must be first. */
  int rowid;                 /* Current row index. */
} cryptoTypesCursor;

/*
** This method is called to create a new cryptoTypes virtual table or
** connect to an existing one. For an eponymous module, we typically
** treat CREATE and CONNECT the same.
*/
static int cryptoTypesConnect(
  sqlite3 *db, void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  UNUSED(db);
  UNUSED(pAux);
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(ppVtab);
  UNUSED(pzErr);
  /* Define the schema for our table. */
  char *zSchema = sqlite3_mprintf(
      "CREATE TABLE x(symbol TEXT, name TEXT)"
  );
  if (!zSchema) {
    return SQLITE_NOMEM;
  }

  /* Tell SQLite about the schema for our virtual table. */
  int rc = sqlite3_declare_vtab(db, zSchema);
  sqlite3_free(zSchema);
  if (rc != SQLITE_OK) {
    return rc;
  }

  /* Allocate the cryptoTypesVtab structure. */
  cryptoTypesVtab *pNew = (cryptoTypesVtab*)sqlite3_malloc(sizeof(*pNew));
  if (!pNew) return SQLITE_NOMEM;
  memset(pNew, 0, sizeof(*pNew));
  *ppVtab = (sqlite3_vtab*)pNew;

  return SQLITE_OK;
}

/* The disconnect method - free any allocated resources here. */
static int cryptoTypesDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  return SQLITE_OK;
}

/*
** The query planner calls this to figure out the best way to query
** your virtual table. If you have no indexes, you can often just
** return OK. For demonstration, we’ll set some arbitrary cost.
*/
static int cryptoTypesBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info *pIdxInfo){
  UNUSED(pVTab);
  UNUSED(pIdxInfo);
  pIdxInfo->estimatedCost = (double)1;
  pIdxInfo->estimatedRows = CRYPTO_COUNT; /* We know there are CRYPTO_COUNT crypto types */
  return SQLITE_OK;
}

/* Opens a new cursor. */
static int cryptoTypesOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  UNUSED(p);
  UNUSED(ppCursor);
  cryptoTypesCursor *pCur = (cryptoTypesCursor*)sqlite3_malloc(sizeof(*pCur));
  if (!pCur) return SQLITE_NOMEM;
  memset(pCur, 0, sizeof(*pCur));
  *ppCursor = &pCur->base;
  return SQLITE_OK;
}

/* Closes a cursor. */
static int cryptoTypesClose(sqlite3_vtab_cursor *cur){
  cryptoTypesCursor *pCur = (cryptoTypesCursor*)cur;
  sqlite3_free(pCur);
  return SQLITE_OK;
}

/* Resets the cursor to the first row of results. */
static int cryptoTypesFilter(sqlite3_vtab_cursor *pCursor, int idxNum,
                             const char *idxStr, int argc, sqlite3_value **argv){
  UNUSED(idxNum);
  UNUSED(idxStr);
  UNUSED(argc);
  UNUSED(argv);
  cryptoTypesCursor *pCur = (cryptoTypesCursor*)pCursor;
  pCur->rowid = 0;
  return SQLITE_OK;
}

/* Advances the cursor to the next row. */
static int cryptoTypesNext(sqlite3_vtab_cursor *pCursor){
  cryptoTypesCursor *pCur = (cryptoTypesCursor*)pCursor;
  pCur->rowid++;
  return SQLITE_OK;
}

/* Returns true (1) if we are at the end of our data. */
static int cryptoTypesEof(sqlite3_vtab_cursor *pCursor){
  cryptoTypesCursor *pCur = (cryptoTypesCursor*)pCursor;
  /* We know there are CRYPTO_COUNT crypto types. */
  return (pCur->rowid >= CRYPTO_COUNT);
}

/* Returns the column data for the current row/column. */
static int cryptoTypesColumn(sqlite3_vtab_cursor *pCursor,
                             sqlite3_context *ctx, int i){
  cryptoTypesCursor *pCur = (cryptoTypesCursor*)pCursor;

  switch (i) {
    case 0: /* symbol */
      sqlite3_result_text(ctx, crypto_defs[pCur->rowid].symbol, -1, SQLITE_STATIC);
      break;
    case 1: /* name */
      sqlite3_result_text(ctx, crypto_defs[pCur->rowid].name, -1, SQLITE_STATIC);
      break;
    default:
      /* Should never happen with our declared schema. */
      sqlite3_result_null(ctx);
      break;
  }

  return SQLITE_OK;
}

/* Returns the current rowid value (just our internal 'rowid'). */
static int cryptoTypesRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  cryptoTypesCursor *pCur = (cryptoTypesCursor*)pCursor;
  *pRowid = (sqlite_int64)pCur->rowid;
  return SQLITE_OK;
}

// The module definition for our virtual table.
sqlite3_module cryptoTypesModule = {
  0,                         /* iVersion      */
  cryptoTypesConnect,        /* xCreate       */
  cryptoTypesConnect,        /* xConnect      */
  cryptoTypesBestIndex,      /* xBestIndex    */
  cryptoTypesDisconnect,     /* xDisconnect   */
  cryptoTypesDisconnect,     /* xDestroy      */
  cryptoTypesOpen,           /* xOpen         */
  cryptoTypesClose,          /* xClose        */
  cryptoTypesFilter,         /* xFilter       */
  cryptoTypesNext,           /* xNext         */
  cryptoTypesEof,            /* xEof          */
  cryptoTypesColumn,         /* xColumn       */
  cryptoTypesRowid,          /* xRowid        */
  0,                         /* xUpdate       */
  0,                         /* xBegin        */
  0,                         /* xSync         */
  0,                         /* xCommit       */
  0,                         /* xRollback     */
  0,                         /* xFindFunction */
  0,                         /* xRename       */
  0,                         /* xSavepoint    */
  0,                         /* xRelease      */
  0,                         /* xRollbackTo   */
  0,                         /* xShadowName   */
  0                          /* xIntegrity    */
};
