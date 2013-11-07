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
#include <vector>
#include <string>
#include <fstream>

#include <glob.h>
#include <wordexp.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "collins0.hpp"
#include "train.hpp"
#include "utils.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

using namespace std;

vector<string> glob(const string& pattern)
{
	glob_t g;
	glob(pattern.c_str(), GLOB_TILDE | GLOB_BRACE | GLOB_NOSORT | GLOB_MARK, NULL, &g);

	vector<string> result;
	for(unsigned int i=0; i<g.gl_pathc; ++i)
		result.emplace_back(g.gl_pathv[i]);
	globfree(&g);
	return result;
}

string expand(const string& path)
{
	wordexp_t p;
	wordexp(path.c_str(), &p, WRDE_NOCMD);
	if(p.we_wordc == 0)
		return path;
	string result = p.we_wordv[0];
	wordfree(&p);
	return result;
}

// relative path
// http://stackoverflow.com/questions/10167382/boostfilesystem-get-relative-path/10167550#10167550

template <> fs::path&
fs::path::append<fs::path::iterator>(
	fs::path::iterator begin,
	fs::path::iterator end,
	const codecvt_type& cvt)
{ 
	for( ; begin != end ; ++begin )
		*this /= *begin;
	return *this;
}

// Return path when appended to from will resolve to same as to
// a + make_relative(a, b) == b
fs::path make_relative(fs::path from, fs::path to)
{
	// namespace fs = boost::filesystem;
	from = fs::absolute(from);
	to = fs::absolute(to);
	fs::path ret;
	fs::path::const_iterator it_from(from.begin()), it_to(to.begin());
	// Find common base
	for(fs::path::const_iterator to_end(to.end()), from_end(from.end());
			it_from != from_end && it_to != to_end && *it_from == *it_to; ++it_from, ++it_to);
	// Navigate backwards in directory to reach previously found base
	for(fs::path::const_iterator from_end(from.end()); it_from != from_end; ++it_from)
	{
		if(*it_from != ".")
			ret /= "..";
	}
	// Now navigate down the directory branch
	ret.append(it_to, to.end());
	return ret;
}

void readDirectory(IndexMap& idMap, Trees& trees, const string& path)
{
	for(boost::filesystem::directory_iterator it(path), end; it!=end; it++)
	{
		if(it->path().extension() == ".conll")
		{
			readFile(idMap, trees, (const string&)it->path());
		}
	}
}

bool find_file(fs::path& filePath, const fs::path& possibleBaseDirAbsolute, string& display, const string& baseDirDisplay, bool absolute = false)
{
	if(filePath.empty() || (!fs::is_regular_file(filePath)
		&& (filePath.is_absolute() || *filePath.begin() == "." || !fs::is_regular_file(possibleBaseDirAbsolute / filePath))))
		return false;

	fs::path filePathAbsolute;

	if(filePath.is_absolute())
	{
		filePathAbsolute = fs::canonical(filePath);
		filePath = filePathAbsolute;
		display = filePath.string();
	}
	else if(filePath.is_relative())
	{
		// divi varianti: relatīvs pret tekošo direktoriju, relatīvs pret kādu iespējamu bāzes direktoriju
		// vispirms: ja nesākas ar ./, tad var būt relatīvs pret tekošo direktoriju
		// šo apskata vispirms, jo vajadzības gadījumā ir viegli uzspiest relatīvi pret tekošo pievienojot sākumā ./
		if(!possibleBaseDirAbsolute.empty() && *filePath.begin() != "." && fs::is_regular_file(possibleBaseDirAbsolute / filePath))
		{
			filePathAbsolute = fs::canonical(possibleBaseDirAbsolute / filePath);
			// vai bāzes direktorija pieprasa absolūtu, vai relatīvu ceļu ?
			if(absolute)
				filePath = filePathAbsolute;
			else
				filePath = make_relative(fs::current_path(), filePathAbsolute);
			display = (baseDirDisplay / make_relative(possibleBaseDirAbsolute, filePathAbsolute)).string();
		}
		else
			return false;
		// {
		// 	filePathAbsolute = fs::canonical(filePath);
		// 	filePath = make_relative(fs::current_path(), filePathAbsolute);
		// 	display = (fs::path(".") / filePath).string();
		// }
	}

	return true;
}

