/*
 * =====================================================================================
 *
 *       Filename:  features.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013-03-04 20:33:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Didzis Gosko (dg), didzis@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */


#ifndef __FEATURES_HPP__
#define __FEATURES_HPP__

#define STD_MAP -1
#define STD_UNORDERED_MAP 0
#define DENSE_HASH_MAP 1
#define SPARSE_HASH_MAP 2

// #if USE_MAP == STD_MAP
// #pragma message ("Using std::map")
// #elif USE_MAP == STD_UNORDERED_MAP
// #pragma message ("Using std::unordered_map")
// #elif USE_MAP == DENSE_HASH_MAP
// #pragma message ("Using google::dense_hash_map")
// #elif USE_MAP == SPARSE_HASH_MAP
// #pragma message ("Using google::sparse_hash_map")
// #endif

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <unordered_map>

// #if USE_MAP == STD_UNORDERED_MAP
// #include <unordered_map>
#if USE_MAP == DENSE_HASH_MAP
#include <sparsehash/dense_hash_map>
#elif USE_MAP == SPARSE_HASH_MAP
#include <sparsehash/sparse_hash_map>
#endif

//
// Ideja:
// vispirms salasīt visus iespējamos unikālos feature komponenšu stringus un piešķirt tiem augošus indeksus (to dara tokenu izveides stadijā).
// Pēc tam notiek darbošanās tikai ar šiem indeksiem (skaitļiem).
// Šos indeksus var savirknēt veidojot iezīmi. Cerība ir, ka darboties ar indeksiem būs ātrāk nekā ar stringiem.
// Bieži vien, lai iekodētu iezīmi vajadzēs vairāk kā 64 bitus, bet nav izdevīgi izmantot fiksētu bitu garumu, piemēram, 20 baiti katrai iezīmei.
// Tad sanāk pārvietot lielāku salīdzinoši lielu baitu skaitu, iespējams stringa reprezentācijā mazākās iezīmes būtu stipri īsākas par 20.
//
// Kur veidojas 20?
// Komponenšu indeksu izmēri var būt (bitos): 2, 4, 8, 16 (pagaidām vairāk par 16 nav nepieciešams)
// Pieņemsim, ka garākā iezīme satur trīs tokenus, to vārdus un tagus. Tādas informācijas kodēšanai vajag garāko: 16 bitu izmēru.
// Tad (16+16) * 3 = 96 jeb 12 baiti, klāt vēl var piekabināt pāris baitus, kas saturēs distanci (8 biti) un dažādus karogus (8 biti)
// Kopā būtu kādi 14 baiti.
//
// Piemērs salīdzināšanai ar stringiem:
// 12345678901234567890
// I|Gaaja|iet|vmdpio9o3|....
// Rādās, ka situācija nav nemaz tik slikta.
// Varbūt var pie rezervēšanas izvēlēties maksimālo atmiņas apjomu
//
// Atmiņas izkārtojums
// pirmais baits: 3 biti tips + 5 biti garums baitos (vēl x baiti sekos)
//

#define FEATURE_MAX_SIZE 20		// word aligned (2 baiti)


class Feature
{
public:

	typedef uint8_t byte;
	typedef uint16_t word;
	typedef uint32_t dword;

	// byte data[FEATURE_MAX_SIZE]{};	// zero initialization
	// Feature() { }
	byte data[FEATURE_MAX_SIZE];		// zero initialization off, just the first byte (for performance)
	Feature() { data[0] = 0; }

	int size() const { return data[0]; }	// lietderīgo datu izmērs pirmo baitu ieskaitot
	bool operator==(const struct Feature& o) const { for(int i=0, sz=size(); i<sz; ++i) if(data[i] != o.data[i]) return false; return true; }
	bool operator!=(const struct Feature& o) const { return !operator==(o); }
	bool operator<(const struct Feature& o) const
		{ for(int i=0, sz=size(); i<sz; ++i) if(data[i] != o.data[i]) return data[i] < o.data[i]; return false; }
	bool operator>(const struct Feature& o) const
		{ for(int i=0, sz=size(); i<sz; ++i) if(data[i] != o.data[i]) return data[i] > o.data[i]; return false; }

	void print() const;
	void print_raw() const;
	void print_raw_full() const;

	// nedaudz efektīvāks copy konstruktors, kas nevajadzīgo nekopē
	Feature(const Feature& s) { data[0] = s.data[0]; for(int i=1, sz=size(); i<sz; ++i) data[i] = s.data[i]; }


	//
	// Iespējamie izmēri:
	// byte : 8 biti 	256 varianti
	// word : 16 biti 	65 535 varianti
	// (?)x : 24 biti 	16 777 215 varianti
	// dword : 32 biti 	4 294 967 295 varianti
	//
	// Vai vajag arī 24 bitu ?
	// Kā to realizēt ?
	// Custom type (struct kā constexpr, kas satur int), tad pie saglabāšanas saglabā byte un word, jeb... &src nokopē tikai 3 baitus.
	// 
	// Svarīgākais ir enkodēšana, dekodēšana ir otrajā plānā jeb vispār nevajadzīga.
	//


