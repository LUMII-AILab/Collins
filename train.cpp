/*
 * =====================================================================================
 *
 *       Filename:  train.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013-02-15 16:40:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Didzis Gosko (dg), didzis@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <iomanip>
#include <set>
#include <sstream>

// #include <boost/filesystem.hpp>

#include "train.hpp"
#include "utils.hpp"

// #include "MurmurHash3.h"

using namespace std;
// using namespace boost::filesystem;



// 
// Funkcija, kas ielasa kokus no CoLL faila
//
void readFile(IndexMap& idMap, Trees& trees, const string& path, bool useGeneralTags)
{
	cout << path << " ... ";
	if(trees.readCoNLL(idMap, path, useGeneralTags))
		cout << "ok" << endl;
	else
		cout << "fail" << endl;
}



// 
// Funkcija, kas var ielasīt visus conll failus no direktorijas
//
// Prasības: boost/filesystem.hpp
//
// Pašlaik netiek izmantota
//
// void readDirectory(Trees& trees, const string& path)
// {
// 	for(directory_iterator it(path), end; it!=end; it++)
// 	{
// 		if(it->path().extension() == ".conll")
// 		{
// 			readFile(trees, (const string&)it->path());
// 		}
// 	}
// }


// #define ANSI


//
// Trenēšanas funkcija
//
void TrainCase::train(FeatureVector& featureVector)
{
	int start = arguments.trainStart;
	int stop = arguments.trainStop;
	Trees& trainTrees = arguments.trainTrees;
	int limit = arguments.trainLimit;
	int T = arguments.iterations;
	bool allowNonProjective = arguments.allowTrainNonProjective;
	bool permutate = arguments.permutate;

	// // rezervē iezīmju vektoram atmiņu, būtiski optimālai ātrdarbībai
	// cout << "Allocate memory for feature vector ... ";
	// cout.flush();
	// featureVector.reserve(arguments.featureVectorSize);
	// cout << "ok" << endl;

	// izgūst visu iezīmju precedentus treniņu kokos
	cout << "Extracting initial features ... ";
	cout.flush();
	trainTrees.extractFeatures(featureVector);
	cout << "ok" << endl;

	// IndexMap index;
	// trainTrees.index(index);
	/* cout << "size = " << index.size() << endl; */

	// cout << "Testing feature vector ... ";
	// cout.flush();
	// struct HHH {
	// 	uint64_t b1;
	// 	uint64_t b2;
	// 	bool operator <(const struct HHH& f) const {
	// 		return f.b1 < b1 && f.b2 < b2;
	// 	}
	// };
	// map<struct HHH, string> testMap;
	// int c = 0;
	// for(const Feature& feature : featureVector.features())
	// {
	// 	struct HHH hash;
	// 	MurmurHash3_x64_128(feature.key().c_str(), feature.key().size(), 0, (void*)&hash);
	// 	if(testMap.find(hash) != testMap.end() && testMap[hash] != feature.key())
	// 		c++;
	// 	else
	// 		testMap[hash] = feature.key();
	// }
	// if(c > 0)
	// 	cout << c << " collisions" << endl;
	// else
	// 	cout << "ok" << endl;

#if USE_MAP != STD_MAP
	cout << "Feature vector size: " << featureVector.size() << endl;
	cout << "Feature vector bucket count: " << featureVector.bucket_count() << endl;
	cout << "Initial feature vector collisions: " << featureVector.collisions() << endl;
