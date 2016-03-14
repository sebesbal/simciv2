#pragma once
#include "map.h"	
#include <vector>
#include <queue>

namespace simciv
{
	typedef std::deque<double> history_t;
	const int history_count = 20;
	
	struct Trader
	{
		Trader();
		double& storage() { return _storage; }
		int prod_id;
		bool is_buyer;
		Trader* storage_pair; ///< for storages: one producer for supply and one for consumption, storage_pair connects the two
		bool fix_price;
		Area* area;
		void* owner;
		double volume; ///< trade offer.
		double free_volume; ///< volume - actually traded volume
		double vol_out;
		double price; // the current price
		double worst_profit; // the worst profit of the producers deals
		double partner_price;

		void modify_storage(double ideal_vol, double actual_vol);
		void set_storage(double vol);

		double money();
		void update_price();
		void synchronize_price();
		void update_storage();
		double free_capacity() { return storage_capacity - _storage; }

		history_t history_trade;
		history_t history_storage;
		history_t history_price;

		double storage_capacity;
		double ideal_fullness; ///< 0..1, negative value means auto control
		double storage_last;
		double _storage;
		double _d_storage;
	};

	/// Trade data of an Area
	struct AreaTrade
	{
		AreaTrade();
		double p;
		double p_buy; // buying price: the lowest buying price in the area for what there is a seller somewhere
		double p_sell; // selling price: the highest selling price in the area for what there is a buyer somewhere

		double v_buy; // volume buy
		double v_sell; // volume sell
		double v;

		double resource; // how effective the production is

		std::pair<double, Trader*> best_seller;
		std::pair<double, Trader*> best_buyer;
	};

	struct Transport
	{
		Trader* seller;
		Trader* buyer;
		Route* route;
		bool invert_route; ///< the route directs buyer to seller
		double volume;
		double demand_volume;
		double cost;
		double profit;
		int active_time;
	};

	/// Represents one single product's volumes and prices on the map
	class TradeMap
	{
	public:
		TradeMap(int prod_id);
		void update();
		AreaTrade& get_trade(Area* a) { return (*_production)[a->index]; }
		AreaTrade& get_new_prod(Area* a) { return (*_new_production)[a->index]; }
		Trader* add_prod(Area* area, double volume, double price);
		Trader* create_prod(Area* area, bool consumer, double volume, double price);
		void remove_prod(Trader* prod);
		void remove_prod(Area* area, double volume, double price);
		void move_prod(Trader* prod, Area* new_area);
		void generate_resources();
		std::vector<Transport*>& transports() { return _transports; }
		void update_transports(); ///< find possible trade routes
		void update_trade(); ///< calculate trade on the routes
		void update_producer_prices();
		void before_rules();
		void update_area_prices(); ///< calculate price info on every area
		void update_area_prices2(bool full_update = false);
		void find_best_producers_for_areas();
		void update_producer_storages();
	protected:
		int prod_id;
		int update_count;
		bool unique_mode;
		Transport* get_transport(Trader* src, Trader* dst);
		std::vector<Trader*> _sellers;
		std::vector<Trader*> _buyers;
		std::vector<std::vector<Trader*>> _area_sellers;
		std::vector<std::vector<Trader*>> _area_buyers;

		std::vector<Transport*> _transports; // "all" possible transports. volume > 0 means that the transport is used
		std::vector<AreaTrade>* _production;
		std::vector<AreaTrade>* _new_production;
	};
}