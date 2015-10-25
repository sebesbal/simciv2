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
		double profit(const Prices& prices);	// used for m2m rules
		double expense(const Prices& prices);	// used for m2e rules
	};

	enum SpeciesType
	{
		ST_TYPECOLOR,
		ST_STORAGE
	};

	struct Species
	{
		//int id;
		SpeciesType type;
		std::vector<ProductionRule> m2m_rules;	///< products materials form materials
		std::vector<ProductionRule> m2a_rules;	///< products articles form materials.
		MaterialVec build_cost;			///< cost of build a new instance
		MaterialMap maintenance_cost;	///< cost of maintain the instance
		ProductionRule* find_best_m2m_rule(const Prices& prices);
		std::string icon_file;

		int level;
		int color;
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

	//class Articles
	//{
	//	std::map<int, int> 
	//};

	struct Animal
	{
		Animal(Species& species);
		Species& species;
		//MaterialVec storage;
		std::vector<Producer*> supplies;
		std::vector<Producer*> consumers;
		Area* area;
		void update();
		double apply_rule(ProductionRule* rule, double ideal_rate); ///< tries to apply the rule with "rate" times. returns the applicable rate. (depending on the storage)
		double consume_article(int art_ind, Prices& prices, double& volume); ///< changes volume to the consumed volume, and returns the price
		double consume_articles(Prices& prices);
		void check_supply_storage(MaterialMap& vols, double& rate);
		void check_consumption_storage(MaterialMap& vols, double& rate);
		Prices get_prices();
	};

	class AnimalWorld : public WorldModel
	{
	public:
		virtual void create_map(int width, int height, int prod_count) override;
		void generate_species();
		void generate_animals();
		Animal* create_animal(Area* a, Species& species);
		Animal* find_animal(Area* a);
		std::vector<Animal*>& get_animals() { return animals; }
		std::vector<Species>& get_species() { return species; }
		Species* get_species(int level, int color);
		Species* get_storage_species() { return &species.back(); }
		virtual void update() override;
	protected:
		void move_animal(Animal* ani, Area* new_area);
		void add_producers(Animal* ani, Area* area);
		void remove_producers(Animal* ani, Area* area);
		//double check_supply_storage(Animal* ani, MaterialMap& vols); ///< returns 1 if the producer's storage can be changed with vols, and with 0 if not at all
		//double check_consumption_storage(Animal* ani, MaterialMap& vols); ///< returns 1 if the producer's storage can be changed with vols, and with 0 if not at all
		// double get_modify_storage_success(Producer* prod, double dol);
		//double get_rule_success(Animal* ani, MaterialMap& vols, MaterialVec& prices);
		//virtual void update_animal_m2m(Animal* ani, ProductionRule* rule);
		//virtual void update_animal_m2a(Animal* ani, MaterialMap& vols, Prices& prices);
		Prices get_prices(Area* a);
		std::vector<Species> species;
		std::vector<Animal*> animals;
	};

}