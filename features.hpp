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

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <unordered_map>

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

typedef unsigned char byte;

// vajag ātrās versijas
typedef int_fast8_t int8;
typedef int_fast16_t int16;
typedef int_fast32_t int32;


// typedef struct Feature {
class Feature
{
public:

	byte data[FEATURE_MAX_SIZE]{};	// zero initialization
	int size() const { return (data[0] & 0x1F) + 1; }	// lietderīgo datu izmērs pirmo baitu ieskaitot
	bool operator==(const struct Feature& o) const { for(int i=0, sz=size(); i<sz; ++i) if(data[i] != o.data[i]) return false; return true; }
	bool operator!=(const struct Feature& o) const { return !operator==(o); }
	bool operator<(const struct Feature& o) const
		{ for(int i=0, sz=size(); i<sz; ++i) if(data[i] != o.data[i]) return data[i] < o.data[i]; return false; }
	bool operator>(const struct Feature& o) const
		{ for(int i=0, sz=size(); i<sz; ++i) if(data[i] != o.data[i]) return data[i] > o.data[i]; return false; }

	void print() const;
	void print_raw() const;
	void print_raw_full() const;

	// Feature() { data[0] = Undefined; }
	// Feature() { for(int i=0; i<FEATURE_MAX_SIZE; ++i) data[i] = 0; }	// var iztikt bez, ja ir apsolījums vispirms ņemt
	Feature() { }

	// TODO: copy constructor, lai pārkopētu tikai būtisko
	
	typedef int8 T;
	typedef int8 S;
	typedef int8 N;
	typedef int16 L;

	// Komponenšu nosaukumi:
	// 2 biti - Tiny			( int8 )	4
	// 4 biti - Small			( int8 )	16
	// 8 biti - Normal			( int8 )	256
	// 16 biti - Large			( int16 )	65 535
	// 20 biti - Mega large		( int32 )	1 048 575
	// 24 biti - Super large	( int32 )	16 777 215
	// 32 biti - Huge			( int32 )	4 294 967 295
	// TODO: vēl varētu 20 un 24 bitu variantus
	// Tātad: TSNL - 2 + 4 + 8 + 16 biti
	// Svarīgākais ir enkodēšana, dekodēšana ir otrajā plānā jeb vispār nevajadzīga.
	//
	// 4 LL
	// 5 LLT
	// 6 LLL
	// 7 LLLT
	// 8 LLLL
	// 9 LLLLT
	//
	// 2 NN
	// 3 NNT
	// 3 NNN
	// 4 NNNT
	// 4 NNNN
	// 5 NNNNT
	//
	// 7 NLLL
	// 8 NLLLT
	// 6 NNLL
	// 7 NNLLT
	// 10 NNLLLL
	// 11 NNLLLLT
	//
	//
	//
	//
	// 1 2 NN
	// 1 3 NNT
	// 2 3 NNN
	// 1 4 LL
	// 2 4 NNNT
	// 3 4 NNNN
	// 1 5 LLT
	// 2 5 NNNNT
	// 1 6 LLL
	// 2 6 NNLL
	// 1 7 LLLT
	// 2 7 NLLL
	// 3 7 NNLLT
	// 1 8 NLLLT
	// 2 8 LLLL
	// 1 9 LLLLT
	// 1 10 NNLLLL
	// 1 11 NNLLLLT
	//

