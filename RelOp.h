#ifndef REL_OP_H
#define REL_OP_H

#include "Pipe.h"
#include "DBFile.h"
#include "Record.h"
#include "Function.h"
#include <pthread.h>

class RelationalOp {
	public:
		//blocks the caller until the particular relational operator
 //has run to completion 
  	virtual void WaitUntilDone();
	// tells us how much internal memory the operation we can use
  	virtual int Use_n_Pages(int n) = 0;

	protected:
  	pthread_t thread;
  	static int create_join_thread(pthread_t *thread, void *(*start_routine) (void *), void *arg);
};
//SelectFile takes a DBFile and a pipe as input. You can assume that this file is all set up; it has been opened and is ready to go. It also takes a CNF. It then performs a scan of the underlying file, and for every tuple accepted by the CNF, it stuffs the tuple into the pipe as output. The DBFile should not be closed by the SelectFile class; that is the job of the caller.
class SelectFile : public RelationalOp { 

	private:
	DBFile *dbFile;
	Pipe *outPipe;
	CNF *selectOp;
	Record *literal;
	int numberOfPg;

	public:
	void Run (DBFile &inFile, Pipe &outPipe, CNF &selectOP, Record &literal);
	void WaitUntilDone ();
	int Use_n_Pages (int n);

	~SelectFile();
	static void * HelperRun(void *param);
	void Start();
};
//SelectPipe  class takes two pipes as input: an input pipe and an output pipe. It also takes a CNF. It simply applies that CNF to every tuple that comes through the pipe, and every tuple that is accepted is stuffed into the output pipe.
class SelectPipe : public RelationalOp {
	private:
	Pipe *inPipe, *outPipe;
	CNF *selectOp;
	Record *literal;
	int numberOfPg;

	public:
	void Run (Pipe &inPipe, Pipe &outPipe, CNF &selectOP, Record &literal);
	void WaitUntilDone ();
	int Use_n_Pages (int n);

	~SelectPipe();
	static void * HelperRun(void *param);
	void Start();
};
//Project takes an input pipe and an output pipe as input. It also takes an array of integers keepMe as well as the number of attributes for the records coming through the input pipe and the number of attributes to keep from those input records. The array of integers tells Project which attributes to keep from the input records, and which order to put them in. So, for example, say that the array keepMe had the values [3, 5, 7, 1]. This means that Project should take the third attribute from every input record and treat it as the first attribute of those records that it puts into the output pipe. Project should take the fifth attribute from every input record and treat it as the second attribute of every record that it puts into the output pipe. The seventh input attribute becomes the third. And so on.
class Project : public RelationalOp { 
	private:
	Pipe *inPipe, *outPipe;
	int *keepMe;
	int numAttsIn, numAttsOut;
	int numberOfPg;

	public:
	void Run (Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput);
	void WaitUntilDone ();
	int Use_n_Pages (int n);

	~Project();
	static void * HelperRun(void *param);
	void Start();
};
//Join takes two input pipes, an output pipe, and a CNF, and joins all of the records from the two pipes according to that CNF. Join should use a BigQ to store all of the tuples coming from the left input pipe, and a second BigQ for the right input pipe, and then perform a merge in order to join the two input pipes. You’ll create the OrderMakers for the two BigQ’s using the CNF (the function GetSortOrders will be used to create the OrderMakers). If you can’t get an appropriate pair of OrderMakers because the CNF can’t be implemented using a sort-merge join (due to the fact it does not have an equality check) then your Join operation should default to a block-nested loops join.
class Join : public RelationalOp { 
	private:
	Pipe *inPipeL, *inPipeR, *outPipe;
	CNF *selectOP;
	Record *literal;
	int numberOfPg;

	public:
	void Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selectOP, Record &literal);
	void WaitUntilDone ();
	int Use_n_Pages (int n);

	~Join();
	static void * HelperRun(void *param);
	void Start();
	void SortMergeJoin(OrderMaker &sortOrderL, OrderMaker sortOrderR);
	void BlockNestedJoin();
};
//	DuplicateRemoval takes an input pipe, an output pipe, as well as the schema for the tuples coming through the input pipe, and does a duplicate removal. That is, everything that comes through the output pipe will be distinct. It will use the BigQ class to do the duplicate removal. The OrderMaker that will be used by the BigQ (which you’ll need to write some code to create) will simply list all of the attributes from the input tuples.
class DuplicateRemoval : public RelationalOp {
	private:
	Pipe *inPipe, *outPipe;
	Schema *mySchema;
	int numberOfPg;

	public:
	void Run (Pipe &inPipe, Pipe &outPipe, Schema &mySchema);
	void WaitUntilDone ();
	int Use_n_Pages (int n);

	~DuplicateRemoval();
	static void * HelperRun(void *param);
	void Start();
};
//Sum computes the SUM SQL aggregate function over the input pipe, and puts a single tuple into the output pipe that has the sum. The function over each tuple (for example: (l_extendedprice*(1-l_discount)) in the case of the TPC-H schema) that is summed is stored in an instance of the Function class that is also passed to Sum as an argument .
class Sum : public RelationalOp {
	private:
	Pipe *inPipe, *outPipe;
	Function *computeMe;
	int numberOfPg;

	public:
	void Run (Pipe &inPipe, Pipe &outPipe, Function &computeMe);
	void WaitUntilDone ();
	int Use_n_Pages (int n);

	~Sum();
	static void * HelperRun(void *param);
	void Start();
};
//	GroupBy is a lot like Sum, except that it does grouping, and then puts one sum into the output pipe for each group. Every tuple put into the output pipe has a sum as the first attribute, followed by the values for each of the grouping attributes as the remainder of the attributes. The grouping is specified using an instance of the OrderMaker class that is passed in. The sum to compute is given in an instance of the Function class.
class GroupBy : public RelationalOp {
	private:
	Pipe *inPipe, *outPipe;
	OrderMaker *groupAtts;
	Function *computeMe;
	int numberOfPg;

	public:
	void Run (Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe);
	void WaitUntilDone ();
	int Use_n_Pages (int n);

	~GroupBy();
	static void * HelperRun(void *param);
	void Start();
	void ApplyFunction(Record &rec, int &intResult, double &doubleResult, Type &resType);
	void PackResultInRecord(Type resType, int &intResult, double &doubleResult, Record &groupRec, Record &resultRec);
};
//WriteOut accepts an input pipe, a schema, and a FILE*, and uses the schema to write text version of the output records to the file.
class WriteOut : public RelationalOp {
	private:
	Pipe *inPipe;
	FILE *outFile;
	Schema *mySchema;
	int numberOfPg;
	
	public:
	void Run (Pipe &inPipe, FILE *outFile, Schema &mySchema);
	void WaitUntilDone ();
	int Use_n_Pages (int n);

	~WriteOut();
	static void * HelperRun(void *param);
	void Start();
};

#endif