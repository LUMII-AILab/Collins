/*
 * =====================================================================================
 *
 *       Filename:  features.cpp
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

#include <iostream>
#include <iomanip>
#include <fstream>

#include <boost/algorithm/string.hpp>

#include "features.hpp"

using namespace std;


void Feature::print() const
{
	print_raw();

	// NOTE: šeit bija domāta dekodēšana, kas vairs nav aktuāla/iespējama tekošajā variantā
}

void Feature::print_raw() const
{
	cout << "RAW(" << size() << "): ";
	cout << std::setprecision(2) << std::setfill('0') << std::uppercase << std::hex;
	// for(byte b : data)
	// 	cout << setw(2) << (int)b << " ";
	cout << setw(2) << (int)data[0] << " | ";
	// for(int i=0, sz=size(); i<sz; ++i)
	for(int i=1, sz=size(); i<sz; ++i)
		cout << setw(2) << (int)data[i] << " ";
	cout << std::nouppercase << std::dec << std::setfill(' ');
	cout << endl;
}

void Feature::print_raw_full() const
{
	cout << "RAW(" << size() << "): ";
	cout << std::setprecision(2) << std::setfill('0') << std::uppercase << std::hex;
	for(byte b : data)
		cout << setw(2) << (int)b << " ";
	// cout << setw(2) << (int)data[0] << " | ";
	// // for(int i=0, sz=size(); i<sz; ++i)
	// for(int i=1, sz=size(); i<sz; ++i)
	// 	cout << setw(2) << (int)data[i] << " ";
	cout << std::nouppercase << std::dec << std::setfill(' ');
	cout << endl;
}

void Feature::overflow_assert() const
{
	cout << "Feature overflow!!!" << endl;
}


//
// Primitīvs FeatureVector izvads
//
void FeatureVector::print() const
{
	// for(Map::const_iterator it=features.begin(); it!=features.end(); ++it)
	// {
	// 	cout << "#" << it->second << " " << it->first << " = " << weights[it->second] << endl;
	// }
	for(Index i=0, size=_features.size(); i<size; ++i)
	{
		// cout << "#" << i << " " << _features[i].key() << " = " << _weights[i] << endl;
		cout << _weights[i] << " | ";
		_features[i].print();
	}
}


//
// Uzstāda nulles svarus visiem elementiem.
//
void FeatureVector::zero()
{
	fill(_weights.begin(), _weights.end(), 0);
}


//
// Kolīzijas
//
FeatureVector::Index FeatureVector::collisions() const
{
	Index collisions = 0;
#if USE_MAP != STD_MAP
	// Index sum = 0, count = 0;
	for(size_t i=0, bucket_count=_map.bucket_count(); i<bucket_count; ++i)
	{
		if(_map.bucket_size(i) > 1)
			collisions += _map.bucket_size(i) - 1;
		// if(_map.bucket_size(i) > 1)
		// {
		// 	sum += _map.bucket_size(i);
		// 	count++;
		// }
	}
	// if(count > 0)
	// 	cout << "!!! Average bucket size: " << (double)sum/(double)count << endl;
#endif
	return collisions;
}


bool FeatureVector::save(const std::string& filename) const
{
	fstream file(filename, ios::out | ios::trunc);
	
	if(!file)
		return false;

	int count = 0;
	for(int i=0, size=_features.size(); i<size; ++i)
		if(_weights[i] != 0)
			count++;
	// int count  = size();

	file << count << "\t" << FEATURE_MAX_SIZE << endl;

	for(int i=0, size=_features.size(); i<size; ++i)
	{
		// ****
		// if(_map[_features[i]] != i)
		// 	cout << "FV SAVE: mapping error !!!" << endl;

		if(_weights[i] != 0)
		{
			file << _weights[i];
			const Feature& feature = _features[i];
			for(int j=0, sz=feature.size(); j<sz; ++j)
				file << "\t" << (int)feature[j];
			file << endl;
		}
	}

	return true;
}

bool FeatureVector::load(const std::string& filename)
{
	fstream file(filename, ios::in);

	if(!file)
		return false;

	typedef boost::split_iterator<string::iterator> SplitIterator;

	string line;
	Value weight;

	getline(file, line);
	size_t next = 0;
	int count = stoi(line, &next);
	int maxSize = stoi(line.substr(next));

	if(maxSize > FEATURE_MAX_SIZE)
	{
		cerr << "ERROR!!! Saved feature vector (file: " << filename
			<< ") has greater feature max size (" << maxSize << ") than current system can handle (" << FEATURE_MAX_SIZE << ") !" << endl;
		return false;
	}

	if(count + _features.size() < min(_features.capacity(), _weights.capacity()))
		reserve(count + _features.size());

	while(getline(file, line))
	{
		SplitIterator part = make_split_iterator(line, boost::first_finder("\t", boost::is_equal()));

		if(_features.size() != _weights.size())
		{
			cerr << "WARNING!!! feature vector and weight vectore have different sizes!" << endl;
			return false;
		}

		/*
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
			*/

		weight = stoi(boost::copy_range<string>(*part++));
		int size = stoi(boost::copy_range<string>(*part++));

		int index = _features.size();
		_weights.push_back(weight);
		_features.emplace_back();
		if(_weights.size() != _features.size())
			cout << "!!! ERROR: weight and features size mismatch" << endl;
		Feature& feature = _features.back();

		// cout << "SZ("<<size<<")";
		for(int i=1; i<size; ++i)
		{
			unsigned char b = stoi(boost::copy_range<string>(*part++));
			feature += b;
			// cout << " " << (int)b;

		}
			// feature += stoi(boost::copy_range<string>(*part++));

		// _features.back().print();
	
		_map[feature] = index;
	}

	return true;
}

