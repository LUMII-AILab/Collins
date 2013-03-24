/*
 * =====================================================================================
 *
 *       Filename:  collins0.cpp
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

#include <iostream>
#include <vector>
#include <fstream>
#include <set>

#include <boost/algorithm/string.hpp>

#include "features.hpp"
#include "collins0.hpp"

using namespace std;


// Maksimāla ātruma optimizācijai pieejas tiesības ievērot nevajag, bez dinamiskiem atmiņas pieprasījumiem.

class Span
{
public:

	// g, h, m tokeni
	// incomplete span ir iekšā divi: complete span un reverse complete, varbūt var ar union ?
	
	union
	{
		// Complete Span
		struct {
			const Span* ics;	// Incomplete Span
			const Span* cs;		// Complete Span
		} cs;

		// Incomplete Span
		struct {
			const Span* cs;		// Complete Span
			const Span* rcs;	// Reverse complete span
		} ics;
	};

	typedef enum {
		Undefined,
		Terminal,
		Complete,
		Incomplete
	} Type;

	Type type;

	const Token* gtoken;
	const Token* htoken;
	union {
		const Token* mtoken;
		const Token* etoken;
	};

	FeatureVector::Value score;

	// TODO: kopēšana
	
	Span()
	{
		type = Undefined;
		score = 0;
		gtoken = nullptr;
		htoken = nullptr;
		mtoken = nullptr;
		cs.ics = nullptr;
		cs.cs = nullptr;
		// nav vajadzīgs, jo cs un ics ir union
		// ics.cs = nullptr;
		// ics.rcs = nullptr;
	}

	void terminal(const Token* g, const Token* h)
	{
		type = Terminal;
		gtoken = g;
		htoken = h;
		mtoken = h;			// vai labāk nullptr ?
		cs.ics = nullptr;
		cs.cs = nullptr;
		score = 0;
	}
	
	void complete(const Span* incomplete, const Span* complete, FeatureVector::Value score_ = 0)
	{
		type = Complete;
		cs.ics = incomplete;
		cs.cs = complete;
		gtoken = incomplete->gtoken;
		htoken = incomplete->htoken;
		mtoken = incomplete->mtoken;	// mtoken vai labāk etoken ? (var ar union)
		// score = incomplete->score + complete->score;
		score = score_;
	}

	void incomplete(const Span* complete, const Span* reverse, FeatureVector::Value score_ = 0)
	{
		type = Incomplete;
		ics.cs = complete;
		ics.rcs = reverse;
		gtoken = complete->gtoken;
		htoken = complete->htoken;
		mtoken = reverse->htoken;
		// score = complete->score + reverse->score;
		score = score_;
	}

	void scoreSelf(const FeatureVector& features, vector<Feature>& localFeatures)
	{
		// if(type == Incomplete)
		{
			localFeatures.clear();

			getFeatures(localFeatures, gtoken, htoken, mtoken);

			for(const Feature& feature : localFeatures)
				score += features(feature);
		}
	}

	void print(int level = 0) const;
};


class SpanStore
{
public:

	// SpanStore() { szg = 0; szi = 0; szj = 0; }
	// SpanStore(int sizeg, int sizei, int sizej) { resize(sizeg, sizei, sizej); }
	SpanStore(int size) { resize(size, size, size); }

	void resize(int sizeg, int sizei, int sizej) { szg = sizeg+1; szi = sizei; szj = sizej; store.reserve(szg*szi*szj); store.resize(szg*szi*szj); }

	typedef struct Spans {
		Span complete;
		Span incomplete;
		Spans() { complete.type = Span::Complete; incomplete.type = Span::Incomplete; }
	} Spans;

	Spans& operator()(int g, int i, int j) { return store[(g+1)*szi*szj + i*szj + j]; }

private:
	int szg, szi, szj;
	std::vector<Spans> store;
};



// iezīmju tipu nosaka pēc pašas funkcijas nosaukuma

// TODO: restricted pointeri

void getFeatures(vector<Feature>& features, const Token* gtoken, const Token* htoken, const Token* mtoken, bool dh, bool dim, bool dom)
{
#if 0
	const int maxFeatureCount = 10;

	// rezervē vēl dažus baitus
	if(features.capacity() - features.size() < maxFeatureCount)
		features.reserve(features.size() + maxFeatureCount);
#endif
	
	// formā...
	//features.emplace_back(byte/word/dword elements...);

#define f(...) features.emplace_back(__VA_ARGS__);

	const int g = gtoken ? gtoken->index() : -1;
	const int h = htoken->index();
	const int m = mtoken->index();

	const Feature::word gftag = gtoken ? gtoken->fullTagID() : -1;
	const Feature::word gtag = gtoken ? gtoken->tagID() : -2;
	const Feature::byte gptag = gtoken ? gtoken->tag()[0] : -1;
	const Feature::word hftag = htoken->fullTagID();
	const Feature::word htag = htoken->tagID();
	const Feature::byte hptag = htoken->tag()[0];
	const Feature::word mftag = mtoken->fullTagID();
	const Feature::word mtag = mtoken->tagID();
	const Feature::byte mptag = mtoken->tag()[0];

	const Feature::word gword = gtoken ? gtoken->wordID() : -3;
	const Feature::word hword = htoken->wordID();
	const Feature::word mword = mtoken->wordID();

	const Feature::byte hm = h < m ? 0 : 1;
	const Feature::byte ghm = (g < h ? 0 : 2) + (h < m ? 0 : 1);
	
	const Feature::byte zero = 0;

	Feature::byte hmdist = 0;
	{
		int d = h-m;
		if(d < 0) d = -d;
		if(d > 1) hmdist++;
		if(d > 2) hmdist++;
		// if(d > 3) hmdist++;
		if(d > 5) hmdist++;
		if(d > 10) hmdist++;
		if(d > 20) hmdist++;
		if(d > 30) hmdist++;
		if(d > 40) hmdist++;
	}

	Feature::byte ghdist = 0;
	{
		int d = g-h;
		if(d < 0) d = -d;
		// if(d > 1) ghdist++;
		if(d > 2) ghdist++;
		// if(d > 3) ghdist++;
		if(d > 5) ghdist++;
		if(d > 10) ghdist++;
		if(d > 20) ghdist++;
		if(d > 30) ghdist++;
		if(d > 40) ghdist++;
	}
	
	// potenciālās iezīmes
	
	//
	// Nepieciešamais bitu apjoms uz vienību:
	//
	// hm, ghm - 2 biti (T)
	// ghdist|hmdist|degen - 4 biti (S)
	// tagP - 8 biti (N)
	// tag, fulltag, word, lemma - 16 biti (L)
	//
	// Iezīmes:
	//
	// 1.k. dep.
	// [-|ghdist|hmdist], [-|degen], fulltag, fulltag, [hm]
	// [-|ghdist|hmdist], [-|degen], tagP, tagP, [hm]
	// [-|ghdist|hmdist], [-|degen], tag, tag, [hm]
	// [-|hmdist], [word], tag, [word], tag, [hm]
	//
	// 2.k. dep.
	// [dgen], fulltag, fulltag, fulltag, [ghm]
	// [dgen], tagP, tagP, tagP, [ghm]
	// [dgen], tag, tag, tag, [ghm]
	//
	// NOTE: word, tag, fullTag savā starpā nekonfliktē, jo tie ir zem viena indeksa
	// izņemot gadījumus, kad kāds no tagiem sakrīt ar vārdu, bet lai būtu reāls konflikts, arī citām sakritībām jānotiek
	//
	// TODO: lai nekur nebūtu nejauša pārklāšanās, var pievienot pirmo elementu kā tipu,
	// kurus iepriekš definē kā const Feature::byte TT, WTWT, ... (tag-tag, word-tag-word-tag, ...)
	//

	f(hftag, mftag);
	f(htag, mtag);
	f(hptag, mptag);
	f(hftag, mftag, hm);
	f(htag, mtag, hm);
	f(hptag, mptag, hm);

	f(hmdist, zero, hftag, mftag);
	f(hmdist, zero, htag, mtag);
	f(hmdist, zero, hptag, mptag);
	f(hmdist, zero, hftag, mftag, hm);
	f(hmdist, zero, htag, mtag, hm);
	f(hmdist, zero, hptag, mptag, hm);
	
	// ar word
	f(hword, htag, mword, mtag);
	f(htag, mword, mtag);
	f(hword, htag, mtag);
	f(hword, htag, mword, mtag, hm);
	f(htag, mword, mtag, hm);
	f(hword, htag, mtag, hm);
	
	// ar word
	f(hmdist, zero, hword, htag, mword, mtag);
	f(hmdist, zero, htag, mword, mtag);
	f(hmdist, zero, hword, htag, mtag);
	f(hmdist, zero, hword, htag, mword, mtag, hm);
	f(hmdist, zero, htag, mword, mtag, hm);
	f(hmdist, zero, hword, htag, mtag, hm);

	// 2.k. dep.
	// [dgen], fulltag, fulltag, fulltag, [ghm]
	// [dgen], tagP, tagP, tagP, [ghm]
	// [dgen], tag, tag, tag, [ghm]
	
	f(gftag, hftag, mftag);
	f(gptag, hptag, mptag);
	f(gtag, htag, mtag);
	f(gftag, hftag, mftag, ghm);
	f(gptag, hptag, mptag, ghm);
	f(gtag, htag, mtag, ghm);


	// te var redzēt kā pareizā secībā izsaukt deģenerēto gadījumu funkcijas
	if(dom)
		getFeaturesDegenCS(features, gtoken, htoken, mtoken, dim, dh);
	if(dh || dim)
		getFeaturesDegenICS(features, gtoken, htoken, mtoken, dh, dim);
}

// par degenouterm neko nezina
void getFeaturesDegenICS(vector<Feature>& features, const Token* gtoken, const Token* htoken, const Token* mtoken, bool dh, bool dim)
{
#if 0
	const int maxFeatureCount = 10;

	// rezervē vēl dažus baitus
	if(features.capacity() - features.size() < maxFeatureCount)
		features.reserve(features.size() + maxFeatureCount);
#endif

	const int g = gtoken ? gtoken->index() : -1;
	const int h = htoken->index();
	const int m = mtoken->index();

	const Feature::word gftag = gtoken ? gtoken->fullTagID() : -1;
	const Feature::word gtag = gtoken ? gtoken->tagID() : -2;
	const Feature::byte gptag = gtoken ? gtoken->tag()[0] : -1;
	const Feature::word hftag = htoken->fullTagID();
	const Feature::word htag = htoken->tagID();
	const Feature::byte hptag = htoken->tag()[0];
	const Feature::word mftag = mtoken->fullTagID();
	const Feature::word mtag = mtoken->tagID();
	const Feature::byte mptag = mtoken->tag()[0];

	const Feature::word gword = gtoken ? gtoken->wordID() : -3;
	const Feature::word hword = htoken->wordID();
	const Feature::word mword = mtoken->wordID();

	const Feature::byte hm = h < m ? 0 : 1;
	const Feature::byte ghm = (g < h ? 0 : 2) + (h < m ? 0 : 1);

	const Feature::byte zero = 0;

	Feature::byte hmdist = 0;
	{
		int d = h-m;
		if(d < 0) d = -d;
		if(d > 1) hmdist++;
		if(d > 2) hmdist++;
		// if(d > 3) hmdist++;
		if(d > 5) hmdist++;
		if(d > 10) hmdist++;
		if(d > 20) hmdist++;
		if(d > 30) hmdist++;
		if(d > 40) hmdist++;
	}

	Feature::byte ghdist = 0;
	{
		int d = g-h;
		if(d < 0) d = -d;
		// if(d > 1) ghdist++;
		if(d > 2) ghdist++;
		// if(d > 3) ghdist++;
		if(d > 5) ghdist++;
		if(d > 10) ghdist++;
		if(d > 20) ghdist++;
		if(d > 30) ghdist++;
		if(d > 40) ghdist++;
	}

	// dh, dim
	// katram deģenerētajam savs bits
	const Feature::byte dgen = (dh ? 4 : 0) + (dim ? 2 : 0);

	// dgen pietiek ar S - small (4 bitiem !!!)
	
	// 1.k. dep.
	// [-|ghdist|hmdist], [-|degen], fulltag, fulltag, [hm]
	// [-|ghdist|hmdist], [-|degen], tagP, tagP, [hm]
	// [-|ghdist|hmdist], [-|degen], tag, tag, [hm]
	// [-|hmdist], [word], tag, [word], tag, [hm]
	
	f(zero, dgen, hftag, mftag);
	f(zero, dgen, htag, mtag);
	f(zero, dgen, hptag, mptag);
	f(zero, dgen, hftag, mftag, hm);
	f(zero, dgen, htag, mtag, hm);
	f(zero, dgen, hptag, mptag, hm);

	f(hmdist, dgen, hftag, mftag);
	f(hmdist, dgen, htag, mtag);
	f(hmdist, dgen, hptag, mptag);
	f(hmdist, dgen, hftag, mftag, hm);
	f(hmdist, dgen, htag, mtag, hm);
	f(hmdist, dgen, hptag, mptag, hm);

	// ar word
	f(zero, dgen, hword, htag, mword, mtag);
	f(zero, dgen, htag, mword, mtag);
	f(zero, dgen, hword, htag, mtag);
	f(zero, dgen, hword, htag, mword, mtag, hm);
	f(zero, dgen, htag, mword, mtag, hm);
	f(zero, dgen, hword, htag, mtag, hm);

	// ar word
	f(hmdist, dgen, hword, htag, mword, mtag);
	f(hmdist, dgen, htag, mword, mtag);
	f(hmdist, dgen, hword, htag, mtag);
	f(hmdist, dgen, hword, htag, mword, mtag, hm);
	f(hmdist, dgen, htag, mword, mtag, hm);
	f(hmdist, dgen, hword, htag, mtag, hm);
	
	// 2.k. dep.
	// [dgen], fulltag, fulltag, fulltag, [ghm]
	// [dgen], tagP, tagP, tagP, [ghm]
	// [dgen], tag, tag, tag, [ghm]
	f(dgen, gftag, hftag, mftag);
	f(dgen, gptag, hptag, mptag);
	f(dgen, gtag, htag, mtag);
	f(dgen, gftag, hftag, mftag, ghm);
	f(dgen, gptag, hptag, mptag, ghm);
	f(dgen, gtag, htag, mtag, ghm);
}

// degenouterm = true tiek pieņemts automātiski
void getFeaturesDegenCS(vector<Feature>& features, const Token* gtoken, const Token* htoken, const Token* mtoken, bool dim, bool dh)
{
#if 0
	const int maxFeatureCount = 10;

	// rezervē vēl dažus baitus
	if(features.capacity() - features.size() < maxFeatureCount)
		features.reserve(features.size() + maxFeatureCount);
#endif

	const int g = gtoken ? gtoken->index() : -1;
	const int h = htoken->index();
	const int m = mtoken->index();

	const Feature::word gftag = gtoken ? gtoken->fullTagID() : -1;
	const Feature::word gtag = gtoken ? gtoken->tagID() : -2;
	const Feature::byte gptag = gtoken ? gtoken->tag()[0] : -1;
	const Feature::word hftag = htoken->fullTagID();
	const Feature::word htag = htoken->tagID();
	const Feature::byte hptag = htoken->tag()[0];
	const Feature::word mftag = mtoken->fullTagID();
	const Feature::word mtag = mtoken->tagID();
	const Feature::byte mptag = mtoken->tag()[0];

	const Feature::word gword = gtoken ? gtoken->wordID() : -3;
	const Feature::word hword = htoken->wordID();
	const Feature::word mword = mtoken->wordID();

	const Feature::byte hm = h < m ? 0 : 1;
	const Feature::byte ghm = (g < h ? 0 : 2) + (h < m ? 0 : 1);

	const Feature::byte zero = 0;

	Feature::byte hmdist = 0;
	{
		int d = h-m;
		if(d < 0) d = -d;
		if(d > 1) hmdist++;
		if(d > 2) hmdist++;
		// if(d > 3) hmdist++;
		if(d > 5) hmdist++;
		if(d > 10) hmdist++;
		if(d > 20) hmdist++;
		if(d > 30) hmdist++;
		if(d > 40) hmdist++;
	}

	Feature::byte ghdist = 0;
	{
		int d = g-h;
		if(d < 0) d = -d;
		// if(d > 1) ghdist++;
		if(d > 2) ghdist++;
		// if(d > 3) ghdist++;
		if(d > 5) ghdist++;
		if(d > 10) ghdist++;
		if(d > 20) ghdist++;
		if(d > 30) ghdist++;
		if(d > 40) ghdist++;
	}

	const Feature::byte dgen = (dh ? 4 : 0) + (dim ? 2 : 0) + 1;

	// bool dom = true;

	if(dim)	// m tokens ir terminal tokens (nevienā pusē nav dependency
	{
	}
	else 	// tikai no ārpuses deģenerēts bez dependency uz ārpusi
	{
	}

	// 1.k. dep.
	// [-|ghdist|hmdist], [-|degen], fulltag, fulltag, [hm]
	// [-|ghdist|hmdist], [-|degen], tagP, tagP, [hm]
	// [-|ghdist|hmdist], [-|degen], tag, tag, [hm]
	// [-|hmdist], [word], tag, [word], tag, [hm]
	
	f(zero, dgen, hftag, mftag);
	f(zero, dgen, htag, mtag);
	f(zero, dgen, hptag, mptag);
	f(zero, dgen, hftag, mftag, hm);
	f(zero, dgen, htag, mtag, hm);
	f(zero, dgen, hptag, mptag, hm);

	f(hmdist, dgen, hftag, mftag);
	f(hmdist, dgen, htag, mtag);
	f(hmdist, dgen, hptag, mptag);
	f(hmdist, dgen, hftag, mftag, hm);
	f(hmdist, dgen, htag, mtag, hm);
	f(hmdist, dgen, hptag, mptag, hm);

	// ar word
	f(zero, dgen, hword, htag, mword, mtag);
	f(zero, dgen, htag, mword, mtag);
	f(zero, dgen, hword, htag, mtag);
	f(zero, dgen, hword, htag, mword, mtag, hm);
	f(zero, dgen, htag, mword, mtag, hm);
	f(zero, dgen, hword, htag, mtag, hm);

	// ar word
	f(hmdist, dgen, hword, htag, mword, mtag);
	f(hmdist, dgen, htag, mword, mtag);
	f(hmdist, dgen, hword, htag, mtag);
	f(hmdist, dgen, hword, htag, mword, mtag, hm);
	f(hmdist, dgen, htag, mword, mtag, hm);
	f(hmdist, dgen, hword, htag, mtag, hm);
	
	// 2.k. dep.
	// [dgen], fulltag, fulltag, fulltag, [ghm]
	// [dgen], tagP, tagP, tagP, [ghm]
	// [dgen], tag, tag, tag, [ghm]
	f(dgen, gftag, hftag, mftag);
	f(dgen, gptag, hptag, mptag);
	f(dgen, gtag, htag, mtag);
	f(dgen, gftag, hftag, mftag, ghm);
	f(dgen, gptag, hptag, mptag, ghm);
	f(dgen, gtag, htag, mtag, ghm);
}

FeatureVector::Value scoreDegenICS(const FeatureVector& features, vector<Feature>& localFeatures, const Span& cs, const Span& rcs)
{
	FeatureVector::Value score = 0;

	// TODO: var atbrīvoties no šiem starpmainīgajiem
	const Token* gtoken = cs.gtoken;
	const Token* htoken = cs.htoken;
	const Token* mtoken = rcs.htoken;

	localFeatures.clear();

	bool dh = cs.type == Span::Terminal;
	bool dim = rcs.type == Span::Terminal;

	getFeaturesDegenICS(localFeatures, gtoken, htoken, mtoken, dh, dim);

	for(const Feature& feature : localFeatures)
		score += features(feature);

	return score;
}

FeatureVector::Value scoreDegenCS(const FeatureVector& features, vector<Feature>& localFeatures, const Span& ics)
{
	FeatureVector::Value score = 0;

	// TODO: var atbrīvoties no šiem starpmainīgajiem
	const Token* gtoken = ics.gtoken;
	const Token* htoken = ics.htoken;
	const Token* mtoken = ics.mtoken;

	localFeatures.clear();

	bool dh = ics.ics.cs->type == Span::Terminal;
	bool dim = ics.ics.rcs->type == Span::Terminal;

	getFeaturesDegenCS(localFeatures, gtoken, htoken, mtoken, dim, dh);

	for(const Feature& feature : localFeatures)
		score += features(feature);

	return score;
}


//
// Tipiski iezīmes vajag izvilkt pie incomplete span veidošanas, jo tur parārādās g-h-m saites
// Papildus var apskatīt deģenerētos gadījumus: h=r un r+1=m, tas notiek incomplete span veidošanas procesā
// Ir vēl viens gadījums, kas ir jāiekļauj complete span veidošanas procesā: m=e
//
// Būtu labi izdalīt atsevišķi deģenerētos gadījumus, lai vieglāk aprēķināt...
// complete span gadījums noteikti ir jāaprēķina atsevišķi, bet... kombinēt ???
//
// Domājams, ka vajadzētu sekojošu algoritmu:
// * funkcija, kas aprēķina score neko nezinot par deģenerētajiem gadījumiem
// * funkcija, kas zina par incomplete deģenerētajiem gadījumiem h=r un r+1=m un rēķina tikai tos
// * funkcija, kas zina par complete deģenerēto gadījumu m=e un arī incomplete, bet rēķina tikai complete (varbūt komplektā ar...)


// atrākais veids kā veikt extract features ?
// jāņem vērā arī deģenerētos gadījumus, no kuriem ir izsaukumi arī no complete span veidošanas punkta
// vēl ir jautājums par atmiņas reģionu: nav jēga veikt jaunas atmiņas alloc, tāpēc, ka pats process neparalelizējas,
// bet ja ir vietas, kuras paralelizējas, tad pietiketu ar veinu features vektoru uz pavediena
//
// ir vietas, kur vajag uzreiz izņemt visas iezīmes: pie teikumu salīdzināšanas treniņprocesā ir visas iezīmes vajadzīgas
//

// double parse(Tokens& tokens, const FeatureVector& features, bool showProgress = true, const std::string& postfix = "");


// dažādo iezīmju izgūšanu var dalīt pa funkcijām (cerot uz inline)


void parse(Tokens& tokens, const FeatureVector& features)
{
	int n = tokens.size();

	SpanStore spans(n);

	// darbojas kā buferis, lai nav dinamiski jārezervē atmiņa
	vector<Feature> localFeatures;
	localFeatures.reserve(256);

	FeatureVector::Value score;

	for(int w=0; w<n; w++)	// span'a platums
	{
		for(int i=0; i<n-w; i++)	// i indekss
		{
			int j = i + w;				// j indekss

			for(int g=-1; g<n; g++)		// parent/grandparent - g indekss
			{
				if(g >= i && g <= j)	// jābūt ārpus [i,j] intervāla
					continue;
				
				// terminal spans
				if(w == 0)	// i == j - TerminalSpan
				{
					spans(g, i, i).complete.terminal(g == -1 ? nullptr : &tokens[g], &tokens[i]);
					continue;
				}

				// incomplete spans, w > 0
				if(w > 0)
				{
					// incomplete = complete + reverse complete
					{
						Span& span = spans(g, i, j).incomplete;

						for(int r=i; r<j; r++)
						{
							const Span& cs = spans(g, i, r).complete;
							const Span& rcs = spans(i, j, r+1).complete;
							score = cs.score + rcs.score;

							if(r==i || r+1 == j)	// deģenerētie gadījumi
								score += scoreDegenICS(features, localFeatures, cs, rcs);

							if(r == i || score > span.score)	// bez r == i neiztikt, jo score var būt mazāks par 0, nav zināma minimālā vērtība
							{
								span.incomplete(&cs, &rcs, score);
								// span.score = score;
								// span.ics.cs = &cs;
								// span.ics.rcs = &rcs;
							}
						}

						span.scoreSelf(features, localFeatures);
					}

					// reverse incomplete = complete + reverse complete
					{
						Span& span = spans(g, j, i).incomplete;

						for(int r=i; r<j; r++)
						{
							const Span& cs = spans(g, j, r+1).complete;
							const Span& rcs = spans(j, i, r).complete;
							score = cs.score + rcs.score;

							if(r==i || r+1 == j)
								score += scoreDegenICS(features, localFeatures, cs, rcs);

							if(r == i || score > span.score)
							{
								span.incomplete(&cs, &rcs, score);
								// span.score = score;
								// span.ics.cs = &cs;
								// span.ics.rcs = &rcs;
							}
						}

						span.scoreSelf(features, localFeatures);
					}
				}

				// complete spans
				
				// complete = incomplete + complete
				{
					Span& span = spans(g, i, j).complete;

					for(int m=i+1; m<=j; m++)
					{
						const Span& ics = spans(g, i, m).incomplete;
						const Span& cs = spans(i, m, j).complete;
						score = ics.score + cs.score;
						
						if(m == j)
							score += scoreDegenCS(features, localFeatures, ics);

						if(m == i+1 || score > span.score)
						{
							span.complete(&ics, &cs, score);
							// span.score = score;
							// span.cs.ics = &ics;
							// span.cs.cs = &cs;
						}
					}
				}

				// reverse complete = incomplete + complete
				{
					Span& span = spans(g, j, i).complete;

					for(int m=i; m<j; m++)
					{
						const Span& ics = spans(g, j, m).incomplete;
						const Span& cs = spans(j, m, i).complete;
						score = ics.score + cs.score;
						
						if(m == i)
							score += scoreDegenCS(features, localFeatures, ics);

						if(m == i || score > span.score)
						{
							span.complete(&ics, &cs, score);
							// span.score = score;
							// span.cs.ics = &ics;
							// span.cs.cs = &cs;
						}
					}
				}
			}
		}
	}

	// salinko parentus kokā
	tokens.link(&spans(-1, 0, tokens.size()-1).complete);
}



string normalizedTag(const string& tag)
{
	string t;
	t += tag[0];	// pirmais simbols būs vienmēr
// #define T(i)	t += tag[i]

	// Gunta variants
	if(tag[0] == 'a')
	{
		// mpn(P,a,MPN) :- P=[a,_,M,P,N|_],atom_chars(MPN,[M,P,N]),!.
		t += tag[2]; 
		t += tag[3]; 
		t += tag[4]; 
	}
	else if(tag[0] == 'm')
	{
		// mpn(P,m,MPN) :- P=[m,_,_,M,P,N|_],atom_chars(MPN,[M,P,N]),!.
		t += tag[3]; 
		t += tag[4]; 
		t += tag[5]; 
	}
	else if(tag[0] == 'n')
	{
		// mpn(P,n,MPN) :- P=[n,_,M,P,N|_],atom_chars(MPN,[M,P,N]),!.
		t += tag[2]; /* 0.89 */
		t += tag[3]; /* 0.91 */
		t += tag[4]; /* 0.98 */
	}
	else if(tag[0] == 'p')
	{
		// mpn(P,p,MPN) :- P=[p,_,_,M,P,N|_],atom_chars(MPN,[M,P,N]),!.
		t += tag[3]; 
		t += tag[4]; 
		t += tag[5]; 
	}
	else if(tag[0] == 'v' && tag[3] == 'p')
	{
		// mpn(P,vp,MPN) :- P=[v,_,_,p,_,M,P,N|_],atom_chars(MPN,[M,P,N]),!.
		
		t += 'p';
		// t += tag[3]; 	// p
		t += tag[5]; /* 0.42 */
		t += tag[6]; /* 0.42 */
		t += tag[7]; 
	}
	else
	{
		// mpn(P,X,xxx) :- P=[X|_].
		t += "xxx";
	}

	return t;
	
