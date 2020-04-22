#ifndef DBFILE_H
#define DBFILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "GenericDBFile.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include <string>

using namespace std;

class DBFile {

private:
	GenericDBFile *dbfile;
	//static const char* META_FILE_PREFIX;

public:
	DBFile ();
	~DBFile(); 

	int Create (char *fpath, fType file_type, void *startup);
	int Open (char *fpath);
	int Close ();
	
	int GetRecNum();

	void Load (Schema &myschema, char *loadpath);

	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);
};
#endif