bool make_path(fs::path& filePath, const fs::path& possibleBaseDirAbsolute, string& display, const string& baseDirDisplay, bool absolute = false)
{
	if(filePath.empty())
		return false;

	fs::path filePathAbsolute;

	if(filePath.is_absolute())
	{
		display = filePath.string();
	}
	else if(filePath.is_relative())
	{
		// divi varianti: relatīvs pret tekošo direktoriju, relatīvs pret kādu iespējamu bāzes direktoriju
		// vispirms: ja nesākas ar ./, tad var būt relatīvs pret tekošo direktoriju
		// šo apskata vispirms, jo vajadzības gadījumā ir viegli uzspiest relatīvi pret tekošo pievienojot sākumā ./
		if(!possibleBaseDirAbsolute.empty() && *filePath.begin() != ".")
		{
			filePathAbsolute = fs::absolute(possibleBaseDirAbsolute / filePath);
			// vai bāzes direktorija pieprasa absolūtu, vai relatīvu ceļu ?
			if(absolute)
				filePath = filePathAbsolute;
			else
				filePath = make_relative(fs::current_path(), filePathAbsolute);
			display = (baseDirDisplay / make_relative(possibleBaseDirAbsolute, filePathAbsolute)).string();
		}
		else
		{
			filePathAbsolute = fs::absolute(filePath);
			filePath = make_relative(fs::current_path(), filePathAbsolute);
			if(*filePath.begin() != ".")
				display = (fs::path(".") / filePath).string();
			else
				display = filePath.string();
		}
	}

	return true;
}