	typedef enum {
		TTSNL = (0 << 5) + 4,

		// NN = (1 << 5) + 2,
		// NNT = (1 << 5) + 3,
		// NNN = (2 << 5) + 3,
		// LL = (1 << 5) + 4,
		// NNNT = (2 << 5) + 4,
		// NNNN = (3 << 5) + 4,
		// LLT = (1 << 5) + 5,
		// NNNNT = (2 << 5) + 5,
		// LLL = (1 << 5) + 6,
		// NNLL = (2 << 5) + 6,
		// LLLT = (1 << 5) + 7,
		// NLLL = (2 << 5) + 7,
		// NNLLT = (3 << 5) + 7,
		// NLLLT = (1 << 5) + 8,
		// LLLL = (2 << 5) + 8,
		// LLLLT = (1 << 5) + 9,
		// NNLLLL = (1 << 5) + 10,
		// NNLLLLT = (1 << 5) + 11,

		// TNN = (3 << 5) + 3,
		// TNNT = (4 << 5) + 4,
		// TNNN = (5 << 5) + 4,
		// TLL = (3 << 5) + 5,
		// TNNNT = (4 << 5) + 5,
		// TNNNN = (5 << 5) + 5,
		// TLLT = (3 << 5) + 6,
		// TNNNNT = (4 << 5) + 6,
		// TLLL = (4 << 5) + 7,
		// TNNLL = (5 << 5) + 7,
		// TLLLT = (3 << 5) + 8,
		// TNLLL = (4 << 5) + 8,
		// TNNLLT = (5 << 5) + 8,
		// TNLLLT = (2 << 5) + 9,
		// TLLLL = (3 << 5) + 9,
		// TLLLLT = (2 << 5) + 10,
		// TNNLLLL = (2 << 5) + 11,
		// TNNLLLLT = (1 << 5) + 12,
		
		// TNNLLL = (4 << 5) + 9,
		// NNLLL = (6 << 5) + 8,
		// TNNLLLT = (3 << 5) + 10,
		// NNLLLT = (5 << 5) + 9,

		Undefined = 0
	} Type;


	typedef struct _NN {
		const static byte value = (1 << 5) + 2;
	} NN;

	typedef struct _NNT {
		const static byte value = (1 << 5) + 3;
	} NNT;

	typedef struct _NNN {
		const static byte value = (2 << 5) + 3;
	} NNN; 
	
	typedef struct _LL {
		const static byte value = (1 << 5) + 4;
	} LL;
	
	typedef struct _NNNT {
		const static byte value = (2 << 5) + 4;
	} NNNT;
	
	typedef struct _NNNN {
		const static byte value = (3 << 5) + 4;
	} NNNN;
	
	typedef struct _LLT {
		const static byte value = (1 << 5) + 5;
	} LLT;
	
	typedef struct _NNNNT {
		const static byte value = (2 << 5) + 5;
	} NNNNT;
	
	typedef struct _LLL {
		const static byte value = (1 << 5) + 6;
	} LLL;
	
	typedef struct _NNLL {
		const static byte value = (2 << 5) + 6;
	} NNLL;
	
	typedef struct _LLLT {
		const static byte value = (1 << 5) + 7;
	} LLLT;
	
	typedef struct _NLLL {
		const static byte value = (2 << 5) + 7;
	} NLLL;
	
	typedef struct _NNLLT {
		const static byte value = (3 << 5) + 7;
	} NNLLT;
	
	typedef struct _NLLLT {
		const static byte value = (1 << 5) + 8;
	} NLLLT;
	
	typedef struct _LLLL {
		const static byte value = (2 << 5) + 8;
	} LLLL;
	
	typedef struct _LLLLT {
		const static byte value = (1 << 5) + 9;
	} LLLLT;
	
	typedef struct _NNLLLL {
		const static byte value = (1 << 5) + 10;
	} NNLLLL;
	
	typedef struct _NNLLLLT {
		const static byte value = (1 << 5) + 11;
	} NNLLLLT;

	// jaunāki
	
	typedef struct _TNN {
		const static byte value = (3 << 5) + 3;
	} TNN;
	
	typedef struct _TNNT {
		const static byte value = (4 << 5) + 4;
	} TNNT;
	
	typedef struct _TNNN {
		const static byte value = (5 << 5) + 4;
	} TNNN;
	
	typedef struct _TLL {
		const static byte value = (3 << 5) + 5;
	} TLL;
	
	typedef struct _TNNNT {
		const static byte value = (4 << 5) + 5;
	} TNNNT;
	
	typedef struct _TNNNN {
		const static byte value = (5 << 5) + 5;
	} TNNNN;
	
	typedef struct _TLLT {
		const static byte value = (3 << 5) + 6;
	} TLLT;
	
	typedef struct _TNNNNT {
		const static byte value = (4 << 5) + 6;
	} TNNNNT;
	
	typedef struct _TLLL {
		const static byte value = (4 << 5) + 7;
	} TLLL;
	