	template <typename... Args>
	Feature(const Args&... args)
	{
		const int start = 1;
		// p = data+start;
		const int size = add(start, args...);
#ifdef DEBUG
		if(size >= 0x100)
			overflow_assert();
#endif
		data[0] = (byte)size;
	}

	void overflow_assert() const;	// tikai debug režīmā

private:

	// byte* p;	// kas ir ātrāk? data[pos] vai p += ... ?

	template <typename... Args>
	const int add(const int pos, const byte& v, const Args&... args)
	{
#ifdef DEBUG
		if(pos > sizeof(data)-sizeof(v))
			return 0x0100+pos;
#endif

		// *((byte*)p++) = v;
		*((byte*)&data[pos]) = v;

		return add(pos+1, args...);
	}

	template <typename... Args>
	const int add(const int pos, const word& v, const Args&... args)
	{
#ifdef DEBUG
		if(pos > sizeof(data)-sizeof(v))
			return 0x0100+pos;
#endif

		*((word*)&data[pos]) = v;
		// *((word*)p) = v;
		// p += sizeof(v);
		
		return add(pos+sizeof(v), args...);
	}

	template <typename... Args>
	const int add(const int pos, const dword& v, const Args&... args)
	{
#ifdef DEBUG
		if(pos > sizeof(data)-sizeof(v))
			return 0x100+pos;
#endif

		*((dword*)&data[pos]) = v;
		// *((dword*)p) = v;
		// p += sizeof(v);
		
		return add(pos+sizeof(v), args...);
	}

	const int add(const int pos) { return pos; }
};



template <class T> inline void hash_combine(std::size_t& seed, const T& v)
{
	static std::hash<T> hash_fn;	// nav vajadzības pie katras izsaukšanas izveidot jaunu mainīgo - der viens globāls, kurš atkarīgs no tipa

	// see: http://stackoverflow.com/questions/8513911/how-to-create-a-good-hash-combine-with-64-bit-output-inspired-by-boosthash-co
	
	const std::size_t k = 0x9ddfea08eb382d69ULL;
	std::size_t a = (hash_fn(v) ^ seed) * k;
	a ^= (a >> 47);
	std::size_t b = (seed ^ a) * k;
	b ^= (b >> 47);
	seed = b * k;

	// cits variants
	// seed ^= hash_fn(v) + 0x9E3779B97F4A7C15 + (seed << 6) + (seed >> 2);
}

namespace std
{
	template <> class hash<Feature>
	{
	public:
		size_t operator()(const Feature &feature) const
		{
			size_t seed = 0;
			for(int i=0, sz=feature.size(); i<sz; ++i)
				hash_combine(seed, feature.data[i]); 
			return seed;
		}

	};
}


//
// Veco kodu būs relatīvi grūti modificēt, tādēļ izdevīgāk ir veidot pavisam jaunu FeatureVector klasi
//
class FeatureVector
{
public:

	typedef int Value;		// score vērtības
	typedef size_t Index;
	typedef std::vector<Feature> Features;
	typedef std::vector<Value> Weights;

	// see: http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
	// or: http://www.orcca.on.ca/~yxie/courses/cs2210b-2011/htmls/extra/PlanetMath_%20goodhashtable.pdf
// #define FV_DEFAULT_SIZE 53
// #define FV_DEFAULT_SIZE 97
// #define FV_DEFAULT_SIZE 193
// #define FV_DEFAULT_SIZE 389
// #define FV_DEFAULT_SIZE 769
// #define FV_DEFAULT_SIZE 1543
// #define FV_DEFAULT_SIZE 3079
// #define FV_DEFAULT_SIZE 6151
// #define FV_DEFAULT_SIZE 12289
// #define FV_DEFAULT_SIZE 24593
// #define FV_DEFAULT_SIZE 49157
// #define FV_DEFAULT_SIZE 98317
// #define FV_DEFAULT_SIZE 196613
// #define FV_DEFAULT_SIZE 393241
// #define FV_DEFAULT_SIZE 786433
// #define FV_DEFAULT_SIZE 1572869
// #define FV_DEFAULT_SIZE 3145739
// #define FV_DEFAULT_SIZE 6291469
// #define FV_DEFAULT_SIZE 12582917
// #define FV_DEFAULT_SIZE 25165843
// #define FV_DEFAULT_SIZE 50331653
// #define FV_DEFAULT_SIZE 100663319
#define FV_DEFAULT_SIZE 201326611
// #define FV_DEFAULT_SIZE 402653189
// #define FV_DEFAULT_SIZE 805306457
// #define FV_DEFAULT_SIZE 1610612741

	FeatureVector(Index reserveSize = FV_DEFAULT_SIZE) { reserve(reserveSize); }

	void reserve(Index reserve)
	{
#if USE_MAP == DENSE_HASH_MAP
		_map.set_empty_key(Feature((Feature::word)-1));
#endif
#if USE_MAP != STD_MAP
		_map.rehash(reserve);
#endif
		_weights.reserve(reserve);
		_features.reserve(reserve);
	}

#if USE_MAP == STD_MAP
	Index capacity() const { return 0; }
#else
	Index capacity() const { return _map.bucket_count(); }
#endif

