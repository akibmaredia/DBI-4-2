#include <algorithm>
#include <utility>
#include <cstdio>
#include <queue>
#include "BigQ.h"

BigQ :: BigQ (Pipe &_in, Pipe &_out, OrderMaker &_sortorder, int _runlen):
in(_in), out(_out), sortorder(_sortorder), runlen(_runlen), totalruns(0), totalrecords(0), curr_pageno(0) {
	pthread_create (&worker_thread, NULL, &BigQ::thread_starter, this);
	//pthread_join(worker_thread, NULL);
}

void * BigQ :: thread_starter(void *context){
  ((BigQ *)context)->WorkerThread();
}

void * BigQ :: WorkerThread(void) {
	char tempfilename[] = "/tmp/tempfileXXXXXX";
	tempfile.OpenTemp(tempfilename);
const size_t max_runsize = runlen * PAGE_SIZE;
	size_t curr_runsize = 0;

	vector<Record> run;
	run.reserve(runlen);

	Record temporaryRecord;
	while(in.Remove(&temporaryRecord)){
		totalrecords++;
		size_t recsize = temporaryRecord.GetSize();

		if(curr_runsize + recsize > max_runsize){
			SortRun(run);
			pair<off_t, off_t> offset = WriteRunToTempFile(run);
			run_page_offsets.push_back(offset);

			totalruns++;
			run.clear();
			curr_runsize = 0;
		}

		run.push_back(temporaryRecord);
		curr_runsize += recsize;
	}

	if(curr_runsize > 0){
		SortRun(run);
		pair<off_t, off_t> offset = WriteRunToTempFile(run);
		run_page_offsets.push_back(offset);

		totalruns++;
		run.clear();
		curr_runsize = 0;
	}
 	if(totalruns > RUN_THRESHOLD){
		vector<Run> runs;
	runs.reserve(totalruns);
	for(int i=0; i<totalruns; i++){
		runs.emplace_back(Run(i, run_page_offsets[i].first, run_page_offsets[i].second, &tempfile));
	}

	priority_queue<RunRecord, vector<RunRecord>, CompareRunRecord> pq(sortorder);
	for(int i=0; i<totalruns; i++){
		Record temp;
		runs[i].GetNextRecord(&temp);
		pq.push(RunRecord(i, temp));
	}

	for(int i=0; i<totalrecords; i++){
		RunRecord rr(pq.top());
		Record r(rr.record);
		int runid = rr.runid;

		out.Insert(&r);
		pq.pop();

		if(runs[runid].GetNextRecord(&r)){
			pq.push(RunRecord(runid, r));
		}
	}
	}else{
		vector<Run> runs;
	runs.reserve(totalruns);
	for(int i=0; i<totalruns; i++){
		runs.emplace_back(Run(i, run_page_offsets[i].first, run_page_offsets[i].second, &tempfile));
	}

	//cout<<"Loading first record of all runs"<<endl;
	vector<Record> records;
	records.reserve(totalruns);
	for(int i=0; i<totalruns; i++){
		Record temp;
		runs[i].GetNextRecord(&temp);
		records.push_back(temp);
	}

	//cout<<"Merging all runs"<<endl;
	CompareRecord comp(sortorder);
	for(int i=0; i<totalrecords; i++){

		int runid = distance(records.begin(), min_element(records.begin(), records.end(), comp));

		out.Insert(&records[runid]);

		if(!runs[runid].GetNextRecord(&records[runid])){
			records.erase(records.begin() + runid);
			runs.erase(runs.begin() + runid);
		}
	}
	}


	out.ShutDown ();

 	tempfile.Close();
 	//delete tempfile
 	remove(tempfilename);

 	pthread_exit(NULL); 
}

BigQ::~BigQ () {}



void BigQ::SortRun(vector<Record> &run){
	CompareRecord comp(sortorder);
	sort(run.begin(), run.end(), comp);
}

pair<off_t, off_t> BigQ::WriteRunToTempFile(vector<Record> &run){
	off_t start = curr_pageno;

	Page page;
	for(Record r: run){
		if(!page.Append(&r)){
			tempfile.AddPage(&page, curr_pageno);
			page.EmptyItOut();
			curr_pageno++;
			
			page.Append(&r);
		}
	}

	tempfile.AddPage(&page, curr_pageno);
	page.EmptyItOut();
	curr_pageno++;

	return make_pair(start, curr_pageno);
}









Run::Run(int _runid, off_t _startpage, off_t _endpage, File *_tempfile): 
runid(_runid), startpage(_startpage), endpage(_endpage), currpage(_startpage), tempfile(_tempfile) {
	tempfile->GetPage(&page, startpage);
}

Run::Run(const Run &run) :
  runid(run.runid), startpage(run.startpage), endpage(run.endpage), currpage(run.currpage), tempfile(run.tempfile){
      tempfile->AddPage(const_cast<Page *>(&run.page), currpage);
      tempfile->GetPage(&page,currpage);
  }

Run & Run::operator= (const Run &run){
      runid = run.runid;
      startpage = run.startpage;
      endpage = run.endpage;
      currpage = run.currpage; 

      run.tempfile->AddPage(const_cast<Page *>(&run.page),currpage);
      tempfile = run.tempfile;
      tempfile->GetPage(&page,currpage);
      return * this;
}

Run::~Run(){}

int Run::GetNextRecord(Record *record){
	if(!page.GetFirst(record)){
		if(currpage < endpage-1){
			page.EmptyItOut();
			currpage++;
			tempfile->GetPage(&page, currpage);

			return page.GetFirst(record);
		}else{
			return 0;
		}
	}

	return 1;
}


RunRecord::RunRecord(int _runid, Record _record):runid(_runid), record(_record) {}
RunRecord::~RunRecord(){ //delete record;
}




CompareRecord::CompareRecord(OrderMaker _sortorder): sortorder(_sortorder){}
CompareRecord::~CompareRecord(){}

bool CompareRecord::operator()(const Record &r1, const Record &r2){
	return (comp.Compare(const_cast<Record*>(&r1), const_cast<Record*>(&r2), &sortorder) < 0);
}


CompareRunRecord::CompareRunRecord(OrderMaker _sortorder): sortorder(_sortorder){}
CompareRunRecord::~CompareRunRecord(){}

bool CompareRunRecord::operator()(const RunRecord &rr1, const RunRecord &rr2){
	return (comp.Compare(const_cast<Record*>(&rr1.record), const_cast<Record*>(&rr2.record), &sortorder) < 0);
}