	typedef struct _TNNLL {
		const static byte value = (5 << 5) + 7;
	} TNNLL;
	
	typedef struct _TLLLT {
		const static byte value = (3 << 5) + 8;
	} TLLLT;
	
	typedef struct _TNLLL {
		const static byte value = (4 << 5) + 8;
	} TNLLL;

	// TODO: vajag ietērpt makrosā (x << 5) + y

		// TNNLLL = (4 << 5) + 9,
		// NNLLL = (6 << 5) + 8,
	typedef struct _TNNLLL {
		const static byte value = (4 << 5) + 9;
	} TNNLLL;

	typedef struct _NNLLL {
		const static byte value = (6 << 5) + 8;
	} NNLLL;

	typedef struct _TNNLLLT {
		const static byte value = (3 << 5) + 10;
	} TNNLLLT;

	typedef struct _NNLLLT {
		const static byte value = (5 << 5) + 9;
	} NNLLLT;
		// TNNLLLT = (3 << 5) + 10,
		// NNLLLT = (5 << 5) + 9,
	
	typedef struct _TNNLLT {
		const static byte value = (5 << 5) + 8;
	} TNNLLT;
	
	typedef struct _TNLLLT {
		const static byte value = (2 << 5) + 9;
	} TNLLLT;
	
	typedef struct _TLLLL {
		const static byte value = (3 << 5) + 9;
	} TLLLL;
	
	typedef struct _TLLLLT {
		const static byte value = (2 << 5) + 10;
	} TLLLLT;
	
	typedef struct _TNNLLLL {
		const static byte value = (2 << 5) + 11;
	} TNNLLLL;
	
	typedef struct _TNNLLLLT {
		const static byte value = (1 << 5) + 12;
	} TNNLLLLT;

		// TNN = (3 << 5) + 3,
		// TNNT = (4 << 5) + 4,
		// TNNN = (5 << 5) + 4,
		// TLL = (3 << 5) + 5,
		// TNNNT = (4 << 5) + 5,
		// TNNNN = (5 << 5) + 5,
		// TLLT = (3 << 5) + 6,
		// TNNNNT = (4 << 5) + 6,
		// TLLL = (4 << 5) + 7,
		// TNNLL = (5 << 5) + 7,
		// TLLLT = (3 << 5) + 8,
		// TNLLL = (4 << 5) + 8,
		// TNNLLT = (5 << 5) + 8,
		// TNLLLT = (2 << 5) + 9,
		// TLLLL = (3 << 5) + 9,
		// TLLLLT = (2 << 5) + 10,
		// TNNLLLL = (2 << 5) + 11,
		// TNNLLLLT = (1 << 5) + 12,
		


	template <typename... Arguments>
	Feature(NN, Arguments... arguments)
	{
		setNN(arguments...);
	}

	template <typename... Arguments>
	Feature(NNT, Arguments... arguments)
	{
		setNNT(arguments...);
	}

	template <typename... Arguments>
	Feature(NNN, Arguments... arguments)
	{
		setNNN(arguments...);
	}

	template <typename... Arguments>
	Feature(LL, Arguments... arguments)
	{
		setLL(arguments...);
	}

	template <typename... Arguments>
	Feature(NNNT, Arguments... arguments)
	{
		setNNNT(arguments...);
	}

	template <typename... Arguments>
	Feature(NNNN, Arguments... arguments)
	{
		setNNNN(arguments...);
	}

	template <typename... Arguments>
	Feature(LLT, Arguments... arguments)
	{
		setLLT(arguments...);
	}

	template <typename... Arguments>
	Feature(NNNNT, Arguments... arguments)
	{
		setNNNNT(arguments...);
	}

	template <typename... Arguments>
	Feature(LLL, Arguments... arguments)
	{
		setLLL(arguments...);
	}

	template <typename... Arguments>
	Feature(NNLL, Arguments... arguments)
	{
		setNNLL(arguments...);
	}

	template <typename... Arguments>
	Feature(LLLT, Arguments... arguments)
	{
		setLLLT(arguments...);
	}

	template <typename... Arguments>
	Feature(NLLL, Arguments... arguments)
	{
		setNLLL(arguments...);
	}

