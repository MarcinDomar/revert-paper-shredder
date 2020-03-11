#pragma once
#include "RatingGivers.h"
#include <fstream>
#include<future>
#include<unordered_set>
#include "PresenceRegister.h"

template <int PermutationSize, typename ColIndexType, typename PresenceRegisterType = PresenceRegister>
class SmallPermutationSelector {
	using PositionType = unsigned int;
	using Indexes = std::array<ColIndexType, PermutationSize>;
	using ColumnsPermutation = std::vector<ColIndexType>;
	using ListPageColumnsPermutations = std::list<ColumnsPermutation>;

	struct IndexesExt :Indexes {
		IndexesExt(const Indexes& indexes) :Indexes(indexes) {
			presence_register.add(indexes);
		}
		PresenceRegisterType presence_register;
		std::vector<PositionType > connections;
		template <typename Pred>
		void trySetUpConnection(const IndexesExt & indexesEx, PositionType ix, Pred & isConnectionInOkIndexes) {
			if (!indexesEx.presence_register.isThereCommonPart(presence_register))
			{
				Indexes temp;
				int i = 1, j = 0;
				for (; i < PermutationSize - 1; i++) {
					for (int k = PermutationSize - i; k < PermutationSize; k++, j++)
						temp[j] = (*this)[k];
					for (int k = 0; k < PermutationSize - i; k++, j++)
						temp[j] = (indexesEx[k]);
					if (!isConnectionInOkIndexes(temp))
						break;
				}
				if (i == PermutationSize - 1)
					connections.push_back(ix);
			}
		}
	};

	class GrowingColumnsPermutationCtrl {
		std::vector<ColIndexType> permutation;
		PresenceRegisterType presence_register;
	public:
		GrowingColumnsPermutationCtrl() = default;
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
			if (!presence_register.isThereCommonPart(indexes.presence_register)) {
				presence_register.add(indexes.presence_register);
				permutation.insert(permutation.end(), indexes.begin(), indexes.end());
				return true;
			}
			return false;
		}

		template <int OverlappingSize>
		bool tryAddRight(const IndexesExt & indexes) {
			auto & newIndexes = *reinterpret_cast<const std::array<ColIndexType, PermutationSize - OverlappingSize>*>(indexes.data() + OverlappingSize);
			auto & overlappingIndexes = *reinterpret_cast<const std::array<ColIndexType, OverlappingSize>*>(indexes.data());
			if (!presence_register.isThereCommonPart(newIndexes) && std::equal(overlappingIndexes.begin(), overlappingIndexes.end(), permutation.end() - OverlappingSize))
			{
				presence_register.add(newIndexes);
				permutation.insert(permutation.end(), newIndexes.begin(), newIndexes.end());

				return true;
			}
			return false;
		}

