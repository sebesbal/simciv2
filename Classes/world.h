#pragma once

#include <map>
#include <mutex>
#include <functional>

#include "map.h"
#include "trade.h"

namespace rapidxml
{
	template<class Ch = char>
	class xml_node;
}

namespace simciv
{
	const double distance_cost = 0.2;
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

	/// Describe the cost of a building (or upgrading, maintaining) process.
	struct ProductionCost
	{
		std::vector<ProductionRule> per_turn;
		std::vector<ProductionRule> total;		///< cost_per_turn * duration
		double duration;
		void calc_per_turn();
	};

	enum IndustryType
	{
		IT_NONE,
		IT_MINE,
		IT_FACTORY
	};

	struct Product
	{
		int id;						///< index in the ProductMap
		std::string name;			///< id in the xml file
		std::string group;			///< gid in the xml file
		std::string display_name;	///< display name
		std::string icon_file;
		TradeMap* map;
		std::map<AreaType, double> tile_res;	///< tile guid -> resource modifier map
		void load(rapidxml::xml_node<>* node);

		double average_price;
	};

	/// The type of a Factory.
	struct Industry
	{
		Industry();
		std::string name;			///< id in the xml file
		std::string group;			///< gid in the xml file
		std::string display_name;	///< display name
		//int level;
		IndustryType type;

		Industry* base;									///< lower level variant of this industry
		std::vector<Industry*> upgrades;				///< higher level variants of this industry
		bool can_upgrade_to(Industry* ind);
		virtual bool can_create_factory(Area* a);
		virtual Factory* create_factory(Area* a);
		
		std::vector<ProductionRule> prod_rules;			///< product products form products

		ProductionCost maint_cost;						///< cost of maintenance
		ProductionCost build_cost;						///< cost of building/repairing

		void find_best_prod_rule(const Prices& prices, Area* area, ProductionRule*& rule, double& profit);
		void find_best_maint_rule(const Prices& prices, ProductionRule*& rule, double& price);
		bool is_infra();								///< is infrastructure. (doesn't have production)
		double get_build_cost(const Prices& prices);
		std::string icon_file;
		Product* product;
		static Industry* create(rapidxml::xml_node<>* node);
		
		Product* get_product();
		static const int default_lifetime = 5;
		static const int default_buildtime = 5;

		std::set<int> buy_products;
		std::set<int> sell_products;

	protected:
		void load(rapidxml::xml_node<>* node);
	};

	struct Explorer : public Industry
	{
		virtual bool can_create_factory(Area* a) override;
	};

	enum FactoryState
	{
		FS_NONE,
		FS_UNDER_CONTRUCTION,
		FS_RUN,
		FS_UPGRADE,
		FS_DEAD
	};

	double consume_articles(std::vector<Trader*>& storage, Prices& prices, std::vector<ProductionRule>& rules, double volume, double& full_expense);
	void check_storage(bool is_buyer, std::vector<Trader*>& storage, ProductMap& vols, double& rate);

	struct Factory
	{
		Factory(Industry* industry);
		~Factory();
		Industry* industry;
		bool marked_as_deleted;
		std::vector<Trader*> sellers;
		std::vector<Trader*> buyers;
		double money;
		double efficiency;
		Area* area;
		FactoryState state;		///< firstly the Factory is under construction
		Products built_in;				///< if built_in == industry.build_cost, the factory is completed
		ProductionCost current_healing_cost;					///< cost for build/repair/upgrade

		double health;
		void set_state(FactoryState state);
		void set_industry(Industry* industry);
		void start_upgrade_to(Industry* industry);
		void update();
		double apply_rule(ProductionRule* rule, double profit, double ideal_rate); ///< tries to apply the rule with "rate" times. returns the applicable rate. (depending on the storage)
		double consume_articles(Prices& prices);
		double consume_articles(Prices& prices, std::vector<ProductionRule>& rules, double volume, double& full_expense);
		void check_money(double price, double& rate);
		void find_best_prod_rule(const Prices& prices, ProductionRule*& rule, double& profit);
		Prices get_prices();
		void income(double money);

		std::function<void(Factory*, FactoryState old_state, FactoryState new_state)> on_state_changed;
	};

#define MULTITHREAD

#ifdef MULTITHREAD
	extern std::recursive_mutex g_mutex;
#define LOCK_WORLD g_mutex.lock();
#define UNLOCK_WORLD g_mutex.unlock();
#define TRY_LOCK_WORLD g_mutex.try_lock()
#define GUARD_LOCK_WORLD std::lock_guard<std::recursive_mutex> guard(g_mutex);
#else
#define LOCK_WORLD
#define UNLOCK_WORLD
#define TRY_LOCK_WORLD true
#define GUARD_LOCK_WORLD
#endif

	struct MilitaryData
	{
		double desired_size;
		double actual_size;
	};

	class World : public Map
	{
	public:
		virtual void create(int width, int height, int prod_count) override;
		void load_from_file(std::string file_name);
		virtual void update() override;
		Factory* create_factory(Area* a, Industry* industry);
		void delete_factory(Factory* f);
		Factory* create_explorer(Area* a, Industry* industry);
		std::vector<Factory*> find_factories(Area* a);
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
		virtual void update_road_maps();
		void area_changed(Area* a);
		bool is_used(Area* a);
		double transport_cost(Area* a, Area* b);
		//double fuel_price(Area* a);
		Trader* fuel_buyer(Area* a);
		void set_mil_state(Area* a, MilitaryState state);
		void set_explored(Area* a);
		void generate_resources();

		MilitaryData mil_data;

		std::function<void(Area*)> on_area_changed;
		Explorer* _explorer;
		int _fuel_id;
	protected:
		void add_product(Product* product) { product->id = products.size(); products.push_back(product); }
		void add_industry(Industry* industry) { this->industries.push_back(industry); }
		void generate_industry();
		void generate_factories();
		std::vector<Industry*> industries;
		std::vector<Product*> products;
		std::vector<Factory*> factories;
		std::vector<TradeMap*> _trade_maps;
		TradeMap* _fuel_map;
		
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