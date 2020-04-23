#ifndef HEAP_DBFILE_H
#define HEAP_DBFILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "GenericDBFile.h"

class HeapDBFile : public GenericDBFile
{
public:
  HeapDBFile ();
  ~HeapDBFile(); 

  int Create (char *fpath, fType file_type, void *startup);
  int Open (char *fpath);
  void Load (Schema &myschema, char *loadpath);
  int Close ();

  void MoveFirst ();
  void Add (Record &addme);
  int GetNext (Record &fetchme);
  int GetNext (Record &fetchme, CNF &cnf, Record &literal);
};

#endif