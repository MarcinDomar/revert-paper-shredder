#include "utillites.h"
#include "RatingGivers.h"

#include <fstream>
#include <list>
#include <algorithm>
#include "utillites.h"

ListOfRows  readListOfRows(const std::string filename)
{
	std::ifstream file(filename, std::ios::in);
	std::string line;
	ListOfRows listofrows;

	while (!file.eof()) {
		getline(file, line);

		size_t off = 1, new_off = 0;
		new_off = line.find('|', off);
		std::list<std::string> one_row;
		while (new_off != std::string::npos) {
			one_row.emplace_back(std::string(line.begin() + off, line.begin() + new_off));
			off = ++new_off;
			new_off = line.find('|', off);
		}
		listofrows.emplace_back(std::move(one_row));
	}
	if (listofrows.back().size() == 0)
		listofrows.pop_back();
	return listofrows;
}

VectorOfRows getVectorOfRows(const std::string filename) {
	auto listofrows = readListOfRows(filename);
	VectorOfRows result(listofrows.size(), VectorOfRows::value_type(listofrows.front().size()));
	for (int i = 0; i < result.size(); i++) {
		auto & row = result[i];
		auto & row_l = listofrows.front();
		for (size_t j = 0, jk = result.front().size(); j < jk; j++, row_l.pop_front()) {
			auto & ret_e = row[j];
			auto & str = row_l.front();

			ret_e[0] = str[0];
			ret_e[1] = str[1];
		}
		listofrows.pop_front();
	}
	return result;
}

PaperSide getPaperSizeFromFile(const std::string & fileName)
{
	PaperSide paperSide;
	std::ifstream file(fileName);
	while (!file.eof()) {
		std::string str;
		getline(file, str);
		paperSide.emplace_back(std::move(str));
	}
	return paperSide;
}