	template <typename... Arguments>
	Feature(NNLLT, Arguments... arguments)
	{
		setNNLLT(arguments...);
	}

	template <typename... Arguments>
	Feature(NLLLT, Arguments... arguments)
	{
		setNLLLT(arguments...);
	}

	template <typename... Arguments>
	Feature(LLLL, Arguments... arguments)
	{
		setLLLL(arguments...);
	}

	template <typename... Arguments>
	Feature(LLLLT, Arguments... arguments)
	{
		setLLLLT(arguments...);
	}

	template <typename... Arguments>
	Feature(NNLLLL, Arguments... arguments)
	{
		setNNLLLL(arguments...);
	}

	template <typename... Arguments>
	Feature(NNLLLLT, Arguments... arguments)
	{
		setNNLLLLT(arguments...);
	}


	/*
	template <typename... Arguments>
	Feature(Type type, Arguments... arguments)
	{
		data[0] = type;

		if(type == TTSNL)
		{
			setTTSNL(arguments...);
		}

		else if(type == NN) setNN(arguments...);
		else if(type == NNT) setNNT(arguments...);

		else if(type == NNN) setNNN(arguments...);
		else if(type == LL) setLL(arguments...);
		else if(type == NNNT) setNNNT(arguments...);
		else if(type == NNNN) setNNNN(arguments...);
		else if(type == LLT) setLLT(arguments...);
		else if(type == NNNNT) setNNNNT(arguments...);
		else if(type == LLL) setLLL(arguments...);
		else if(type == NNLL) setNNLL(arguments...);
		else if(type == LLLT) setLLLT(arguments...);
		else if(type == NLLL) setNLLL(arguments...);
		else if(type == NNLLT) setNNLLT(arguments...);
		else if(type == NLLLT) setNLLLT(arguments...);
		else if(type == LLLL) setLLLL(arguments...);
		else if(type == LLLLT) setLLLLT(arguments...);
		else if(type == NNLLLL) setNNLLLL(arguments...);
		else if(type == NNLLLLT) setNNLLLLT(arguments...);
	}
	*/
	
	// void setTTSNL(int8 tiny1, int8 tiny2, int8 small, int8 normal, int16 large)
	// {
	// 	data[0] = TTSNL::value;
	// 	// vislabāk ir saspiest kopā pirmos trīs elementus vienā
	// 	data[1] = ((tiny1 & 0x3) << 6) + ((tiny2 & 0x3) << 4) + (small & 0xF);
	// 	data[2] = normal;
	// 	*((int16_t*)&data[3]) = large;
	// }

	void setNN(N n1, N n2)
	{
		data[0] = NN::value;
		data[1] = n1;
		data[2] = n2;
	}

	void setNNT(N n1, N n2, T t)
	{
		data[0] = NNT::value;
		data[1] = n1;
		data[2] = n2;
		data[3] = t & 0x3;
	}

	void setNNN(N n1, N n2, N n3)
	{
		data[0] = NNN::value;
		data[1] = n1;
		data[2] = n2;
		data[3] = n3;
	}

	void setLL(L l1, L l2)
	{
		data[0] = LL::value;
		*((int16_t*)&data[1]) = l1;
		*((int16_t*)&data[3]) = l2;
	}

	void setNNNT(N n1, N n2, N n3, T t)
	{
		data[0] = NNNT::value;
		data[1] = n1;
		data[2] = n2;
		data[3] = n3;
		data[4] = t & 0x3;
	}

	void setNNNN(N n1, N n2, N n3, N n4)
	{
		data[0] = NNNN::value;
		data[1] = n1;
		data[2] = n2;
		data[3] = n3;
		data[4] = n4;
	}

	void setLLT(L l1, L l2, T t)
	{
		data[0] = LLT::value;
		*((int16_t*)&data[1]) = l1;
		*((int16_t*)&data[3]) = l2;
		data[5] = t & 0x3;
	}

	void setNNNNT(N n1, N n2, N n3, N n4, T t)
	{
		data[0] = NNNNT::value;
		data[1] = n1;
		data[2] = n2;
		data[3] = n3;
		data[4] = t & 0x3;
	}

