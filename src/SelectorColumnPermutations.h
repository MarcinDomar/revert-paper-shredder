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
#include "RatingGivers.h"
#include <fstream>
#include<future>
#include<unordered_set>
#include "PresenceRegister.h"

template <int PermutationSize, int OverlappingSize, typename ColIndexType, typename PresenceRegisterType = PresenceRegister>
class SelectorOfBestPosssibleColumnPermutations {
	using PositionType = int;
	using Indexes = std::array<ColIndexType, PermutationSize>;
	using ColumnsPermutation = std::vector<ColIndexType>;
	using ListPageColumnsPermutations = std::list<ColumnsPermutation>;

	struct IndexesExt :Indexes {
		IndexesExt(const Indexes& indexes) :Indexes(indexes) {
			presence_register.add(indexes);
		}
		PresenceRegisterType presence_register;
		std::vector<PositionType > connections;

		static void sortConnections(std::list<PositionType> & scores, std::vector<PositionType> & connections) {
			std::vector < std::pair< PositionType, PositionType>> vecPosScores(connections.size());
			auto itScores = scores.begin();
			for (size_t i = 0; i < vecPosScores.size(); i++, itScores++) {
				vecPosScores[i].first = (PositionType)i;
				vecPosScores[i].second = *itScores;
			}
			std::sort(vecPosScores.begin(), vecPosScores.end(), [](auto & l, auto &r) { return l.second < r.second; });
			auto connectionsCopy = std::move(connections);
			connections.assign(connectionsCopy.size(), 0);
			for (size_t i = 0; i < vecPosScores.size(); i++)
				connections[i] = connectionsCopy[vecPosScores[i].first];
		}

		template <int OverlappingSize_>
		static bool canBeJoined(const IndexesExt & iL, const IndexesExt &iR) {
			size_t i = 0;
			for (; i < OverlappingSize_ && iL[PermutationSize - OverlappingSize_ + i] == iR[i]; i++);
			if (i != OverlappingSize_)
				return false;
			using RArray = std::array<ColIndexType, PermutationSize - OverlappingSize_>;
			PresenceRegisterType p1(*reinterpret_cast<const RArray*>(iL.data())), p2(*reinterpret_cast<const RArray*>(iR.data() + OverlappingSize_));
			return !p1.isThereCommonPart(p2);
		}

		template<>
		static bool canBeJoined<0>(const IndexesExt &iL, const IndexesExt &iR) {
			return !iL.presence_register.isThereCommonPart(iR.presence_register);
		}

		template < typename Pred, typename ScoreGiver>
		static void trySetUpConnection(const IndexesExt & ixLeft, const IndexesExt & ixRight, std::list<PositionType > & scores, std::vector<PositionType> & connections, PositionType ix, Pred && isConnectionInOkIndexes, ScoreGiver && scoregiver) {
			if (canBeJoined<OverlappingSize>(ixLeft, ixRight))
			{
				Indexes temp;
				int i = 1, j = 0;
				PositionType score = std::abs(ix);
				for (; i < PermutationSize - OverlappingSize; i++) {
					for (int k = i; k < PermutationSize; k++, j++)
						temp[j] = ixLeft[k];
					for (int k = OverlappingSize; j < PermutationSize; k++, j++)
						temp[j] = (ixRight[k]);
					if (!isConnectionInOkIndexes(temp))
						break;
					score += scoregiver(temp);
				}
				if (i >= PermutationSize - OverlappingSize) {
					connections.push_back(ix);
					scores.push_back(score);
				}
			}
		}

		template < typename Pred, typename ScoreGiver>
		void findAndAddPosibleConnections(const std::vector<IndexesExt> & vecIndexesExt, Pred &&isConnectionOk, ScoreGiver && scoregiver) {
			std::list< PositionType> scores;
			connections.clear();
			for (size_t j = 0; j < vecIndexesExt.size(); j++) {
				trySetUpConnection(*this, vecIndexesExt[j], scores, connections, (PositionType)j + 1, isConnectionOk, scoregiver);
				trySetUpConnection(vecIndexesExt[j], *this, scores, connections, -(PositionType)j - 1, isConnectionOk, scoregiver);
			}
			sortConnections(scores, connections);
		}
	};

	class GrowingColumnsPermutationCtrl {
		std::unique_ptr<ColIndexType[]> buff;
		PresenceRegisterType presence_register;
		unsigned int size_;
		ColIndexType *beg;
	public:
		GrowingColumnsPermutationCtrl(unsigned int colSize, const IndexesExt& indexes) :buff(new ColIndexType[2 * colSize - indexes.size()]) {
			init(colSize, indexes);

		}

		size_t size()const { return size_; }

		void init(unsigned int colSize, const IndexesExt & indexes) {
			beg = buff.get() + colSize - indexes.size();
			std::copy(indexes.begin(), indexes.end(), beg);
			size_ = (unsigned int)indexes.size();
			presence_register.reset();
			presence_register.add(indexes);
		}

