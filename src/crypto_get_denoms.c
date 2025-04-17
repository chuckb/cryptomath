/*
 * Copyright (c) 2025 Charles Benedict, Jr.
 * See LICENSE.md for licensing information.
 * This copyright notice must be retained in its entirety.
 * The LICENSE.md file must be retained and must be included with any distribution of this file.
 */

#include <string.h>
#include <stdlib.h>
#include "cypto_get_denoms.h"
#include "cryptomath.h"
/*
** Eponymous virtual table module: "crypto_denoms"
** Presents a table with four columns:
**   symbol TEXT  (e.g., "GWEI", "SAT", ...)
**   name TEXT   (name of the denomination)
**   crypto_symbol TEXT  (e.g., "BTC", "ETH", "XRP", etc.)
**   decimals INT  (number of decimal places)
**
** Usage in SQL:
**   SELECT symbol, name, crypto_symbol, decimals FROM crypto_denoms();
*/

// Forward declarations
static int cryptoDenomsConnect(sqlite3 *db, void *pAux,
                              int argc, const char *const*argv,
                              sqlite3_vtab **ppVtab, char **pzErr);
static int cryptoDenomsDisconnect(sqlite3_vtab *pVtab);
static int cryptoDenomsBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info *pIdxInfo);
static int cryptoDenomsOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor);
static int cryptoDenomsClose(sqlite3_vtab_cursor *cur);
static int cryptoDenomsFilter(sqlite3_vtab_cursor *pCursor, int idxNum,
                             const char *idxStr, int argc, sqlite3_value **argv);
static int cryptoDenomsNext(sqlite3_vtab_cursor *pCursor);
static int cryptoDenomsEof(sqlite3_vtab_cursor *pCursor);
static int cryptoDenomsColumn(sqlite3_vtab_cursor *pCursor,
                             sqlite3_context *ctx, int i);
static int cryptoDenomsRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid);

#define UNUSED(x) (void)(x)

/* A simple structure for the virtual table (no real data needed). */
typedef struct {
  sqlite3_vtab base;  /* Base class.  Must be first. */
  /* You could store additional fields for state here. */
} cryptoDenomsVtab;

/* Cursor structure - tracks our current row index. */
typedef struct {
  sqlite3_vtab_cursor base;  /* Base class. Must be first. */
  int rowid;                 /* Current row index. */
} cryptoDenomsCursor;

/*
** This method is called to create a new cryptoDenoms virtual table or
** connect to an existing one. For an eponymous module, we typically
** treat CREATE and CONNECT the same.
*/
static int cryptoDenomsConnect(
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
      "CREATE TABLE x(symbol TEXT, name TEXT, crypto_symbol TEXT, decimals INT)"
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

  /* Allocate the cryptoDenomsVtab structure. */
  cryptoDenomsVtab *pNew = (cryptoDenomsVtab*)sqlite3_malloc(sizeof(*pNew));
  if (!pNew) return SQLITE_NOMEM;
  memset(pNew, 0, sizeof(*pNew));
  *ppVtab = (sqlite3_vtab*)pNew;

  return SQLITE_OK;
}

/* The disconnect method - free any allocated resources here. */
static int cryptoDenomsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  return SQLITE_OK;
}

/*
** The query planner calls this to figure out the best way to query
** your virtual table. If you have no indexes, you can often just
** return OK. For demonstration, we’ll set some arbitrary cost.
*/
static int cryptoDenomsBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info *pIdxInfo){
  UNUSED(pVTab);
  UNUSED(pIdxInfo);
  pIdxInfo->estimatedCost = (double)1;
  pIdxInfo->estimatedRows = DENOM_COUNT; /* We know there are DENOM_COUNT crypto denoms */
  return SQLITE_OK;
}