#endif

	// limits tiek norādīts derīgajos tokenus: +1 ir dēļ root tokena "*"
	if(limit > 0)
		limit++;

	// // ja nav norādīts apstāšanās koks, tad līdz pēdējam
	// if(stop <= 0)
	// 	stop = trainTrees.size();

	// w = 0
	featureVector.zero();
	// summārais svaru vektors v = 0
	vector<FeatureVector::Value> v(featureVector.size(), 0);

	// randomizācijas sēkla
	srand(arguments.seed);

	// koku indeksu saraksti (darbojas kā kartes): linērais un permutētais
	vector<int> permutated;		// permutated map
	vector<int> ascending;

	int count = 0;				// koku parsējumu kopskaits
	int countPerIteration = 0;	// koku (parsējumu) skaits uz iterāciju

	cout << "Allocate memory for helper feature vectors ... ";
	// kāds ir aptuvenais iezīmju skaits noparsētā kokā ?
	FeatureVector treeFV(1000);
	FeatureVector goldenFV(1000);
	// FeatureVector treeFV(1572869);
	// FeatureVector goldenFV(1572869);
	cout << "ok" << endl;

	cout << "Training: " << endl;

	for(int t=0; t<T; t++)
	{
		// iterācijas info uz ekrāna
		if(t) cout << endl;
		cout << endl;
		cout << "# [" << t+1 << " / " << T << "] : ";

		// aizpilda lineāro sarakstu
		if(t == 0 || permutate)
		{
			ascending.clear();
			for(int i=start; i<stop; i++)
				ascending.push_back(i);
		}

		// aizpilda permutēto sarakstu
		if(permutate)
		{
			permutated.clear();
			while(ascending.size() > 0)
			{
				int index = rand() % ascending.size();
				permutated.push_back(ascending[index]);
				ascending.erase(ascending.begin()+index);
			}
		}

		// permutētais saraksts, vai sakārtotais-augošais
		vector<int>& list = permutate ? permutated : ascending;

		countPerIteration = 0;	// izmantoto koku skaits uz vienu iterāciju

		int sz = list.size();
		int n = 0;
		int parts = 20;
		int part = 0;
		int nextgoal = sz / parts;

		for(int j : list)
		{
			n++;

			// zelta standarts
			const Tokens& golden = trainTrees[j];

			// neprojektīvs
			if(!allowNonProjective && !golden.projective())
			{
				cout << "N";
				continue;
			}

			// izmērs pārāk liels
			if(limit > 0 && golden.size() > limit)
			{
				cout << "!" << golden.size()-1;
				continue;
			}
				
			// kopija
			Tokens tree(golden);

#ifdef ANSI
			// buferis, kas standarta ANSI terminālī kursoram ļauj pakāpties uz atpakaļu
			RollbackOutput output;

			// info strings pie progresa izvades
			string postfix;
			{
				stringstream ss;
				ss << "  #" << j << ", size: " << golden.size() << " ]";
				postfix = ss.str();
			}
			
			output = " [ ";

			// parsēšanas mēģinājums
			double duration = parse(tree, featureVector, true, postfix);

			output.rollback();
#else
			// parsēšanas mēģinājums
			Timing timing;
			timing.start();
			parse(tree, featureVector);
			timing.stop();
			double duration = timing;
#endif

			cout << ".";	// progresa indikātors
			cout.flush();

			// ja koki atšķiras
			if(tree != golden)
			{
				// FeatureVector treeFV(1572869);
				// FeatureVector goldenFV(1572869);
				treeFV.clear();
				goldenFV.clear();
				tree.extractFeatures(treeFV);
				golden.extractFeatures(goldenFV);

				// pieskaita iezīmes no golden
				{
					const FeatureVector::Features& features = goldenFV.features();
					const FeatureVector::Weights& weights = goldenFV.weights();
					for(int i=0, size = features.size(); i<size; i++)
						featureVector[features[i]] += weights[i];
				}

				// atņem iezīmes no parses mēģinājuma
				{
					const FeatureVector::Features& features = treeFV.features();
					const FeatureVector::Weights& weights = treeFV.weights();
					for(int i=0, size = features.size(); i<size; i++)
						featureVector[features[i]] -= weights[i];
				}
			}

			// ja ir notikušas izmēru izmaiņas starp w un v
			if(featureVector.size() > v.size())
				for(int k=0, size=featureVector.size()-v.size(); k<size; k++)
					v.push_back(0);

			// v += w
			for(int k=0, size=featureVector.size(); k<size; k++)
				v[k] += featureVector[k];

			count++;
			countPerIteration++;

			if(n >= nextgoal)
			{
				part++;
				cout << " (" << (int)(100*part/parts) << "%) "; // [5%]
				nextgoal = (sz*(part+1))/parts;
			}
		}
	}

	// vidējošana: w = v / T*n
	// double koef = 1.0/(double)count;
	// for(int k=0, size=featureVector.size(); k<size; k++)
	// 	featureVector[k] = v[k] * koef;
	
	for(int k=0, size=featureVector.size(); k<size; k++)
		featureVector[k] = v[k];

	trainTreeCount = countPerIteration;

	cout << " done" << endl;
}



