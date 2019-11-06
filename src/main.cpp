#include <iostream>
#include <stdexcept>
#include "Types.h"
#include "utillites.h"
#include "RatingGivers.h"
#include "NarrowerSearches.h"
using namespace std;




int main(int argc, char *argv[])
{
	if (argc != 3) {
		std::cerr << "As first parameter pass path to file with shredded text" << std::endl;
		std::exit(-1);
	}

	auto inputData = getVectorOrRows(argv[1]);
	std::cout << "input readed " << std::endl;

	auto ratingGiverForSibling2Leters = getRatingGiver<CombinedRaingGiver<6>>(argv[2]);
	std::cout << "I've got RatingGiver for Sibling 6 Letters readed " << std::endl;

	auto ratingForPageGiver = getRatingGiverForPage(argv[2]);
	std::cout << "I've got RatingGiver for pages  " << std::endl;

	NarrowerSearches narrowerSearches(inputData, ratingGiverForSibling2Leters);
	std::cout << "NarrowerSearches initialized" << std::endl;
	auto sugestedPages = narrowerSearches.getBestSugestions(100000);
	std::cout << "I've got " << sugestedPages.size() << "suggested pages   " << std::endl;

	std::vector<int> scores(sugestedPages.size());
	std::vector<int> indexes(sugestedPages.size());

	for (int i = 0; i < sugestedPages.size(); i++) {
		scores[i] = ratingForPageGiver.getScore(sugestedPages[i]);
		indexes[i] = i;
	}

	std::sort(indexes.begin(), indexes.end(), [&scores](const auto & ix1, const auto & ix2) {
		return scores[ix1] < scores[ix2];
	});
	for (int i = 0; i < indexes.size(); i++)
		std::cout << indexes[i] << "  ";
	std::cout << std::endl;
	if (indexes.size()) {
		for (int i = indexes.size() - 30; i < indexes.size(); i++) {
			int a = indexes[i];
			auto &bestPage = sugestedPages[a];
			for (auto & row : bestPage)
				std::cout << row << std::endl;
			std::cout << std::endl;
		}

		for (int i = indexes.size() - 100; i < indexes.size(); i++)
		{
			std::cout << "<" << indexes[i] << " " << scores[indexes[i]] << ">";
		}
		std::cout << std::endl;

	}
	else {
		std::cerr << "Nothing to retun " << std::endl;
	}

	int i;
	std::cin >> i;
	return 0;
}