/* Opens a new cursor. */
static int cryptoDenomsOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  UNUSED(p);
  UNUSED(ppCursor);
  cryptoDenomsCursor *pCur = (cryptoDenomsCursor*)sqlite3_malloc(sizeof(*pCur));
  if (!pCur) return SQLITE_NOMEM;
  memset(pCur, 0, sizeof(*pCur));
  *ppCursor = &pCur->base;
  return SQLITE_OK;
}

/* Closes a cursor. */
static int cryptoDenomsClose(sqlite3_vtab_cursor *cur){
  cryptoDenomsCursor *pCur = (cryptoDenomsCursor*)cur;
  sqlite3_free(pCur);
  return SQLITE_OK;
}

/* Resets the cursor to the first row of results. */
static int cryptoDenomsFilter(sqlite3_vtab_cursor *pCursor, int idxNum,
                             const char *idxStr, int argc, sqlite3_value **argv){
  UNUSED(idxNum);
  UNUSED(idxStr);
  UNUSED(argc);
  UNUSED(argv);
  cryptoDenomsCursor *pCur = (cryptoDenomsCursor*)pCursor;
  pCur->rowid = 0;
  return SQLITE_OK;
}

/* Advances the cursor to the next row. */
static int cryptoDenomsNext(sqlite3_vtab_cursor *pCursor){
  cryptoDenomsCursor *pCur = (cryptoDenomsCursor*)pCursor;
  pCur->rowid++;
  return SQLITE_OK;
}

/* Returns true (1) if we are at the end of our data. */
static int cryptoDenomsEof(sqlite3_vtab_cursor *pCursor){
  cryptoDenomsCursor *pCur = (cryptoDenomsCursor*)pCursor;
  /* We know there are DENOM_COUNT crypto denoms. */
  return (pCur->rowid >= DENOM_COUNT);
}

/* Returns the column data for the current row/column. */
static int cryptoDenomsColumn(sqlite3_vtab_cursor *pCursor,
                             sqlite3_context *ctx, int i){
  cryptoDenomsCursor *pCur = (cryptoDenomsCursor*)pCursor;

  switch (i) {
    case 0: /* symbol */
      sqlite3_result_text(ctx, crypto_denoms[pCur->rowid].symbol, -1, SQLITE_STATIC);
      break;
    case 1: /* name */
      sqlite3_result_text(ctx, crypto_denoms[pCur->rowid].name, -1, SQLITE_STATIC);
      break;
    case 2: /* crypto_symbol */
      sqlite3_result_text(ctx, crypto_defs[crypto_denoms[pCur->rowid].crypto_type].symbol, -1, SQLITE_STATIC);
      break;
    case 3: /* decimals */
      sqlite3_result_int(ctx, crypto_denoms[pCur->rowid].decimals);
      break;
    default:
      /* Should never happen with our declared schema. */
      sqlite3_result_null(ctx);
      break;
  }

  return SQLITE_OK;
}

/* Returns the current rowid value (just our internal 'rowid'). */
static int cryptoDenomsRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  cryptoDenomsCursor *pCur = (cryptoDenomsCursor*)pCursor;
  *pRowid = (sqlite_int64)pCur->rowid;
  return SQLITE_OK;
}

// The module definition for our virtual table.
sqlite3_module cryptoDenomsModule = {
  0,                         /* iVersion      */
  cryptoDenomsConnect,        /* xCreate       */
  cryptoDenomsConnect,        /* xConnect      */
  cryptoDenomsBestIndex,      /* xBestIndex    */
  cryptoDenomsDisconnect,     /* xDisconnect   */
  cryptoDenomsDisconnect,     /* xDestroy      */
  cryptoDenomsOpen,           /* xOpen         */
  cryptoDenomsClose,          /* xClose        */
  cryptoDenomsFilter,         /* xFilter       */
  cryptoDenomsNext,           /* xNext         */
  cryptoDenomsEof,            /* xEof          */
  cryptoDenomsColumn,         /* xColumn       */
  cryptoDenomsRowid,          /* xRowid        */
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
