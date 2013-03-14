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

#include "features.hpp"

using namespace std;


void Feature::print() const
{
	print_raw();

	// dekodēšana
	if(data[0] == TTSNL)
	{
		cout << "SIZE = " << size();
		cout << ", ";
		cout << "T1 = " << (unsigned int)((data[1] >> 6) & 0x3);
		cout << ", ";
		cout << "T2 = " << (unsigned int)((data[1] >> 4) & 0x3);
		cout << ", ";
		cout << "S = " << (unsigned int)(data[1] & 0xF);
		cout << ", ";
		cout << "N = " << (unsigned int)data[2];
		cout << ", ";
		cout << "L = " << (unsigned int)*((int16_t*)&data[3]);
		cout << endl;
	}
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
	for(size_t i=0, bucket_count=_map.bucket_count(); i<bucket_count; ++i)
	{
		if(_map.bucket_size(i) > 1)
			collisions += _map.bucket_size(i) - 1;
	}
	return collisions;
}





