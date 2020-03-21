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
class SmallPermutationSelector {
	using PositionType = int;
	using Indexes = std::array<ColIndexType, PermutationSize>;
	using ColumnsPermutation = std::vector<ColIndexType>;
	using ListPageColumnsPermutations = std::list<ColumnsPermutation>;

	struct IndexesExt :Indexes {
		IndexesExt(const Indexes& indexes) :Indexes(indexes) {
			presence_register.add(indexes);
		}
		PresenceRegisterType presence_register;
		std::vector<PositionType > connectionsRight, connectionsLeft;

		static void sortConnections(std::list<PositionType> & scores, std::vector<PositionType> & connections) {
			std::vector < std::pair< PositionType, PositionType>> vecPosScores(connections.size());
			auto itScores = scores.begin();
			for (size_t i = 0; i < vecPosScores.size(); i++, itScores++) {
				vecPosScores[i].first = (PositionType)i;
				vecPosScores[i].second = *itScores;
			}
			std::sort(vecPosScores.begin(), vecPosScores.end(), [](auto & l, auto &r) { return l.second < r.second; });
			auto connectionsCopy = connections;
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
				PositionType score =ix ;
				for (; i < PermutationSize - OverlappingSize - 1; i++) {
					for (int k = PermutationSize - i; k < PermutationSize; k++, j++)
						temp[j] = ixLeft[k];
					for (int k = 0; k < PermutationSize - OverlappingSize - i; k++, j++)
						temp[j] = (ixRight[k + OverlappingSize]);
					if (!isConnectionInOkIndexes(temp))
						break;
					score += scoregiver(temp);
				}
				if (i >= PermutationSize - OverlappingSize - 1)   {
					connections.push_back(ix);
					scores.push_back(score);
				}
			}
		}


		template < typename Pred, typename ScoreGiver>
		void findAndAddPosibleConnections(const std::vector<IndexesExt> & vecIndexesExt, Pred &&isConnectionOk, ScoreGiver && scoregiver) {
			std::list<unsigned int > scoresRight, scoresLeft;
			connectionsLeft.clear();
			connectionsRight.clear();
			for (size_t j = 0; j < vecIndexesExt.size(); j++) {
				trySetUpConnection(*this, vecIndexesExt[j], scoresRight, connectionsRight, (PositionType)j, isConnectionOk, scoregiver);
				trySetUpConnection(vecIndexesExt[j], *this, scoresLeft, connectionsLeft, (PositionType)j, isConnectionOk, scoregiver);
			}
			sortConnections(scoresRight, connectionsRight);
			sortConnections(scoresLeft, connectionsLeft);
		}
	};

	class GrowingColumnsPermutationCtrl {
		std::vector<ColIndexType> permutation;
		PresenceRegisterType presence_register;
	public:
		GrowingColumnsPermutationCtrl(const IndexesExt& indexes) {
			init(indexes);
		}

		size_t size()const { return permutation.size(); }

		void init(const IndexesExt & indexes) {
			permutation.clear();
			presence_register.reset();
			permutation.insert(permutation.end(), indexes.begin(), indexes.end());
			presence_register.add(indexes);
		}

		bool tryAddRight(const IndexesExt & indexes) {
			auto & newIndexes = *reinterpret_cast<const std::array<ColIndexType, PermutationSize - OverlappingSize>*>(indexes.data() + OverlappingSize);
			if (!presence_register.isThereCommonPart(newIndexes))
			{
				presence_register.add(newIndexes);
				permutation.insert(permutation.end(), newIndexes.begin(), newIndexes.end());

				return true;
			}
			return false;
		}

		bool tryAddLeft(const IndexesExt & indexes) {
			auto & newIndexes = *reinterpret_cast<const std::array<ColIndexType, PermutationSize - OverlappingSize>*>(indexes.data());

			if (!presence_register.isThereCommonPart(newIndexes)) {
				presence_register.add(newIndexes);
				permutation.insert(permutation.begin(), newIndexes.begin(), newIndexes.end());
				return true;
			}
			return false;
		}


		const std::vector<ColIndexType> & getPermutation()const { return permutation; }
		std::vector<ColIndexType> getMissing(int columnsSize) {
			return presence_register.getMissingColumns<ColIndexType>(columnsSize);
		}

		void removeLastRight() {
			presence_register.subtract(*reinterpret_cast<std::array<ColIndexType, PermutationSize - OverlappingSize>*>(permutation.data() + permutation.size() - PermutationSize + OverlappingSize));
			permutation.erase(permutation.end() - PermutationSize + OverlappingSize, permutation.end());
		}

		void removeLastLeft() {
			presence_register.subtract(*reinterpret_cast<std::array<ColIndexType, PermutationSize - OverlappingSize>*>(permutation.data()));
			permutation.erase(permutation.begin(), permutation.begin() +PermutationSize- OverlappingSize);
		}

	};

	std::vector<IndexesExt> vecIndexesExt;
	std::unordered_map<Indexes, unsigned int> indexesPositionsMap;
	const int columnSize, columnSize_;

	void initStructersOfConnectoins();

	void findMissingTailAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const;

	void findMissingHeadAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const;

	void searchForPageColumnPermutationRight(const IndexesExt & el, ListPageColumnsPermutations & outList, GrowingColumnsPermutationCtrl & permutationCtrl)const;

	ListPageColumnsPermutations sofisticateSearching(size_t ixBeg, size_t size)const;



	std::vector<ColumnsPermutation>  parallelLoop(ListPageColumnsPermutations(SmallPermutationSelector::*sercher)(size_t, size_t)const, size_t repSize)const;
	mutable int numberOfAttemptsPerLevel;
