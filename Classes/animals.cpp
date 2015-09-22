#include "animals.h"
#include <algorithm>

namespace simciv
{
	const int species_count = 6;

	double ProductionRule::profit(const MaterialVec& prices)
	{
		double profit = 0;
		for (auto& p: input)
		{
			int prod_id = p.first;
			double price = prices[prod_id];
			double vol = p.second;
			profit -= price * vol;
		}

		for (auto& p : output)
		{
			int prod_id = p.first;
			double price = prices[prod_id];
			double vol = p.second;
			profit += price * vol;
		}

		return profit;
	}

	ProductionRule* Species::find_best_rule(const MaterialVec& prices)
	{
		double best_profit = 0;
		ProductionRule* best_rule = NULL;
		for (auto& rule : rules)
		{
			double profit = rule.profit(prices);
			if (profit > best_profit)
			{
				best_profit = profit;
				best_rule = &rule;
			}
		}
		if (best_rule)
		{
			return best_rule;
		}
		return NULL;
	}

	Animal::Animal(Species& species): species(species)
	{
		for (int i = 0; i < material_count; ++i)
		{
			producers.push_back(NULL);
		}
	}
	
	void Animal::update()
	{
		//area

		//species.find_best_rule();
	}



	void AnimalWorld::create_map(int width, int height, int prod_count)
	{
		WorldModel::create_map(width, height, prod_count);
		generate_species();
		generate_animals();
	}

	/*
	void AnimalWorld::generate_species()
	{
		for (int i = 0; i < species_count; ++i)
		{
			Species s;
			s.id = i;
			for (int j = 0; j < _products.size(); ++j)
			{
				double d = (double)rand() / RAND_MAX;
				int n = 3;
				int k = std::floor((2 * n + 1) * d - n);
				s.production.push_back(k);
				double e = (double)rand() / RAND_MAX;
				s.maintenance_cost.push_back(e);
			}
			species.push_back(s);
		}
	}
	*/

	/* 
	materials:
	0 = food
	1 = manpower
	2 = wood
	3 = stone

	animals:
	0 = house:	food --> manpower
	1 = farm:   manpower --> food
	2 = mine:	manpower --> stone
	3 = lodge:	manpower --> wood


	a: _ --> 1
	b: 

	*/

	void AnimalWorld::generate_species()
	{
		// Worker
		Species worker;
		worker.id = 0;
		for (int i = 0; i < 3; ++i)
		{
			ProductionRule r;
			r.output[i] = 1;
			worker.rules.push_back(r);
		}
		species.push_back(worker);

		// Smith
		Species smith;
		smith.id = 1;
		for (int i = 0; i < 3; ++i)
		{
			ProductionRule r;
			r.input[i] = 1;
			r.input[(i + 1) % 3] = 1;
			r.output[i + 3] = 1;
			smith.rules.push_back(r);
		}
		species.push_back(smith);

		// Worker2
		Species worker2;
		worker2.id = 2;
		for (int i = 0; i < 3; ++i)
		{
			ProductionRule r;
			r.input[(i + 1) % 3 + 3] = 1;
			r.output[i] = 4;
			worker2.rules.push_back(r);
		}
		species.push_back(worker2);

		// Smith2
		Species smith2;
		smith2.id = 2;
		for (int i = 0; i < 3; ++i)
		{
			ProductionRule r;
			r.input[i] = 2;
			r.input[(i + 1) % 3] = 2;
			r.output[i + 3] = 2;
			smith2.rules.push_back(r);
		}
		species.push_back(smith2);
	}

	void AnimalWorld::generate_animals()
	{
		for (auto& s: species)
		{
			for (int i = 0; i < 10; ++i)
			{
				int area_index = rand() % _areas.size();
				create_animal(_areas[area_index], s);
			}
		}
	}

	Animal* AnimalWorld::create_animal(Area* a, Species& species)
	{
		if (find_animal(a)) return NULL;
		Animal* ani = new Animal(species);
		animals.push_back(ani);
		ani->area = a;
		return ani;
	}

	Animal* AnimalWorld::find_animal(Area* a)
	{
		auto it = find_if(animals.begin(), animals.end(), [a](Animal* ani){ return ani->area == a; });
		if (it == animals.end())
		{
			return NULL;
		}
		else
		{
			return *it;
		}
	}

	void AnimalWorld::update()
	{
		for (Animal* ani : animals)
		{
			MaterialVec prices;
			Area* area = ani->area;
			for (int i = 0; i < material_count; ++i)
			{
				auto& info = get_prod(area, i);
				prices[i] = info.p;
			}
			auto rule = ani->species.find_best_rule(prices);
			if (rule)
			{
				double rate = 1;
				for (auto& p : rule->input)
				{
					int prod_id = p.first;
					double vol = p.second;
					auto& producer = ani->producers[prod_id];
					if (producer)
					{
						rate = std::min(rate, producer->storage / vol);
					}
					else
					{
						producer = _products[prod_id]->create_prod(area, vol, prices[prod_id]);
						rate = 0;
					}
				}

				if (rate == 0)
				{
					// Skip
				}
				else
				{
					for (auto& p : rule->input)
					{
						int prod_id = p.first;
						double vol = p.second;
						auto& producer = ani->producers[prod_id];
						producer->storage -= rate * vol;
					}
				}
			}
		}


		// update product maps
		WorldModel::update();

	}

	void AnimalWorld::move_animal(Animal* ani, Area* new_area)
	{
		for (int i = 0; i < _products.size(); ++i)
		{
			_products[i]->move_prod(ani->producers[i], new_area);
		}
		ani->area = new_area;
	}
}