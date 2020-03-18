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

#include <map>
#include <set>
#include <cstdio>
#include <fstream>
#include "InitializerOfIndexes.h"
#include <unordered_map>
#include <unordered_set>
#include <tuple>


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
			return std::hash<unsigned int >()(*reinterpret_cast<const unsigned int *>(k.data()));
		}
	};
	template <>
	struct hash<std::array<char, 6> >
	{
		std::size_t operator()(const std::array<char, 6>& k) const {
			return std::hash<unsigned int >()(*reinterpret_cast<const unsigned int *>(k.data())) ^
				std::hash<unsigned int>()((unsigned int)*reinterpret_cast<const unsigned short*>(&k[4]));
		}
	};
	template <>
	struct hash<std::array<char, 8> >
	{
		std::size_t operator()(const std::array<char, 8>& k) const
		{
			return std::hash<unsigned int >()(*reinterpret_cast<const unsigned int *>(k.data())) ^
				std::hash<unsigned int>()(*reinterpret_cast<const unsigned int*>(&k[4]));
		}
	};

	template <>
	struct hash<std::array<char, 10> >
	{
		std::size_t operator()(const std::array<char, 10>& k) const
		{
			return std::hash<unsigned int >()(*reinterpret_cast<const unsigned int *>(k.data())) ^
				std::hash<unsigned int>()(*reinterpret_cast<const unsigned int*>(&k[4])) ^
				std::hash<unsigned int>()((unsigned int)*reinterpret_cast<const unsigned short*>(&k[8]));;
		}
	};
	template <>
	struct hash<std::array<unsigned char,2> >
	{
		std::size_t operator()(const std::array<unsigned char, 2>& k) const
		{
			return
				std::hash<unsigned int>()((unsigned int)*reinterpret_cast<const unsigned short*>(&k[0]) );;
		}
	};


	template <>
	struct hash<std::array<unsigned char,3> >
	{
		std::size_t operator()(const std::array<unsigned char, 3>& k) const
		{
			return
				std::hash<unsigned int>()((unsigned int)*reinterpret_cast<const unsigned short*>(&k[0])+
				(((unsigned int)k[2])<<16));;
		}
	};

	template <>
	struct hash<std::array<unsigned char,4> >
	{
		std::size_t operator()(const std::array<unsigned char, 4>& k) const
		{
			return
				std::hash<unsigned int>()(*reinterpret_cast<const unsigned int*>(&k[0]) );;
		}
	};

	template <>
	struct hash<std::array<unsigned char,5> >
	{
		std::size_t operator()(const std::array<unsigned char, 5>& k) const
		{
			return
				std::hash<unsigned int>()(*reinterpret_cast<const unsigned int*>(&k[0]) )^
				std::hash<unsigned int>()((unsigned int)k[4] );
		}
	};
}





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




void RatingGiver::add(const TwoChars & twoChars) {
	value_type e{ twoChars,1 };
	auto it_bool = insert(e);
	if (!it_bool.second)
		it_bool.first->second++;
}

template <int n2>
class CombinedRatingGiver {
	using Weight = short;
	using Key = std::array<char, n2>;
	using Dictonary = std::unordered_map<Key, Weight>;
	Dictonary begins;
	Dictonary ends;
	Dictonary middels;
	std::unordered_set<std::string> set_of_words;
	const std::string delim=std::string(" ,.;?");
	Weight countNegativScore(const Key & key)const {
		return key[n2 - 1] != 0 ? -n2 : -(Weight)strlen(key.data());
	}
public:

	int getSubOffalfa(int & off, const Key& key, Key &ret) const {
		for (; off < n2 && !isalpha(key[off]); off++);
		int j = off;
		for (; j < n2 && isalpha(key[j]); j++) ret[j - off] = key[j];
		int size = j - off;
		for (j = size; j < n2; j++)
			ret[j] = 0;
		return size;
	}
	template <int sizeK>
	int getSubOffalfa(int & off, const std::array<char,sizeK> & key, Key &ret) const {
		for (; off < sizeK && !isalpha(key[off]); off++);
		int j = off;
		for (; j < sizeK && (j-off) < n2 && isalpha(key[j]); j++) ret[j - off] = key[j];
		int size = j - off;
		for (j = size; j < n2; j++)
			ret[j] = 0;
		return size;
	}
	auto getScoreWord(Key  & key)const {
		try {
			auto b = begins.at(key);
			auto e = ends.at(key);
			return (b < e) ? b : e;
		}
		catch (...) {
			return countNegativScore(key);
		}
	}
	auto getScoreBeg(Key  & key) const {
		try {
			return begins.at(key);
		}
		catch (...) {
			return countNegativScore(key);
		}
	}
	auto  getScoreEnd(Key  & key) const {
		try {
			return ends.at(key);
		}
		catch (...) {
			return countNegativScore(key);

		}
	}
	auto getScoreMid(Key  & key) const {
		try {
			return middels.at(key);
		}
		catch (...) {
			return countNegativScore(key);
		}
	}
	void initKey(Key &key, const std::string &word, int off, int size)
	{
		int i = 0;
		for (; i < size; i++) key[i] = word[off + i];
		for (; i < n2; i++)key[i] = 0;
	}
	void add(Dictonary &dict, const Key & key, Weight weight = 1) {
		Dictonary::value_type e{ key, weight };
		auto it_bool = dict.insert(e);
		//if (!it_bool.second)
		//	it_bool.first->second+=weight;
	}
public:
	void addWord(const std::string & word);
template <int sizeK>
	auto getScore(const std::array<char,sizeK> & key)const {
		int off = 0, size;
		Weight score = 0;
		Key temp;
		do {
			size = getSubOffalfa(off, key, temp);
			if (size > 2) {
				if (off > 0)
				{
					if (off + size < n2)
						score += getScoreWord(temp);
					score += getScoreBeg(temp);
				}
				else if (size == n2) {
					score += getScoreMid(temp);
				}
				else
					score += getScoreEnd(temp);
			}
			off += size;
		} while (sizeK- off > 2);

		return score;
	}
	int getScore(const PaperSide& page)const;


};

template <int n2>
void  CombinedRatingGiver<n2> ::addWord(const std::string & word) {
	size_t ik, i = 0, size, off;
	set_of_words.insert(word);
	Key temp;
	if (word.size() > n2) {
		for (i = 0, ik = word.size() - n2; i < ik; i++) {
			initKey(temp, word, (int)i, n2);
			add(middels, temp, n2);
		}
	}
	size_t size_ = (n2 > word.size()) ? word.size() : n2;
	for (size = 2; size < size_; size++) {
		off = word.size() - size;
		initKey(temp, word, (int)off, (int)size);
		add(ends, temp, (int)size);
		initKey(temp, word, (int)i, (int)size);
		add(begins, temp, (int)size);
	}
}
template <int n2>
int  CombinedRatingGiver<n2>::getScore(const PaperSide & page) const
{
	int result = 0;
	for (auto & line : page)
	{
		char *next_token = nullptr;
		char *token = strtok_s(const_cast<char*>(line.c_str()), delim.c_str(), &next_token);

		while (token != nullptr)
		{
			auto v = set_of_words.find(token)!=set_of_words.end();
			if (v)
				result += (int)strlen(token);
			else
				result-=(int)strlen(token);
			token = strtok_s(nullptr, delim.c_str(), &next_token);
		}
	}
	return result;
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
