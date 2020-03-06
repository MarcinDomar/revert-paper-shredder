#include <iostream>
#include <stdexcept>
#include "utillites.h"
#include "RatingGivers.h"
#include "NarrowerSearches.h"
using namespace std;


int main_() {
	std::cout << sizeof(size_t) << "size_t" << std::endl;
	InitializerOfIndexsNPermutation<4> ini(5);
	auto f = ini.getFirst();
	int errors = 0;
	for (size_t j, i = 0; i < ini.howManyPermutation(); i++, ini.initToNext(f)) {
		auto ix = ini.getIndexes((int)i);
		for (j = 0; j < 4 && f[j] == ix[j]; j++);
		if (j != 4)
			errors++;
	}
	std::cout << errors << "errors" << std::endl;
	int i;
	std::cin >> i;
	return 0;

}

int main(int argc, char *argv[])
{
	constexpr int CharsSize = 8;
	constexpr int OverlapingSize =2;
	if (argc != 3) {
		std::cerr << "As first parameter pass path to file with shredded text" << std::endl;
		std::exit(-1);
	}

	auto inputData = getVectorOrRows(argv[1]);
	std::cout << "input readed " << std::endl;

	auto ratingGiverForSibling2Leters = getRatingGiver<CombinedRaingGiver<CharsSize>>(argv[2]);
	std::cout << "I've got RatingGiver for Sibling  Letters readed " << std::endl;

	auto ratingForPageGiver = getRatingGiverForPage(argv[2]);
	std::cout << "I've got RatingGiver for pages  " << std::endl;

	NarrowerSearches<CharsSize> narrowerSearches(inputData, ratingGiverForSibling2Leters);
	std::cout << "NarrowerSearches initialized" << std::endl;
	auto sugestedPages = narrowerSearches.getBestSugestions<OverlapingSize>(1000);
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
	if (indexes.size()) {
		for (size_t i = indexes.size()>10?indexes.size() - 10:0; i < indexes.size(); i++) {
			int a = indexes[i];
			auto &bestPage = sugestedPages[a];
			for (auto & row : bestPage)
				std::cout << row << std::endl;
			std::cout << std::endl;
		}

		for (size_t i = indexes.size()>100?indexes.size()-20:0; i < indexes.size(); i++)
		{
			std::cout << "<" << indexes[i] << " " << scores[indexes[i]] << ">";
		}
		std::cout << std::endl;

	}
	else {
		std::cerr << "Nothing to return " << std::endl;
	}

	int i;
	std::cin >> i;
	return 0;
}