	void setLLL(L l1, L l2, L l3)
	{
		data[0] = LLL::value;
		*((int16_t*)&data[1]) = l1;
		*((int16_t*)&data[3]) = l2;
		*((int16_t*)&data[5]) = l3;
	}

	void setNNLL(N n1, N n2, L l1, L l2)
	{
		data[0] = NNLL::value;
		data[1] = n1;
		data[2] = n2;
		*((int16_t*)&data[3]) = l1;
		*((int16_t*)&data[5]) = l2;
	}

	void setLLLT(L l1, L l2, L l3, T t)
	{
		data[0] = LLLT::value;
		*((int16_t*)&data[1]) = l1;
		*((int16_t*)&data[3]) = l2;
		*((int16_t*)&data[5]) = l3;
		data[7] = t & 0x3;
	}

	void setNLLL(N n, L l1, L l2, L l3)
	{
		data[0] = NLLL::value;
		data[1] = n;
		*((int16_t*)&data[2]) = l1;
		*((int16_t*)&data[4]) = l2;
		*((int16_t*)&data[6]) = l3;
	}

	void setNNLLT(N n1, N n2, L l1, L l2, T t)
	{
		data[0] = NNLLT::value;
		data[1] = n1;
		data[2] = n2;
		*((int16_t*)&data[3]) = l1;
		*((int16_t*)&data[5]) = l2;
		data[7] = t & 0x3;
	}

	void setNLLLT(N n, L l1, L l2, L l3, T t)
	{
		data[0] = NLLLT::value;
		data[1] = n;
		*((int16_t*)&data[2]) = l1;
		*((int16_t*)&data[4]) = l2;
		*((int16_t*)&data[6]) = l3;
		data[8] = t & 0x3;
	}

	void setLLLL(L l1, L l2, L l3, L l4)
	{
		data[0] = LLLL::value;
		*((int16_t*)&data[1]) = l1;
		*((int16_t*)&data[3]) = l2;
		*((int16_t*)&data[5]) = l3;
		*((int16_t*)&data[7]) = l4;
	}

	void setLLLLT(L l1, L l2, L l3, L l4, T t)
	{
		data[0] = LLLLT::value;
		*((int16_t*)&data[1]) = l1;
		*((int16_t*)&data[3]) = l2;
		*((int16_t*)&data[5]) = l3;
		*((int16_t*)&data[7]) = l4;
		data[9] = t & 0x3;
	}

	void setNNLLLL(N n1, N n2, L l1, L l2, L l3, L l4)
	{
		data[0] = NNLLLL::value;
		data[1] = n1;
		data[2] = n2;
		*((int16_t*)&data[3]) = l1;
		*((int16_t*)&data[5]) = l2;
		*((int16_t*)&data[7]) = l3;
		*((int16_t*)&data[9]) = l4;
	}

	void setNNLLLLT(N n1, N n2, L l1, L l2, L l3, L l4, T t)
	{
		data[0] = NNLLLLT::value;
		data[1] = n1;
		data[2] = n2;
		*((int16_t*)&data[3]) = l1;
		*((int16_t*)&data[5]) = l2;
		*((int16_t*)&data[7]) = l3;
		*((int16_t*)&data[9]) = l4;
		data[11] = t & 0x3;
	}


	/// --------------------------

	Feature(TNN, T t, N n1, N n2)
	{
		data[0] = NN::value;
		data[1] = t & 0x3;
		data[2] = n1;
		data[3] = n2;
	}

	Feature(TNNT, T t1, N n1, N n2, T t2)
	{
		data[0] = TNNT::value;
		data[1] = t1 & 0x3;
		data[2] = n1;
		data[3] = n2;
		data[4] = t2 & 0x3;
	}

	Feature(TNNN, T t, N n1, N n2, N n3)
	{
		data[0] = TNNN::value;
		data[1] = t & 0x3;
		data[2] = n1;
		data[3] = n2;
		data[4] = n3;
	}

	Feature(TLL, T t, L l1, L l2)
	{
		data[0] = TLL::value;
		data[1] = t & 0x3;
		*((int16_t*)&data[2]) = l1;
		*((int16_t*)&data[4]) = l2;
	}