//
// Pārbaudes funkcija
//
void TrainCase::check(const FeatureVector& featureVector)
{
	const Trees& checkTrees = arguments.checkTrees;
	int limit = arguments.checkLimit;
	int start = arguments.checkStart;
	int stop = arguments.checkStop;
	bool allowNonProjective = arguments.allowCheckNonProjective;

	// limits tiek norādīts derīgajos tokenus: +1 ir dēļ root tokena "*"
	if(limit > 0)
		limit++;

	// // ja nav norādīts apstāšanās koks, tad līdz pēdējam
	// if(stop <= 0)
	// 	stop = checkTrees.size();

	checkUASTotalCount = 0;
	checkUASTotalMatches = 0;
	int count = 0;		// pārbaudīto koku skaits

	cout << "Verification:" << endl;

	// reproducēšanas mēģinājums
	for(int j=start; j<stop; j++)
	{
		// zelta standarts
		const Tokens& golden = checkTrees[j];

		// informācija terminālī par tekošo koku
		cout << setw(5) << j << " / [" << start << "," << stop << ")";
		cout << setw(8) << golden.size()-1 << " # ";

		bool projective = golden.projective();

		if(projective)
			cout << "P    ";
		else
			cout << "NP   ";

		// neprojektīvs
		// if(!allowNonProjective && !golden.projective())
		if(!allowNonProjective && !projective)
		{
			cout << "Non-Projective" << endl;
			continue;
		}

		// izmērs pārāk liels
		if(limit > 0 && golden.size() > limit)
		{
			cout << "Too Large" << endl;
			continue;
		}

		// neliela atstarpe
		cout << "     ";
		cout.flush();

		// kopija
		Tokens tree(golden);
		
#ifdef ANSI
		// parsēšanas mēģinājums
		double duration = parse(tree, featureVector);
#else
		// double duration = parse(tree, featureVector, false);
		Timing timing;
		timing.start();
		parse(tree, featureVector);
		timing.stop();
		double duration = timing;
#endif

		// TODO: remove this, for analysis only
		// tree.examine(featureVector, golden);

		// double similarity = tree.compare(golden);
		int matches = tree.compare(golden);
		checkUASTotalCount += tree.size()-1;
		checkUASTotalMatches += matches;

		double similarity = (double)matches/(double)(tree.size()-1);

		cout << setw(8) << setprecision(4) << defaultfloat << similarity * 100 << " %        ";
		outputDuration(duration);
		cout << endl;

		checkResults.emplace_back(j, matches, tree.size()-1, duration);

		count++;
	}

	checkTreeCount = count;

	// gala rezultāta izvade terminālī
	cout << "Total score: " << setprecision(4) << 100 * (double)checkUASTotalMatches/(double)checkUASTotalCount << " %" << endl;
}



