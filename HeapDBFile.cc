#include "HeapDBFile.h"
#include "DBFile.h"
#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"
#include <fstream>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


HeapDBFile::HeapDBFile () {
  GenericDBFile::file_type = heap;
}

HeapDBFile::~HeapDBFile () {}

int HeapDBFile::Open (char *f_path)
{
  db_file.Open(1, f_path);
  MoveFirst();
  writingMode = false;

  // read in the second few bits, which is the number of records
  lseek (db_file.GetMyFilDes(), sizeof (off_t), SEEK_SET);
  read (db_file.GetMyFilDes(), &num_rec, sizeof (off_t));

  return 1;
}

int HeapDBFile::Create (char *f_path, fType f_type, void *startup)
{
  //create a meta file and write file type in it
  string meta_file_name;
  meta_file_name.append(f_path);
  //meta_file_name.append(META_FILE_PREFIX);
  meta_file_name.append(".meta");
  ofstream meta_file; 
  meta_file.open(meta_file_name.c_str());
  if (!meta_file) {
    cerr << "Not able to create meta file"<<endl;
    return 0;
  }
  meta_file << f_type << endl;
  meta_file.close();

  db_file.Open(0, f_path);
  writingMode = true;

  return 1;
}

//TODO: use File::AppendPage in this function after below code works fine
void HeapDBFile::Load (Schema &f_schema, char *loadpath) {
  //open file from loadpath
  FILE *input_file = fopen(loadpath, "r");
  if (!input_file){
    //#ifdef verbose
    cerr << "Not able to open input file " << loadpath << endl;
    //#endif
    exit(1);
  }

  //if empty file then page number starts with 0 else current file length - 1
  off_t page_no = db_file.GetLength();
  if (page_no != 0) page_no--;

  //if program is in reading mode and there are records in page then empty out the page and set mode to writing mode
  if (!writingMode && curr_page.NumRecords() != 0){
    curr_page.EmptyItOut();
  }
  writingMode = true;

  Record temp;
  while (temp.SuckNextRecord(&f_schema, input_file)){
    //write current record to page
    int isAppended = curr_page.Append(&temp);

    if (!isAppended){
      //page is full, write page to file on disk
      db_file.AddPage(&curr_page, page_no++);
      curr_page.EmptyItOut();

      //write current record to page
      curr_page.Append(&temp);
    }
    num_rec++;
  }
  //write last page to file on disk
  db_file.AddPage(&curr_page, page_no);
  curr_page.EmptyItOut();

  fclose(input_file);
}

//TODO: use File::AppendPage in this function after below code works fine
void HeapDBFile::MoveFirst () {
  //if there is anything that is not written to file yet, write it
  if (writingMode && curr_page.NumRecords()){
    //if empty file then page number starts with 0 else current file length - 1
    off_t page_no = db_file.GetLength();
    if (page_no != 0) page_no--;

    db_file.AddPage(&curr_page, page_no);
    curr_page.EmptyItOut();
  }
  writingMode = false;

  curr_page_index = 0;
  db_file.GetPage(&curr_page, curr_page_index);
  curr_page_index++;
}

//TODO: use File::AppendPage in this function after below code works fine
void HeapDBFile::Add (Record &rec) {
  if (!writingMode && curr_page.NumRecords() != 0){
    curr_page.EmptyItOut();

    //if last page written to file was not full then we will get that page and write in it otherwise in a new page
    if(db_file.GetLength() > 0){
      //cout<<"curr page records: "<<curr_page.NumRecords()<<endl;
      //cout<<"file length: "<<db_file.GetLength();

      off_t page_no = db_file.GetLength() - 2;
      db_file.GetPage(&curr_page, page_no);

      if(page_no == 1) db_file.SetLength(0);
      else db_file.SetLength(page_no);

      //cout<<"curr page records: "<<curr_page.NumRecords()<<endl;
      //cout<<"file length: "<<db_file.GetLength();
    }
  }
  writingMode = true;

  //write record to page
  Record temp;
  temp.Consume(&rec);
  int isAppended = curr_page.Append(&temp);
  num_rec++;

  if (!isAppended){
    //if empty file then page number starts with 0 else current file length - 1
    off_t page_no = db_file.GetLength();
    if (page_no != 0) page_no--;

    //page is full, write page to file on disk
    db_file.AddPage(&curr_page, page_no);
    curr_page.EmptyItOut();

    //write record to page
    curr_page.Append(&temp);
  }
}

int HeapDBFile::GetNext (Record &fetchme) {
  //if there is anything that is not written to file yet, write it
  if (writingMode && curr_page.NumRecords() != 0){
    //if empty file then page number starts with 0 else current file length - 1
    off_t page_no = db_file.GetLength();
    if (page_no != 0) page_no--;

    db_file.AddPage(&curr_page, page_no);
    curr_page.EmptyItOut();

    //fetch current page from disk for reading
    //db_file.GetPage(&curr_page, curr_page_index);
  }
  writingMode = false;

  if (!writingMode && curr_page.NumRecords() == 0){
    //if reached at the end of current page then fetch next page if current page is not last page
    if (curr_page_index < db_file.GetLength()-1){
      db_file.GetPage(&curr_page, curr_page_index);
      curr_page_index++;
    }
    else{
      return 0;
    }
  }
  
  return curr_page.GetFirst(&fetchme);
}

int HeapDBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
  ComparisonEngine comp;
  while (GetNext(fetchme)){
    if (comp.Compare(&fetchme, &literal, &cnf)){
      return 1;
    }
  }

  return 0;
}

//TODO: use File::AppendPage in this function after below code works fine
int HeapDBFile::Close () {
  //if there is anything that is not written to file yet, write it
  if (writingMode && curr_page.NumRecords() != 0){
    //if empty file then page number starts with 0 else current file length - 1
    off_t page_no = db_file.GetLength();
    if (page_no != 0) page_no--;

    db_file.AddPage(&curr_page, page_no);
    curr_page.EmptyItOut();
  }
  //write number of records after number of pages
  lseek (db_file.GetMyFilDes(), sizeof (off_t), SEEK_SET);
  write (db_file.GetMyFilDes(), &num_rec, sizeof (off_t));
  //close the file
  int file_len = db_file.Close();
  return 1;
}