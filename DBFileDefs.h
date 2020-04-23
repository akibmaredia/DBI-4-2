#ifndef DBFILE_DEFS_H
#define DBFILE_DEFS_H

#include "Comparison.h"

typedef enum {heap, sorted, tree} fType;

//const char* META_FILE_PREFIX = ".meta";

struct SortInfo {
  OrderMaker *myOrder;
  int runLength;
};

#endif