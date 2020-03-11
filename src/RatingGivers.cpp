#include "RatingGivers.h"
#include <iostream>
#include <fstream>
#include <string.h>



void RatingGiver::addWord(const std::string & word) {
	if (word.size() > 0) {
		TwoChars  tw;
		for (size_t i = 0, ik = word.size() - 1;
			i < ik;
			i++) {
			tw[0] = word[i];
			tw[1] = word[i + 1];
			add(tw);
		}
	/*	tw[0] = 0;
		tw[1] = word[0];
		add(tw);

		tw[1] = toupper(word[0]);
		add(tw);

		tw[0] = word.back();
		tw[1] = 0;
		add(tw);
	*/}
}


std::set<std::string> ReadDictBasicToSet(const std::string filePath) {
	std::set<std::string> dict;
	std::string word;

	// here we should add try-catch block
	std::ifstream file;
	file.open(filePath, std::ios_base::in);
	while (file.good())
	{
		file >> word;
		dict.insert(word);
	}
	file.close();

	return dict;
}
RatingForPageGiver  getRatingGiverForPage(const std::string & filePath) {
	return RatingForPageGiver(ReadDictBasicToSet(filePath));
}



