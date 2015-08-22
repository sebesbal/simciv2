#include "animals.h"
#include <algorithm>

namespace simciv
{
	const int species_count = 6;

	void AnimalWorld::create_map(int width, int height, int prod_count)
	{
		WorldModel::create_map(width, height, prod_count);
		generate_species();
		generate_animals();
	}

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
		add_producers(a, species);
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

	void AnimalWorld::move_animal(Animal* ani, Area* new_area)
	{
		remove_producers(ani->area, ani->species);
		add_producers(new_area, ani->species);
		ani->area = new_area;
	}

	void AnimalWorld::add_producers(Area* a, Species& species)
	{
		for (int prod_id = 0; prod_id < _pc; ++prod_id)
		{
			add_prod(a, prod_id, species.production[prod_id], 0);
		}
	}

	void AnimalWorld::remove_producers(Area* a, Species& species)
	{
		for (int prod_id = 0; prod_id < _pc; ++prod_id)
		{
			remove_prod(a, prod_id, species.production[prod_id], 0);
		}
	}
}