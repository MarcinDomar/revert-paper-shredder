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
		for (int j = 0, jk = result.front().size(); j < jk; j++,row_l.pop_front()) {
			auto & ret_e = row[j];
			auto & str = row_l.front();
	
			ret_e[0] = str[0];
			ret_e[1] = str[1];
		}
		listofrows.pop_front();
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
	std::cout << "input readed " << std::endl;

	RatingGiver ratingGiverForSibling2Leters= getRatingGiver(argv[2]);
	std::cout << "I've got RatingGiver for Sibling two Letters readed " << std::endl;
	
	auto ratingForPageGiver = getRatingGiverForPage(argv[2]);
	std::cout << "I've got RatingGiver for pages  " << std::endl;
	
	NarrowerSearches narrowerSearches(inputData, ratingGiverForSibling2Leters);
	std::cout << "NarrowerSearches initialized" << std::endl;
	auto sugestedPages = narrowerSearches.getBestSugestions(10000);
	std::cout << "I've got "<<sugestedPages.size()<<"suggested pages   " << std::endl;

	std::vector<int> scores(sugestedPages.size());
	std::vector<int> indexes(sugestedPages.size());

	for (int i = 0; i < sugestedPages.size(); i++) {
		scores[i] = ratingForPageGiver.getScore(sugestedPages[i]);
		indexes[i] = i;
	}

	std::sort(indexes.begin(), indexes.end(),[&scores](const auto & ix1, const auto & ix2) {
		return scores[ix1] < scores[ix2];
	});
	for (int i = 0; i < indexes.size(); i++)
		std::cout << indexes[i]<< "  ";
	std::cout << std::endl;
	if (indexes.size()) {
		for (int i = indexes.size() - 30; i < indexes.size();i++) {
			int a = indexes[i];
			auto &bestPage = sugestedPages[a];
			for (auto & row : bestPage)
				std::cout << row << std::endl;
			std::cout << std::endl;
		}

		for (int i = 0; i < indexes.size(); i++)
		{
			std::cout << "<" << scores[indexes[i]] << ">";
		}
		std::cout << std::endl;

	}
	else {
		std::cerr << "Nothing to retun " << std::endl;
	}



	return 0;
}
