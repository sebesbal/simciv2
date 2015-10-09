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
		double p_con; // price demand
		double p_sup; // supply

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
		int prod_id;
		//bool is_consumer() { return volume < 0; }
		bool is_consumer() { return _is_consumer; }
		bool _is_consumer;
		bool _fix_price;
		Area* area;
		double volume; // trade offer. negative volume means consumer
		double free_volume; // offer - actually traded volume
		double price; // the current price
		double profit; // the worst profit of the producers deals
		double partner_price;

		double storage_capacity;
		double storage_last;
		double storage;
		double storage_d;

		double prod_volume; // production volume. negative volume means consumer
		void produce(double vol);
		void update_price();
		void update_storage();
		double free_capacity() { return storage_capacity - storage; }

		history_t history_vol;
		history_t history_storage;
		history_t history_price;
	};

	struct Transport
	{
		Producer* sup;
		Producer* dem;
		Route* route;
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
		Producer* create_prod(Area* area, double volume, double price);
		void remove_prod(Producer* prod);
		void remove_prod(Area* area, double volume, double price);
		void move_prod(Producer* prod, Area* new_area);
		void routes_to_areas(int prod_id);
		void generate_resources();
		std::vector<Transport*>& routes() { return _routes; }
	protected:
		int prod_id;
		Node* g;
		int update_count;
		bool unique_mode;
		void create_g();
		Transport* get_transport(Producer* src, Producer* dst);
		void update_routes();
		void update_prices();
		void update_storages();
		WorldModel& _world;
		std::vector<Producer*> _supplies;
		std::vector<Producer*> _consumers;
		std::vector<std::vector<Producer*>> _area_supplies;
		std::vector<std::vector<Producer*>> _area_consumers;

		std::vector<Transport*> _routes; // "all" possible routes. volume > 0 means that the route is used
		std::vector<AreaProd>* _production;
		std::vector<AreaProd>* _new_production;
	};

}