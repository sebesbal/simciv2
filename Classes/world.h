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

	/// Describes a production phase. Creates a set of products from a set of input products
	struct ProductionRule
	{
		ProductMap input;						///< The id's and volumes of the input products
		ProductMap output;						///< The id's and volumes of the output products
		void load(rapidxml::xml_node<>* node);
		double profit(const Prices& prices);	///< used for prod rules
		double expense(const Prices& prices);	///< used for maint. rules
		ProductionRule multiply(double multiply_input, double multiply_outut); ///< multiply the volumes of the input and output
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
		int id;						///< index in the ProductMap
		std::string name;			///< id in the xml file
		std::string group;			///< gid in the xml file
		std::string display_name;	///< display name
		std::string icon_file;
		void load(rapidxml::xml_node<>* node);
	};

	/// The type of a Factory.
	struct Industry
	{
		Industry();
		std::string name;			///< id in the xml file
		std::string group;			///< gid in the xml file
		std::string display_name;	///< display name
		int level;
		IndustryType type;
		std::vector<ProductionRule> prod_rules;			///< product products form products
		std::vector<ProductionRule> maint_rules;		///< maintenance cost per turn
		std::vector<ProductionRule> build_total_cost;	///< total building cost of a Factory. build_total_cost = lifetime * maint_rules
		std::vector<ProductionRule> build_rules;		///< build cost per turn. build_rules = build_total_cost / build_time
		double buildtime;								///< number of turns of the building process.
		double lifetime;								///< lifetime in world time. During "lifetime" number turns, the Factory has to be "rebuilt" (maintain) via maint_rules
		void find_best_prod_rule(const Prices& prices, Area* area, ProductionRule*& rule, double& profit);
		void find_best_maint_rule(const Prices& prices, ProductionRule*& rule, double& price);
		double get_build_cost(const Prices& prices);
		std::string icon_file;
		Product* product;
		void load(rapidxml::xml_node<>* node);
		Product* get_product();
		const int default_lifetime = 5;
		const int default_buildtime = 5;
	};

	enum FactoryState
	{
		FS_NONE,
		FS_UNDER_CONTRUCTION,
		FS_RUN,
		FS_DEAD
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
		FactoryState state;		///< firstly the Factory is under construction
		Products built_in;				///< if built_in == industry.build_cost, the factory is completed
		double health;
		void update();
		double apply_rule(ProductionRule* rule, double profit, double ideal_rate); ///< tries to apply the rule with "rate" times. returns the applicable rate. (depending on the storage)
		double consume_articles(Prices& prices);
		double consume_articles(Prices& prices, std::vector<ProductionRule>& rules, double volume, double& full_expense);
		void check_seller_storage(ProductMap& vols, double& rate);
		void check_buyer_storage(ProductMap& vols, double& rate);
		void check_money(double price, double& rate);
		void find_best_prod_rule(const Prices& prices, ProductionRule*& rule, double& profit);
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
		Industry* get_industry(std::string name) { for (auto p : industries) if (p->name == name) return p; return NULL; }
		Product* get_product(std::string name) { for (auto p : products) if (p->name == name) return p; return NULL; }
		std::vector<Product*>& get_products() { return products; }
		Prices get_prices(Area* a);
		double get_profit(Industry* industry, Area* a);
		double get_build_cost(Industry* industry, Area* a);
		double get_resources(Industry* industry, Area* a);
		AreaData& get_trade(Area* a, int id);
		std::vector<TradeMap*>& trade_maps() { return _trade_maps; }
		int max_road_level;
	protected:
		void add_product(Product* product) { product->id = products.size(); products.push_back(product); }
		void add_industry(Industry* industry) { this->industries.push_back(industry); }
		void generate_industry();
		void generate_factories();
		std::vector<Industry*> industries;
		std::vector<Product*> products;
		std::vector<Factory*> factories;
		std::vector<TradeMap*> _trade_maps;
	};

	extern World world;
	extern int max_road_level;

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