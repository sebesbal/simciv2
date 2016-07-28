#pragma once
#include <vector>
#include <queue>

#include "map.h"

namespace simciv
{
	typedef std::deque<double> history_t;
	const int history_count = 20;
	struct Factory;
	struct Product;

	/// Class for trading products on the map. It manages storage and pricing.
	/// Every Factory has two Trader for every Product: a buyer (to buy inputs), and a seller (to sell output). (most of them are unused)
	struct Trader
	{
		Trader();
		Product* product;
		Area* area;
		Factory* owner;
		Trader* storage_pair;					///< the other trader of a seller-buyer pair (for a Factory-Product)
		bool is_buyer;							///< this is a buyer (not seller)
		double vol_in;							///< the value what the owner Factory wanted to put in/take out from this Trader.
		double vol_out;							///< the value what the partners wanted to buy/sell from this Trader. (so, there are possible partners on this price)
		double volume;							///< the value what Trader wants to buy/sell in one turn. This is a combined value of val_in and val_out, considering the fullness of the storage too.
		double free_volume;						///< volume - actually traded volume. (the partners didn't want to buy/sell this volume)
		double price;							///< the current price
		double worst_profit;					///< the profit of the worst transport. (used for Area's price calculation)
		double worst_price;						///< the price of the worst transport. (used for Area's price calculation)

		void modify_storage(double vol);
		void set_storage(double vol);

		double& storage() { return _storage; }
		double money();
		void update_price();
		void update_volume();
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

	/// Trade data of an Area-Product
	struct AreaData
	{
		AreaData();
		double p;			///< price
		double p_buy;		///< buying price: the lowest buying price in the area for what there is a seller somewhere
		double p_sell;		///< selling price: the highest selling price in the area for what there is a buyer somewhere

		double v_buy;		///< volume buy
		double v_sell;		///< volume sell
		double v;			///< volume

		double resource;	///< how effective the production is

		std::pair<double, Trader*> best_seller;
		std::pair<double, Trader*> best_buyer;
	};

	struct WorldTradeData
	{
		double storage;
		double d_storage;
		double price;
	};

	/// A trade route between two Trader
	struct Transport
	{
		Trader* seller;
		Trader* buyer;
		Route* route;
		bool invert_route; ///< the route directs buyer to seller
		double volume;
		double demand_volume;
		double cost;
		double fuel_volume; ///< fuel 
		double profit;
		int active_time;
		int creation_time;
	};

	/// Manages Traders of one Product on the map
	class TradeMap
	{
		friend class World;

	public:
		TradeMap(Product& p);
		void update();
		AreaData& get_trade(Area* a) { return (*_production)[a->id]; }
		AreaData& get_new_prod(Area* a) { return (*_new_production)[a->id]; }
		Trader* create_prod(Area* area, bool consumer, double price);
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
		bool is_used(Area* a);
		WorldTradeData& get_world_data() { return data; }
		Product& product;
		void TradeMap::update_data();
	protected:
		WorldTradeData data;
		int update_count;
		bool unique_mode;
		Transport* get_transport(Trader* src, Trader* dst);
		std::vector<Trader*> _sellers;
		std::vector<Trader*> _buyers;
		std::vector<std::vector<Trader*>> _area_sellers;
		std::vector<std::vector<Trader*>> _area_buyers;

		std::vector<Transport*> _transports; // "all" possible transports. volume > 0 means that the transport is used
		std::vector<AreaData>* _production;
		std::vector<AreaData>* _new_production; // temporary variable for _production
	};
}