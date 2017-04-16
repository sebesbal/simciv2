#pragma once

#include <vector>


namespace indgen
{
	struct FactoryType
	{

	};

	const int prodn = 4;

	typedef double production[prodn];

	struct Factory
	{
		int x;
		int y;
		production prod;
	};

	struct EconomyType
	{
		std::vector<FactoryType> factories;
		double fitness;
	};
	
	struct Economy
	{
		EconomyType* type;
		std::vector<Factory> factories;
	};

	struct EconomyTypeGenerator
	{
		std::vector<EconomyType> population;
		void mutate(EconomyType& a);
		void recombine(EconomyType& a, EconomyType& b);
		double fitness(EconomyType& a);
		void step();

	};
}
