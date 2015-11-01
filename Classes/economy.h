#pragma once
#include "world_model.h"	
#include <vector>
#include <queue>

namespace simciv
{
	// typedef std::queue<double> history_t;
	typedef std::deque<double> history_t;

	const int history_count = 20;
	
	struct Material
	{

	};

	/// Represents one single product's volumes and prices on an area
	struct AreaProd
	{
		AreaProd();
		double p;
		double p_con; // consumption price: the lowest buying price in the area for what there is a seller somewhere
		double p_sup; // supply price: the highest selling price in the area for what there is a buyer somewhere

		double v_con; // volume demand
		double v_sup; // volume supply
		double v;

		double m_con; // the profit of the consumer's worst deals in this area
		double m_sup; // the profit of the suppliers's worst deals in this area

		double resource; // how effective the production is
	};

	struct Producer
	{
		Producer();
		double storage() { return _storage; }
		int prod_id;
		bool is_consumer;
		Producer* storage_pair; ///< for storages: one producer for supply and one for consumption, storage_pair connects the two
		bool fix_price;
		Area* area;
		double volume; ///< trade offer.
		double free_volume; ///< volume - actually traded volume
		double ideal_volume;
		double price; // the current price
		double profit; // the worst profit of the producers deals
		double partner_price;


		

		double prod_volume; // production volume. 
		//void produce(double vol);
		void modify_storage(double ideal_vol, double actual_vol);
		void set_storage(double vol);

		void update_price();
		void update_storage();
		double free_capacity() { return storage_capacity - _storage; }

		history_t history_vol;
		history_t history_storage;
		history_t history_price;

		double storage_capacity;

	protected:
		
		double storage_last;
		double _storage;
		double storage_d; ///< that's how the storage would be changed if there is no trade
	};

	struct Transport
	{
		Producer* sup;
		Producer* dem;
		Route* route;
		bool invert_route; ///< the route directs dem to sup
		double volume;
		double trans_price;
		double profit;
	};

	/// Represents one single product's volumes and prices on the map
	class ProductMap
	{
	public:
		ProductMap(WorldModel& world, int prod_id);
		void update();
		AreaProd& get_prod(Area* a) { return (*_production)[a->index]; }
		AreaProd& get_new_prod(Area* a) { return (*_new_production)[a->index]; }
		Producer* add_prod(Area* area, double volume, double price);
		Producer* create_prod(Area* area, bool consumer, double volume, double price);
		void remove_prod(Producer* prod);
		void remove_prod(Area* area, double volume, double price);
		void move_prod(Producer* prod, Area* new_area);
		void routes_to_areas(int prod_id);
		void generate_resources();
		std::vector<Transport*>& transports() { return _transports; }
		void update_transports(); ///< find possible trade routes
		void update_trade(); ///< calculate trade on the routes
		void update_producer_prices();
		void update_area_prices(); ///< calculate price info on every area
		void update_producer_storages();
	protected:
		int prod_id;
		int update_count;
		bool unique_mode;
		void create_g();
		Transport* get_transport(Producer* src, Producer* dst);
		WorldModel& _world;
		std::vector<Producer*> _supplies;
		std::vector<Producer*> _consumers;
		std::vector<std::vector<Producer*>> _area_supplies;
		std::vector<std::vector<Producer*>> _area_consumers;

		std::vector<Transport*> _transports; // "all" possible transports. volume > 0 means that the transport is used
		std::vector<AreaProd>* _production;
		std::vector<AreaProd>* _new_production;
	};

}