#if 0
	// Ģenerēts ar tags.py no .conll failiem
	if(tag[0] == 'a')
	{
		// t += "2:";
		t += tag[2]; 
		// t += "3:";
		t += tag[3]; 
		// t += "4:";
		t += tag[4]; 
	}
	else if(tag[0] == 'c')
	{
		// t += "1:";
		t += tag[1]; 
	}
	else if(tag[0] == 'i')
	{
	}
	else if(tag[0] == 'm')
	{
		// t += "3:";
		t += tag[3]; 
		// t += "4:";
		t += tag[4]; 
		// t += "5:";
		t += tag[5]; 
	}
	else if(tag[0] == 'n')
	{
		// t += "2:";
		t += tag[2]; /* 0.89 */
		// t += "3:";
		t += tag[3]; /* 0.91 */
		// t += "4:";
		t += tag[4]; /* 0.98 */
	}
	else if(tag[0] == 'q')
	{
	}
	else if(tag[0] == 'p')
	{
		// t += "2:";
		t += tag[2]; 
		// t += "3:";
		t += tag[3]; 
		// t += "4:";
		t += tag[4]; 
		// t += "5:";
		t += tag[5]; 
	}
	else if(tag[0] == 's')
	{
		// t += "1:";
		t += tag[1]; 
		// t += "2:";
		t += tag[2]; 
		// t += "3:";
		t += tag[3]; 
		// t += "4:";
		t += tag[4]; 
	}
	else if(tag[0] == 'r')
	{
		// t += tag[1]; /* 0.5 */
	}
	else if(tag[0] == 'v')
	{
		// t += "1:";
		t += tag[1]; 
		// t += "3:";
		t += tag[3]; 
		// t += "4:";
		t += tag[4]; 
		// t += tag[5]; /* 0.42 */
		// t += tag[6]; /* 0.42 */
		// t += "7:";
		t += tag[7]; 
		// t += tag[8]; /* 0.57 */
		// t += tag[9]; /* 0.42 */
	}
	else if(tag[0] == 'y')
	{
	}
	else if(tag[0] == 'x')
	{
		// t += "1:";
		t += tag[1]; 
	}
	else if(tag[0] == 'z')
	{
		// t += "1:";
		t += tag[1]; 
	}
	else if(tag[0] == '_')
	{
	}

	return t;
