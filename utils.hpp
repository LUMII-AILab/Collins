/*
 * =====================================================================================
 *
 *       Filename:  utils.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013-01-31 17:16:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Didzis Gosko (dg), didzis@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __UTILS_HPP__
#define __UTILS_HPP__


#include <string>
#include <ctime>


//
// Klase ļauj aprēķināt izmantot laiku
//
class Timing
{
public:

	void start() { _duration = 0; _stop = _start = clock(); }
	double stop() { _stop = clock(); _duration = (double)(_stop-_start)/(double)CLOCKS_PER_SEC; return _duration; }
	double duration() const { return _duration; }
	operator double() const { return _duration; }

private:
	clock_t _start;
	clock_t _stop;
	double _duration;
};



void outputDuration(double duration);



// 
// Standarta ANSI termināļos ļauj attīt atpakaļ izvadi.
// Skat: http://ascii-table.com/ansi-escape-sequences.php
//
class RollbackOutput
{
public:

	RollbackOutput() { rollbackSize = 0; fillSymbol = ' '; }
	RollbackOutput(char fillSymbol) { rollbackSize = 0; this->fillSymbol = fillSymbol; }
	~RollbackOutput() { rollback(); }

	const std::string& operator+=(const std::string& s) { append(s); return s; }
	const std::string& operator=(const std::string& s) { replace(s); return s; }

	void append(const std::string& s);
	void replace(const std::string& s);

	void rollback();

	char fillSymbol;

private:

	int rollbackSize;
};

class ProgressIndicator
{
public:

	ProgressIndicator(int total, int start = 0)
	{ _total = total == 0 ? 1 : total; _current = start; _prevDisplay = 0; }

	int operator=(int value) { update(value); return value; }

	void display();
	void clear();

	std::string postfix;

private:

	const double displayUnit = 0.001;
	
	void update(int value)
	{
		_current = value;
		if((double)(_current - _prevDisplay)/_total >= displayUnit)
			display();
	}

	RollbackOutput output;

	double _total;
	int _current;
	int _prevDisplay;
};


#endif // __UTILS_HPP__
