/*
 * Maxime Moge dda119141@gmail.com 05.07.2021
 * StaticMapAnalyzer.cpp : This file contains the 'main' function. Program execution begins and ends there.
 *
*/

#include <iostream>
#include "StaticMapAnalyzer.h"


int main()
{

	try {

		lineParser pPars;
		pPars.update_total_values();
	}
	catch (const std::exception & e)
	{
		std::cerr << "json error: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Abnormal termination " << std::endl;
	}

    std::cout << "PARSING END!\n";

}