#endif
}


void Token::print(int level) const
{
	for(int i=0; i<level; i++)	
		cout << " ";
	cout << "#" << _index << " : " << _word << " : " << _lemma << " : " << _tag << " : " << _parentIndex << endl;
}

void Token::printTree(int level) const
{
	for(int i=0; i<level; i++)	
		cout << " ";
	// cout << _word << endl;
	print(level);
	for(int i=0; i<_children.size(); i++)
		_children[i]->printTree(level+1);
}

void Tokens::add(const string& word, const string& lemma, const string& tag, const set<string>& tags, int parentIndex)
{
	tokens.emplace_back();
	Token& token = tokens.back();

	token._word = word;
	token._lemma = lemma;
	token._tag = normalizedTag(tag);
	token._fullTag = tag;
	token._index = tokens.size()-1;
	token._parentIndex = parentIndex;

	token._wordID = idMap(token._word);
	token._lemmaID = idMap(token._lemma);
	token._tagID = idMap(token._tag);
	token._fullTagID = idMap(token._fullTag);

	// TODO: more here
}

// Izgūst features no lokālajiem tokeniem.
bool Tokens::extractFeatures(FeatureVector& targetFeatureVector) const
{
	vector<Feature> localFeatures;
	localFeatures.reserve(256);

	for(const Token& token : tokens)
	{
		if(token.parent())
		{
			const Token* gtoken = token.parent()->parent();
			const Token* htoken = token.parent();
			const Token* mtoken = &token;
			const int g = gtoken ? gtoken->index() : -1;

			const int m = token.index();
			const int h = token.parent()->index();

			// noskaidro vai ir h deģenerētais gadījums
			bool dh = true;
			for(const Token* token : token.parent()->_children)
			{
				const int x = token->index();
				// ja starp h un m atrodas arī kaut kāds x, kas ir h childs (m siblings), tad nav h deģenerētais gadījums
				if((h < m && h < x && x < m) || (m < h && m < x && x < h))
					dh = false;
			}

			// noskaidro vai ir m iekšējais un ārējais deģenerētais gadījums
			bool dim = true;
			bool dom = true;
			for(const Token* token : token._children)
			{
				const int x = token->index();
				// ja ir kāds m childs, kas atrodas iekšpusē, tad nav m iekšējais deģenerētais gadījums
				if((h < m && x < m) || (m < h && m < x))
					dim = false;
				// ja ir kāds m childs, kas atrodas ārpusē, tad nav m ārējais deģenerētais gadījums
				else if((h < m && x > m) || (m < h && m > x))
					dom = false;
			}

			localFeatures.clear();

			getFeatures(localFeatures, gtoken, htoken, mtoken, dh, dim, dom);	// izsauks deģenerēto CS un ICS

			for(const Feature& feature : localFeatures)
				targetFeatureVector[feature] += 1;
		}
	}

	return true;
}


