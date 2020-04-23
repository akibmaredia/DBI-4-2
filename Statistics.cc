#include "Statistics.h"

//This function adds new relation specified by name and number of tuples. If relation name is not in relation map then we add a new statisticSchema else update the numberOfTuples.
void Statistics :: AddRel (char *relationame, int numberOfTuples) {
	
	string relationString (relationame);
	
	RelationInfo temp(relationString, numberOfTuples);
	
	relationMap[relationString] = temp;
	
}
//This function adds attribute to one of relations by specifying relation name, what relation the attribute is attached to, and the number of distinct values that the relation has for that particular attribute. If new attribute name is not in attributes map then add the new attribute else
//update number of matches.
void Statistics :: AddAtt (char *relationame, char *newAttributename, int numberOfMatches) {
	
	string relationString (relationame);
	string attributeString (newAttributename);
	
	AttributeInfo temp(attributeString, numberOfMatches);
	
	relationMap[relationString].attributeMap[attributeString] = temp;
	
}
//This function creates the copy of the relation and saves it with new name. It can also write into text file and read back from it.
//If the object has to read itself from the file which is not present we get empty output.
void Statistics :: CopyRel (char *oldName, char *newName) {
	
	string oldNameString (oldName);
	string newNameString (newName);
	
	relationMap[newNameString] = relationMap[oldNameString];
	relationMap[newNameString].relationName = newNameString;
	
	AttributeMap newattributeMap;
	
	for (
		auto iterator = relationMap[newNameString].attributeMap.begin ();
		iterator != relationMap[newNameString].attributeMap.end ();
		iterator++
	) {
		
		string newAttributeString = newNameString;
		newAttributeString.append (".");
		newAttributeString.append (iterator->first);
		
		AttributeInfo temp (iterator->second);
		temp.attributeName = newAttributeString;
		
		newattributeMap[newAttributeString] = temp;
		
	}
	
	relationMap[newNameString].attributeMap = newattributeMap;
	
}
//function to read input	
void Statistics :: Read (char *fromWhere) {
	
	int numberOfRelations, numberOfJoint, numberOfAttributes, numberOfTuples, numDistincts;
	string relationName, jointName, attributeName;
	
	ifstream in (fromWhere);
	
	if (!in) {
		
		cout << "File \"" << fromWhere << "\" does not exist!" << endl;
		
	}
	
	relationMap.clear ();
	
	in >> numberOfRelations;
	
	for (int counter = 0; counter < numberOfRelations; counter++) {
		
		in >> relationName;
		in >> numberOfTuples;
		
		RelationInfo relation (relationName, numberOfTuples);
		relationMap[relationName] = relation;
		
		in >> relationMap[relationName].isJoint;
		
		if (relationMap[relationName].isJoint) {
			
			in >> numberOfJoint;
			
			for (int counter2 = 0; counter2 < numberOfJoint; counter2++) {
				
				in >> jointName;
				
				relationMap[relationName].relJoint[jointName] = jointName;
				
			}
			
		}
		
		in >> numberOfAttributes;
		
		for (int counter3 = 0; counter3 < numberOfAttributes; counter3++) {
			
			in >> attributeName;
			in >> numDistincts;
			
			AttributeInfo attributeInfo (attributeName, numDistincts);
			
			relationMap[relationName].attributeMap[attributeName] = attributeInfo;
			
		}
		
	}
	
}

void Statistics :: Write (char *toWhere) {
	
	ofstream out (toWhere);
	
	out << relationMap.size () << endl;
	
	for (
		auto iterator = relationMap.begin ();
		iterator != relationMap.end ();
		iterator++
	) {
		
		out << iterator->second.relationName << endl;
		out << iterator->second.numberOfTuples << endl;
		out << iterator->second.isJoint << endl;
		
		if (iterator->second.isJoint) {
			
			out << iterator->second.relJoint.size () << endl;
			
			for (
				auto it = iterator->second.relJoint.begin ();
				it != iterator->second.relJoint.end ();
				it++
			) {
				
				out << it->second << endl;
				
			}
			
		}
		
		out << iterator->second.attributeMap.size () << endl;
		
		for (
			auto it = iterator->second.attributeMap.begin ();
			it != iterator->second.attributeMap.end ();
			it++
		) {
			
			out << it->second.attributeName << endl;
			out << it->second.distinctTuples << endl;
			
		}
		
	}
	
	out.close ();
	
}

