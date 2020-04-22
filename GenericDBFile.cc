#include "GenericDBFile.h"

GenericDBFile::GenericDBFile(): writingMode(true), curr_page_index(0), num_rec(0)
{}

GenericDBFile::~GenericDBFile()
{}

int GenericDBFile::GetRecNum(){
	return num_rec;
}