// Funkcija, kas izvelk parent saites no jau izparsēta CS -> izveido tokenu koku
bool Tokens::link(const Span* span)
{
	// span ir jābūt Complete
	
	// ja neizsauc unlink, tad children netiek iztīrīti (ar parent'iem viss ir kārtībā)
	unlink();	// link() tiek iztīrīti children, bet katram gadījumam, lai iztīra arī parents

	extractParents(span);
	return link();
}

// Funkcija, kas izvelk parent indeksus no jau izparsēta CS, rezultāts vēl ir jāsalinko ar link()
void Tokens::extractParents(const Span* span)
{
	// span ir jābūt Complete
	
	if(!span)
		return;

	// if(span->type == Span::Terminal)
	// {
	// 	// ir jāuzstāda tokena k-tais tags
	// 	// lai nebūtu tas jāveic vairākkārt nelietderīgi,
	// 	// tad izdevīgāk to ir darīt katram terminal spanam, kas atbild tikai par vienu tokenu
	// 	tokens[span->self()->index()].setTag(span->ki());
	// 	return;
	// }

	const Span* ics = span->cs.ics;
	if(ics) // saites ņem tikai no IS
	{
		tokens[ics->mtoken->index()].setParentIndex(ics->htoken->index());

		extractParents(ics->ics.cs);
		extractParents(ics->ics.rcs);
	}
	extractParents(span->cs.cs);
}

