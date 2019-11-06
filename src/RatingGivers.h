#pragma once
#include "Types.h"
#include <map>
#include <set>
#include <cstdio>
#include <fstream>
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
	template <>
	struct hash<std::array<char, 4> >
	{
		std::size_t operator()(const std::array<char, 4>& k) const
		{
			using std::size_t;
			using std::hash;
			unsigned int val = ((unsigned int )k[3] << 24) +
				((unsigned int)k[2] << 16) +
				((unsigned int)k[1] << 8) +
				k.front();
			return std::hash<unsigned int >()(val);
		}
	};
	template <>
	struct hash<std::array<char, 6> >
	{
		std::size_t operator()(const std::array<char, 6>& k) const
		{
			using std::size_t;
			using std::hash;
			unsigned int val = ((unsigned int)k[3] << 24) +
				((unsigned int)k[2] << 16) +
				((unsigned int)k[1] << 8) +
				k.front();
			return std::hash<unsigned int >()(val) ^ std::hash<unsigned int>()( ((unsigned int)k[5]<<8 ) +	((unsigned int)k[4]) );
		}
	};


}
class RatingForPageGiver {
	const std::set<std::string> set_of_words;
	const std::string delim;
public:

	RatingForPageGiver(std::set<std::string> && setOfWords, std::string delim = " ,.;?") :set_of_words(setOfWords), delim(delim) {}
	RatingForPageGiver(std::set<std::string> & setOfWords, std::string delim = " ,.;?") :set_of_words(setOfWords), delim(delim) {}
	unsigned int getScore(const PaperSide& page)const;
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

template <int n2>
class CombinedRaingGiver {
	using Key = std::array<char, n2>;
	std::unordered_map<Key, unsigned int > begins;
	std::unordered_map<Key, unsigned int > ends;
	std::unordered_map<Key, unsigned int > middels;
public:
	inline void add(const Key & key);
	int getSubOffalfa(int & off, const Key& key, Key &ret) const {
		for (; off < n2 && !isalpha(key[off]); off++);
		int j = off;
		for (; j < n2 && isalpha(key[j]); j++) ret[j - off] = key[j];
		int size =   j-off;
		for (j = size; j< n2; j++)
			ret[j] = 0;
		return size;
	}
	unsigned int getScoreWord(Key  & key)const  {
		try {
			auto b = begins.at(key);
			auto e = ends.at(key);
			return (b < e) ? b : e;
		}
		catch (...) {
			return 0;
		}
	}
	unsigned int getScoreBeg(Key  & key) const {
		try {
			return begins.at(key);
		}
		catch (...) {
			return 0;
		}
	}
	unsigned int getScoreEnd(Key  & key) const {
		try {
			return ends.at(key);
		}
		catch (...) {
			return 0;
		}
	}
	unsigned int getScoreMid(Key  & key) const {
		try {
			return middels.at(key);
		}
		catch (...) {
			return 0;
		}
	}
	void initKey(Key &key, const std::string &word,int off,int size)
	{
		int i = 0;
		for (; i < size; i++) key[i] = word[off + i];
		for (; i < n2; i++)key[i] = 0;
	}
	void add(std::unordered_map<Key, unsigned int > &dict ,const Key & key,int weight=1) {
		std::unordered_map<Key, unsigned int > :: value_type e{ key, 1};
	auto it_bool = dict.insert(e);
		//if (!it_bool.second)
		//	it_bool.first->second+=weight;
	}
public:
	void addWord(const std::string & word);
	unsigned int getScore(Key & key)const {
		int off = 0, size;
		Key temp;
		do {
			size = getSubOffalfa(off, key, temp);
			if (size > 3) {
				if (off > 0)
				{
					if (off + size < n2)
						return getScoreWord(key);
					return getScoreBeg(key);
				}
				else if (size == n2) {
					return getScoreMid(key);
				}
				else
					return getScoreEnd(key);
			}
			off += size;
		} while (n2 - off > 3);

		return 0;
	}
};

template <int n2>
void  CombinedRaingGiver<n2> ::addWord(const std::string & word) {
	int ik , i,size,off;
	Key temp;
	for (i = 0, ik = word.size() - n2; i < ik; i++) {
		initKey(temp, word, i, n2);
		add(middels, temp,n2);
	}
	int size_ = (n2  > word.size()) ? word.size() : n2;
	for (size = 4; size < size_; size++) {
		off = word.size() - size;
		initKey(temp, word, off, size);
		add(ends, temp,size);
		initKey(temp, word, i, size);
		add(begins, temp,size);
	}
}


template <typename RG>
RG getRatingGiver(const std::string filePath) {
	RG result;
	std::string word;

	std::ifstream file;
	file.open(filePath, std::ios_base::in);
	while (file.good())
	{
		file >> word;
		result.addWord(word);
	}
	file.close();
	return result;
}

