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

VectorOrRows getVectorOrRows(const std::string filename) {
	auto listofrows = readListOfRows(filename);
	VectorOrRows result(listofrows.size(), VectorOrRows::value_type(listofrows.front().size()));
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

ColumnsPermutation getPermutationFromOriginPage(const PaperSide & originText, const VectorOrRows & scrumbled) {
	ColumnsPermutation positionInOriginText(scrumbled.front().size());

	for (int i = 0; i < positionInOriginText.size(); i++) {
		for (int j = 0, k; j < positionInOriginText.size(); j++) {
			if (positionInOriginText.begin() + i == std::find(positionInOriginText.begin(), positionInOriginText.begin() + i, j))
			{
				for (k = 0; k < scrumbled.size(); k++) {
					auto & rowO = originText[k];
					auto & rowS = scrumbled[k];
					if (rowO[2 * j] != rowS[i][0] || rowO[2 * j + 1] != rowS[i][1])
						break;
				}
				if (k == scrumbled.size()) {
					positionInOriginText[i] = j;
					break;
				}
			}
		}
	}
	ColumnsPermutation result(positionInOriginText.size());
	for (int i = 0; i < result.size(); i++) {
		result[positionInOriginText[i]] = i;
	}
	return result;
}

unsigned int getRatingOfPermutation(const ColumnsPermutation & origin, const ColumnsPermutation & toScore)
{
	unsigned int score = 0;
	for (int i = 0, j; i < toScore.size(); )
	{
		for (j = 0; j < toScore.size(); j++)
		{
			if (origin[i] == toScore[j])
				break;
		}
		unsigned int rate = 0;
		i++;
		for (; j < toScore.size() && i < toScore.size() && origin[i] == toScore[j]; j++, i++)
			rate++;
		score += rate*rate;
	}
	return score;
}

PaperSide getPageSizeFromColumnPermutation(const VectorOrRows & scrumbled, const ColumnsPermutation & columnsPermutations) {
	auto rows = scrumbled.size();
	PaperSide paperSide;
	for (int row = 0; row != rows; row++) {
		std::string str;
		for (auto ixCol : columnsPermutations)
			str.insert(str.end(), scrumbled[row][ixCol].begin(), scrumbled[row][ixCol].end());
		paperSide.emplace_back(std::move(str));
	}
	return paperSide;
}