// Salinko tokenus kokā, ja tiem ir uzstādīti derīgi parentIndeksi
bool Tokens::link()
{
	if(!check())
		return false;

	// clear children
	for(int i=0, size=tokens.size(); i<size; i++)
		tokens[i].clearChildren();

	// NOTE: root tokenu (*) ar indeksu 0 liek mierā
	for(int i=1, size=tokens.size(); i<size; i++)
	{
		Token& token = tokens[i];

		token.setParent(&tokens[token.parentIndex()]);
	}

	return true;
}

bool Tokens::projective() const
{
	for(int i=1, size=tokens.size(); i<size; i++)
		for(int j=1; j<size; j++)
		{
			if(i == j)
				continue;

			int pi = tokens[i].parentIndex();
			int pj = tokens[j].parentIndex();

			if(i < j && j < pi && (pj < i || pi < pj))
			{
				// cout << "\n1: i=" << i << ", pi=" << pi << ", j=" << j << ", pj=" << pj << endl;
				return false;
			}

			if(j < i && i < pj && (pi < j || pj < pi))
			{
				// cout << "\n2: i=" << i << ", pi=" << pi << ", j=" << j << ", pj=" << pj << endl;
				return false;
			}
		}

	return true;
}

bool Tokens::check() const
{
	for(int i=0, size = tokens.size(); i<size; i++)
	{
		const Token& token = tokens[i];
		if(token.index() != i)
			return false;
		if(i == 0 && token.parentIndex() == -1)
			continue;
		int parentIndex = token.parentIndex();
		if(parentIndex < 0 || parentIndex >= size)
			return false;
	}
	// TODO: pārbaudīt vai ir projektīvs koks (varbūt citur) un vai nav ciklisku saišu (šito nevajag)
	return true;
}