void Statistics :: Apply (struct AndList *parseTree, char *relationNames[], int numToJoin) {
	
	int index = 0;
	int numberOfJoins = 0;
	char *names[100];
	
	RelationInfo rel;
	
	while (index < numToJoin) {
		
		string tempString (relationNames[index]);
		
		auto iterator = relationMap.find (tempString);
		
		if (iterator != relationMap.end ()) {
			
			rel = iterator->second;
			
			names[numberOfJoins++] = relationNames[index];
			
			if (rel.isJoint) {
				
				int size = rel.relJoint.size();
				
				if (size <= numToJoin) {
					
					for (int count = 0; count < numToJoin; count++) {
						
						string buf (relationNames[count]);
						
						if (rel.relJoint.find (buf) == rel.relJoint.end () &&
							rel.relJoint[buf] != rel.relJoint[tempString]) {
							
							cout << "Cannot be joined!" << endl;
							
							return;
							
						}
						
					}
					
				} else {
					
					cout << "Cannot be joined!" << endl;
					
				}
			
			} else {
				
				index++;
				
				continue;
				
			}
			
		} else {
			
			// cout << tempString << " errmsg!" << endl;
			
		}
		
		index++;
		
	}
	
	double estimation = Estimate (parseTree, names, numberOfJoins);
	
	index = 1;
	string firstRelationationName (names[0]);
	RelationInfo firstRelation = relationMap[firstRelationationName];
	RelationInfo temp;
	
	relationMap.erase (firstRelationationName);
	firstRelation.isJoint = true;
	firstRelation.numberOfTuples = estimation;
	
	
	while (index < numberOfJoins) {
		
		string tempString (names[index]);
		
		firstRelation.relJoint[tempString] = tempString;
		temp = relationMap[tempString];
		relationMap.erase (tempString);
		
		firstRelation.attributeMap.insert (temp.attributeMap.begin (), temp.attributeMap.end ());
		index++;
		
	}
	
	relationMap.insert (pair<string, RelationInfo> (firstRelationationName, firstRelation));
	
}

double Statistics :: Estimate (struct AndList *parseTree, char **relationNames, int numToJoin) {
	
	int index = 0;
	
	double factor = 1.0;
	double product = 1.0;
	
	while (index < numToJoin) {
		
		string tempString (relationNames[index]);
		
		if (relationMap.find (tempString) != relationMap.end ()) {
			
			product *= (double) relationMap[tempString].numberOfTuples;
			
		} else {
			
			// cout << tempString << " error!" << endl;
			
		}
		
		index++;
	
	}
	
	if (parseTree == NULL) {
		
		return product;
		
	}
	
	factor = AndOperation (parseTree, relationNames, numToJoin);
	

	return factor * product;
	
}

AttributeInfo :: AttributeInfo () {}

AttributeInfo :: AttributeInfo (string name, int num) {
	
	attributeName = name;
	distinctTuples = num;
	
}

AttributeInfo :: AttributeInfo (const AttributeInfo &copyMe) {
	
	attributeName = copyMe.attributeName;
	distinctTuples = copyMe.distinctTuples;
	
}

AttributeInfo &AttributeInfo :: operator= (const AttributeInfo &copyMe) {
	
	attributeName = copyMe.attributeName;
	distinctTuples = copyMe.distinctTuples;
	
	return *this;
	
}

RelationInfo :: RelationInfo () {
	
	isJoint = false;
	
}

RelationInfo :: RelationInfo (string name, int tuples) {
	
	isJoint = false;
	relationName = name;
	numberOfTuples = tuples;
	
}

RelationInfo :: RelationInfo (const RelationInfo &copyMe) {
	
	isJoint = copyMe.isJoint;
	relationName = copyMe.relationName;
	numberOfTuples = copyMe.numberOfTuples;
	attributeMap.insert (copyMe.attributeMap.begin (), copyMe.attributeMap.end ());
	
}

RelationInfo &RelationInfo :: operator= (const RelationInfo &copyMe) {
	
	isJoint = copyMe.isJoint;
	relationName = copyMe.relationName;
	numberOfTuples = copyMe.numberOfTuples;
	attributeMap.insert (copyMe.attributeMap.begin (), copyMe.attributeMap.end ());
	
	return *this;
	
}

