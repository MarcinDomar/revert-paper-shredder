#include <iostream>
#include <stdexcept>
#include "Types.h"
#include "RatingGivers.h"
#include "NarrowerSearches.h"
#include <fstream>
#include <list>
#include <algorithm>

using namespace std;

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
			one_row.emplace_back(string(line.begin() + off, line.begin() + new_off));
			off = ++new_off;
			new_off = line.find('|', off);
		}
		listofrows.emplace_back(std::move(one_row));
	}
	if (listofrows.back().size() == 0)
		listofrows.pop_back();
	return listofrows;
}

VectorOrRows getVectorOrRows(const std::string filename) {
	auto listofrows = readListOfRows(filename);
	VectorOrRows result(listofrows.size(), VectorOrRows::value_type(listofrows.front().size()));
	for (int i = 0; i < result.size(); i++) {
		auto & row = result[i];
		auto & row_l = listofrows.front();
		for (int j = 0, jk = result.front().size(); j < jk; j++) {
			auto & ret_e = row[j];
			auto & str = row_l.front();
	
			ret_e[0] = str[0];
			ret_e[1] = str[1];
		}
	}
	return result;

}


int main(int argc, char *argv[])
{
	if (argc != 3) {
        std::cerr << "As first parameter pass path to file with shredded text" << std::endl;
        std::exit(-1);
	}
	auto inputData = getVectorOrRows(argv[1]);
	RatingGiver ratingGiverL1= getRatingGiver(argv[2]);
	
	NarrowerSearches solutionFinder(inputData, ratingGiverL1);
	auto ratingForPageGiver = getRatingGiverForPage(argv[2]);
	
	auto sugestedPages = solutionFinder.getBestSugestions(100);
	

	std::vector<int> scores(sugestedPages.size());
	std::vector<int> indexes(sugestedPages.size());

	for (int i = 0; i < sugestedPages.size(); i++) {
		scores[i] = ratingForPageGiver.getScore(sugestedPages[i]);
		indexes[i] = i;
	}
	std::sort(indexes.begin(), indexes.end(),[&scores](const auto & ix1, const auto & ix2) {
		return scores[ix1] < scores[ix2];
	});

	auto &bestPage = sugestedPages[indexes.back()];
    return 0;
}
