/*
 * =====================================================================================
 *
 *       Filename:  collins0.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013-03-10 18:26:19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Didzis Gosko (dg), didzis@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __COLLINS0_HPP__
#define __COLLINS0_HPP__

#include <string>
#include <vector>
#include <set>
#include <memory>

#include "features.hpp"


//
// Tokens ir salīdzinoši smagnēja struktūra, bet tas nav šķērslis, jo:
// pat 200 tokenu teikumam, ir jāizveido tikai 200 tokenu objekti un tas ir jādara vienreiz katram teikumam
//
class Token
{
public:

	typedef int id;

	Token()
	{
		_parent = nullptr;
		_parentIndex = -1;
	}

	Token(const Token& s, bool unlink = false)
	{
		_index = s._index;
		_word = s._word;
		_lemma = s._lemma;
		_tag = s._tag;
		_fullTag = s._fullTag;
		_parentIndex = unlink ? -1 : s._parentIndex;
		_parent = nullptr;
		_wordID = s._wordID;
		_lemmaID = s._lemmaID;
		_tagID = s._tagID;
		_fullTagID = s._fullTagID;
	}

	bool operator==(const Token& t) const { return compare(t); }
	bool operator!=(const Token& t) const { return !compare(t); }
	bool compare(const Token& t) const
	{
		return _index == t._index && _parentIndex == t._parentIndex && _wordID == t._wordID && _lemmaID == t._lemmaID && _fullTagID == t._fullTagID;
	}
	
	id wordID() const { return _wordID; }
	id lemmaID() const { return _lemmaID; }
	id tagID() const { return _tagID; }
	id fullTagID() const { return _fullTagID; }

	int index() const { return _index; }
	int parentIndex() const { return _parentIndex; }

	const std::string& word() const { return _word; }
	const std::string& lemma() const { return _lemma; }
	const std::string& tag() const { return _tag; }
	const std::string& fullTag() const { return _fullTag; }

	const Token* parent() const { return _parent; }

	void printTree(int level = 0) const;
	void print(int level = 0) const;

private:
	
	// Uzstāda parent tokenu un parentam pievienojas pie children tokeniem.
	// Paredzēts izsaukšanai no Tokens klases.
	void setParent(Token* parent) { _parent = parent; parent->_children.push_back(this); }
	// uzstāda tikai indeksu
	void setParentIndex(int parentIndex) { _parentIndex = parentIndex; }

	// uzstāda gan indeksu, gan saites
	// void setParentAndIndex(Token* parent) { setParentIndex(parent->_index); setParent(parent); }	// netiek izmantots pašlaik
	
	// dzēst jebkādu saišu informāciju, ieskaitot parentIndex
	void unlink() { _parent = nullptr; _parentIndex = -1; _children.clear(); }
	// iztīra children, izsauc pirms link() izsaukšanas
	void clearChildren() { _children.clear(); }


	int _index;				// obligāts
	int _parentIndex;		// default (-1) ir root sakne (**)
	std::string _word;		// default: empty
	std::string _lemma;		// default: empty
	std::string _tag;		// default: empty
	std::string _fullTag;	// default: empty
	// atbilstošie identifikātori
	id _wordID;
	id _lemmaID;
	id _tagID;
	id _fullTagID;

	const Token* _parent;					// default: nullptr
	std::vector<const Token*> _children;	// default: empty

	friend class Tokens;
	// friend class Trees;
};

class Span;

class Tokens
{
public:

	Tokens(IndexMap& identificatorMap) : idMap(identificatorMap) { addRoot(); }
	// Tokens(const Tokens& s) : idMap(s.idMap) { tokens = s.tokens; }

	void add(const std::string& word, const std::string& lemma, const std::string& tag, const std::set<std::string>& tags, int parentIndex);
	bool add(const std::string& line, bool useGeneralTags = false);
	void reserve(int size) { tokens.reserve(size); }
	void clear() { tokens.clear(); addRoot(); /* status = Unchecked; */ }

	bool operator==(const Tokens& o) const
		{ if(this == &o) return true; if(tokens.size() != o.tokens.size()) return false;
		for(int i=0, size=tokens.size(); i<size; i++) if(tokens[i] != o.tokens[i]) return false; return true; }
	bool operator!=(const Tokens& o) const
		{ if(this == &o) return false; if(tokens.size() != o.tokens.size()) return true;
		for(int i=0, size=tokens.size(); i<size; i++) if(tokens[i] == o.tokens[i]) return true; return false; }

	int compare(const Tokens& other) const
		{ if(this == &other) return 1; if(tokens.size() != other.tokens.size()) return 0;
		int similarity = 0; for(int i=1, size=tokens.size(); i<size; i++) similarity += tokens[i] == other.tokens[i] ? 1 : 0;
		return similarity; }

	const Token& operator[](int index) const { return tokens[index]; }
	int size() const { return tokens.size(); }

	bool emtpy() const { return tokens.size() <= 1; }
	operator bool() const { return tokens.size() > 1; }

	void print() const;

	bool link();							// link tokens with valid parent indexes
	bool link(const Span* span);			// extract parent indexes from CS and subspans and link
	void unlink() { for(Token& token : tokens) token.unlink(); }

	bool extractFeatures(FeatureVector& targetFeatureVector) const;

	bool projective() const;
	bool check() const;

	void output(std::ostream& stream) const;

