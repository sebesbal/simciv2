#pragma once
#include <vector>

#include "Eigen/Dense"

typedef Eigen::Vector2d vec_t;

namespace indgen
{

	const int prodn = 4;

	typedef double production[prodn];
	struct FactoryType
	{
		production prod;
	};
	struct Factory;
	struct Economy;
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
		std::vector<FactoryType*> mines;
		std::vector<FactoryType*> factories;
		Economy* generate(std::map<FactoryType*, int> counts, int w, int h);
	};

	struct Economy
	{
		Economy(std::vector<Factory>& mines) : mines(mines), fitness(0) { }
		EconomyType* type;
		std::vector<Factory> factories;
		std::vector<Factory> mines;
		double fitness;
		void optimize_positions();
		bool optimize_positions_step();
		double transport_cost();
	};

	struct EconomyGenerator
	{
		const int mines_per_prod = 3;

		std::vector<Factory> mines;
		std::vector<Economy> population;
		static std::vector<Factory> generate_random_mines();
		static Economy generate_random_economy(std::vector<Factory>& mines);
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
