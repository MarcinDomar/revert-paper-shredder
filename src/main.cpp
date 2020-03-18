
#include <iostream>
#include <stdexcept>
#include "utillites.h"
#include "RatingGivers.h"
#include "SmallPermutationNarrower.h"
#include "SmallPermutationSelector.h"

using namespace std;


int main_() {
	std::cout << sizeof(size_t) << "size_t" << std::endl;
	InitializerOfIndexsNPermutation<4> ini(5);
	auto f = ini.getFirst();
	int errors = 0;
	for (size_t j, i = 0; i < ini.getSizeOfAllPermutations(); i++, ini.initToNext(f)) {
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
	using ColIndex = unsigned char;
	constexpr int CharsSize =8;
	constexpr int RatierCharsSize = 10;//Variable used to build dictionaries for scoring small permutaions shoud be at least as beg as CharsSize
	constexpr int OverlappingSize =2; //Varliable use in simple arlgorithm which  select small permutation
	static_assert(CharsSize % 2 == 0, "CharsSize should be multipication of 2 ");
	static_assert(OverlappingSize%2==0&& OverlappingSize < CharsSize && OverlappingSize>0," OverlappingSize should be even and greater then 0 and smaller then CharsSize");
	if (argc != 3) {
		std::cerr << "As first parameter pass path to file with shredded text" << std::endl;
		std::exit(-1);
	}

	auto inputStripes = getVectorOfRows(argv[1]);
	std::cout << "input readed " << std::endl;

	auto ratingGiver = getRatingGiver<CombinedRatingGiver<RatierCharsSize>>(argv[2]);
	std::cout << "I've got RatingGiver for Sibling  Letters readed " << std::endl;
	SmallPermutationNarrower<CharsSize,CombinedRatingGiver<RatierCharsSize>, ColIndex> spermutationNarrower(inputStripes,ratingGiver);
	auto vecIndexes=spermutationNarrower();
	std::cout << "Narrow "<<(CharsSize/2)<<" from "<<inputStripes.front().size()<<" permutations , finded " <<vecIndexes.size()<<" intersting permutations" << std::endl;
	SmallPermutationSelector<CharsSize / 2, ColIndex> narrowerSelector(vecIndexes,(int)inputStripes.front().size() );
	auto suggestedPages1 = narrowerSelector(2);
	auto suggestedPages2 = narrowerSelector.simpleAlgorithm<OverlappingSize>(14000);

	std::cout << "I've got from sophisticated algorithm " << suggestedPages1.size() <<" and "<<suggestedPages2.size() << " from simpler algorithm suggested pages   " << std::endl;

	PermutationRatier< CombinedRatingGiver<RatierCharsSize>> permutationRatier(inputStripes, ratingGiver);
	auto showResult = [&](auto &suggestedPages) {
		auto vecScoreIx = permutationRatier(suggestedPages);
		if (vecScoreIx.size()) {
			for (size_t i = vecScoreIx.size() > 4 ? vecScoreIx.size() - 4 : 0; i < vecScoreIx.size(); i++) {

				auto bestPage = getPageSideFromColumnPermutation(inputStripes, suggestedPages[vecScoreIx[i].ix]);
				for (auto & row : bestPage)
					std::cout << row << std::endl;
				std::cout << "Page scored : " << vecScoreIx[i].score << "\n" << std::endl;
			}
			std::cout << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;


		}
		else {
			std::cerr << "Nothing to return " << std::endl;
		}
	};
	showResult(suggestedPages1);
	showResult(suggestedPages2);

	int i;
	std::cin >> i;
	return 0;
}
