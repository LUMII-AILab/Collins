/*
 * =====================================================================================
 *
 *       Filename:  utils.cpp
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

#include <iostream>
#include <iomanip>
#include <cstdlib>

#include "utils.hpp"

using namespace std;



void outputDuration(double duration)
{
	cout << setprecision(7) << defaultfloat << duration << " s";
	if(duration > 60)
	{
		cout << fixed;
		if(duration < 600)
			cout << setprecision(2);
		else
			cout << setprecision(1);
		cout << " = " << duration/60.0 << " min";
	}
}




void RollbackOutput::append(const string& s)
{
	cout << s;
	cout.flush();
	rollbackSize += s.size();
}

void RollbackOutput::replace(const string& s)
{
	rollback();
	cout << s;
	cout.flush();
	rollbackSize = s.size();
}

void RollbackOutput::rollback()
{
	if(rollbackSize > 0)
	{
		if(fillSymbol)
		{
			// rollback for fill
			cout << "\033[" << rollbackSize << "D";
			// remove (fill with fillSymbol)
			cout << setfill(fillSymbol) << setw(rollbackSize) << " ";
		}
		// rollback
		cout << "\033[" << rollbackSize << "D";
		rollbackSize = 0;
	}
}


void ProgressIndicator::display()
{
	_prevDisplay = _current;

	char buf[20];

	double percents = ((double)_current)/_total * 100.0;

	sprintf(buf, "%.01f %%", percents);

	output = buf;
	output += postfix;
}

void ProgressIndicator::clear()
{
	output.rollback();
}