public:
	SmallPermutationSelector(const std::vector<Indexes> & vecIndexes, int columnSize) :vecIndexesExt(vecIndexes.begin(), vecIndexes.end()),
		columnSize(columnSize),
		columnSize_(PermutationSize+(columnSize-PermutationSize) / (PermutationSize-OverlappingSize)*(PermutationSize-OverlappingSize)) {
		std::cout << columnSize << "  " << columnSize_ << std::endl;
		for (unsigned int i = 0; (size_t)i < vecIndexes.size(); i++)
			indexesPositionsMap.insert(std::make_pair(*(vecIndexes.begin() + i), i));
		initStructersOfConnectoins();
	}
	std::vector<ColumnsPermutation >  operator () (int numberOfAttemptsPerLevel = 2) const {
		this->numberOfAttemptsPerLevel = numberOfAttemptsPerLevel;
		return parallelLoop(&SmallPermutationSelector::sofisticateSearching, vecIndexesExt.size());
	}


};


template <int PermutationSize,int OverlappingSize, typename ColIndexType, typename PresenceRegisterType  >
void SmallPermutationSelector<PermutationSize,OverlappingSize, ColIndexType, PresenceRegisterType>::initStructersOfConnectoins() {
	auto isConnectionOk = [this](const Indexes  indexes) {
		return indexesPositionsMap.find(indexes) != indexesPositionsMap.end();
	};

	auto scoregiver = [this](auto & indexes) {return indexesPositionsMap[indexes]; };

	std::list<std::future<int> > futures;
	auto worker = [&](size_t ixBeg, size_t size) {
		for (auto ix = ixBeg, ixEnd = ixBeg + size; ix < ixEnd; ix++) {
			vecIndexesExt[ix].findAndAddPosibleConnections(vecIndexesExt, isConnectionOk, scoregiver);
		}
		return 0;
	};
	size_t i = 0, delta = vecIndexesExt.size() / std::thread::hardware_concurrency();
	for (; i < std::thread::hardware_concurrency() - 1; i++)
		futures.push_back(std::async(std::launch::async, worker, i*delta, delta));
	worker(i*delta, vecIndexesExt.size() - i*delta);
	while (futures.size()) {
		futures.front().get();
		futures.pop_front();
	}
}