bool RelationInfo :: isRelationPresent (string relationName) {
	
	if (relationName == relationName) {
		
		return true;
		
	}
	
	auto it = relJoint.find(relationName);
	
	if (it != relJoint.end ()) {
		
		return true;
		
	}
	
	return false;
	
}

Statistics :: Statistics () {}

Statistics :: Statistics (Statistics &copyMe) {
	
	relationMap.insert (copyMe.relationMap.begin (), copyMe.relationMap.end ());
	
}

Statistics :: ~Statistics () {}

Statistics Statistics :: operator= (Statistics &copyMe) {
	
	relationMap.insert (copyMe.relationMap.begin (), copyMe.relationMap.end ());
	
	return *this;
	
}

double Statistics :: AndOperation (AndList *andList, char *relationName[], int numberOfJoins) {
	
	if (andList == NULL) {
		
		return 1.0;
		
	}
	
	double left = 1.0;
	double right = 1.0;
	
	left = OrOperation (andList->left, relationName, numberOfJoins);
	right = AndOperation (andList->rightAnd, relationName, numberOfJoins);

	
	return left * right;
	
}

double Statistics :: OrOperation (OrList *orList, char *relationName[], int numberOfJoins) {
	
	if (orList == NULL) {
		
		return 0.0;
		
	}
	
	double left = 0.0;
	double right = 0.0;
	
	left = ComparisionOperation (orList->left, relationName, numberOfJoins);
	
	int count = 1;
	
	OrList *temp = orList->rightOr;
	char *attributeName = orList->left->left->value;
	
	while (temp) {
		
		if (strcmp (temp->left->left->value, attributeName) == 0) {
			
			count++;
			
		}
		
		temp = temp->rightOr;
		
	}
	
	if (count > 1) {
		
		return (double) count * left;
		
	}
	
	right = OrOperation (orList->rightOr, relationName, numberOfJoins);
	

	return (double) (1.0 - (1.0 - left) * (1.0 - right));
	
}

double Statistics :: ComparisionOperation (ComparisonOp *compOp, char *relationName[], int numberOfJoins) {
	
	RelationInfo leftRel, rightRel;
	
	double left = 0.0;
	double right = 0.0;
	
	int leftResult = GetRelationForOperation (compOp->left, relationName, numberOfJoins, leftRel);
	int rightResult = GetRelationForOperation (compOp->right, relationName, numberOfJoins, rightRel);
	int code = compOp->code;
	
	if (compOp->left->code == NAME) {
		
		if (leftResult == -1) {
			
			cout << compOp->left->value << " belongs to no relation!" << endl;
			left = 1.0;
			
		} else {
			
			string tempString (compOp->left->value);
			
			left = leftRel.attributeMap[tempString].distinctTuples;
			
		}
		
	} else {
		
		left = -1.0;
		
	}
	
	if (compOp->right->code == NAME) {
		
		if (rightResult == -1) {
			
			cout << compOp->right->value << " does not belong to any relation!" << endl;
			right = 1.0;
			
		} else {
			
			string tempString (compOp->right->value);
			
			right = rightRel.attributeMap[tempString].distinctTuples;
			
		}
		
	} else {
		
		right = -1.0;
		
	}
	
	if (code == LESS_THAN || code == GREATER_THAN) {
		
		return 1.0 / 3.0;
		
	} else if (code == EQUALS) {
		
		if (left > right) {
			
			return 1.0 / left;
			
		} else {
			
			return 1.0 / right;
			
		}
		
	}
	
	cout << "Error!" << endl;
	return 0.0;
	
}

int Statistics :: GetRelationForOperation (Operand *operand, char *relationName[], int numberOfJoins, RelationInfo &relInfo) {
	
	if (operand == NULL) {
		
		return -1;
		
	}
	
	if (relationName == NULL) {
		
		return -1;
		
	}
	
	string tempString (operand->value);
	
	for (auto iterator = relationMap.begin (); iterator != relationMap.end (); iterator++) {
		
		if (iterator->second.attributeMap.find (tempString) != iterator->second.attributeMap.end()) {
			
			relInfo = iterator->second;
			
			return 0;
			
		}
		
	}
	
	return -1;
	
}