bool FeatureVector::verify(const std::string& filename)
{
	fstream file(filename, ios::in);

	if(!file)
		return false;

	typedef boost::split_iterator<string::iterator> SplitIterator;

	string line;
	Value weight;

	getline(file, line);
	size_t next = 0;
	int count = stoi(line, &next);
	int maxSize = stoi(line.substr(next));

	int expectedCount = 0;
	for(int i=0, size=_features.size(); i<size; ++i)
		if(_weights[i] != 0)
			expectedCount++;

	if(count != expectedCount)
		cout << "VERIFY FAIL: sizes differ" << endl;

	if(maxSize > FEATURE_MAX_SIZE)
	{
		cerr << "ERROR!!! Saved feature vector (file: " << filename
			<< ") has greater feature max size (" << maxSize << ") than current system can handle (" << FEATURE_MAX_SIZE << ") !" << endl;
		return false;
	}

	int k = 0;
	while(getline(file, line))
	{
		SplitIterator part = make_split_iterator(line, boost::first_finder("\t", boost::is_equal()));

		weight = stoi(boost::copy_range<string>(*part++));
		int size = stoi(boost::copy_range<string>(*part++));

		while(k < _weights.size() && _weights[k] == 0) k++;

		Feature& feature = _features[k];
		if(feature[0] != size)
			cout << "FV VERIFY: feature size mismatch !" << endl;

		// cout << "SZ("<<size<<")";
		for(int i=1; i<size; ++i)
		{
			unsigned char b = stoi(boost::copy_range<string>(*part++));

			// cout << " " << (int)b;

			if(feature[i] != b)
				cout << "FV VERIFY: feature byte mismatch !" << endl;
		}
			// feature += stoi(boost::copy_range<string>(*part++));

		// _features.back().print();
	
		k++;
	}

	return true;
}


bool IndexMap::load(const string& filename)
{
	fstream file(filename, ios::in);

	if(!file)
		return false;

	typedef boost::split_iterator<string::iterator> SplitIterator;

	string line;
	string str;
	ID id;
	int size = 0;

	getline(file, line);
	size = stoi(line);
	// TODO: reserve memory ?

	Map::const_iterator it;
	while(getline(file, line))
	{
		SplitIterator part = make_split_iterator(line, boost::first_finder("\t", boost::is_equal()));
		str = boost::copy_range<string>(*part++);
		id = stoi(boost::copy_range<string>(*part++));

		if(primary)
		{
			it = primary->map.find(str);
			if(it != primary->map.end())
				continue;
		}

		// it = map.find(str);
		// if(it != map.end())
		// 	cout << "WARNING: string in string2id map already exists!" << endl;

		map[str] = id;
	}

	next = map.size();

	return true;
}

bool IndexMap::verify(const string& filename)
{
	fstream file(filename, ios::in);

	if(!file)
		return false;

	typedef boost::split_iterator<string::iterator> SplitIterator;

	string line;
	string str;
	ID id;
	int size = 0;

	getline(file, line);
	size = stoi(line);
	// TODO: reserve memory ?

	if(size != map.size())
		cout << "IDMAP VERIFY: size mismatch" << endl;

	Map::const_iterator it;
	while(getline(file, line))
	{
		SplitIterator part = make_split_iterator(line, boost::first_finder("\t", boost::is_equal()));
		str = boost::copy_range<string>(*part++);
		id = stoi(boost::copy_range<string>(*part++));

		// if(primary)
		// {
		// 	it = primary->map.find(str);
		// 	if(it != primary->map.end())
		// 		continue;
		// }

		// it = map.find(str);
		// if(it != map.end())
		// 	cout << "WARNING: string in string2id map already exists!" << endl;

		if(map[str] != id)
			cout << "IDMAP VERIFY: id mismatch !!!" << endl;
		// map[str] = id;
	}

	if(next != map.size())
		cout << "IDMAP VERIFY: next mismatch" << endl;

	return true;
}


bool IndexMap::save(const string& filename, bool excludePrimary) const
{
	fstream file(filename, ios::out | ios::trunc);
	
	if(!file)
		return false;


	if(!excludePrimary && primary)
	{
		file << primary->map.size() + map.size() << endl;
		for(Map::const_iterator it=primary->map.cbegin(), end=primary->map.cend(); it!=end; ++it)
			file << it->first << "\t" << it->second << endl;
	}
	else
		file << map.size() << endl;

	for(Map::const_iterator it=map.cbegin(), end=map.cend(); it!=end; ++it)
		file << it->first << "\t" << it->second << endl;

	return true;
}

void IndexMap::print() const
{
	for(Map::const_iterator it=map.cbegin(), end=map.cend(); it!=end; ++it)
		cout << it->first << "\t" << it->second << endl;
}





