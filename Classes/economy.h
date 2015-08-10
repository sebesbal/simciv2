#pragma once
#include "world_model.h"	
#include <vector>

namespace simciv
{
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
		Area* area;
		double volume; // negative volume means consumer
		double free_volume;
		double price; // the current price
		double profit; // the worst profit of the producers deals
		double partner_price;
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
		ProductMap(WorldModel& world);
		void update();
		AreaProd& get_prod(Area* a) { return (*_production)[a->index]; }
		AreaProd& get_new_prod(Area* a) { return (*_new_production)[a->index]; }
		void add_prod(Area* area, double volume, double price);
		void remove_prod(Area* area, double volume, double price);
		void routes_to_areas(int prod_id);
		void generate_resources();
	protected:
		int update_count;
		bool unique_mode;
		void update_routes();
		void update_prices();
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