		bool tryAddRight(const IndexesExt & indexes) {
			auto & newIndexes = *reinterpret_cast<const std::array<ColIndexType, PermutationSize - OverlappingSize>*>(indexes.data() + OverlappingSize);
			if (!presence_register.isThereCommonPart(newIndexes))
			{
				presence_register.add(newIndexes);
				std::copy(newIndexes.begin(), newIndexes.end(), beg + size_);
				size_ += (unsigned int)newIndexes.size();
				return true;
			}
			return false;
		}

		bool tryAddLeft(const IndexesExt & indexes) {
			auto & newIndexes = *reinterpret_cast<const std::array<ColIndexType, PermutationSize - OverlappingSize>*>(indexes.data());

			if (!presence_register.isThereCommonPart(newIndexes)) {
				presence_register.add(newIndexes);
				std::copy(newIndexes.begin(), newIndexes.end(), beg - newIndexes.size());
				beg -= newIndexes.size();
				size_ += (unsigned int)newIndexes.size();
				return true;
			}
			return false;
		}

		std::vector<ColIndexType>  getPermutation()const { return std::vector<ColIndexType>(beg, beg + size_); }

		std::vector<ColIndexType> getMissing(int columnsSize) {
			return presence_register.getMissingColumns<ColIndexType>(columnsSize);
		}

		void removeLastRight() {
			presence_register.subtract(*reinterpret_cast<std::array<ColIndexType, PermutationSize - OverlappingSize>*>(beg + (size_ - PermutationSize + OverlappingSize)));
			size_ -= PermutationSize - OverlappingSize;

		}

		void removeLastLeft() {
			presence_register.subtract(*reinterpret_cast<std::array<ColIndexType, PermutationSize - OverlappingSize>*>(beg));
			beg += PermutationSize - OverlappingSize;
			size_ -= PermutationSize - OverlappingSize;
		}

	};

	std::vector<IndexesExt> vecIndexesExt;
	std::unordered_map<Indexes, unsigned int> indexesPositionsMap;
	const int columnSize, columnSize_;

	void initStructersOfConnectoins();

	void findMissingTailAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const;

	void findMissingHeadAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const;

	void buildPageList(const IndexesExt & el, ListPageColumnsPermutations & outList, GrowingColumnsPermutationCtrl & permutationCtrl)const;

	ListPageColumnsPermutations searching(size_t ixBeg, size_t size)const;


	mutable int numberOfAttemptsPerLevel;
public:
	SelectorOfBestPosssibleColumnPermutations(const std::vector<Indexes> & vecIndexes, int columnSize) :vecIndexesExt(vecIndexes.begin(), vecIndexes.end()),
		columnSize(columnSize),
		columnSize_(PermutationSize + (columnSize - PermutationSize) / (PermutationSize - OverlappingSize)*(PermutationSize - OverlappingSize)) {
		for (unsigned int i = 0; (size_t)i < vecIndexes.size(); i++)
			indexesPositionsMap.insert(std::make_pair(*(vecIndexes.begin() + i), i));
		initStructersOfConnectoins();
	}


	std::vector<ColumnsPermutation >  operator () (int numberOfAttemptsPerLevel = 1, unsigned int narrowSizeOfSetToFirstOf = 100) const {
		this->numberOfAttemptsPerLevel = numberOfAttemptsPerLevel;
		using Futures = std::list < std::future<ListPageColumnsPermutations>>;
		Futures futures;
		ListPageColumnsPermutations retList;
		size_t repSize = vecIndexesExt.size()*narrowSizeOfSetToFirstOf / 100;
		size_t i, delta = repSize / std::thread::hardware_concurrency();
		size_t size_first = repSize - (std::thread::hardware_concurrency() - 1)*delta;
		for (i = 0; i < std::thread::hardware_concurrency() - 1; i++)
			futures.push_back(std::async(std::launch::async, (&SelectorOfBestPosssibleColumnPermutations::searching), this, size_first + i*delta, delta));
		retList = (this->*(&SelectorOfBestPosssibleColumnPermutations::searching))(0, size_first);

		while (futures.size()) {
			auto rl = futures.front().get();
			retList.insert(retList.end(), rl.begin(), rl.end());
			futures.pop_front();
		}
		return{ retList.begin(),retList.end() };
	}
};



template <int PermutationSize, int OverlappingSize, typename ColIndexType, typename PresenceRegisterType  >
void SelectorOfBestPosssibleColumnPermutations<PermutationSize, OverlappingSize, ColIndexType, PresenceRegisterType>::initStructersOfConnectoins() {
	auto isConnectionOk = [this](const Indexes  indexes) {
		return indexesPositionsMap.find(indexes) != indexesPositionsMap.end();
	};

	auto scoregiver = [this](auto & indexes) {return indexesPositionsMap[indexes]; };

	makeWorkParallel(vecIndexesExt.size(), [&](size_t ixBeg, size_t size) {
		for (auto ix = ixBeg, ixEnd = ixBeg + size; ix < ixEnd; ix++) {
			vecIndexesExt[ix].findAndAddPosibleConnections(vecIndexesExt, isConnectionOk, scoregiver);
		}
		return 0;
	}
	);
}


