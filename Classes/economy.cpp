
#include "economy.h"
#include <algorithm>
#include <assert.h>

namespace simciv
{
	AreaProd::AreaProd(): 
		p(-1),
		p_con(max_price),
		p_sup(0),
		v_con(0),
		v_sup(0),
		v(0)
	{

	}

	Producer::Producer() : storage(50), storage_last(0), storage_d(0), storage_capacity(100), prod_volume(0), _fix_price(false)
	{
	}

	void Producer::produce(double vol)
	{
		//const double a = 0.9;
		//volume = a * volume + (1 - a) * vol;
		storage += vol;
		prod_volume += vol;
	}

	void Producer::update_price()
	{
		/*
		fullness
		balance: production - trade
		producer:
		- ha a fullness túl nagy, félünk hogy megtelik, próbálunk többet eladni: --> változatlan áron többet eladni, vagy csökkenteni az árat
			--> ha free_volume == 0, vagyis mindent el tudtunk adni
				--> növeljük volume-t, árat hagyjuk békén
			--> ha free_volume > 0, már így se tudtunk mindent eladni
				--> csökkenteni kell az árat, volume-t hagyjuk békén
		- ha fullness kicsi, csökkentsük az eladást: --> megpróbálnánk növelni az árat, vagy raktározni jobb idõkre
			--> ha free_volume == 0, mindent el tudtunk adni
				--> növeljük az árat
			--> ha free_volume > 0, már így se tudtunk mindent eladni
				--> csökkentsük volumet. (abban bízunk hogy késõbb jobb áron tudunk eladni)
		*/

		//const double a = 0.9;
		//storage_d = a * storage_d + (1 - a) * (storage - storage_last);
		//storage_last = storage;

		if (_fix_price) return;

		double fullness = storage / storage_capacity;

		double vol_d = 0.1;
		double price_d = 1;

		if (is_consumer())
		{
			if (fullness < 0.5)
			{
				// want to buy
				if (free_volume > 0)
				{
					// raise price
					price += price_d;
				}
				else
				{
					// raise volume
					volume += vol_d;
				}
			}
			else
			{
				// want to buy less
				if (free_volume > 0)
				{
					//reduce volume
					volume -= vol_d;
				}
				else
				{
					// reduce price
					price -= price_d;
				}
			}
			volume = std::min(volume, free_capacity());
		}
		else
		{
			if (fullness < 0.5)
			{
				// want to store
				if (free_volume > 0)
				{
					// reduce volume
					volume -= vol_d;
				}
				else
				{
					// raise price
					price += price_d;
				}
			}
			else
			{
				// want to sell
				if (free_volume > 0)
				{
					// reduce price
					price -= price_d;
				}
				else
				{
					// raise volume
					volume += vol_d;
				}
			}
			volume = std::max(0.0, volume);
		}

		price = std::max(0.0, price);
	}

	ProductMap::ProductMap(WorldModel& world): 
		_world(world), 
		_production(new std::vector<AreaProd>()), 
		_new_production(new std::vector<AreaProd>()), 
		unique_mode(true),
		update_count(0)
	{
		int n = world.areas().size();
		_production->resize(n);
		_new_production->resize(n);
		_area_consumers.resize(n);
		_area_supplies.resize(n);
		// generate_resources();
		create_g();
	}

	void ProductMap::update()
	{
		if (update_count++ % 10 == 0)
		{
			update_routes();
		}
		update_prices();
		update_storages();
	}

	void ProductMap::create_g()
	{
		auto& areas = _world.areas();
		int n = areas.size();
		g = new Node[n];
		for (int i = 0; i < n; ++i)
		{
			Area* a = areas[i];
			g[i].area = a;
		}
	}

	Transport* ProductMap::get_transport(Producer* src, Producer* dst)
	{
		auto it = std::find_if(_routes.begin(), _routes.end(), [src, dst](const Transport* t) { return t->sup == src && t->dem == dst; });
		if (it == _routes.end())
		{
			Node& o = g[dst->area->index];
			Transport* r = new Transport();
			Node& m = g[src->area->index];
			_world.get_distances(&m, g);
			r->route = _world.get_route(&m, &o, g);
			r->trans_price = r->route->trans_price;
			r->dem = dst;
			r->sup = src;
			//r->profit = (q->price - p->price - r->trans_price) / r->trans_price;
			r->profit = dst->price - src->price - r->trans_price;
			_routes.push_back(r);
		}
		else
		{
			return *it;
		}
	}

	void ProductMap::update_routes()
	{
		for (Producer* p: _supplies)
		{
			Node& m = g[p->area->index];
			_world.get_distances(&m, g);
			for (Producer* q: _consumers)
			{
				auto t = get_transport(p, q);
			}
		}

		for (Producer* p: _supplies)
		{
			p->free_volume = p->volume;
			p->profit = max_price;
		}
		for (Producer* p: _consumers)
		{
			p->free_volume = p->volume;
			p->profit = max_price;
		}

		std::sort(_routes.begin(), _routes.end(), [](Transport* a, Transport* b) {
			return a->profit > b->profit;
		});

		for (Transport* r: _routes)
		{
			double& v_sup = r->sup->free_volume;
			double& v_con = r->dem->free_volume;
			double v = std::min(v_sup, v_con);
			if (v > 0)
			{
				v_sup -= v;
				v_con -= v;
				r->volume = v;
				auto& sup = r->sup->profit;
				sup = std::min(sup, r->profit);
				auto& con = r->dem->profit;
				con = std::min(con, r->profit);
			}
		}

		for (Producer* p: _supplies)
		{
			p->partner_price = p->price + p->profit;
		}
		for (Producer* p: _consumers)
		{
			p->partner_price = std::max(0.0, p->price - p->profit);
		}
	}

