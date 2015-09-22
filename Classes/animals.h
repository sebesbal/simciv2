#pragma once
#include "world_model.h"
#include "economy.h"
#include <map>

namespace simciv
{
	struct ProductionRule
	{
		std::vector<int> area_types;
		MaterialMap input;
		MaterialMap output;
		double profit(const MaterialVec& prices);
	};

	struct Species
	{
		int id;
		std::vector<ProductionRule> rules;	///< products materials form materials
		MaterialVec build_cost;			///< cost of build a new instance
		MaterialVec maintenance_cost;	///< cost of maintain the instance
		ProductionRule* find_best_rule(const MaterialVec& prices);
	};

	//class Market
	//{
	//public:

	//protected:
	//	MaterialVec storage_capacity;
	//	MaterialVec storage_current;
	//	MaterialVec storage_ideal;
	//	MaterialVec price;

	//};

	struct Animal
	{
		Animal(Species& species);
		Species& species;
		std::vector<double> storage;
		std::vector<Producer*> producers;
		Area* area;
		void update();
	};

	class AnimalWorld: public WorldModel
	{
	public:
		virtual void create_map(int width, int height, int prod_count) override;
		void generate_species();
		void generate_animals();
		Animal* create_animal(Area* a, Species& species);
		Animal* find_animal(Area* a);
		std::vector<Animal*>& get_animals() { return animals; }
		std::vector<Species>& get_species() { return species; }
		virtual void update() override;
	protected:
		void move_animal(Animal* ani, Area* new_area);
		void add_producers(Animal* ani, Area* area);
		void remove_producers(Animal* ani, Area* area);
		std::vector<Species> species;
		std::vector<Animal*> animals;
	};

}