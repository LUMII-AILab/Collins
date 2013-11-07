/*
 * =====================================================================================
 *
 *       Filename:  train.hpp
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

#ifndef __TRAIN_HPP__
#define __TRAIN_HPP__

#include <functional>
#include <memory>
#include <iostream>
#include <list>
#include <fstream>
#include <string>

#include <boost/filesystem.hpp>

#include "collins0.hpp"




// Mērķis ir izveidot bāzes klasi, kura spēj kombinēt straumes tā, lai tās būtu secīgi viena pēc otras
// Izdarīt to pilnīgi caurspīdīgi būs sarežģīti (tā šķiet), tāpēc var izveidot kaut ko līdzīgu next()
//
// Pielietojums:
//
// while(streams::stream& stream = streams.next())
// {
// 		// darbības ar stream objektu šeit (līdz iztērē)
// }
//
// abstrakta bāzes klase
class streams
{
public:
	typedef std::basic_istream<char> stream;
	streams() : cnull(0) {}
	virtual stream& next() = 0;
protected:
	std::istream cnull;		// pēdējais next() izvads - apzīmē kā beigas
};

// atvasinātā klase cin straumei
class cinstream : public streams
{
public:
	cinstream() { done = false; }
	stream& next() { return !done ? std::cin : cnull; }
private:
	bool done;
};

// atvasinātā klase ievadfailu straumei
class filestreams : public streams
{
public:
	filestreams(const std::vector<boost::filesystem::path> paths) { current = nullptr; for(auto& path : paths) files.emplace_back(path.string()); }
	stream& next() 
	{
		if(current)
		{
			delete current;
			current = nullptr;
		}

		if(files.empty())
			return cnull;

		current = new std::ifstream(files.front());
		files.pop_front();

		return *current;
	}
private:
	std::ifstream* current;
	std::list<std::string> files;
};




// 
// Funkcija, kas ielasa kokus no CoLL faila
//
void readFile(IndexMap& idMap, Trees& trees, const std::string& path, bool useGeneralTags = false);


//
// ConfigValue - šabolons konfigurācijas vērtībām
//
template <typename T, typename R=const T>
class ConfigValue
{
public:
	ConfigValue() { rval = NULL; }
	ConfigValue(R& r) { rval = &rval; }
	// ConfigValue(const T& value) { rval = NULL; _value = value; if(onChange) onChange(); }
	const ConfigValue<T,R>& operator=(const ConfigValue<T,R>& s) { _value = s._value; return *this; }
	R& operator=(const T& value) { _value = value; if(onChange) onChange(); if(rval) return *rval; return value; }
	R& set(const T& value) { _value = value; if(onChange) onChange(); if(rval) return *rval; return value; }
	operator const T&() const { return _value; }
	operator T&() { return _value; }
	const T& value() const { return _value; }
	T& operator ()() { return _value; }
	const T& operator ()() const { return _value; }
	T& value() { return _value; }
	std::function<void ()> onChange;
private:
	T _value;
	R* rval;
};


//
// ConfigValueByPtr - šabolons konfigurācijas vērtībām (glabā kā pointeri)
//
template <typename T, typename R=T>
class ConfigValueByPtr
{
public:
	ConfigValueByPtr() { _value = NULL; rval = NULL; }
	ConfigValueByPtr(R& r) { rval = &rval; }
	// ConfigValueByPtr(T& value) { _value = &value; if(onChange) onChange(); }
	const ConfigValueByPtr<T,R>& operator=(const ConfigValueByPtr<T,R>& s) { _value = s._value; return *this; }
	T& operator=(T& value) { _value = &value; if(onChange) onChange(); if(rval) return *rval; return value; }
	// TODO: throw exception
	operator const T&() const { return *_value; }
	operator T&() { return *_value; }
	const T& value() const { return _value; }
	T& value() { return *_value; }
	T& operator ()() { return *_value; }
	const T& operator ()() const { return *_value; }
	operator bool() const { return _value != NULL; }
	bool valid() const { return _value != NULL; }
	std::function<void ()> onChange;
private:
	T* _value;
	R* rval;
};




//
// TrainCase - viens treniņa notikums
//
class TrainCases;
class TrainCase
{
public:

	// Argumenti treniņam
	class Arguments
	{
	public:

		Arguments() {
			setupCallbacks();
			// noklusēti būs maksimālā kopa
			limit = 0;
			allowNonProjective = true;
			iterations = 5;
			trainStart = 0;
			trainStop = 0;
			checkStart = 0;
			checkStop = 0;
			featureVectorSize = 300000000;
			seed = 0;
			permutate = true;
			useGeneralTags = false;
			quiet = false;
			ner = false;
		}

		Arguments(const Arguments& arguments) {
			trainCoNLL = arguments.trainCoNLL;
			checkCoNLL = arguments.checkCoNLL;
			setupCallbacks();
			trainLimit = arguments.trainLimit;
			checkLimit = arguments.checkLimit;
			trainStart = arguments.trainStart;
			checkStart = arguments.checkStart;
			trainStop = arguments.trainStop;
			checkStop = arguments.checkStop;
			iterations = arguments.iterations;
			allowTrainNonProjective = arguments.allowTrainNonProjective;
			allowCheckNonProjective = arguments.allowCheckNonProjective;
			featureVectorSize = arguments.featureVectorSize;
			seed = arguments.seed;
			permutate = arguments.permutate;
			useGeneralTags = arguments.useGeneralTags;
			quiet = arguments.quiet;
			ner = arguments.ner;

			trainTrees = arguments.trainTrees;
			checkTrees = arguments.checkTrees;
			idMap = arguments.idMap;
			_trainTrees = arguments._trainTrees;
			_checkTrees = arguments._checkTrees;
			_idMap = arguments._idMap;
		}

		Arguments& setLimit(int value) { limit = value; return *this; }
		Arguments& setTrainLimit(int value) { trainLimit = value; return *this; }
		Arguments& setCheckLimit(int value) { checkLimit = value; return *this; }
		Arguments& setIterations(int value) { iterations = value; return *this; }
		Arguments& setTrainCoNLL(std::string filename) { trainCoNLL = filename; return *this; }
		Arguments& setCheckCoNLL(std::string filename) { checkCoNLL = filename; return *this; }
		Arguments& setTrainTrees(Trees& value) { trainTrees = value; return *this; }
		Arguments& setCheckTrees(Trees& value) { checkTrees = value; return *this; }
		Arguments& setTrainStart(int value) { trainStart = value; return *this; }
		Arguments& setTrainStop(int value) { trainStart = value; return *this; }
		Arguments& setCheckStart(int value) { checkStart = value; return *this; }
		Arguments& setCheckStop(int value) { checkStart = value; return *this; }
		Arguments& setFeatureVecorSize(int value) { featureVectorSize = value; return *this; }
		Arguments& setSeed(int value) { seed = value; return *this; }
		Arguments& setPermutate(bool value) { permutate = value; return *this; }
		Arguments& setUseGeneralTags(bool value) { useGeneralTags = value; return *this; }
		Arguments& setIDMap(IndexMap& value) { idMap = value; return *this; }
		Arguments& setQuiet(bool value) { quiet = value; return *this; }
		Arguments& setNER(bool value) { ner = value; return *this; }

		ConfigValue<int> featureVectorSize;
		ConfigValue<int> trainStart;
		ConfigValue<int> trainStop;
		ConfigValue<int> checkStart;
		ConfigValue<int> checkStop;
		ConfigValue<int> iterations;
		ConfigValue<int> limit;
		ConfigValue<int> trainLimit;
		ConfigValue<int> checkLimit;
		ConfigValue<bool> allowNonProjective;
		ConfigValue<bool> allowTrainNonProjective;
		ConfigValue<bool> allowCheckNonProjective;
		ConfigValue<bool> quiet;
		ConfigValue<std::string> trainCoNLL;
		ConfigValue<std::string> checkCoNLL;
		ConfigValueByPtr<Trees> trainTrees;
		ConfigValueByPtr<Trees> checkTrees;
		ConfigValue<int> seed;
		ConfigValue<bool> permutate;
		ConfigValue<bool> useGeneralTags;
		ConfigValueByPtr<IndexMap> idMap;
		ConfigValue<bool> ner;

		IndexMap& getIDMap()
		{
			if(idMap.valid())
				return idMap;
			if(!_idMap.get())
				_idMap.reset(new IndexMap());
			return *_idMap.get();
		}

	private:
		
		void setupCallbacks()
		{
			limit.onChange = [this]() { trainLimit = limit; checkLimit = limit; };
			allowNonProjective.onChange = [this]() { allowTrainNonProjective = allowNonProjective; allowCheckNonProjective = allowNonProjective; };
			trainCoNLL.onChange = [this]() {
				_trainTrees.reset(new Trees());
				trainTrees = *_trainTrees;
				readFile(getIDMap(), trainTrees, trainCoNLL, useGeneralTags());
			};
			checkCoNLL.onChange = [this]() {
				_checkTrees.reset(new Trees());
				checkTrees = *_checkTrees;
				readFile(getIDMap(), checkTrees, checkCoNLL, useGeneralTags());
			};
		}

		std::shared_ptr<Trees> _trainTrees;
		std::shared_ptr<Trees> _checkTrees;
		std::shared_ptr<IndexMap> _idMap;
	};
	
	template <typename... OtherArguments>
	TrainCase(const Arguments& args, const OtherArguments&... otherArgs) : arguments(args), featureVector(0) { run(); }

	FeatureVector featureVector;
	
private:

	class CheckResult
	{
	public:

		CheckResult(int index, int matched, int size, double duration) { _index = index; _matched = matched; _size = size; _duration = duration; }

		int index() const { return _index; }
		int matched() const { return _matched; }
		int size() const { return _size; }
		double duratoin() const { return _duration; }

	private:
		int _index;
		int _matched;
		int _size;
		double _duration;
	};

	void run();
	void train(FeatureVector& featureVector);
	void check(const FeatureVector& featureVector);

	Arguments arguments;
	// FeatureVector featureVector;

	int collisions;
	int trainTreeCount;
	int checkTreeCount;
	int checkUASTotalMatches;
	int checkUASTotalCount;
	
	// rezultātu glabātuve
	double trainTime;
	double checkTime;	

	std::vector<CheckResult> checkResults;

	friend class TrainCases;
};



//
// Treniņu gadījumu apvienojums
//
class TrainCases
{
public:
	TrainCases(int reserve = 100) { trainCases.reserve(reserve); }

	template <typename... OtherArguments>
	void operator()(const TrainCase::Arguments& args, const OtherArguments&... otherArgs) { trainCases.emplace_back(args); } 
	const TrainCase& last() const { return trainCases.back(); }

	void summary();	

private:

	std::vector<TrainCase> trainCases;
};


bool train(TrainCase::Arguments& arguments, FeatureVector& featureVector, IndexMap& idMap, streams& istreams);
bool verify(TrainCase::Arguments& arguments, const FeatureVector& featureVector, const IndexMap& idMap, streams& istreams);
bool parse(TrainCase::Arguments& arguments, const FeatureVector& featureVector, const IndexMap& idMap,
		streams& istreams, std::basic_ostream<char>& ostream);


#endif // __TRAIN_HPP__
