#pragma once

#include <vector>

#include "Eigen/Dense"

typedef Eigen::Vector2d vec_t;

namespace indgen
{
	struct FactoryType
	{

	};

	const int prodn = 4;

	typedef double production[prodn];

	struct Factory;

	struct Transport
	{
		Factory* a;
		Factory* b;
		double cost;
	};

	struct Factory
	{
		bool is_mobile;
		vec_t pos;
		production prod;
		std::vector<Transport> transports;
	};

	struct EconomyType
	{
		std::vector<FactoryType> factories;
		double fitness;
	};
	


	struct Economy
	{
		Economy(std::vector<Factory>& mines) : mines(mines) { }
		EconomyType* type;
		std::vector<Factory> factories;
		std::vector<Factory>& mines;
		void optimize_positions();
		double transport_cost();
	};

	struct EconomyGenerator
	{
		const int mines_per_prod = 3;

		std::vector<Factory> mines;
		std::vector<Economy> population;
		void generate_random_mines();
		void generate_random_economy();
		void init();
		
		void mutate(Economy& a);
		void recombine(Economy& a, Economy& b);
		double fitness(Economy& a);

		void step();
		void run();
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
