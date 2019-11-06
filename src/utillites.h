#pragma once
#include "Types.h"

ListOfRows  readListOfRows(const std::string filename);

VectorOrRows getVectorOrRows(const std::string filename);


PaperSide getPaperSizeFromFile(const std::string & fileName);


ColumnsPermutation getPermutationFromOriginPage(const PaperSide & originText, const VectorOrRows & scrumbled);

unsigned int getRatingOfPermutation(const ColumnsPermutation & origin, const ColumnsPermutation & toScore);

PaperSide getPageSizeFromColumnPermutation(const VectorOrRows & scrumbled, const ColumnsPermutation & columnsPermutations);