		template <int OverlappingSize>
		bool tryAddLeft(const IndexesExt & indexes) {
			auto & newIndexes = *reinterpret_cast<const std::array<ColIndexType, PermutationSize - OverlappingSize>*>(indexes.data());
			auto & overlappingIndexes = *reinterpret_cast<const std::array<ColIndexType, OverlappingSize>*>(indexes.data() + newIndexes.size());

			if (!presence_register.isThereCommonPart(newIndexes) && std::equal(permutation.begin(), permutation.begin() + OverlappingSize, overlappingIndexes.begin())) {
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

		void removeLast() {
			presence_register.subtract(*reinterpret_cast<Indexes*>(permutation.data() + permutation.size() - PermutationSize));
			permutation.erase(permutation.end() - PermutationSize, permutation.end());
		}

	};

	std::vector<IndexesExt> vecIndexesExt;
	std::unordered_set<Indexes> setOfOkIndexes;
	const int columnSize, columnSize_;

	void initStructersOfConnectoins();

	void findMissingTailAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const;

	void findMissingHeadAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const;

	void searchForPageColumnPermutationRight(
		const IndexesExt & el, ListPageColumnsPermutations & outList, GrowingColumnsPermutationCtrl & permutationCtrl)const;

	ListPageColumnsPermutations sofisticateSearching(size_t ixBeg, size_t size)const;

	template <int OverlappingSize>
	ListPageColumnsPermutations  simpleSearching(size_t ixBeg, size_t size)const;


	std::vector<ColumnsPermutation>  parallelLoop(ListPageColumnsPermutations(SmallPermutationSelector::*sercher)(size_t, size_t)const, size_t repSize)const;
	mutable int numberOfAttemptsPerLevel;
	public:
	SmallPermutationSelector(const std::vector<Indexes> & vecIndexes, int columnSize) :vecIndexesExt(vecIndexes.begin(), vecIndexes.end()),
		setOfOkIndexes(vecIndexes.begin(), vecIndexes.end()),
		columnSize(columnSize),
		columnSize_(columnSize / PermutationSize*PermutationSize) {
		initStructersOfConnectoins();
	}
	std::vector<ColumnsPermutation >  operator () (int numberOfAttemptsPerLevel=2) const {
		this->numberOfAttemptsPerLevel = numberOfAttemptsPerLevel;
		return parallelLoop(&SmallPermutationSelector::sofisticateSearching, vecIndexesExt.size());
	}
	template<int OverlappingSize>
	std::vector<ColumnsPermutation >  simpleAlgorithm(size_t maxSuggestions)const {
		maxSuggestions = maxSuggestions > vecIndexesExt.size() ? vecIndexesExt.size() : maxSuggestions;
		return parallelLoop(&SmallPermutationSelector::simpleSearching<OverlappingSize>, maxSuggestions);
	}
};


template <int PermutationSize, typename ColIndexType, typename PresenceRegisterType  >
void SmallPermutationSelector<PermutationSize, ColIndexType, PresenceRegisterType>::initStructersOfConnectoins() {
	auto isConnectionOk = [this](const Indexes  indexes) {
		return setOfOkIndexes.find(indexes) != setOfOkIndexes.end();
	};
	std::list<std::future<int> > futures;
	auto worker = [&](size_t ixBeg, size_t size) {
		for (auto ix = ixBeg, ixEnd = ixBeg + size; ix < ixEnd; ix++) {
			auto & indexes1 = vecIndexesExt[ix];
			for (size_t j = 0; j < vecIndexesExt.size(); j++)
				indexes1.trySetUpConnection(vecIndexesExt[j], (PositionType)j, isConnectionOk);
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

template <int PermutationSize, typename ColIndexType, typename PresenceRegisterType  >
void SmallPermutationSelector<PermutationSize, ColIndexType, PresenceRegisterType>::findMissingTailAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const
{
	Indexes tem;
	size_t s = PermutationSize - missing.size();
	std::copy(permutation.end() - s, permutation.end(), tem.begin());
	auto push_back_tem = [&]() {
		if (setOfOkIndexes.find(tem) != setOfOkIndexes.end()) {
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

template <int PermutationSize, typename ColIndexType, typename PresenceRegisterType  >
void SmallPermutationSelector<PermutationSize, ColIndexType, PresenceRegisterType>::findMissingHeadAndAdd(ListPageColumnsPermutations & outList, const std::vector<ColIndexType> & permutation, std::vector<ColIndexType> missing)const
{
	Indexes tem;
	size_t s = PermutationSize - missing.size();
	std::copy(permutation.begin(), permutation.begin() + s, tem.begin() + missing.size());
	auto push_back_tem = [&]() {
		if (setOfOkIndexes.find(tem) != setOfOkIndexes.end()) {
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
			std::copy(missing.begin(), missing.end(), tem.begin() + s);
			push_back_tem();
		} while (std::next_permutation(missing.begin(), missing.end()));
	}
}

template <int PermutationSize, typename ColIndexType, typename PresenceRegisterType  >
void SmallPermutationSelector<PermutationSize, ColIndexType, PresenceRegisterType>::searchForPageColumnPermutationRight(
	const IndexesExt & el, ListPageColumnsPermutations & outList, GrowingColumnsPermutationCtrl & permutationCtrl)const
{
	int counter = 0;
	for (auto & pos : el.connections)
	{
		auto &candidate = vecIndexesExt[pos];
		if (permutationCtrl.tryAddRight(candidate)) {
			counter++;

			if (permutationCtrl.size() == columnSize_) {
				if (columnSize > columnSize_) {
					findMissingTailAndAdd(outList, permutationCtrl.getPermutation(), permutationCtrl.getMissing(columnSize));
				}
				else {
					outList.push_back(permutationCtrl.getPermutation());
				}
				permutationCtrl.removeLast();
			}
			else
				searchForPageColumnPermutationRight(candidate, outList, permutationCtrl);
			if (counter >= numberOfAttemptsPerLevel )
				break;
		}
	}
	permutationCtrl.removeLast();
}

template <int PermutationSize, typename ColIndexType, typename PresenceRegisterType  >
typename SmallPermutationSelector<PermutationSize, ColIndexType, PresenceRegisterType>::ListPageColumnsPermutations SmallPermutationSelector<PermutationSize, ColIndexType, PresenceRegisterType>::sofisticateSearching(size_t ixBeg, size_t size)const {
	ListPageColumnsPermutations retList;
	for (auto ix = ixBeg, ixEnd = ixBeg + size; ix < ixEnd; ix++) {
		GrowingColumnsPermutationCtrl permutationCtrl(vecIndexesExt[ix]);
		searchForPageColumnPermutationRight(vecIndexesExt[ix], retList, permutationCtrl);
	}
	return retList;
}

template <int PermutationSize, typename ColIndexType, typename PresenceRegisterType  >
template <int OverlappingSize>
typename SmallPermutationSelector<PermutationSize, ColIndexType, PresenceRegisterType>::ListPageColumnsPermutations  SmallPermutationSelector<PermutationSize, ColIndexType, PresenceRegisterType>::simpleSearching(size_t ixBeg, size_t size)const {

	ListPageColumnsPermutations listOut;
	GrowingColumnsPermutationCtrl permutationCtrl;
	int cols_ = ((int)columnSize - PermutationSize) / (PermutationSize - OverlappingSize)*(PermutationSize - OverlappingSize) + PermutationSize;

	for (auto i = ixBeg, ixEnd = ixBeg + size; i < ixEnd; i++) {
		int next = 0;
		permutationCtrl.init(vecIndexesExt[i]);
		while (next < vecIndexesExt.size() && permutationCtrl.size() < cols_) {
			if (permutationCtrl.tryAddRight < OverlappingSize>(vecIndexesExt[next]) || permutationCtrl.tryAddLeft < OverlappingSize>(vecIndexesExt[next]))

				next = 0;
			else
				next++;
		}
		next = 0;

		if (permutationCtrl.size() >= cols_-2)
		{
			findMissingTailAndAdd(listOut, permutationCtrl.getPermutation(), permutationCtrl.getMissing(columnSize));
			findMissingHeadAndAdd(listOut, permutationCtrl.getPermutation(), permutationCtrl.getMissing(columnSize));
		}
		else	if (permutationCtrl.size() == columnSize) {
			listOut.push_back(permutationCtrl.getPermutation());
		}
	}
	return listOut;
}

template <int PermutationSize, typename ColIndexType, typename PresenceRegisterType  >
std::vector<std::vector<ColIndexType>> SmallPermutationSelector<PermutationSize, ColIndexType, PresenceRegisterType>::parallelLoop(ListPageColumnsPermutations(SmallPermutationSelector<PermutationSize, ColIndexType, PresenceRegisterType>::*searcher)(size_t, size_t)const, size_t repSize) const {
	using Futures = std::list < std::future<ListPageColumnsPermutations>>;
	Futures futures;
	ListPageColumnsPermutations retList;
	size_t i = 0, delta = repSize / std::thread::hardware_concurrency();
//	for (; i < std::thread::hardware_concurrency() - 1; i++)
//		futures.push_back(std::async(std::launch::async, searcher, this, i*delta, delta));
	retList = (this->*searcher)(i*delta, repSize - i*delta);

	while (futures.size()) {
		auto rl = futures.front().get();
		retList.insert(retList.end(), rl.begin(), rl.end());
		futures.pop_front();
	}
	return{ retList.begin(),retList.end() };
}
