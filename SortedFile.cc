#include "SortedFile.h"
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


SortedFile::SortedFile () {
  GenericDBFile::file_type = sorted;
  pipe_in = NULL;
  pipe_out = NULL;
  big_q = NULL;
  isQueryCreated = false;
  doesQueryExist = false;
  isSearchdone = false;
  doesRecExist = false;
}

SortedFile::~SortedFile () {
    delete pipe_in;
    delete pipe_out;
    delete big_q;
}



void SortedFile::MoveFirst () {
  if(writingMode){
    MergeDifferential();
  }

  writingMode = false;
  curr_page_index = 0;

  if(0 != db_file.GetLength()){
    db_file.GetPage(&curr_page, curr_page_index);
  }else{
    curr_page.EmptyItOut();
  }
  
  curr_page_index++;
}





void SortedFile::Load (Schema &f_schema, char *loadpath) {
  writingMode = true;
  
  if (NULL == big_q) {
      pipe_in = new Pipe(pipe_buffer_size);
      pipe_out = new Pipe(pipe_buffer_size);
      big_q = new BigQ(*pipe_in, *pipe_out, sortorder, runlen);
  }

  //open file from loadpath
  FILE *input_file = fopen(loadpath, "r");
  if (!input_file){
    cerr << "Not able to open input file " << loadpath << endl;
    exit(1);
  }

  Record temp;
  while (temp.SuckNextRecord (&f_schema, input_file)){ 
      pipe_in->Insert(&temp);
  }

  fclose(input_file);
}





int SortedFile::Create (char *f_path, fType f_type, void *startup){
  string meta_file_name;
  meta_file_name.append(f_path);
  meta_file_name.append(".meta");
  ofstream meta_file; 
  meta_file.open(meta_file_name.c_str());
  if (!meta_file) {
    cerr << "Not able to create meta file"<<endl;
    return 0;
  }
  meta_file << f_type << endl;
  SortInfo sortinfo = *((SortInfo *) startup);
  runlen = sortinfo.runLength;
  meta_file << runlen << endl;
  sortorder = *((OrderMaker *) sortinfo.myOrder);
  meta_file << sortorder; 
  meta_file.close();


  sortorder.Print();

  filepath.append(f_path);
  db_file.Open(0, f_path);
  writingMode = true;

  return 1;
}




void SortedFile::Add (Record &rec) {
  if (writingMode) {
      pipe_in->Insert(&rec);
  }else { 
      writingMode = true;
      if (NULL == big_q) {
        pipe_in = new Pipe(pipe_buffer_size);
        pipe_out = new Pipe(pipe_buffer_size);
        big_q = new BigQ(*pipe_in, *pipe_out, sortorder, runlen);
      }
      pipe_in->Insert(&rec);
    }
}