void Tokens::print() const
{
	for(const Token& token : tokens)
	{
		token.print();
	}
}

bool Trees::readCoNLL(IndexMap& idMap, const string& filename)
{
	ifstream ifs(filename);

	if(!ifs)
		return false;

	// remove UTF-8 signature, if present
	if(ifs.peek() == 0xEF)
		ifs.get();
	if(ifs.peek() == 0xBB)
		ifs.get();
	if(ifs.peek() == 0xBF)
		ifs.get();

	string line;
	string col;
	int index, parentIndex;
	string word, lemma, tag, dummy;
	// Token::Tags tags;
	set<string> tags;

	Tokens tokens(idMap);
	tokens.reserve(500);	// diez vai būs teikumi ar vairāk kā 500 tokeniem

	typedef boost::split_iterator<string::iterator> SplitIterator;

	int maxTags = 0;

	while(getline(ifs, line))
	{
		if(line.size() > 2)
		{
			SplitIterator part = make_split_iterator(line, boost::first_finder("\t", boost::is_equal()));

			index = stoi(boost::copy_range<std::string>(*part++));
			word = boost::copy_range<string>(*part++);
			lemma = boost::copy_range<string>(*part++);
			part++;		// POS tags
			tag = boost::copy_range<string>(*part++);
			dummy = boost::copy_range<string>(*part++);
			parentIndex = stoi(boost::copy_range<std::string>(*part++));

			// transform(word.begin(), word.end(), word.begin(), (int (*)(int))tolower);
			// cout << word << endl;

			tags.clear();
			tags.emplace(tag);

			for(SplitIterator it = make_split_iterator(dummy, boost::first_finder("|", boost::is_equal())),
					end = boost::split_iterator<string::iterator>(); it!=end; ++it)
			{
				const string& part = boost::copy_range<string>(*it);		// valīda reference ?

				size_t endpos;
				if((endpos = part.find("-LV-TAG")) != string::npos)
				{
					// ja vajag izšķirt
					// if(part.find("-PREV") != string::npos)
					// else if(part.find("-NEXT") != string::npos)
					// else

					// cout << string(part, 0, endpos);
					// cout << part << " ";
					tags.emplace(part, 0, endpos);
				}
			}

			tokens.add(word, lemma, tag, tags, parentIndex);
		}
		else if(tokens.size() > 1)
		{
			// cout << tokens.check() << endl;
			// tokens.print();
			trees.emplace_back(tokens);
			
			// beidzas iepriekšējais koks, sākas nākamais
			tokens.clear();
		}
	}

	for(Tokens& tokens : trees)
		tokens.link();
	
	return true;
}