//
// Treniņš + pārbaude
//
void TrainCase::run()
{
	// ievadparametri: iterācijas, treniņa tokenu limits, pārbaudes tokenu limits, treniņkoku skaits, pārbaudes koku skaits
	// iezīmju vektora bucket skaits
	
	bool verify = arguments.checkTrees.valid();
	bool quiet = arguments.quiet();
	
	Trees& trainTrees = arguments.trainTrees;

	Trees dummy;
	// Diemžēl šo atslēgt nav iespējams, var vienīgi norādīt uz trainTrees, bet tad ir avots jaunām, iespējams, grūti nosakāmām kļūdām.
	// Tad jau drīzāk uz tukšu kopu!
	Trees& checkTrees = verify ? arguments.checkTrees() : dummy;

	if(trainTrees.size() == 0 || (verify && checkTrees.size() == 0))
	{
		if(!quiet)
		{
			if(trainTrees.size() == 0)
				cout << "Error: empty training set!" << endl;
			if(verify && checkTrees.size() == 0)
				cout << "Error: empty verification set!" << endl;
		}
		return;
	}

	// pārbauda start, stop vērtības

	if(arguments.trainStop == 0 || arguments.trainStop > trainTrees.size())
		arguments.trainStop = trainTrees.size();
	else if(arguments.trainStop < 0)
		arguments.trainStop = trainTrees.size() + arguments.trainStop;

	if(verify)
	{
		if(arguments.checkStop == 0 || arguments.checkStop > checkTrees.size())
			arguments.checkStop = checkTrees.size();
		else if(arguments.checkStop < 0)
			arguments.checkStop = checkTrees.size() + arguments.checkStop;
	}

	if(arguments.trainStart < 0)
		arguments.trainStart = trainTrees.size() + arguments.trainStart;
	if(arguments.trainStart > arguments.trainStop)
		arguments.trainStart = arguments.trainStop;

	if(verify)
	{
		if(arguments.checkStart < 0)
			arguments.checkStart = checkTrees.size() + arguments.checkStart;
		if(arguments.checkStart > arguments.checkStop)
			arguments.checkStart = arguments.checkStop;
	}
	

	if(!quiet)
	{
		if(!arguments.trainCoNLL().empty())
			cout << "Training tree set from " << arguments.trainCoNLL() << endl;
		if(verify && !arguments.checkCoNLL().empty())
			cout << "Verification tree set from " << arguments.checkCoNLL() << endl;
		cout << "Will perform " << arguments.iterations << " iterations " << (arguments.permutate ? "with" : "without") << " permutations";
		if(arguments.permutate)
			cout << " using seed " << arguments.seed;
		if(arguments.useGeneralTags)
			cout << " with general tags";
		cout << endl;
		cout << "Training set non-projective trees " << (arguments.allowTrainNonProjective ? "included" : "excluded") << endl;
		if(verify)
			cout << "Verification set non-projective trees " << (arguments.allowCheckNonProjective ? "included" : "excluded") << endl;
		if(arguments.trainLimit > 0)
			cout << "Will limit to " << arguments.trainLimit();
		else
			cout << "Unlimited";
		cout << " training tokens per tree" << endl;
		if(verify)
		{
			if(arguments.checkLimit > 0)
				cout << "Will limit to " << arguments.checkLimit();
			else
				cout << "Unlimited";
			cout << " verification tokens per tree" << endl;
		}
		// cout << "Feature vector bucket count: " << arguments.featureVectorSize() << endl;
		cout << "Training trees: [" << arguments.trainStart() << "," << arguments.trainStop() << ") of " << trainTrees.size() << " trees" << endl;
		if(verify)
			cout << "Verification trees: [" << arguments.checkStart() << "," << arguments.checkStop() << ") of "
				<< checkTrees.size() << " trees" << endl;
	}


	// rezervē iezīmju vektoram atmiņu, būtiski optimālai ātrdarbībai
	if(!quiet)
	{
		cout << "Allocate memory for feature vector ... ";
		cout.flush();
	}
	// FeatureVector featureVector;
	// FeatureVector featureVector(arguments.featureVectorSize);	// TODO: vajag pieselektēt tuvāko pirmskaitli
	// FeatureVector featureVector;				// NOTE: lai būtu pirmskaitlis (noklusētais)
	// featureVector.reserve(arguments.featureVectorSize);
	// FeatureVector featureVector(arguments.featureVectorSize);
	featureVector.reserve(arguments.featureVectorSize);
	if(!quiet)
	{
		cout << "ok" << endl;
#if USE_MAP != STD_MAP
		cout << "Feature vector initial bucket count: " << featureVector.bucket_count() << endl;
#endif
		cout << "Identificator map size: " << arguments.getIDMap().size() << endl;
	}



	Timing timing;

	timing.start();

	train(featureVector);
	
	timing.stop();
	trainTime = timing;
	timing.start();

	if(verify)
		check(featureVector);

	timing.stop();
	if(verify)
		checkTime = timing;
	else
		checkTime = 0;

	collisions = featureVector.collisions();


	if(!quiet)
	{
		cout << "Trained with " << arguments.iterations() << " iterations " << (arguments.permutate ? "with" : "without") << " permutations";
		if(arguments.permutate)
			cout << " using seed " << arguments.seed;
		if(arguments.useGeneralTags)
			cout << " with general tags";
		cout << endl;
		if(arguments.trainLimit > 0)
			cout << "Training set limited to " << arguments.trainLimit() << " tokens per tree" << endl;
		else if(arguments.trainLimit == 0)
			cout << "Training set with unlimited token count per tree" << endl;
		if(verify && arguments.checkLimit > 0)
			cout << "Verification set limited to " << arguments.checkLimit() << " tokens per tree" << endl;
		else if(verify && arguments.checkLimit == 0)
			cout << "Verification set with unlimited token count per tree" << endl;
		cout << "Trained on " << trainTreeCount << " trees" << endl;
		if(verify)
			cout << "Verified on " << checkTreeCount << " trees" << endl;
		cout << "Final feature vector size: " << featureVector.size() << endl;
#if USE_MAP != STD_MAP
		cout << "Feature vector bucket count: " << featureVector.capacity() << endl;
		cout << "Final feature vector collision count: " << featureVector.collisions() << endl;
#endif
		cout << "Identificator map size: " << arguments.getIDMap().size() << endl;
		cout << "Training time: ";
		outputDuration(trainTime);
		cout << endl;
		if(verify)
		{
			cout << "Verification time: ";
			outputDuration(checkTime);
			cout << endl;
		}
		cout << "Total time: ";
		outputDuration(trainTime + checkTime);
		cout << endl;
	}
}



