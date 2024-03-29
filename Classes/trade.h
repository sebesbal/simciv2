#pragma once
#include <vector>
#include <queue>

#include "sc_map.h"

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
		const double vol_step = 0.1;
		const double price_step = 0.1;
		Trader();
		Product* product;
		Area* area;
		int ref_count;
		//Factory* owner;
		Trader* storage_pair;					///< the other trader of a seller-buyer pair (for a Factory-Product)
		bool is_buyer;							///< this is a buyer (not seller)
		//double vol_in;							///< the volume what the owner Factory wanted to put in/take out from this Trader.
		//double vol_out;							///< the volume what the partners wanted to buy/sell from this Trader. (so, there are possible partners on this price)
		//double volume;							///< the volume what Trader wants to buy/sell in one turn. This is a combined value of val_in and val_out, considering the fullness of the storage too.
		//double free_volume;						///< volume - actually traded volume. (the partners didn't want to buy/sell this volume)
		//double price;							///< the current price
		double worst_profit;					///< the profit of the worst transport. (used for Area's price calculation)
		double worst_price;						///< the price of the worst transport. (used for Area's price calculation)

		double goal_vol;		///< The trade won't be larger than the goal
		double goal_price;		///< The price can't be larger (for a buyer) or lower (for a seller) than the goal
		double vol;				///< Current volume
		double price;			///< Current price
		double max_remote_vol;	///< Maximum tradeable volume
		double max_local_vol;	///< Maximum consumeable (produceable) volume
		double remote_vol;		///< Current remote volume
		double local_vol;		///< Current local volume
		double avg_remote_vol;	///< Moving average of the actual remote volume
		double avg_local_vol;	///< Moving average of the actual local volume

		void update();

		void modify_storage(double vol);
		void set_storage(double vol);

		double& storage() { return _storage; }
		void add_money(double d) { }
		void update_history();
		double free_capacity() { return storage_capacity - _storage; }

		bool storage_is_ok();	///< Storage is "OK" (not too low in case of buyer, not too high in case of seller)

		history_t history_trade;
		history_t history_storage;
		history_t history_price;
		history_t history_remote_vol;
		history_t history_local_vol;

		double storage_capacity;
		double ideal_fullness; ///< 0..1, negative value means auto control
		double storage_last;
		double _storage;
		double _d_storage;
	};

	class Trader2;

	struct Contract
	{
		double vol;
		double price;
		bool operator < (const Contract& contract) const;
		Trader2* buyer;
		Trader2* seller;
		Trader2* another(Trader2* trader) { return (trader == buyer ? seller : buyer); }

	};

	class Trader2
	{
	public:
		enum State {
			NONE, OK, NO_SUPPLY_DEMAND, CHANGING
		};
		const double vol_step = 0.1;
		const double price_step = 0.1;
		Trader2();
		Product* product;
		Area* area;
		int ref_count;
		bool is_buyer;  ///< this is a buyer (not seller)
		double goal_vol;
		double goal_price;
		double vol;
		double price;
		State state;
		std::priority_queue<Contract> contracts;
		std::vector<std::shared_ptr<Contract>> contract_candidates;
		Contract* best_next_contract;
		void set_goals(double vol, double price);
		void update_contracts();
		void sign_contract();
		void delete_contract();
		void add_contract_candidate(std::shared_ptr<Contract>& contract);
		void remove_contract_candidate(std::shared_ptr<Contract>& contract);
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
		//double d_storage;
		double price;		///< avg. price
		double max_price = 100;	///< max avg. price
		double vol;
		double max_vol = std::numeric_limits<double>::infinity();
	};

	/// A trade route between two Trader
	struct Transport
	{
		Transport() : marked_as_deleted(false) { }
		bool marked_as_deleted;
		Trader* seller;
		Trader* buyer;
		Trader* fuel_buyer;
		Route* route;
		bool invert_route; ///< the route directs buyer to seller
		double volume;
		double demand_volume;
		double cost;		///< delivery cost for one unit of product
		double fuel_volume; ///< fuel consumption of delivering one unit of product
		double profit;
		int active_time;
		int creation_time;

		double smooth_volume;
	};

	/// Manages Traders of one Product on the map
	class TradeMap
	{
		friend class World;

	public:
		TradeMap(Product& p);
		AreaData& get_trade(Area* a) { return (*_production)[a->id]; }
		AreaData& get_new_prod(Area* a) { return (*_new_production)[a->id]; }
		Trader* create_prod(Area* area, bool consumer, double price);
		void remove_prod(Trader* t);
		void generate_resources();
		std::vector<Transport*>& transports() { return _transports; }
		void update_transports(); ///< find possible trade routes
		void update_trade_volume(); ///< calculate trade on the routes
		void update_producer_prices();
		void before_rules();
		void update_area_prices2(bool full_update = false);
		void find_best_producers_for_areas();
		void update_producer_storages();
		bool is_used(Area* a);  ///< The product is sold ot bought on this area
		WorldTradeData& get_world_data() { return data; }  ///< Global production and average prices
		Product& product;
		void TradeMap::update_data();  ///< Computes global prudction and average prices 
		void sync_area_traders(Area* a);
	protected:
		WorldTradeData data;
		int update_count;
		bool unique_mode;
		Transport* get_transport(Trader* src, Trader* dst);
		void get_transports(Area* a, std::vector<Transport*>& in, std::vector<Transport*>& out);
		std::vector<Trader*> _sellers;
		std::vector<Trader*> _buyers;
		std::vector<std::vector<Trader*>> _area_sellers;
		std::vector<std::vector<Trader*>> _area_buyers;

		std::vector<Transport*> _transports; // "all" possible transports. volume > 0 means that the transport is used
		std::vector<AreaData>* _production;
		std::vector<AreaData>* _new_production; // temporary variable for _production
	};
}