template <int PermutationSize, int OverlappingSize, typename ColIndexType, typename PresenceRegisterType  >
void SelectorOfBestPosssibleColumnPermutations<PermutationSize, OverlappingSize, ColIndexType, PresenceRegisterType>::findMissingTailAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const
{
	Indexes tem;
	size_t s = PermutationSize - missing.size();
	std::copy(permutation.end() - s, permutation.end(), tem.begin());
	auto push_back_tem = [&]() {
		if (indexesPositionsMap.find(tem) != indexesPositionsMap.end()) {
			auto permWithTail = permutation;
			permWithTail.insert(permWithTail.end(), tem.end() - missing.size(), tem.end());
			outList.push_back(permWithTail);
		}
	};

	if (missing.size() == 1) {
		tem.back() = missing.front();
		push_back_tem();
	}
	else {
		do {
			std::copy(missing.begin(), missing.end(), tem.begin() + s);
			push_back_tem();
		} while (std::next_permutation(missing.begin(), missing.end()));
	}
}


template <int PermutationSize, int OverlappingSize, typename ColIndexType, typename PresenceRegisterType  >
void SelectorOfBestPosssibleColumnPermutations<PermutationSize, OverlappingSize, ColIndexType, PresenceRegisterType>::findMissingHeadAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const
{
	Indexes tem;
	size_t s = PermutationSize - missing.size();
	std::copy(permutation.begin(), permutation.begin() + s, tem.begin() + missing.size());
	auto push_back_tem = [&]() {
		if (indexesPositionsMap.find(tem) != indexesPositionsMap.end()) {
			auto permWithHead = permutation;
			permWithHead.insert(permWithHead.begin(), tem.begin(), tem.begin() + missing.size());
			outList.push_back(permWithHead);
		}
	};

	if (missing.size() == 1) {
		tem.front() = missing.front();
		push_back_tem();
	}
	else {
		do {
			std::copy(missing.begin(), missing.end(), tem.begin());
			push_back_tem();
		} while (std::next_permutation(missing.begin(), missing.end()));
	}
}


template <int PermutationSize, int OverlappingSize, typename ColIndexType, typename PresenceRegisterType  >
void SelectorOfBestPosssibleColumnPermutations<PermutationSize, OverlappingSize, ColIndexType, PresenceRegisterType>::buildPageList(
	const IndexesExt & el, ListPageColumnsPermutations & outList, GrowingColumnsPermutationCtrl & permutationCtrl)const
{
	auto continueDeeper = [&](const IndexesExt& candidate) {
		if (permutationCtrl.size() == columnSize_) {
			if (columnSize > columnSize_) {
				findMissingTailAndAdd(outList, permutationCtrl.getPermutation(), permutationCtrl.getMissing(columnSize));
				findMissingHeadAndAdd(outList, permutationCtrl.getPermutation(), permutationCtrl.getMissing(columnSize));
			}
			else {
				outList.push_back(permutationCtrl.getPermutation());
			}
		}
		else
			buildPageList(candidate, outList, permutationCtrl);
	};

	int counter = 0;

	for (auto & pos : el.connections)
	{
		if (pos > 0)
		{
			auto &candidate = vecIndexesExt[pos - 1];
			if (permutationCtrl.tryAddRight(candidate)) {
				counter++;
				continueDeeper(candidate);

				permutationCtrl.removeLastRight();
				if (counter >= numberOfAttemptsPerLevel)
					break;
			}
		}
		else
		{
			auto &candidate = vecIndexesExt[-pos - 1];
			if (permutationCtrl.tryAddLeft(candidate)) {
				counter++;
				continueDeeper(candidate);

				permutationCtrl.removeLastLeft();
				if (counter >= numberOfAttemptsPerLevel)
					break;
			}
		}
	}
}


template <int PermutationSize, int OverlappingSize, typename ColIndexType, typename PresenceRegisterType  >
typename SelectorOfBestPosssibleColumnPermutations<PermutationSize, OverlappingSize, ColIndexType, PresenceRegisterType>::ListPageColumnsPermutations SelectorOfBestPosssibleColumnPermutations<PermutationSize, OverlappingSize, ColIndexType, PresenceRegisterType>::searching(size_t ixBeg, size_t size)const {
	ListPageColumnsPermutations retList;
	for (auto ix = ixBeg, ixEnd = ixBeg + size; ix < ixEnd; ix++) {
		GrowingColumnsPermutationCtrl permutationCtrl(columnSize, vecIndexesExt[ix]);
		buildPageList(vecIndexesExt[ix], retList, permutationCtrl);
	}
	return retList;
}