	Feature(TNNNT, T t1, N n1, N n2, N n3, T t2)
	{
		data[0] = TNNNT::value;
		data[1] = t1 & 0x3;
		data[2] = n1;
		data[3] = n2;
		data[4] = n3;
		data[5] = t2 & 0x3;
	}

	Feature(TNNNN, T t, N n1, N n2, N n3, N n4)
	{
		data[0] = TNNNN::value;
		data[1] = t & 0x3;
		data[2] = n1;
		data[3] = n2;
		data[4] = n3;
		data[5] = n4;
	}

	Feature(TLLT, T t1, L l1, L l2, T t2)
	{
		data[0] = TLLT::value;
		data[1] = t1 & 0x3;
		*((int16_t*)&data[2]) = l1;
		*((int16_t*)&data[4]) = l2;
		data[6] = t2 & 0x3;
	}

	Feature(TNNNNT, T t1, N n1, N n2, N n3, N n4, T t2)
	{
		data[0] = TNNNNT::value;
		data[1] = t1 & 0x3;
		data[2] = n1;
		data[3] = n2;
		data[4] = n3;
		data[5] = t2 & 0x3;
	}

	Feature(TLLL, T t, L l1, L l2, L l3)
	{
		data[0] = TLLL::value;
		data[1] = t & 0x3;
		*((int16_t*)&data[2]) = l1;
		*((int16_t*)&data[4]) = l2;
		*((int16_t*)&data[6]) = l3;
	}

	Feature(TNNLL, T t, N n1, N n2, L l1, L l2)
	{
		data[0] = TNNLL::value;
		data[1] = t & 0x3;
		data[2] = n1;
		data[3] = n2;
		*((int16_t*)&data[4]) = l1;
		*((int16_t*)&data[6]) = l2;
	}

	Feature(TLLLT, T t1, L l1, L l2, L l3, T t2)
	{
		data[0] = TLLLT::value;
		data[1] = t1 & 0x3;
		*((int16_t*)&data[2]) = l1;
		*((int16_t*)&data[4]) = l2;
		*((int16_t*)&data[6]) = l3;
		data[8] = t2 & 0x3;
	}

	Feature(TNLLL, T t, N n, L l1, L l2, L l3)
	{
		data[0] = TNLLL::value;
		data[1] = t & 0x3;
		data[2] = n;
		*((int16_t*)&data[3]) = l1;
		*((int16_t*)&data[5]) = l2;
		*((int16_t*)&data[7]) = l3;
	}
	
	Feature(NNLLL, N n1, N n2, L l1, L l2, L l3)
	{
		data[0] = NNLLL::value;
		data[1] = n1;
		data[2] = n2;
		*((int16_t*)&data[3]) = l1;
		*((int16_t*)&data[5]) = l2;
		*((int16_t*)&data[7]) = l3;
	}

	Feature(TNNLLL, T t, N n1, N n2, L l1, L l2, L l3)
	{
		data[0] = TNNLLL::value;
		data[1] = t & 0x3;
		data[2] = n1;
		data[3] = n2;
		*((int16_t*)&data[4]) = l1;
		*((int16_t*)&data[6]) = l2;
		*((int16_t*)&data[8]) = l3;
	}
		// TNNLLLT = (3 << 5) + 10,
		// NNLLLT = (5 << 5) + 9,
	Feature(NNLLLT, N n1, N n2, L l1, L l2, L l3, T t)
	{
		data[0] = NNLLLT::value;
		data[1] = n1;
		data[2] = n2;
		*((int16_t*)&data[3]) = l1;
		*((int16_t*)&data[5]) = l2;
		*((int16_t*)&data[7]) = l3;
		data[9] = t & 0x3;
	}

	Feature(TNNLLLT, T t1, N n1, N n2, L l1, L l2, L l3, T t2)
	{
		data[0] = TNNLLLT::value;
		data[1] = t1 & 0x3;
		data[2] = n1;
		data[3] = n2;
		*((int16_t*)&data[4]) = l1;
		*((int16_t*)&data[6]) = l2;
		*((int16_t*)&data[8]) = l3;
		data[10] = t2 & 0x3;
	}