	void ProductMap::routes_to_areas(int prod_id)
	{
		for (Road* r: _world.roads())
		{
			r->t[prod_id] = 0;
		}

		for (Transport* route: _routes)
		{
			Area* a = route->sup->area;
			for (Road* r: route->route->roads)
			{
				Area* b = r->other(a);
				if (a == r->a)
				{
					r->t[prod_id] += route->volume;
				}
				else
				{
					r->t[prod_id] -= route->volume;
				}
				a = b;
			}
		}
	}

	void ProductMap::generate_resources()
	{
		for (size_t i = 0; i < _world.areas().size(); ++i)
		{
			//AreaProd& ap = (*_production)[i];
			(*_new_production)[i].resource = (*_production)[i].resource = pow( (double)rand() / RAND_MAX, 3);
		}
	}

	void ProductMap::update_prices()
	{
		for (Area* a: _world.areas())
		{
			auto& p = get_new_prod(a);
			double new_supply_price = 0; // the highest price in this area what can a supplier use (to sell the product).
			auto& v = _area_consumers[a->index];
			if (v.size() > 0)
			{
				auto it = std::max_element(v.begin(), v.end(), [](Producer* a, Producer* b){ return a->partner_price < b->partner_price; });
				new_supply_price = (*it)->partner_price;
			}

			for (Road* r: a->_roads)
			{
				Area* b = r->other(a);
				auto& bp = get_prod(b);
				new_supply_price = std::max(new_supply_price, bp.p_sup - r->t_price);
			}
			p.p_sup = new_supply_price;

			double new_cons_price = max_price; // the lowest price in this area what can a supplier use (to sell the product).
			auto& u = _area_supplies[a->index];
			if (u.size() > 0)
			{
				auto it = std::min_element(u.begin(), u.end(), [](Producer* a, Producer* b){ return a->partner_price < b->partner_price; });
				new_cons_price = (*it)->partner_price;
			}

			for (Road* r: a->_roads)
			{
				Area* b = r->other(a);
				auto& bp = get_prod(b);
				new_cons_price = std::min(new_cons_price, bp.p_con + r->t_price);
			}
			p.p_con = new_cons_price;

			p.p = (p.p_con + p.p_sup) / 2;
		}

		//*_production = *_new_production;
		 std::swap(_production, _new_production);

		 // update prices in the producers
		 for (Producer* p : _supplies)
		 {
			 p->update_price();
		 }
		 for (Producer* p : _consumers)
		 {
			 p->update_price();
		 }
	}

	void ProductMap::update_storages()
	{
		for (Transport* t : _routes)
		{
			double& vol = t->volume;
			if (vol == 0) continue;

			Producer* a = t->sup;
			Producer* b = t->dem;
			
			vol = std::min(a->storage, vol);
			vol = std::min(b->free_capacity(), vol);

			a->storage -= t->volume;
			b->storage += t->volume;
		}
	}

	Producer* ProductMap::create_prod(Area* area, double volume, double price)
	{
		Producer* p = new Producer();
		p->price = price;
		p->_is_consumer = volume < 0;
		p->volume = abs(volume);
		p->area = area;

		auto& v = p->_is_consumer ? _consumers : _supplies;
		AreaProd& a = get_prod(area);
		auto it = std::find_if(v.begin(), v.end(), [area](Producer* p) { return p->area == area; });

		if (p->_is_consumer)
		{
			_consumers.push_back(p);
			_area_consumers[area->index].push_back(p);
		}
		else
		{
			// p->volume *= a.resource;
			_supplies.push_back(p);
			_area_supplies[area->index].push_back(p);
		}

		return p;
	}

	Producer* ProductMap::add_prod(Area* area, double volume, double price)
	{
		bool consumer = volume < 0;
		volume = abs(volume);
		auto& v = consumer ? _consumers : _supplies;
		AreaProd& a = get_prod(area);
		auto it = std::find_if(v.begin(), v.end(), [area](Producer* p) { return p->area == area; });

		if (it == v.end())
		{
			return NULL;
		}
		else
		{
			Producer* p = *it;
			p->volume += volume;
			return p;
		}
	}

	void ProductMap::remove_prod(Producer* prod)
	{
		int id = prod->area->index;

		auto& u = prod->is_consumer() ? _consumers : _supplies;
		auto it = std::find(u.begin(), u.end(), prod);
		u.erase(it);

		auto& v = prod->is_consumer() ? _area_consumers[id] : _area_supplies[id];
		it = std::find(v.begin(), v.end(), prod);
		v.erase(it);

		delete prod;
	}

	void ProductMap::remove_prod(Area* area, double volume, double price)
	{
		assert(unique_mode);
		bool consumer = volume < 0;
		volume = abs(volume);
		auto& v = consumer ? _consumers : _supplies;
		AreaProd& a = get_prod(area);
		auto it = std::find_if(v.begin(), v.end(), [area](Producer* p) { return p->area == area; });

		if (it == v.end())
		{
			throw "area is empty";
		}
		else
		{
			(*it)->volume -= volume;
		}

		if ((*it)->volume == 0)
		{
			remove_prod(*it);
		}
	}

	void ProductMap::move_prod(Producer* prod, Area* new_area)
	{
		int id = prod->area->index;
		auto& v = prod->is_consumer() ? _area_consumers[id] : _area_supplies[id];
		auto it = std::find(v.begin(), v.end(), prod);
		v.erase(it);

		id = new_area->index;
		auto& u = prod->is_consumer() ? _area_consumers[id] : _area_supplies[id];
		u.push_back(prod);
	}
}