template <int PermutationSize,int OverlappingSize, typename ColIndexType, typename PresenceRegisterType  >
void SmallPermutationSelector<PermutationSize, OverlappingSize,ColIndexType, PresenceRegisterType>::findMissingTailAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const
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

template <int PermutationSize,int OverlappingSize, typename ColIndexType, typename PresenceRegisterType  >
void SmallPermutationSelector<PermutationSize,OverlappingSize, ColIndexType, PresenceRegisterType>::findMissingHeadAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const
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
			std::copy(missing.begin(), missing.end(), tem.begin() );
			push_back_tem();
		} while (std::next_permutation(missing.begin(), missing.end()));
	}
}

template <int PermutationSize,int OverlappingSize, typename ColIndexType, typename PresenceRegisterType  >
void SmallPermutationSelector<PermutationSize,OverlappingSize, ColIndexType, PresenceRegisterType>::searchForPageColumnPermutationRight(
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
			searchForPageColumnPermutationRight(candidate, outList, permutationCtrl);
	};

	int counter = 0;

	for (auto & pos : el.connectionsRight)
	{
		auto &candidate = vecIndexesExt[pos];
		if (permutationCtrl.tryAddRight(candidate)) {
			counter++;
			continueDeeper(candidate);

			permutationCtrl.removeLastRight();
			if (counter >= numberOfAttemptsPerLevel)
				break;

		}
	}
	counter = 0;
	for (auto & pos : el.connectionsLeft)
	{
		auto &candidate = vecIndexesExt[pos];
		if (permutationCtrl.tryAddLeft(candidate)) {
			counter++;
			continueDeeper(candidate);

			permutationCtrl.removeLastLeft();
			if (counter >= numberOfAttemptsPerLevel)
				break;
		}
	}

}

template <int PermutationSize,int OverlappingSize ,typename ColIndexType, typename PresenceRegisterType  >
typename SmallPermutationSelector<PermutationSize,OverlappingSize ,ColIndexType, PresenceRegisterType>::ListPageColumnsPermutations SmallPermutationSelector<PermutationSize,OverlappingSize, ColIndexType, PresenceRegisterType>::sofisticateSearching(size_t ixBeg, size_t size)const {
	ListPageColumnsPermutations retList;
	for (auto ix = ixBeg, ixEnd = ixBeg + size; ix < ixEnd; ix++) {
		GrowingColumnsPermutationCtrl permutationCtrl(vecIndexesExt[ix]);
		searchForPageColumnPermutationRight(vecIndexesExt[ix], retList, permutationCtrl);
	}
	return retList;
}

template <int PermutationSize, int OverlappingSize,typename ColIndexType, typename PresenceRegisterType  >
std::vector<std::vector<ColIndexType>> SmallPermutationSelector<PermutationSize,OverlappingSize ,ColIndexType, PresenceRegisterType>::parallelLoop(ListPageColumnsPermutations(SmallPermutationSelector<PermutationSize, OverlappingSize,ColIndexType, PresenceRegisterType>::*searcher)(size_t, size_t)const, size_t repSize) const {
	using Futures = std::list < std::future<ListPageColumnsPermutations>>;
	Futures futures;
	ListPageColumnsPermutations retList;
	size_t i = 0, delta = repSize / std::thread::hardware_concurrency();
	for (; i < std::thread::hardware_concurrency() - 1; i++)
		futures.push_back(std::async(std::launch::async, searcher, this, i*delta, delta));
	retList = (this->*searcher)(i*delta, repSize - i*delta);

	while (futures.size()) {
		auto rl = futures.front().get();
		retList.insert(retList.end(), rl.begin(), rl.end());
		futures.pop_front();
	}
	return{ retList.begin(),retList.end() };
}
