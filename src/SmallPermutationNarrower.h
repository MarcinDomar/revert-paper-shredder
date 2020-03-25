#pragma once
/*
Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2020 Marcin Domarski.
Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "types.h"
#include "InitializerOfIndexes.h"
#include <vector>
#include <list>

template <int CharsSize, typename RatingGiver>
struct ScoreCounter {
	mutable std::array<char, CharsSize> chars;
	const RatingGiver & ratingGiver;
	const VectorOfRows &rows;
public:
	ScoreCounter(const RatingGiver & ratingGiver, const VectorOfRows & rows) : rows(rows), ratingGiver(ratingGiver) {}

	template <typename ColIndexType>
	auto  operator ()(const std::array<ColIndexType, CharsSize / 2> & indexes) const{
		short score = 0;
		for (auto & r : rows) {
			size_t off = 0;
			for (auto ix : indexes) {
				auto & twoChars = r[ix];
				std::copy(twoChars.begin(), twoChars.end(), chars.begin() + off);
				off += 2;
			}
			score += ratingGiver.getScore(chars);
		}
		return score;
	}
};


template <int CharsSize, typename RatierType ,typename ColIndexType = unsigned char>
class SmallPermutationNarrower {
	static constexpr int PermutationSize = CharsSize / 2;
	const InitializerOfIndexesNPermutation<PermutationSize, ColIndexType> indexNexter;
	using Indexes = typename  InitializerOfIndexesNPermutation<PermutationSize, ColIndexType>::Indexes;
	const VectorOfRows & inputStripes;
	const RatierType ratier;

	std::vector<Indexes> getAllIndexes() const{
		Indexes index = indexNexter.getFirst();
		std::vector<Indexes> vecIndexes(indexNexter.getSizeOfAllPermutations());
		for (size_t i = 0; i < vecIndexes.size(); i++) {
			vecIndexes[i] = index;
			indexNexter.initToNext(index);
		}
		return vecIndexes;
	}

	std::vector<Indexes>  getSortedPermutationsScoredOnGreaterThenZero(const std::vector<short> &cols_scores)const;

public:
	SmallPermutationNarrower(const VectorOfRows & inputStripes, const RatierType & ratier):ratier(ratier),inputStripes(inputStripes), indexNexter((int)inputStripes.front().size()) {}

	std::vector<Indexes> operator () ()const;

};

template <int CharsSize, typename RatierType,typename ColIndexType>
std::vector<typename SmallPermutationNarrower<CharsSize,RatierType,ColIndexType>::Indexes>  SmallPermutationNarrower<CharsSize,RatierType, ColIndexType>::getSortedPermutationsScoredOnGreaterThenZero(const std::vector<short> &cols_scores)const {
	std::vector<unsigned int > positions(cols_scores.size());
	auto temVecIndexes = getAllIndexes();
	std::vector<Indexes> vecIndexes(temVecIndexes.size());
	auto cols_predicate = [&cols_scores](auto & p1, auto & p2) {
		return cols_scores[p1] > cols_scores[p2];
	};

	for (unsigned int i = 0; i < positions.size(); i++)	positions[i] = i;
	std::sort(positions.begin(), positions.end(), cols_predicate);

	for (size_t i = 0; i < positions.size(); i++) {
		vecIndexes[i] = temVecIndexes[positions[i]];
	}
	auto size_of_permutations_considarated = std::lower_bound(positions.begin(), positions.end(), 0, cols_predicate) - positions.begin();
	vecIndexes.erase( vecIndexes.begin() + size_of_permutations_considarated, vecIndexes.end());
	return vecIndexes;
}


template <int CharsSize, typename RatierType,typename ColIndexType>
std::vector<typename SmallPermutationNarrower<CharsSize,RatierType, ColIndexType>::Indexes> SmallPermutationNarrower<CharsSize,RatierType, ColIndexType>::operator () ()const {

	auto cols_scores = std::vector< short >(indexNexter.getSizeOfAllPermutations(), 0ll);
	auto worker = [this,&cols_scores](int dist_from_first, int size) {
		typename ScoreCounter<CharsSize,RatierType>  scoreCounter(ratier, inputStripes);

		Indexes index = indexNexter.getIndexes(dist_from_first);
		for (int i = 0; i < size; i++) {
			cols_scores[i + dist_from_first] = scoreCounter(index);
			indexNexter.initToNext(index);
		}
		return 1;
	};
	using Future = std::future<int>;
	std::list<Future> futures;

	int size = (int)indexNexter.getSizeOfAllPermutations() / std::thread::hardware_concurrency();
	size_t i = 0;
	for (i; i < std::thread::hardware_concurrency() - 1; i++) {
		futures.push_back(std::async(std::launch::async, worker, (int)i*size, size));
	}
	worker((int)i*size, (int)indexNexter.getSizeOfAllPermutations() - size*(int)i);

	while (futures.size()) {
		futures.front().get();
		futures.pop_front();
	}
	return getSortedPermutationsScoredOnGreaterThenZero(cols_scores);

}