// 
// Rezumējums
//
void TrainCases::summary()
{
	set<int> iterations;
	set<int> trainLimits;
	set<int> checkLimits;
	set<bool> trainNonProjective;
	set<bool> checkNonProjective;
	int totalTokens = 0;
	int totalMatchedTokens = 0;
	double totalTrainTime = 0;
	double totalCheckTime = 0;
	int totalCheckCount = 0;

	for(const TrainCase& trainCase : trainCases)
	{
		iterations.insert(trainCase.arguments.iterations);
		trainLimits.insert(trainCase.arguments.trainLimit);
		checkLimits.insert(trainCase.arguments.checkLimit);
		trainNonProjective.insert(trainCase.arguments.allowTrainNonProjective);
		checkNonProjective.insert(trainCase.arguments.allowCheckNonProjective);

		totalCheckCount += trainCase.checkTreeCount;
		totalTokens += trainCase.checkUASTotalCount;
		totalMatchedTokens += trainCase.checkUASTotalMatches;
		totalTrainTime += trainCase.trainTime;
		totalCheckTime += trainCase.checkTime;
	}

	bool first;

	cout << "--- SUMMARY ---" << endl;

	cout << "Training set count: " << trainCases.size() << endl;

	cout << "Iterations: ";
	first = true;
	for(int i : iterations)
	{
		if(!first)
			cout << ", ";
		cout << i;
		first = true;
	}
	cout << endl;

	cout << "Training limit: ";
	first = true;
	for(int limit : trainLimits)
	{
		if(!first)
			cout << ", ";
		if(limit == 0)
			cout << "unlimited";
		else
			cout << limit;
		first = true;
	}
	cout << endl;

	cout << "Verification limit: ";
	first = true;
	for(int limit : checkLimits)
	{
		if(!first)
			cout << ", ";
		if(limit == 0)
			cout << "unlimited";
		else
			cout << limit;
		first = true;
	}
	cout << endl;

	cout << "Trained on non-projective trees: ";
	first = true;
	for(bool allow : trainNonProjective)
	{
		if(!first)
			cout << " and ";
		cout << (allow ? "yes" : "no");
		first = true;
	}
	cout << endl;

	cout << "Verified on non-projective trees: ";
	first = true;
	for(bool allow : checkNonProjective)
	{
		if(!first)
			cout << " and ";
		cout << (allow ? "yes" : "no");
		first = true;
	}
	cout << endl;

	cout << "Total tree verification parses made: " << totalCheckCount << endl;
	cout << "Average verification time per tree: ";
	outputDuration(totalCheckTime / (double)totalCheckCount);
	cout << endl;

	cout << "Total parsed token count: " << totalTokens << endl;
	cout << "Total matched token count: " << totalMatchedTokens << endl;
	cout << "Total UAS: " << setprecision(4) << defaultfloat << 100.0*(double)totalMatchedTokens/(double)totalTokens << " %" << endl;

	cout << "Total training time: ";
	outputDuration(totalTrainTime);
	cout << endl;
	cout << "Total verification time: ";
	outputDuration(totalCheckTime);
	cout << endl;
	cout << "Total time: ";
	outputDuration(totalTrainTime + totalCheckTime);
	cout << endl;
}

