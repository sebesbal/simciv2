#pragma once

#include <map>

#include "map.h"
#include "trade.h"

namespace rapidxml
{
	template<class Ch = char>
	class xml_node;
}

namespace simciv
{
	const double max_price = 100000000000;
	extern int product_count; ///< The number of product types

	/// Just a vector with product_count size
	class Products : public std::vector<double>
	{
	public:
		Products() { for (int i = 0; i < product_count; ++i) push_back(0); }
	};

	/// Describes all prices of an Area or Factory
	struct Prices
	{
		Products sell;
		Products buy;
	};

	typedef std::map<int, double> ProductMap; ///< (product id, volume) pairs.

	/// Describes a production chain. Creates a set of products from a set of input products
	struct ProductionRule
	{
		ProductMap input;						///< The id's and volumes of the input products
		ProductMap output;						///< The id's and volumes of the output products
		void load(rapidxml::xml_node<>* node);
		double profit(const Prices& prices);	///< used for m2m rules
		double expense(const Prices& prices);	///< used for m2a rules
	};

	enum IndustryType
	{
		IT_NONE,
		IT_MINE,
		IT_FACTORY,
		IT_STORAGE
	};

	struct Product
	{
		std::string name;
		int id;
		std::string icon_file;
		void load(rapidxml::xml_node<>* node);
	};

	/// The type of a Factory.
	struct Industry
	{
		int id;
		std::string name;
		IndustryType type;
		std::vector<ProductionRule> m2m_rules;	///< creates products form products
		std::vector<ProductionRule> m2a_rules;	///< creates articles form products.
		Products build_cost;					///< cost of build a new instance
		ProductMap maintenance_cost;			///< cost of maintain the instance
		void find_best_m2m_rule(const Prices& prices, Area* area, ProductionRule*& rule, double& profit);
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
		double efficiency;
		Area* area;
		void update();
		double apply_rule(ProductionRule* rule, double profit, double ideal_rate); ///< tries to apply the rule with "rate" times. returns the applicable rate. (depending on the storage)
		double consume_article(int art_ind, Prices& prices, double& volume); ///< changes volume to the consumed volume, and returns the price
		double consume_articles(Prices& prices);
		void check_supply_storage(ProductMap& vols, double& rate);
		void check_consumption_storage(ProductMap& vols, double& rate);
		void check_money(double price, double& rate);
		void find_best_m2m_rule(const Prices& prices, ProductionRule*& rule, double& profit);
		Prices get_prices();
		void income(double money);
	};

	class World : public Map
	{
	public:
		virtual void create(int width, int height, int prod_count) override;
		void load_from_file(std::string file_name);
		virtual void update() override;
		Factory* create_factory(Area* a, Industry& industry);
		Factory* find_factory(Area* a);
		std::vector<Factory*>& get_factories() { return factories; }
		std::vector<Industry*>& get_industries() { return industries; }
		Industry* get_industries(std::string name) { for (auto p : industries) if (p->name == name) return p; return NULL; }
		Product* get_product(std::string name) { for (auto p : products) if (p->name == name) return p; return NULL; }
		std::vector<Product*>& get_products() { return products; }
		Prices get_prices(Area* a);
		double get_profit(Industry* industry, Area* a);
		double get_build_cost(Industry* industry, Area* a);
		double get_resources(Industry* industry, Area* a);
		AreaData& get_trade(Area* a, int id);
		std::vector<TradeMap*>& trade_maps() { return _trade_maps; }
	protected:
		void add_product(Product* product) { product->id = products.size(); products.push_back(product); }
		void add_industry(Industry* industry) { industry->id = this->industries.size(); this->industries.push_back(industry); }
		void generate_industry();
		void generate_factories();
		void move_factory(Factory* f, Area* new_area);
		std::vector<Industry*> industries;
		std::vector<Product*> products;
		std::vector<Factory*> factories;
		std::vector<TradeMap*> _trade_maps;
	};

	extern World world;

	class Info
	{
	public:
		static double price_buy(Area* a, Product* p) { return world.get_trade(a, p->id).p_buy; }
		static double price_sell(Area* a, Product* p) { return world.get_trade(a, p->id).p_sell; }
		static double resources(Area* a, Product* p) { return world.get_trade(a, p->id).resource; }

		static double profit(Area* a, Industry* s) { return world.get_profit(s, a); }
		static double build_cost(Area* a, Industry* s) { return world.get_build_cost(s, a); }
	};
}