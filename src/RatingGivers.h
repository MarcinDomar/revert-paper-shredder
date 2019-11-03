#pragma once
#include "Types.h"
#include <map>
#include <set>
#include <cstdio>

namespace std {
	template <>
	struct hash<TwoChars>
	{
		std::size_t operator()(const TwoChars& k) const
		{
			using std::size_t;
			using std::hash;
			unsigned short val = ((unsigned short)k.back() << 8) + k.front();
			return std::hash<unsigned short>()(val);
		}
	};

}
class RatingForPageGiver {
	const std::set<std::string> set_of_words;
	const std::string delim;
public :

	RatingForPageGiver(std::set<std::string> && setOfWords, std::string delim = " ,.;?") :set_of_words(setOfWords),delim(delim) {}
	RatingForPageGiver(std::set<std::string> & setOfWords, std::string delim = " ,.;?") :set_of_words(setOfWords), delim(delim) {}
	unsigned int getScore(const PaperSide& page)const ;
};

RatingForPageGiver  getRatingGiverForPage(const std::string & filePath);




class RatingGiver :private std::unordered_map<TwoChars, unsigned int > {
	inline void add(const TwoChars & a);
public:
	void addWord(const std::string & word);
	unsigned int getScore(TwoChars & tw)const {
		try {
			return  at(tw);
		}
		catch (...) {
			return 0;
		}
	}
};


RatingGiver getRatingGiver(const std::string filePath);

void RatingGiver::add(const TwoChars & twoChars) {
	value_type e{ twoChars,1 };
	auto it_bool = insert(e);
	if (!it_bool.second)
		it_bool.first->second++;
}