bool train(TrainCase::Arguments& arguments, FeatureVector& featureVector, IndexMap& idMap, streams& istreams)
{
	Trees trees(idMap);

	// vispirms ielasa no visām straumēm
	while(streams::stream& stream = istreams.next())
	{
		while(stream)
			stream >> trees;
	}

	// pēc tam var sākt treniņus

	int start = 0;
	int stop = trees.size();
	int limit = arguments.trainLimit;
	int T = arguments.iterations;
	bool allowNonProjective = arguments.allowTrainNonProjective;
	bool permutate = arguments.permutate;
	bool quiet = arguments.quiet;

	// // rezervē iezīmju vektoram atmiņu, būtiski optimālai ātrdarbībai
	// cout << "Allocate memory for feature vector ... ";
	// cout.flush();
	// featureVector.reserve(arguments.featureVectorSize);
	// cout << "ok" << endl;

	// izgūst visu iezīmju precedentus treniņu kokos
	if(!quiet)
	{
		cout << "Extracting initial features ... ";
		cout.flush();
	}
	trees.extractFeatures(featureVector);
	if(!quiet)
		cout << "ok" << endl;

#if USE_MAP != STD_MAP
	if(!quiet)
	{
		cout << "Feature vector size: " << featureVector.size() << endl;
		cout << "Feature vector bucket count: " << featureVector.bucket_count() << endl;
		cout << "Initial feature vector collisions: " << featureVector.collisions() << endl;
	}
#endif

	// limits tiek norādīts derīgajos tokenus: +1 ir dēļ root tokena "*"
	if(limit > 0)
		limit++;

	// // ja nav norādīts apstāšanās koks, tad līdz pēdējam
	// if(stop <= 0)
	// 	stop = trainTrees.size();

	// w = 0
	featureVector.zero();
	// summārais svaru vektors v = 0
	vector<FeatureVector::Value> v(featureVector.size(), 0);

	// randomizācijas sēkla
	srand(arguments.seed);

	// koku indeksu saraksti (darbojas kā kartes): linērais un permutētais
	vector<int> permutated;		// permutated map
	vector<int> ascending;

	int count = 0;				// koku parsējumu kopskaits
	int countPerIteration = 0;	// koku (parsējumu) skaits uz iterāciju

	if(!quiet)
		cout << "Allocate memory for helper feature vectors ... ";
	// kāds ir aptuvenais iezīmju skaits noparsētā kokā ?
	FeatureVector treeFV(1000);
	FeatureVector goldenFV(1000);
	// FeatureVector treeFV(1572869);
	// FeatureVector goldenFV(1572869);
	if(!quiet)
	{
		cout << "ok" << endl;

		cout << "Training: " << endl;
	}

	for(int t=0; t<T; t++)
	{
		if(!quiet)
		{
			// iterācijas info uz ekrāna
			if(t) cout << endl;
			cout << endl;
			cout << "# [" << t+1 << " / " << T << "] : ";
		}

		// aizpilda lineāro sarakstu
		if(t == 0 || permutate)
		{
			ascending.clear();
			for(int i=start; i<stop; i++)
				ascending.push_back(i);
		}

		// aizpilda permutēto sarakstu
		if(permutate)
		{
			permutated.clear();
			while(ascending.size() > 0)
			{
				int index = rand() % ascending.size();
				permutated.push_back(ascending[index]);
				ascending.erase(ascending.begin()+index);
			}
		}

		// permutētais saraksts, vai sakārtotais-augošais
		vector<int>& list = permutate ? permutated : ascending;

		countPerIteration = 0;	// izmantoto koku skaits uz vienu iterāciju

		int sz = list.size();
		int n = 0;
		int parts = 20;
		int part = 0;
		int nextgoal = sz / parts;

		for(int j : list)
		{
			n++;

			// zelta standarts
			const Tokens& golden = trees[j];

			// neprojektīvs
			if(!allowNonProjective && !golden.projective())
			{
				if(!quiet)
					cout << "N";
				continue;
			}

			// izmērs pārāk liels
			if(limit > 0 && golden.size() > limit)
			{
				if(!quiet)
					cout << "!" << golden.size()-1;
				continue;
			}
				
			// kopija
			Tokens tree(golden);

			// parsēšanas mēģinājums
			Timing timing;
			timing.start();
			parse(tree, featureVector);
			timing.stop();
			double duration = timing;

			if(!quiet)
			{
				cout << ".";	// progresa indikātors
				cout.flush();
			}

			// ja koki atšķiras
			if(tree != golden)
			{
				// FeatureVector treeFV(1572869);
				// FeatureVector goldenFV(1572869);
				treeFV.clear();
				goldenFV.clear();
				tree.extractFeatures(treeFV);
				golden.extractFeatures(goldenFV);

				// pieskaita iezīmes no golden
				{
					const FeatureVector::Features& features = goldenFV.features();
					const FeatureVector::Weights& weights = goldenFV.weights();
					for(int i=0, size = features.size(); i<size; i++)
						featureVector[features[i]] += weights[i];
				}

				// atņem iezīmes no parses mēģinājuma
				{
					const FeatureVector::Features& features = treeFV.features();
					const FeatureVector::Weights& weights = treeFV.weights();
					for(int i=0, size = features.size(); i<size; i++)
						featureVector[features[i]] -= weights[i];
				}
			}

			// ja ir notikušas izmēru izmaiņas starp w un v
			if(featureVector.size() > v.size())
				for(int k=0, size=featureVector.size()-v.size(); k<size; k++)
					v.push_back(0);

			// v += w
			for(int k=0, size=featureVector.size(); k<size; k++)
				v[k] += featureVector[k];

			count++;
			countPerIteration++;

			if(n >= nextgoal)
			{
				part++;
				if(!quiet)
					cout << " (" << (int)(100*part/parts) << "%) "; // [5%]
				nextgoal = (sz*(part+1))/parts;
			}
		}
	}

	// vidējošana: w = v / T*n
	// double koef = 1.0/(double)count;
	// for(int k=0, size=featureVector.size(); k<size; k++)
	// 	featureVector[k] = v[k] * koef;
	
	for(int k=0, size=featureVector.size(); k<size; k++)
		featureVector[k] = v[k];

	featureVector.save("fvnew");
	// trainTreeCount = countPerIteration;

	if(!quiet)
		cout << " done" << endl;

	return true;
}

