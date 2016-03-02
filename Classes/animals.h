#pragma once
#include "world_model.h"
#include "economy.h"
#include <map>

namespace rapidxml
{
	template<class Ch = char>
	class xml_node;
}

namespace simciv
{
	class AnimalWorld;
	extern AnimalWorld _model;

	struct ProductionRule
	{
		std::vector<int> area_types;
		MaterialMap input;
		MaterialMap output;
		void load(rapidxml::xml_node<>* node);
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
		int id;
		std::string name;
		SpeciesType type;
		std::vector<ProductionRule> m2m_rules;	///< products materials form materials
		std::vector<ProductionRule> m2a_rules;	///< products articles form materials.
		MaterialVec build_cost;			///< cost of build a new instance
		MaterialMap maintenance_cost;	///< cost of maintain the instance
		void find_best_m2m_rule(const Prices& prices, ProductionRule*& rule, double& profit);
		void find_best_m2a_rule(const Prices& prices, ProductionRule*& rule, double& price);
		std::string icon_file;

		//int level;
		//int color;
		void load(rapidxml::xml_node<>* node);
	};

	struct Plant
	{
		std::string name;
		int id;
		std::string icon_file;
		void load(rapidxml::xml_node<>* node);
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
		double money;
		Area* area;
		void update();
		double apply_rule(ProductionRule* rule, double profit, double ideal_rate); ///< tries to apply the rule with "rate" times. returns the applicable rate. (depending on the storage)
		double consume_article(int art_ind, Prices& prices, double& volume); ///< changes volume to the consumed volume, and returns the price
		double consume_articles(Prices& prices);
		void check_supply_storage(MaterialMap& vols, double& rate);
		void check_consumption_storage(MaterialMap& vols, double& rate);
		void check_money(double price, double& rate);
		Prices get_prices();
		void income(double money);
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
		std::vector<Species*>& get_species() { return species; }
		Species* get_species(std::string name) { for (auto p : species) if (p->name == name) return p; return NULL; }
		Species* get_storage_species() { return species.back(); }
		Plant* get_plant(std::string name) { for (auto p : plants) if (p->name == name) return p; return NULL; }
		std::vector<Plant*>& get_plants() { return plants; }
		void add_plant(Plant* plant) { plant->id = plants.size(); plants.push_back(plant); }
		virtual void update() override;
		void load_from_file(std::string file_name);
		void add_species(Species* species) { species->id = this->species.size(); this->species.push_back(species); }
		Prices get_prices(Area* a);
		double get_profit(Species* species, Area* a);
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
		//Prices get_prices(Area* a);
		//std::map<std::string, Species*> species_map;
		//std::map<std::string, Plant*> plant_map;
		std::vector<Species*> species;
		std::vector<Plant*> plants;
		std::vector<Animal*> animals;
	};

}