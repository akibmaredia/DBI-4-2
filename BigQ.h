#ifndef BIGQ_H
#define BIGQ_H
#include <pthread.h>
#include <iostream>
#include <utility>
#include <vector>
#include "Pipe.h"
#include "File.h"
#include "Record.h"

using namespace std;

class BigQ {
private:
	Pipe &in, &out;
	OrderMaker sortorder;
	int runlen;
	int totalruns;
	long totalrecords;
	off_t curr_pageno;
	vector<pair<off_t, off_t> > run_page_offsets; //start and end page number for each run
	File tempfile;

	pthread_t worker_thread;
  	static void *thread_starter(void *context);
  	void * WorkerThread(void);
  	
public:
	BigQ (Pipe &_in, Pipe &_out, OrderMaker &_sortorder, int _runlen);
	~BigQ ();


	void SortRun(vector<Record> &run);
	pair<off_t, off_t> WriteRunToTempFile(vector<Record> &run);


};

class Run{
private:
	int runid;
	off_t startpage;
	off_t endpage;
	off_t currpage;
	File *tempfile;
	Page page;

public:
	Run(int _runid, off_t _startpage, off_t _endpage, File *_tempfile);
	Run(const Run &run);
	Run & operator= (const Run &run);
	~Run();
	int GetNextRecord(Record *record);
};

class RunRecord{
public:
	int runid;
	Record record;

	RunRecord(int _runid, Record _record);
	~RunRecord();


};

class CompareRecord{
private:
	OrderMaker sortorder;
	ComparisonEngine comp;
	
public:
	CompareRecord(OrderMaker _sortorder);
	~CompareRecord();
	bool operator()(const Record &r1, const Record &r2);
};

class CompareRunRecord{
private:
	OrderMaker sortorder;
	ComparisonEngine comp;

public:
	CompareRunRecord(OrderMaker _sortorder);
	~CompareRunRecord();
	bool operator()(const RunRecord &rr1, const RunRecord &rr2);
};

#endif
