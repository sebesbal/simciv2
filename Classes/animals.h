#pragma once
#include "map.h"
#include "trade.h"
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

	const double max_price = 100000000000;
	extern int product_count;

	class Products : public std::vector < double >
	{
	public:
		Products() { for (int i = 0; i < product_count; ++i) push_back(0); }
	};

	struct Prices
	{
		Products sell;
		Products buy;
	};

	typedef std::map<int, double> ProductMap;

	struct ProductionRule
	{
		std::vector<int> area_types;
		ProductMap input;
		ProductMap output;
		void load(rapidxml::xml_node<>* node);
		double profit(const Prices& prices);	// used for m2m rules
		double expense(const Prices& prices);	// used for m2e rules
	};

	enum SpeciesType
	{
		ST_NONE,
		ST_MINE,
		ST_factory,
		ST_STORAGE
	};

	struct Product
	{
		std::string name;
		int id;
		std::string icon_file;
		void load(rapidxml::xml_node<>* node);
	};

	struct Industry
	{
		int id;
		std::string name;
		SpeciesType type;
		std::vector<ProductionRule> m2m_rules;	///< products materials form materials
		std::vector<ProductionRule> m2a_rules;	///< products articles form materials.
		Products build_cost;			///< cost of build a new instance
		ProductMap maintenance_cost;	///< cost of maintain the instance
		void find_best_m2m_rule(const Prices& prices, ProductionRule*& rule, double& profit);
		void find_best_m2a_rule(const Prices& prices, ProductionRule*& rule, double& price);
		double get_build_cost(const Prices& prices);
		std::string icon_file;
		Product* product;
		void load(rapidxml::xml_node<>* node);
		Product* get_product();
	};

	struct Factory
	{
		Factory(Industry& industry);
		Industry& industry;
		std::vector<Trader*> sellers;
		std::vector<Trader*> buyers;
		double money;
		Area* area;
		void update();
		double apply_rule(ProductionRule* rule, double profit, double ideal_rate); ///< tries to apply the rule with "rate" times. returns the applicable rate. (depending on the storage)
		double consume_article(int art_ind, Prices& prices, double& volume); ///< changes volume to the consumed volume, and returns the price
		double consume_articles(Prices& prices);
		void check_supply_storage(ProductMap& vols, double& rate);
		void check_consumption_storage(ProductMap& vols, double& rate);
		void check_money(double price, double& rate);
		Prices get_prices();
		void income(double money);
	};

	class AnimalWorld : public Map
	{
	public:
		virtual void create(int width, int height, int prod_count) override;
		void generate_species();
		void generate_factories();
		Factory* create_factory(Area* a, Industry& industry);
		Factory* find_factory(Area* a);
		std::vector<Factory*>& get_factories() { return factories; }
		std::vector<Industry*>& get_species() { return industry; }
		Industry* get_species(std::string name) { for (auto p : industry) if (p->name == name) return p; return NULL; }
		Industry* get_storage_species() { return industry.back(); }
		Product* get_plant(std::string name) { for (auto p : plants) if (p->name == name) return p; return NULL; }
		std::vector<Product*>& get_plants() { return plants; }
		void add_plant(Product* plant) { plant->id = plants.size(); plants.push_back(plant); }
		virtual void update() override;
		void load_from_file(std::string file_name);
		void add_species(Industry* industry) { industry->id = this->industry.size(); this->industry.push_back(industry); }
		Prices get_prices(Area* a);
		double get_profit(Industry* industry, Area* a);
		double get_build_cost(Industry* industry, Area* a);
		double get_resources(Industry* industry, Area* a);

		AreaTrade& get_trade(Area* a, int id);
		std::vector<TradeMap*>& trade_maps() { return _trade_maps; }
	protected:
		void move_factory(Factory* ani, Area* new_area);
		std::vector<Industry*> industry;
		std::vector<Product*> plants;
		std::vector<Factory*> factories;
		std::vector<TradeMap*> _trade_maps;
	};

	class Info
	{
	public:
		static double price_buy(Area* a, Product* p) { return _model.get_trade(a, p->id).p_buy; }
		static double price_sell(Area* a, Product* p) { return _model.get_trade(a, p->id).p_sell; }
		static double resources(Area* a, Product* p) { return _model.get_trade(a, p->id).resource; }

		static double profit(Area* a, Industry* s) { return _model.get_profit(s, a); }
		static double build_cost(Area* a, Industry* s) { return _model.get_build_cost(s, a); }
	};
}