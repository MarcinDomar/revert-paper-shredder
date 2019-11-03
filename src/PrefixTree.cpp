#include "PrefixTree.h"
#include <iostream>
#include <fstream>

std::vector<std::string> ReadDictLemmaNum(const std::string filePath) {
	std::vector<std::string> dict;
	//char buf [100];
	//FILE *file;
	//
	//file = fopen (filePath.c_str(),"r");
	//int scanf_result, n;
	//do {
	//	scanf_result = std::fscanf_s (file, "%*d %*d %99s %*s", buf);
	//	//std::printf("%s\n", buf);
	//	dict.push_back(buf);
	//}
	//while (scanf_result!=EOF);
	//fclose (file);
	
	return dict;
}

std::vector<std::string> ReadDictBasic(const std::string filePath) {
	std::vector<std::string> dict;
	std::string word;
	
	// here we should add try-catch block
	std::ifstream file;
	file.open(filePath, std::ios_base::in);
	while (file.good())
	{
		file >> word;
		dict.push_back(word);
	}
	file.close();
	
	return dict;
}


void PrefixTree::add(const std::string &word){
	//jesli root wskazuje na null tworzymy go
	if (root == nullptr) {
		root = new PrefixTreeNode;
	}
	auto node = root;
	for(auto letter : word) {
		node->counts[letter] = node->counts[letter]+1;
		//std::cout << node->counts[letter] << ' ';
		if ( node->childs[letter] == nullptr ) //jeżeli węzeł potmony nie istnieje to go tworzymy
			node->childs[letter] = new PrefixTreeNode;
		// przechodzimy do węzła potomnego dla odpowiedniej litery
		node = node->childs[letter];
	}
	//std::cout << std::endl;
}


std::vector<char> PrefixTree::GetNextCharsByFreq(const std::string prefix) const {
	std::vector<char> result;
	
	auto node = root;
	// przechodzimy do węzła dla ostatniej litery prefixu
	for(auto letter : prefix) {
		node = node->childs[letter];
	}
	//jeśli nie ma ani jednej nastepnej litery to wskaźnik będzie nullptr i zwracamy pusty wynik
	if ( node == nullptr )
		return result;
	// zwracamy liste litery w kolejności ile od największej liczby potomków do największej
	//na początek poprostu zwracamy liste potomków
	for (auto it=node->counts.begin(); it!=node->counts.end(); ++it) {
		std::cout << it->first << " => " << it->second << '\n';
		result.push_back(it->first);
	}
}


bool PrefixTree::contains(const std::string& prefix) const
{
	auto node = root;
	if (prefix.size() == 0)
		return false;
	for(auto letter : prefix) {
		if (node->counts[letter] == 0)
			return false;
		else
			node = node->childs[letter];
	}
	return true;
}