	void clear()
	{
#if USE_MAP == DENSE_HASH_MAP
		_map.clear_no_resize();
#else
		_map.clear();
#endif
		_weights.clear();
		_features.clear();
	}

	void zero(); // uzstāda nulles visiem svariem

	Index size() const { return _features.size(); }		// pēc kura no vektoriem labāk ir noteikt izmēru ?

	const Features& features() const { return _features; }
	const Weights& weights() const { return _weights; }
	
	Weights::iterator begin() { return _weights.begin(); }
	Weights::iterator end() { return _weights.end(); }

	Index collisions() const; // diagnostikai

#if USE_MAP == STD_MAP
	int bucket_count() const { return 0; }
#else
	int bucket_count() const { return _map.bucket_count(); }
#endif

	void print() const; // izvade uz ekrāna
	
	// Pievieno jaunu iezīmi (feature) vektora beigām ar nulles svaru (ja jau eksistē, tad nedara neko).
	void add(const Feature& feature)
	{
		if(_map.find(feature) == _map.end())
		{
			_map[feature] = _weights.size();
			_weights.push_back(0);
			_features.emplace_back(feature);
		}
	}

	// Feature adresācija pēc Feature objekta.
	// Piezīme: () ir skaidri redzams funkcijas izsaukums, parasti tāds neatrodas kreisajā pusē (assign konstrukcijā fn() = val;)
	// tādēļ šī funkcija ir tikai getteris: ja feature neeksistē, tad atgriež 0, bet nepievieno jaunu feature pie vektora.
	Value operator()(const Feature& feature) const
	{
		FeatureIndexMap::const_iterator it = _map.find(feature);
		if(it == _map.end())	// ja neatrod, tad atgriež nulles svaru
			return 0;
		return _weights[it->second];
	}

	// Feature adresācija pēc Feature objekta, getter & setter:
	// ja iezīme nav vektorā, tad tiek pievienota jauna (beigās) ar nulles svara vērtību.
	Value& operator[](const Feature& feature)
	{
		FeatureIndexMap::const_iterator it = _map.find(feature);
		if(it == _map.end())	// ja neatrod, tad pievieno jaunu ar nulles svaru
		{
			Index index = _weights.size();
			_map[feature] = index;
			_weights.push_back(0);
			_features.emplace_back(feature);
			return _weights[index];
		}
		return _weights[it->second];
	}
	
	// Feature adresācija pēc index, index ir jābūt derīgam.
	Value& operator[](Index index) { return _weights[index]; }

	// Meklē konkrētam Feature atbilstošo indeksu, ja nav atrasts, tad atgriež -1.
	Index index(const Feature& feature) const
	{
		FeatureIndexMap::const_iterator it = _map.find(feature);
		if(it == _map.end())
			return -1;
		return it->second;
	}

private:

	// Lai būtu iespēja darboties ar svaru vektoru kā ar vektoru (std::vector),
	// svaru (weights) nevar glabāt std::map konteinerī, tos ir jāglabā atsevišķi, tāpēc std::map saturēs indeksus weights vektorā.
#if USE_MAP == STD_MAP
	typedef std::map<Feature, Index> FeatureIndexMap;
#elif USE_MAP == STD_UNORDERED_MAP
	typedef std::unordered_map<Feature, Index> FeatureIndexMap;
#elif USE_MAP == DENSE_HASH_MAP
	typedef google::dense_hash_map<Feature, Index> FeatureIndexMap;
#elif USE_MAP == SPARSE_HASH_MAP
	typedef google::sparse_hash_map<Feature, Index> FeatureIndexMap;
#endif
	Features _features;
	Weights _weights;
	FeatureIndexMap _map;
};


//
// Indeksu karte
//
class IndexMap
{
public:
	IndexMap() { primary = NULL; next = 0; }
	IndexMap(const IndexMap& primaryIndex) { primary = &primaryIndex; next = primary->next; }

	int operator()(const std::string& s) const { return find(s); }
	int operator()(const std::string& s)
	{
		int r = find(s);
		if(r != -1)
			return r;

		map[s] = next;
		return next++;
	}

	int size() const { return map.size(); }

private:

	int find(const std::string& s) const
	{
		if(primary)
		{
			int r = (*primary)(s);

			if(r != -1)
				return r;
		}

		Map::const_iterator it = map.find(s);
		// std::map<std::string, int>::const_iterator it = map.find(s);
		if(it == map.end())
			return -1;

		return it->second;
	}

	int next;
	const IndexMap* primary;
	typedef std::unordered_map<std::string, int> Map;
	// typedef std::map<std::string, int> Map;
	Map map;
};

// 
// Token info ==> indexed feature components
//
// Word, lemma, tag... -> visus vienā biezputrā - vienā indeksā
// Iespējams, ka tas nav izdevīgi, ja tagu variantu skaits nepārsniedz 256, tad nav
// Visas tagu mutācijas arī ir jāsaglabā tajā pašā indeksā.

#endif

