/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013-03-10 18:25:19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Didzis Gosko (dg), didzis@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>

#include <boost/program_options.hpp>

#include "collins0.hpp"
#include "train.hpp"
#include "utils.hpp"

namespace po = boost::program_options;

using namespace std;

int main(int argc, char const* argv[])
{
	TrainCase::Arguments defaultArguments;
	// defaultArguments.featureVectorSize = 300000000;
	defaultArguments.featureVectorSize = 300000000;
	defaultArguments.iterations = 5;
	defaultArguments.limit = 0;
	defaultArguments.allowNonProjective = true;


	po::variables_map vm;

	try
	{
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "this help message")
			("projective,P", "train and verify only projective trees (default: all)")
			("train-projective,p", "train only projective trees (default: no)")
			("verify-projective,r", "verify only projective trees (default: no)")
			("iterations,i", po::value<int>(), "train iteration count (default: 5)")
			("reserve,R", po::value<int>(), "reserve memory for N feature vector units (default: 300000000)")
			("limit,L", po::value<int>(), "limit to trees with less or equal token count (default: 0 - unlimited)")
			("train-limit,l", po::value<int>(), "limit to training trees with less or equal token count (default: 0 - unlimited)")
			("verify-limit,k", po::value<int>(), "limit to verification trees with less or equal token count (default: 0 - unlimited)")
			("trainset,s", po::value<vector<int>>(), "use train & verification sets: 1,2,4,... (default: 0,1,2,3,4,5,6,7,8,9)")
			("seed,S", po::value<int>(), "training tree order permutation seed, -1 to disable")
			// ("disable-permutations,d", "disable training tree order permutations")
		;

		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if(vm.count("help"))
		{
			cout << "Usage: " << argv[0] << " [options]" << endl;
			cout << endl;
			cout << desc << "\n";
			return 0;
		}
	}
	catch(exception& e)
	{
        cerr << "error: " << e.what() << "\n";
		return 1;
	}

	cout << "Collins model 0 parser" << endl << endl;

	// print command line
	cout << "Command Line: ";
	for(int i=0; i<argc; ++i)
	{
		if(i > 0) cout << " ";
		cout << argv[i];
	}
	cout << endl;

	vector<int> trainsets;

	try
	{
		if(vm.count("seed"))
		{
			int seed = vm["seed"].as<int>();
			if(seed == -1)
				defaultArguments.permutate = false;
			else
				defaultArguments.seed = seed;
			// defaultArguments.seed = vm["seed"].as<int>();
		}
		// if(vm.count("disable-permutations"))
		// {
		// 	defaultArguments.permutate = false;
		// }
		if(vm.count("projective"))
		{
			defaultArguments.allowNonProjective = false;
		}
		if(vm.count("train-projective"))
		{
			defaultArguments.allowTrainNonProjective = false;
		}
		if(vm.count("verify-projective"))
		{
			defaultArguments.allowCheckNonProjective = false;
		}
		if(vm.count("iterations"))
		{
			defaultArguments.iterations = vm["iterations"].as<int>();
		}
		if(vm.count("reserve"))
		{
			defaultArguments.featureVectorSize = vm["reserve"].as<int>();
		}
		if(vm.count("limit"))
		{
			defaultArguments.limit = vm["limit"].as<int>();
		}
		if(vm.count("train-limit"))
		{
			defaultArguments.trainLimit = vm["train-limit"].as<int>();
		}
		if(vm.count("verify-limit"))
		{
			defaultArguments.checkLimit = vm["verify-limit"].as<int>();
		}
		if(vm.count("trainset"))
		{
			trainsets = vm["trainset"].as<vector<int>>();
		}
	}
	catch(exception& e)
	{
        cerr << "error: " << e.what() << "\n";
		return 1;
	}

	// ja ir tukšs, tad piepilda
	if(trainsets.size() == 0)
	{
		for(int i=0; i<10; i++)
			trainsets.push_back(i);
	}
	else
		sort(trainsets.begin(), trainsets.end());

	cout << "Used options:" << endl;

	if(defaultArguments.trainLimit == 0)
		cout << "- unlimited training tree token count" << endl;
	else
		cout << "- limit training tree token count = " << defaultArguments.trainLimit << endl;

	if(defaultArguments.checkLimit == 0)
		cout << "- unlimited verification tree token count" << endl;
	else
		cout << "- limit verification tree token count = " << defaultArguments.checkLimit << endl;

	if(defaultArguments.allowTrainNonProjective)
		cout << "- non-projective train trees included" << endl;
	else
		cout << "- non-projective train trees excluded" << endl;

	if(defaultArguments.allowCheckNonProjective)
		cout << "- non-projective verify trees included" << endl;
	else
		cout << "- non-projective verify trees excluded" << endl;

	cout << "- " << defaultArguments.iterations << " training iterations ";
	if(defaultArguments.permutate())
		cout << "(permutated with seed " << defaultArguments.seed << ")" << endl;
	else
		cout << "(no permutations)" << endl;
	cout << "- reserve memory for " << defaultArguments.featureVectorSize << " feature vector units" << endl;

	cout << "Trainsets:" << endl;

	vector<string> trainCoNLL, checkCoNLL;
	char buf[255];
	for(int i : trainsets)
	{
		sprintf(buf, "train/train%i.conll", i);
		trainCoNLL.push_back(buf);
		sprintf(buf, "golden/golden%i.conll", i);
		checkCoNLL.push_back(buf);
		cout << trainCoNLL[trainCoNLL.size()-1] << ", " << checkCoNLL[checkCoNLL.size()-1] << endl;
	}


	cout << endl;
	cout << "Starting..." << endl;
	cout << endl;


	// defaultArguments.featureVectorSize = 300000000;
	// defaultArguments.iterations = 5;
	// defaultArguments.limit = 0;
	// defaultArguments.limit = 20;
	// defaultArguments.allowNonProjective = true;

	Timing timing;
	timing.start();

	TrainCases trainCases;

	for(int i=0; i<trainCoNLL.size(); i++)
	{
		if(i > 0)
			cout << "---------------------------------------------------------------------" << endl;
		cout << "Trainset: " << trainCoNLL[i] << ", " << checkCoNLL[i] << endl;
		cout << endl;
		trainCases(TrainCase::Arguments(defaultArguments).setTrainCoNLL(trainCoNLL[i]).setCheckCoNLL(checkCoNLL[i]));
		cout << endl;
	}


	// trainCases(TrainCase::Arguments(defaultArguments).setTrainCoNLL("train/train0.conll").setCheckCoNLL("golden/golden0.conll"));
	// cout << "---------------------------------------------------------------------" << endl;
	// trainCases(TrainCase::Arguments(defaultArguments).setTrainCoNLL("train/train1.conll").setCheckCoNLL("golden/golden1.conll"));
	// cout << "---------------------------------------------------------------------" << endl;
	// trainCases(TrainCase::Arguments(defaultArguments).setTrainCoNLL("train/train2.conll").setCheckCoNLL("golden/golden2.conll"));
	// cout << "---------------------------------------------------------------------" << endl;
	// trainCases(TrainCase::Arguments(defaultArguments).setTrainCoNLL("train/train3.conll").setCheckCoNLL("golden/golden3.conll"));
	// cout << "---------------------------------------------------------------------" << endl;
	// trainCases(TrainCase::Arguments(defaultArguments).setTrainCoNLL("train/train4.conll").setCheckCoNLL("golden/golden4.conll"));
	// cout << "---------------------------------------------------------------------" << endl;
	// trainCases(TrainCase::Arguments(defaultArguments).setTrainCoNLL("train/train5.conll").setCheckCoNLL("golden/golden5.conll"));
	// cout << "---------------------------------------------------------------------" << endl;
	// trainCases(TrainCase::Arguments(defaultArguments).setTrainCoNLL("train/train6.conll").setCheckCoNLL("golden/golden6.conll"));
	// cout << "---------------------------------------------------------------------" << endl;
	// trainCases(TrainCase::Arguments(defaultArguments).setTrainCoNLL("train/train7.conll").setCheckCoNLL("golden/golden7.conll"));
	// cout << "---------------------------------------------------------------------" << endl;
	// trainCases(TrainCase::Arguments(defaultArguments).setTrainCoNLL("train/train8.conll").setCheckCoNLL("golden/golden8.conll"));
	// cout << "---------------------------------------------------------------------" << endl;
	// trainCases(TrainCase::Arguments(defaultArguments).setTrainCoNLL("train/train9.conll").setCheckCoNLL("golden/golden9.conll"));
	
	// cout << "---------------------------------------------------------------------" << endl;
	
	cout << endl;

	trainCases.summary();
	
	cout << "Total run time: ";
	outputDuration(timing.stop());
	cout << endl;

	cout << endl;

	return 0;
}