private:

	IndexMap& idMap;

	void addRoot() { std::set<std::string> tags; tags.emplace("R"); add("[*]", "[*]", "R", tags, -1); }
	void extractParents(const Span* span);

	std::vector<Token> tokens;
};

std::ostream& operator<<(std::ostream& stream, const Tokens& tokens);
std::istream& operator>>(std::istream& stream, Tokens& tokens);

void getFeatures(std::vector<Feature>& features, const Token* gtoken, const Token* htoken, const Token* mtoken,
		bool degenerateH = false, bool degenerateInnerM = false, bool degenerateOuterM = false);
void getFeaturesDegenICS(std::vector<Feature>& features, const Token* gtoken, const Token* htoken, const Token* mtoken,
		bool degenerateH, bool degenerateInnerM);
void getFeaturesDegenCS(std::vector<Feature>& features, const Token* gtoken, const Token* htoken, const Token* mtoken,
		bool degenerateInnerM = false, bool degenerateH = false);	// assume: degenerateOuterM = true


class Trees
{
public:

	// TODO: te būtu pareizāk izmantot shared_ptr, lai nebūtu viengs galvenais īpašnieks

	// Trees(IndexMap& identificatorMap) : idMap(identificatorMap) {}
	Trees() : idMap(*(new IndexMap())) { _idMap.reset(&idMap); }
	Trees(IndexMap& indexMap) : idMap(indexMap) {}
	// Copy konstruktors ar unlink iespēju
	Trees(const Trees& s, bool unlink = false) : trees(s.trees), idMap(s.idMap) { if(unlink) for(Tokens& tokens : trees) tokens.unlink(); }

	bool readCoNLL(IndexMap& idMap, const std::string& filename, bool useGeneralTags = false);

	void extractFeatures(FeatureVector& targetFeatureVector);

	const Tokens& operator[](int index) const { return trees[index]; }
	int size() const { return trees.size(); }

	void print() const;

	// IndexMap& idMap;
	
	std::vector<Tokens>::const_iterator begin() const { return trees.cbegin(); }
	std::vector<Tokens>::const_iterator end() const { return trees.cend(); }

	IndexMap& idMap;

private:

	std::vector<Tokens> trees;
	std::shared_ptr<IndexMap> _idMap;

	friend std::istream& operator>>(std::istream& stream, Trees& trees);
};

std::ostream& operator<<(std::ostream& stream, const Trees& trees);
std::istream& operator>>(std::istream& stream, Trees& trees);

void parse(Tokens& tokens, const FeatureVector& features);

#endif // __COLLINS0_HPP__