bool parse(TrainCase::Arguments& arguments, const FeatureVector& featureVector, const IndexMap& idMap_,
		streams& istreams, std::basic_ostream<char>& ostream)
{
	// pa vienam kokam no ievadstraumes, parsē, rezultāts izvadstraumē
	while(streams::stream& stream = istreams.next())
	{
		while(stream)
		{
			// index apakškarte, kuru var pārvietot arī uz parent scope, ja tas palielina ātrdarbību
			// doma ir tāda, ka katrs ievadkoks ir neatkarīgs no otra, tāpēc kad viens ir pabeigts,
			// tad nav vajadzīgs uzglabāt nākamā koka identifikātoru informāciju, rezultātā
			// identifikātoru skaits nepalielinās un nav risks potenciālam overflow
			IndexMap idMap(idMap_);

			Tokens tree(idMap);

			try
			{
				stream >> tree;

				if(tree)
					parse(tree, featureVector);

				ostream << tree;
			}
			catch(exception& e)
			{
				cerr << "error: " << e.what() << endl;

				// vēl ir jāpabeidz ielasīt līdz tukšajai rindiņai
				string line;
				while(getline(stream, line))
				{
					if(line.size() == 0 || line == "\n")
						break;
				}

				ostream << endl;	// tukšs rezultāts
			}
		}
	}

	return true;
}