// Iet cauri visiem kokiem un izgūst iezīmes.
void Trees::extractFeatures(FeatureVector& targetFeatureVector)
{
	for(Tokens& tokens : trees)
	{
		tokens.extractFeatures(targetFeatureVector);
	}
}

void Trees::print() const
{
	for(const Tokens& tokens : trees)
	{
		cout << "----------" << endl;
		tokens.print();
	}
}

void Span::print(int level) const
{
	for(int i=0; i<level; i++)	
		cout << " ";

	if(type == Span::Complete)
		cout << "C:";
	else if(type == Span::Terminal)
		cout << "T:";
	else if(type == Span::Incomplete)
		cout << "I:";

	if(gtoken)
		cout << gtoken->index();
	else
		cout << "@";

	if(type == Span::Complete)
	{
		cout << "->" << htoken->index() << " : " << htoken->word() << ", score = " << score;
		cout << endl;
		cs.ics->print(level+1);
		if(cs.cs->type != Terminal)
			cs.cs->print(level+1);

	}
	else if(type == Span::Terminal)
	{
		cout << "->" << htoken->index() << " : " << htoken->word();
		cout << endl;
	}
	else if(type == Span::Incomplete)
	{
		cout << "->" << htoken->index() << " : " << htoken->word() << ", score = " << score;
		cout << endl;
		if(ics.cs->type != Terminal)
			ics.cs->print(level+1);
		ics.rcs->print(level+1);
	}
}


