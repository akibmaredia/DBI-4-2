#ifndef GENERIC_DBFILE_H
#define GENERIC_DBFILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "DBFileDefs.h"
#include "Comparison.h"
#include "ComparisonEngine.h"

/* base clase for dbfile impl*/

class GenericDBFile
{
protected:  
  File db_file;
  Page curr_page;
  fType file_type;
  off_t curr_page_index;
  off_t num_rec;
  bool writingMode;

public:
  GenericDBFile ();
  virtual ~GenericDBFile() = 0;

  virtual int Create (char *fpath, fType file_type, void *startup) = 0;
  virtual int Open (char *fpath) = 0;
  virtual void Load (Schema &myschema, char *loadpath) = 0;
  virtual int Close () = 0;
  
  virtual int GetRecNum();
  virtual void MoveFirst () = 0;
  virtual void Add (Record &addme) = 0;
  virtual int GetNext (Record &fetchme) = 0;
  virtual int GetNext (Record &fetchme, CNF &cnf, Record &literal) = 0;
};

#endif