int SortedFile::GetNext (Record &fetchme) {
  if(writingMode){
    MergeDifferential();
    MoveFirst();
  }

  writingMode = false;

  if (curr_page.NumRecords() == 0){
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

int SortedFile::Open (char *f_path)
{
  int f_type;
  string meta_file_name;
  meta_file_name.append(f_path);
  meta_file_name.append(".meta");
  ifstream meta_file; 
  meta_file.open(meta_file_name.c_str());
  if (!meta_file) {
    cerr << "Not able to open meta file"<<endl;
    return 0;
  }
  meta_file >> f_type;
  meta_file >> runlen;
  meta_file >> sortorder;
  meta_file.close();

  filepath.append(f_path);
  db_file.Open(1, f_path);
  writingMode = false;
  MoveFirst();

  lseek (db_file.GetMyFilDes(), sizeof (off_t), SEEK_SET);
  read (db_file.GetMyFilDes(), &num_rec, sizeof (off_t));

  return 1;
}



int SortedFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {

  ComparisonEngine comp;
  while (GetNext(fetchme)){
    if (comp.Compare(&fetchme, &literal, &cnf)){
      return 1;
    }
  }

  return 0;
}

int SortedFile::Close () {
  MergeDifferential();
  lseek (db_file.GetMyFilDes(), sizeof (off_t), SEEK_SET);
  write (db_file.GetMyFilDes(), &num_rec, sizeof (off_t));
  
  int file_len = db_file.Close();
  return 1;
}


int SortedFile::BinarySearch(Record &fetchme, Record &literal, OrderMaker &query, off_t startInd, off_t endInd){
  ComparisonEngine comp;
  Page temp_page;

  off_t mid = 0;
  int comp_rc = 0;
  while(startInd <= endInd){
    mid  = (startInd + endInd)/2;

    if(mid == curr_page_index && curr_page.NumRecords() != 0){
      temp_page = curr_page;
    }else{
      db_file.GetPage(&temp_page, mid);
    }

    if(temp_page.GetFirst(&fetchme)){
      comp_rc = comp.Compare(&fetchme, &literal, &query);

      if(comp_rc < 0){
        startInd = mid + 1;
      }else if (comp_rc > 0){
        endInd = mid - 1;
      }else{
        if(--mid > 0){
          pair<off_t, int> res = BacktraceFile(fetchme, literal, query, mid, temp_page);
          curr_page_index = res.first;
          curr_page_index++;
          if(res.second){
            db_file.GetPage(&curr_page, res.first);
          }else{
            curr_page = temp_page;
          } 
        }
        return 1;
      }
    }
  }

  if(comp_rc < 0){
    db_file.GetPage(&temp_page, mid);
  }else if (comp_rc > 0 && mid > 0){
    mid = mid - 1;
    db_file.GetPage(&temp_page, mid);
  }
  
  while(temp_page.GetFirst(&fetchme)){
    int rc = comp.Compare(&fetchme, &literal, &query); 
    if(!rc){
      curr_page = temp_page;
      curr_page_index = mid;
      curr_page_index++;
      return 1;
    }
  }

  return 0;
}










pair<off_t, int> SortedFile::BacktraceFile(Record &fetchme, Record &literal, OrderMaker &query, off_t pageno, Page temp_page){
  Record temp_rec;
  ComparisonEngine comp;
  int is_full = 1;

  while(pageno > 0){
    db_file.GetPage(&temp_page, pageno);
    if(temp_page.GetFirst(&temp_rec)){
      if(!comp.Compare(&temp_rec, &literal, &query)){ 
        fetchme = temp_rec;
        pageno--;
      }else{
        while(temp_page.GetFirst(&temp_rec)){
          if(!comp.Compare(&temp_rec, &literal, &query)){
           fetchme = temp_rec;
           is_full = 0;
           break;
          }
        }
        pageno++;
      }
    }
  }

  return make_pair(pageno, is_full) ;
} 



void SortedFile::MergeDifferential(void) {
  writingMode = false;
  
  if(NULL != big_q){
    
    Record temp;
    if(db_file.GetLength() != 0){
      MoveFirst();
      while(GetNext(temp)){
        pipe_in->Insert(&temp);
      }
    }
    pipe_in->ShutDown();

    int file_len = db_file.Close();
    
    db_file.Open(0, const_cast<char *>(filepath.c_str()));
    curr_page_index = 0;
    num_rec = 0;
    

    curr_page.EmptyItOut();
    while(pipe_out->Remove(&temp)){
      if(!curr_page.Append(&temp)){
        db_file.AddPage(&curr_page, curr_page_index++);
        curr_page.EmptyItOut();

        curr_page.Append(&temp);
      }
      num_rec++;
    }
    db_file.AddPage(&curr_page, curr_page_index);
    curr_page.EmptyItOut();
    

    delete pipe_in;
    delete pipe_out;
    delete big_q;

    pipe_in = NULL;
    pipe_out = NULL;
    big_q = NULL;
  }

}