	Feature(TNNLLT, T t1, N n1, N n2, L l1, L l2, T t2)
	{
		data[0] = TNNLLT::value;
		data[1] = t1 & 0x3;
		data[2] = n1;
		data[3] = n2;
		*((int16_t*)&data[4]) = l1;
		*((int16_t*)&data[6]) = l2;
		data[8] = t2 & 0x3;
	}

	Feature(TNLLLT, T t1, N n, L l1, L l2, L l3, T t2)
	{
		data[0] = TNLLLT::value;
		data[1] = t1 & 0x3;
		data[2] = n;
		*((int16_t*)&data[3]) = l1;
		*((int16_t*)&data[5]) = l2;
		*((int16_t*)&data[7]) = l3;
		data[9] = t2 & 0x3;
	}

	Feature(TLLLL, T t, L l1, L l2, L l3, L l4)
	{
		data[0] = TLLLL::value;
		data[1] = t & 0x3;
		*((int16_t*)&data[2]) = l1;
		*((int16_t*)&data[4]) = l2;
		*((int16_t*)&data[6]) = l3;
		*((int16_t*)&data[8]) = l4;
	}

	Feature(TLLLLT, T t1, L l1, L l2, L l3, L l4, T t2)
	{
		data[0] = TLLLLT::value;
		data[1] = t1 & 0x3;
		*((int16_t*)&data[2]) = l1;
		*((int16_t*)&data[4]) = l2;
		*((int16_t*)&data[6]) = l3;
		*((int16_t*)&data[8]) = l4;
		data[10] = t2 & 0x3;
	}

	Feature(TNNLLLL, T t, N n1, N n2, L l1, L l2, L l3, L l4)
	{
		data[0] = TNNLLLL::value;
		data[1] = t & 0x3;
		data[2] = n1;
		data[3] = n2;
		*((int16_t*)&data[4]) = l1;
		*((int16_t*)&data[6]) = l2;
		*((int16_t*)&data[8]) = l3;
		*((int16_t*)&data[10]) = l4;
	}

	Feature(TNNLLLLT, T t1, N n1, N n2, L l1, L l2, L l3, L l4, T t2)
	{
		data[0] = TNNLLLLT::value;
		data[1] = t1 & 0x3;
		data[2] = n1;
		data[3] = n2;
		*((int16_t*)&data[4]) = l1;
		*((int16_t*)&data[6]) = l2;
		*((int16_t*)&data[8]) = l3;
		*((int16_t*)&data[10]) = l4;
		data[12] = t2 & 0x3;
	}

};
// } Feature;



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

	FeatureVector(Index reserve = FV_DEFAULT_SIZE) { _map.reserve(reserve); _weights.reserve(reserve); _features.reserve(reserve); }

	void reserve(Index reserve) { _map.reserve(reserve); _weights.reserve(reserve); _features.reserve(reserve); }
	Index capacity() const { return _map.bucket_count(); }

	void clear() { _map.clear(); _weights.clear(); _features.clear(); }
	void zero(); // uzstāda nulles visiem svariem

	Index size() const { return _features.size(); }		// pēc kura no vektoriem labāk ir noteikt izmēru ?

	const Features& features() const { return _features; }
	const Weights& weights() const { return _weights; }
	
	Weights::iterator begin() { return _weights.begin(); }
	Weights::iterator end() { return _weights.end(); }

	Index collisions() const; // diagnostikai
	int bucket_count() const { return _map.bucket_count(); }

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
	typedef std::unordered_map<Feature, Index> FeatureIndexMap;
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

		std::map<std::string, int>::const_iterator it = map.find(s);
		if(it == map.end())
			return -1;

		return it->second;
	}

	int next;
	const IndexMap* primary;
	std::map<std::string, int> map;
};

// 
// Token info ==> indexed feature components
//
// Word, lemma, tag... -> visus vienā biezputrā - vienā indeksā
// Iespējams, ka tas nav izdevīgi, ja tagu variantu skaits nepārsniedz 256, tad nav
// Visas tagu mutācijas arī ir jāsaglabā tajā pašā indeksā.

#endif

