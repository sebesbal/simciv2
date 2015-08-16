#pragma once
#include "world_model.h"

namespace simciv
{

	struct Species
	{
		int id;
		MaterialVec production;			///< products materials form materials
		MaterialVec build_cost;			///< cost of build a new instance
		MaterialVec maintenance_cost;	///< cost of maintain the instance
	};

	struct Animal
	{
		Animal(Species& species): species(species) { }
		Species& species;
		std::vector<double> storage;
		// std::vector<Producer*> producers;
		Area* area;
	};

	class AnimalWorld: public WorldModel
	{
	public:
		virtual void create_map(int width, int height, int prod_count) override;
		void generate_species();
		void generate_animals();
	protected:
		Animal* create_animal(Area* a, Species& species);
		void move_animal(Animal* ani, Area* new_area);
		void add_producers(Area* a, Species& species);
		void remove_producers(Area* a, Species& species);
		std::vector<Species> species;
		std::vector<Animal*> animals;
	};

}