#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "HeapDBFile.h"
#include "SortedFile.h"
#include "Defs.h"
#include <fstream>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;


//const char* DBFile::META_FILE_PREFIX = ".meta";

DBFile::DBFile () {
	/*writingMode = true;
	curr_page_index = 0;
	num_rec = 0;*/
}

DBFile::~DBFile(){
	delete dbfile;
}

int DBFile::Create (char *f_path, fType f_type, void *startup) {
	//instantiate dbfile based on file type
	if(f_path==NULL)
		return 0;
	if(f_type==heap){
			dbfile = new HeapDBFile();
		}
	else if(f_type == sorted){

		dbfile = new SortedFile();	


	}

	if(dbfile!=NULL){

		dbfile->Create(f_path, f_type, startup);		
		return 1;
	}	
		return 0;
}

int DBFile::Open (char *f_path) {
	//Fetching file type from meta file
	int f_type;
    string meta_file_name;
	meta_file_name.append(f_path);
	//meta_file_name.append(META_FILE_PREFIX);
	meta_file_name.append(".meta");
	ifstream meta_file; 
	meta_file.open(meta_file_name.c_str());
	if (!meta_file) {
		return 0;
	}
    meta_file >> f_type;
    fType file_type = (fType) f_type;
    meta_file.close();
    if(f_type==heap){
			dbfile = new HeapDBFile();
		}
	else if(f_type == sorted){

		dbfile = new SortedFile();	


	}
	else 

	{return -1;

		
    }

	//open db file using file.open, pass length as 1
	return dbfile->Open(f_path);
}


void DBFile::Load (Schema &f_schema, char *loadpath) {
	dbfile->Load(f_schema, loadpath);
}

void DBFile::MoveFirst () {
	dbfile->MoveFirst();
}

//TODO: check why delete dbfile not working
int DBFile::Close () {
	int rc = dbfile->Close();
	//delete dbfile;
	return rc;
}

//TODO: use File::AppendPage in this function after below code works fine
void DBFile::Add (Record &rec) {
	dbfile->Add(rec);
}

int DBFile::GetNext (Record &fetchme) {
	return dbfile->GetNext(fetchme);
}

int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
	return dbfile->GetNext(fetchme, cnf, literal);
}

int DBFile::GetRecNum(){
	return dbfile->GetRecNum();
}