int main(int argc, char const* argv[])
{
	TrainCase::Arguments defaultArguments;
	// defaultArguments.featureVectorSize = 300000000;
#if USE_MAP == DENSE_HASH_MAP || USE_MAP == SPARSE_HASH_MAP
	defaultArguments.featureVectorSize = 1000000;
#else
	defaultArguments.featureVectorSize = FV_DEFAULT_SIZE;
#endif
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
			("general-tags,g", "use general tags (with wildcards: _)")
			("basedir,b", po::value<string>(), "set basedir in paths basedir/train/..., basedir/golden/... (default: current directory)")
			("version,v", "print version information and exit")
			("quiet,q", "don't print anything (for use with stdin & stdout)")
			("featvec", po::value<string>(), "set feature vector absolute or relative (to confdir) filename (default: features.vec)")
			("idmap", po::value<string>(), "set id map absolute or relative (to confdir) filename (default: id.map)")
			("train", po::value<vector<string>>(), "set train tree file(s) (.conll), can use wildcards, can be used multiple times" \
			 	", relative to basedir, relative to current dir if starts with ./ or absolute path (starting with /)")
			("parse", po::value<vector<string>>(), "set parse tree file(s) (.conll), can use wildcards, can be used multiple times" \
			 	", relative to basedir, relative to current dir if starts with ./ or absolute path (starting with /)")
			("verify", "verify parse files if have dependencies specified")
			("save", "save trained feature vector with id map in files specified by --featvec and --idmap")
			("load", "load feature vector and id map from files specified by --features and --idmap")
			("stdin", "read .conll data from stdin (trees separated by blank line)")
			("output", po::value<string>(), "output file name for parsed trees file")
			("stdout", "output .conll to stdout")
			("confdir,c", po::value<string>(),
			 	"set confdir in path confdir/<feature vector or id map relative filenames> (default: current directory)")
			("ner", "expect input from NER (parent index column contains named entity type from NER), use NER as span boundary heuristics")
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



	vector<int> trainsets;
	string basedir = "";
	string confdir = "";
	string featvec = "features.vec";
	string idmap = "id.map";
	bool quiet = false;
	bool load = false;
	bool save = false;
	bool stdin = false;
	bool stdout = false;
	string output;
	bool verify = false;
	vector<string> train;
	vector<string> parse;

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
		if(vm.count("general-tags"))
		{
			defaultArguments.useGeneralTags = true;
		}
		if(vm.count("basedir"))
		{
			basedir = vm["basedir"].as<string>();
		}
		if(vm.count("quiet"))
		{
			defaultArguments.quiet = quiet = true;
		}
		if(vm.count("confdir"))
		{
			confdir = vm["confdir"].as<string>();
		}

		if(vm.count("ner"))
			defaultArguments.ner = true;

		if(vm.count("load"))
			load = true;
		if(vm.count("save"))
			save = true;
		if(vm.count("verify"))
			verify = true;
		if(vm.count("stdin"))
			stdin = true;
		if(vm.count("stdout"))
			stdout = true;

		if(vm.count("confdir"))
		{
			confdir = vm["confdir"].as<string>();
		}
		if(vm.count("featvec"))
		{
			featvec = vm["featvec"].as<string>();
		}
		if(vm.count("idmap"))
		{
			idmap = vm["idmap"].as<string>();
		}
		if(vm.count("output"))
		{
			output = vm["output"].as<string>();
		}
		if(vm.count("train"))
		{
			train = vm["train"].as<vector<string>>();
		}
		if(vm.count("parse"))
		{
			parse = vm["parse"].as<vector<string>>();
		}
	}
	catch(exception& e)
	{
        cerr << "error: " << e.what() << "\n";
		return 1;
	}

	
	// print version information
	if(!quiet)
	{
		cout << "Collins model 0 parser" << endl;

#ifdef NDEBUG
		cout << "Release version";
#else
		cout << "Debug version";
#endif

		// see: http://clang.llvm.org/docs/LanguageExtensions.html#feature_check
		cout << ", built";
#ifdef __clang__
		cout << " with clang " << __clang_version__;
#endif
		cout << " @ ";
		cout << __DATE__ << " " << __TIME__ << endl;

		cout << "With feature vector map container: ";
#if USE_MAP == STD_MAP
		cout << "std::map";
#elif USE_MAP == STD_UNORDERED_MAP
		cout << "std::unordered_map";
#elif USE_MAP == DENSE_HASH_MAP
		cout << "google::dense_hash_map";
#elif USE_MAP == SPARSE_HASH_MAP
		cout << "google::sparse_hash_map";
#else
		cout << "unknown";
#endif
		cout << endl;


		cout << endl;
	}

	// exit if only version information needed
	if(vm.count("version"))
	{
		return 0;
	}

	if(!quiet)
	{
		// print command line
		cout << "Command Line: ";
		for(int i=0; i<argc; ++i)
		{
			if(i > 0) cout << " ";
			cout << argv[i];
		}
		cout << endl;
	}


	//
	// Direktoriju un failu ceļu ģenerācijas algoritmi:
	//
	// Visiem eksistējošiem ceļiem ir jāiegūst kanoniskā forma, kuru pēc tam var pārvērst relatīvi pret tekošo direktoriju gadījumā,
	// ja sākotnējais ceļš nebija absolūts un arī bāzes direktorijas ceļš nebija absolūts (ja bāze nav tekošā direktorija).
	// Papildus saglabā informāciju, relatīvi pret kuru bāzes direktoriju ceļš ir ģenerēts / pieņemts.
	//
	// basedir:
	// 1. expand (~user)
	// 2. kanoniskā forma
	//
	// confdir:
	// 1. expand
	// 2. ja absolūts (sākas ar /), tad kanoniskā forma
	// 3. sākas ar ./ - pieņem par relatīvu pret tekošo direktoriju -> kanoniskā forma
	// 4. nesākas ne ar ./, ne arī / - relatīvs pret bāzes direktoriju (basedir) -> kanoniskā forma
	//
	// faili featvec un idmap:
	// 1. expand
	// 2. ja absolūts, tad kanoniskā forma
	// 3. sākas ar ./ - relatīvs pret tekošo direktoriju -> kanoniskā forma
	// 4. relatīvs pret bāzes dirketoriju (confdir) -> kanoniskā forma
	// Tā kā confdir var būt relatīva pret basedir, tad arī šeit, ja confdir nav norādīts, ceļš var būt relatīvs pret basedir
	//
	// faili train un parse:
	// 1. expand
	// 2. ja absolūts, tad kanoniskā forma
	// 3. sākas ar ./ - relatīvs pret tekošo direktoriju -> kanoniskā forma
	// 4. relatīvs pret bāzes dirketoriju (basedir) -> kanoniskā forma
	//
	// Piezīme: ievadfaili ir eksistējoši, tādēļ 4 punkta gadījumā tie var neatrasties bāzes direktorijā, bet atrasties relatīvi pret tekošo.
	// 
	
	fs::path currentDirectory = fs::canonical(".");
	fs::path dataDirectory(expand(basedir));
	fs::path dataDirectoryAbsolute = fs::canonical(dataDirectory);
	fs::path configDirectory(expand(confdir));
	fs::path configDirectoryAbsolute;
	fs::path featureVectorFile(expand(featvec));
	fs::path identificatorMapFile(expand(idmap));
	fs::path outputFile(expand(output));
	vector<fs::path> trainFiles;
	vector<fs::path> parseFiles;
	vector<string> trainFilesDisplay;
	vector<string> parseFilesDisplay;
	string dataDirectoryDisplay;
	string configDirectoryDisplay;
	fs::path currentPath = fs::current_path();
	bool configDirectoryRelativeToDataDirectory = false;
	string dataDirectoryPlaceholder = "<basedir>";
	string configDirectoryPlaceholder = "<confdir>";
	string featureVectorFileDisplay;
	string identificatorMapFileDisplay;
	string outputFileDisplay;

	// basedir eksistence
	if(!dataDirectory.empty() && !fs::is_directory(dataDirectory))
	{
		cerr << "Error: invalid basedir: " << dataDirectory.string() << endl;
		return 0;
	}

	// basedir ceļa noteikšana
	if(dataDirectory.empty())
	{
		dataDirectoryDisplay = "";
	}
	else if(dataDirectory.is_absolute())
	{
		dataDirectory = dataDirectoryAbsolute;
		dataDirectoryDisplay = dataDirectory.string();
	}
	else if(dataDirectory.is_relative())
	{
		dataDirectory = dataDirectoryAbsolute;
		dataDirectory = make_relative(currentPath, dataDirectory);
		dataDirectoryDisplay = (fs::path(".") / dataDirectory).string();
	}

	// confdir eksistence
	if(!configDirectory.empty() && !fs::is_directory(configDirectory)
		&& (configDirectory.is_absolute() || *configDirectory.begin() == "." || !fs::is_directory(dataDirectoryAbsolute / configDirectory)))
	{
		cerr << "Error: invalid confdir: " << configDirectory.string() << endl;
		return 0;
	}

	// confdir ceļa noteikšana: absolūtā, pret tekošo direktoriju, pret basedir
	if(configDirectory.empty())
	{
		configDirectoryDisplay = "";
	}
	else if(configDirectory.is_absolute())
	{
		configDirectoryAbsolute = fs::canonical(configDirectory);
		configDirectory = configDirectoryAbsolute;
		configDirectoryDisplay = configDirectory.string();
	}
	else if(configDirectory.is_relative())
	{
		// divi varianti: relatīvs pret tekošo direktoriju, relatīvs pret data direktoriju (basedir)
		// vispirms: ja nesākas ar ./, tad var būt relatīvs pret tekošo direktoriju
		// šo apskata vispirms, jo vajadzības gadījumā ir viegli uzspiest relatīvi pret tekošo pievienojot sākumā ./
		if(!dataDirectory.empty() && *configDirectory.begin() != "." && fs::is_directory(dataDirectory / configDirectory))
		{
			configDirectoryRelativeToDataDirectory = true;
			configDirectoryAbsolute = fs::canonical(dataDirectoryAbsolute / configDirectory);
			configDirectoryDisplay = (fs::path(dataDirectoryPlaceholder) / make_relative(dataDirectoryAbsolute, configDirectoryAbsolute)).string();
			// ja pamata direktorija ir ar absolūto ceļu, tad arī šī direktorija ir ar absolūto ceļu
			// vai arī relatīva pret tekošo direktoriju, ja tāda ir pamata direktorija
			if(dataDirectory.is_absolute())
				configDirectory = configDirectoryAbsolute;
			else
				configDirectory = make_relative(currentPath, configDirectoryAbsolute);
		}
		else
		{
			configDirectoryAbsolute = fs::canonical(configDirectory);
			configDirectory = make_relative(currentPath, configDirectoryAbsolute);
			configDirectoryDisplay = (fs::path(".") / configDirectory).string();
		}
	}
	

	// featvec un idmap noteikšana atkarībā no ielādes vai saglabāšanas
	if(load)
	{
		if(!find_file(featureVectorFile, configDirectoryAbsolute, featureVectorFileDisplay, configDirectoryPlaceholder,
			(dataDirectory.is_absolute() && configDirectoryRelativeToDataDirectory) || configDirectory.is_absolute()))
		{
			if(!find_file(featureVectorFile, dataDirectoryAbsolute, featureVectorFileDisplay,
				dataDirectoryPlaceholder, dataDirectory.is_absolute()))
			{
				if(!find_file(featureVectorFile, currentDirectory, featureVectorFileDisplay, ".", false))
				{
					cerr << "Error: invalid feature vector file: " << featureVectorFile.string() << endl;
					return 0;
				}
			}
		}

		if(!find_file(identificatorMapFile, configDirectoryAbsolute, identificatorMapFileDisplay, configDirectoryPlaceholder,
			(dataDirectory.is_absolute() && configDirectoryRelativeToDataDirectory) || configDirectory.is_absolute()))
		{
			if(!find_file(identificatorMapFile, dataDirectoryAbsolute, identificatorMapFileDisplay,
				dataDirectoryPlaceholder, dataDirectory.is_absolute()))
			{
				if(!find_file(identificatorMapFile, currentDirectory, identificatorMapFileDisplay, ".", false))
				{
					cerr << "Error: invalid identificator map file: " << identificatorMapFile.string() << endl;
					return 0;
				}
			}
		}
	}
	else if(save)
	{
		if((!configDirectoryAbsolute.empty()
			&& !make_path(featureVectorFile, configDirectoryAbsolute, featureVectorFileDisplay, configDirectoryPlaceholder,
				(dataDirectory.is_absolute() && configDirectoryRelativeToDataDirectory) || configDirectory.is_absolute()))
			|| (configDirectoryAbsolute.empty()
			&& !make_path(featureVectorFile, dataDirectory.empty() ? dataDirectory : dataDirectoryAbsolute, featureVectorFileDisplay,
				dataDirectoryPlaceholder, dataDirectory.is_absolute())))
		{
			cerr << "Error: invalid feature vector file: " << featureVectorFile.string() << endl;
			return 0;
		}

		if((!configDirectoryAbsolute.empty()
			&& !make_path(identificatorMapFile, configDirectoryAbsolute, identificatorMapFileDisplay, configDirectoryPlaceholder,
				(dataDirectory.is_absolute() && configDirectoryRelativeToDataDirectory) || configDirectory.is_absolute()))
			|| (configDirectoryAbsolute.empty()
			&& !make_path(identificatorMapFile, dataDirectory.empty() ? dataDirectory : dataDirectoryAbsolute, identificatorMapFileDisplay,
				dataDirectoryPlaceholder, dataDirectory.is_absolute())))
		{
			cerr << "Error: invalid identificator map file: " << identificatorMapFile.string() << endl;
			return 0;
		}
	}

	if(!output.empty())
	{
		if(!make_path(outputFile, dataDirectory.empty() ? dataDirectory : dataDirectoryAbsolute, outputFileDisplay,
				dataDirectoryPlaceholder, dataDirectory.is_absolute()))
		{
			cerr << "Error: invalid ouput file: " << outputFile.string() << endl;
			return 0;
		}
	}

	for(const string& filename : train)
	{
		trainFiles.emplace_back(expand(filename));
		trainFilesDisplay.emplace_back();
		fs::path &file = trainFiles.back();
		string &display = trainFilesDisplay.back();
		if(!find_file(file, dataDirectoryAbsolute, display,
			dataDirectoryPlaceholder, dataDirectory.is_absolute()))
		{
			cerr << "Error: invalid train file: " << file.string() << endl;
			return 0;
		}
	}

	for(const string& filename : parse)
	{
		parseFiles.emplace_back(expand(filename));
		parseFilesDisplay.emplace_back();
		fs::path &file = parseFiles.back();
		string &display = parseFilesDisplay.back();
		if(!find_file(file, dataDirectoryAbsolute, display,
			dataDirectoryPlaceholder, dataDirectory.is_absolute()))
		{
			cerr << "Error: invalid parse file: " << file.string() << endl;
			return 0;
		}
	}

	if(!quiet)
	{
		cout << endl;
		cout << "Configured File Paths:" << endl;
		cout << "curdir = " << currentPath.string() << endl;
		cout << "basedir = " << dataDirectoryDisplay << endl;
		cout << "confdir = " << configDirectoryDisplay << endl;
		cout << "featvec = " << featureVectorFileDisplay << endl;
		cout << "idmap = " << identificatorMapFileDisplay << endl;
		cout << "output = " << outputFileDisplay << endl;
		if(parseFiles.size() == 1)
			cout << "parse = " << parseFilesDisplay.back() << endl;
		else if(parseFiles.size() > 1)
		{
			cout << "Parse files:" << endl;
			for(const string& fn : parseFilesDisplay)
				cout << fn << endl;
		}
		if(trainFiles.size() == 1)
			cout << "train = " << trainFilesDisplay.back() << endl;
		else if(trainFiles.size() > 1)
		{
			cout << "Train files:" << endl;
			for(const string& fn : trainFilesDisplay)
				cout << fn << endl;
		}
		cout << endl;
	}

	// legacy algorithm - pagaidām neaiztikt
	if(!basedir.empty() && basedir.back() != '/')
		basedir += '/';

	// ja ir tukšs, tad piepilda
	if(trainsets.size() == 0)
	{
		for(int i=0; i<10; i++)
			trainsets.push_back(i);
	}
	else
		sort(trainsets.begin(), trainsets.end());

	if(!quiet)
	{
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

	}
	



	// defaultArguments.featureVectorSize = 300000000;
	// defaultArguments.iterations = 5;
	// defaultArguments.limit = 0;
	// defaultArguments.limit = 20;
	// defaultArguments.allowNonProjective = true;


	if(!(load || save || stdin || stdout || train.size() > 0 || parse.size() > 0))
	{
		if(!quiet)
		{
			cout << endl << "Will use trainsets!" << endl << endl;
			cout << "Trainsets:" << endl;
		}

		vector<string> trainCoNLL, checkCoNLL;
		char buf[1024];
		for(int i : trainsets)
		{
			sprintf(buf, "%strain/train%i.conll", basedir.c_str(), i);
			trainCoNLL.push_back(buf);
			sprintf(buf, "%sgolden/golden%i.conll", basedir.c_str(), i);
			checkCoNLL.push_back(buf);
			if(!quiet)
				cout << trainCoNLL[trainCoNLL.size()-1] << ", " << checkCoNLL[checkCoNLL.size()-1] << endl;
		}

		if(!quiet)
		{
			cout << endl;
			cout << "Starting..." << endl;
			cout << endl;
		}

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
			// {
			// 	string name = "out/check";
			// 	name += to_string(i);
			// 	ofstream file(name);

			// 	file << trainCases.last().arguments.checkTrees();
			// }
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
		
		if(!quiet)
		{
			cout << endl;

			trainCases.summary();
			
			cout << "Total run time: ";
			outputDuration(timing.stop());
			cout << endl;

			cout << endl;
		}

		return 0;
	}






	

	//
	// Opcijas:
	// --train - treniņa failu ievade no failiem (jābūt derīgiem CoNLL kokiem)
	// --save - treniņa rezultātu saglabāšana failos features.vec un id.map
	// --load - treniņa rezultāte ielāde no failiem features.vec un id.map
	// --parse - parsējamo teikumu (vēl neeksistējošu CoNLL koku) ievade no failiem
	// --stdin - parsējamo teikumu (vēl neeksistējošu CoNLL koku) ievade caur stdin
	// --output - noparsēto parse failu rezultējošo CoNLL koku izvade failā
	// --stdout - noparsēto parse failu rezultējošo CoNLL koku izvade stdout
	// --verify	- vai veikt verifikāciju parse failiem, ja tie jau satur head dep. saites (izvads stdout)
	//
	// pašlaik --output un --stdout viens otru izslēdz
	// kā arī --parse un --stdin viens otru izslēdz
	//
	// Tātad trainCase ir jābūt spējīgam:
	// - ielasīt izvēlētus failus treniņiem
	// - ielasīt izvēlētus failus parsēšanai, kā arī piedāvāt iespēju no stdin
	// - izvadīt parse rezultātus failā vai stdout
	// - iespēja ielādēt iezīmju vektoru
	// - iespēja saglabāt iezīmju vektoru
	// - iespēja veikt standarta verifikāciju (pašlaik nebūtisks)
	//
	// Kuros gadījumos izmantos trainset, kuros --train un --parse ?
	// Kādas ir atļautās parametru kombinācijas vai nosacījumi ?
	//
	// Ja ir uzdots kaut viens no parametriem: --train, --save, --load, --parse, --stdin, --stdout, --output, tad trainset režīms ir atslēgts,
	// tiek izmantots jaunais režīms.
	// Trainset režīms ir iezīmju kvalitātes novērtēšanai, nevis production sistēmai.
	//
	// Lai būtu iespējams kaut ko noparsēt, vajag treniņinformāciju:
	// ir jābūt vienam no: --train (var būt vairāki) vai --load (komplektā ar eksistējošiem --featvec un --idmap failiem)
	//
	// Treniņu rezultātus var saglabāt ar --save (tikai kopā ar --train)
	// 
	// Parsējamās informācijas ievads:
	// --parse (var būt vairāk kā viens) vai --stdin
	//
	// Parsējamās informācijas izvads:
	// --output vai --stdout
	// 
	

	IndexMap idMap;
	FeatureVector featureVector(0);

	if(load)
	{
		if(!quiet)
		{
			cout << "Loading identificator map: " << identificatorMapFileDisplay << " ... ";
			cout.flush();
		}
		bool success;
		success = idMap.load(identificatorMapFile.string());
		if(!quiet)
			cout << (success ? "ok" : "fail") << endl;
		if(!quiet)
		{
			cout << "Loading feature vector: " << featureVectorFileDisplay << " ... ";
			cout.flush();
		}
		success = featureVector.load(featureVectorFile.string());
		if(!quiet)
			cout << (success ? "ok" : "fail") << endl;
	}
	else
	{
		if(!quiet)
		{
			cout << "Allocating memory for feature vector of size " << defaultArguments.featureVectorSize() << " ... ";
			cout.flush();
		}
		featureVector.reserve(defaultArguments.featureVectorSize);
		if(!quiet)
			cout << "ok" << endl;
	}

	// ir ar ko trenēt?
	if(trainFiles.size() > 0)
	{
		filestreams streams(trainFiles);

		::train(defaultArguments, featureVector, idMap, streams);
	}

	if(!load && save)
	{
		if(!quiet)
		{
			cout << "Saving identificator map: " << identificatorMapFileDisplay << " ... ";
			cout.flush();
		}
		bool success;
		success = idMap.save(identificatorMapFile.string());
		if(!quiet)
			cout << (success ? "ok" : "fail") << endl;
		if(!quiet)
		{
			cout << "Verifying identificator map: " << identificatorMapFileDisplay << " ... ";
			cout.flush();
		}
		success = idMap.verify(identificatorMapFile.string());
		if(!quiet)
			cout << (success ? "ok" : "fail") << endl;
		if(!quiet)
		{
			cout << "Saving feature vector: " << featureVectorFileDisplay << " ... ";
			cout.flush();
		}
		success = featureVector.save(featureVectorFile.string());
		if(!quiet)
			cout << (success ? "ok" : "fail") << endl;
		if(!quiet)
		{
			cout << "Verifying feature vector: " << featureVectorFileDisplay << " ... ";
			cout.flush();
		}
		success = featureVector.verify(featureVectorFile.string());
		if(!quiet)
			cout << (success ? "ok" : "fail") << endl;
	}

	if(parseFiles.size() > 0 || stdin)
	{
		if(!stdin && parseFiles.size() > 0 && verify)
		{
			filestreams streams(parseFiles);

			::verify(defaultArguments, featureVector, idMap, streams);
		}
		else
		{
			// message: we are ready
			
			cerr << "ready" << endl;
			
			ofstream file(outputFile.string());
			ostream cnull(0);
			ostream& out = file ? file : (stdout ? cout : cnull);

			if(stdin)
			{
				cinstream streams;

				::parse(defaultArguments, featureVector, idMap, streams, out);
			}
			else
			{
				filestreams streams(parseFiles);

				::parse(defaultArguments, featureVector, idMap, streams, out);
			}
		}
	}

	return 0;


	// IndexMap idMap;
	// idMap.load("id.map");
	// idMap.print();
	// cout << idMap.size() << endl;
	// FeatureVector featureVector(0);
	featureVector.load("features.vec");
	// featureVector.print();
	cout << featureVector.size() << endl;
	// return 0;
	Trees trees;
	// readDirectory(idMap, trees, "all");
	// cout << trees[2];
	cout << trees;
	cin >> trees;
	cout << trees;
	// trees.idMap.print();
	return 0;

	TrainCase tc(TrainCase::Arguments(defaultArguments).setIDMap(idMap).setTrainTrees(trees));
	idMap.save("id.map");
	idMap.print();
	cout << idMap.size() << endl;
	// tc.featureVector.print();
	tc.featureVector.save("features.vec");


	return 0;
}

