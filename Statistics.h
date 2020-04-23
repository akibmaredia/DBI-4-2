#ifndef STATISTICS_H
#define STATISTICS_H
#include "ParseTree.h"
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;

class AttributeInfo;
class RelationInfo;

typedef map<string, AttributeInfo> AttributeMap;
typedef map<string, RelationInfo> RelationMap;

class AttributeInfo {

public:
	
	string attributeName;
	int distinctTuples;
	
	AttributeInfo ();
	AttributeInfo (string name, int num);
	AttributeInfo (const AttributeInfo &copyMe);
	
	AttributeInfo &operator= (const AttributeInfo &copyMe);
	
};

class RelationInfo {

public:
	
	double numberOfTuples;
	
	bool isJoint;
	string relationName;
	
	AttributeMap attributeMap;
	map<string, string> relJoint;
	
	RelationInfo ();
	RelationInfo (string name, int tuples);
	RelationInfo (const RelationInfo &copyMe);
	
	RelationInfo &operator= (const RelationInfo &copyMe);
	
	bool isRelationPresent (string relationName);
	
};

class Statistics {

private:
	
	double AndOperation (AndList *andList, char *relationName[], int numberOfJoins);
	double OrOperation (OrList *orList, char *relationName[], int numberOfJoins);
	double ComparisionOperation (ComparisonOp *comparisionOperation, char *relationName[], int numberOfJoins);
	int GetRelationForOperation (Operand *operand, char *relationName[], int numberOfJoins, RelationInfo &relInfo);
	
public:
	
	RelationMap relationMap;
	
	Statistics();
	Statistics(Statistics &copyMe);	 // Performs deep copy
	~Statistics();
	
	Statistics operator= (Statistics &copyMe);
	
	void AddRel(char *relationame, int numberOfTuples);
	void AddAtt(char *relationame, char *newAttributename, int numberOfMatches);
	void CopyRel(char *oldName, char *newName);
	
	void Read(char *fromWhere);
	void Write(char *toWhere);

	void  Apply(struct AndList *parseTree, char *relationNames[], int numToJoin);
	double Estimate(struct AndList *parseTree, char **relationNames, int numToJoin);
	
	bool isRelInMap (string relationName, RelationInfo &relInfo);

};

#endif
