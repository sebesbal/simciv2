
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

	Producer::Producer() :
		volume(0),
		_storage(0),
		storage_last(0),
		storage_capacity(100),
		fix_price(false),
		storage_pair(NULL),
		ideal_fullness(-1),
		free_volume(0),
		ideal_volume(0),
		price(0),
		profit(0),
		partner_price(0)

	{
		//for (int i = 0; i < 20; ++i)
		//{
		//	price_history.push_back(0);
		//	storage_history.push_back(50);
		//}
	}

	void Producer::modify_storage(double ideal_vol, double actual_vol)
	{
		if (is_consumer)
		{
			set_storage(_storage - actual_vol);
		}
		else
		{
			set_storage(_storage + actual_vol);
		}
		ideal_volume += ideal_vol;
	}

	void Producer::set_storage(double vol)
	{
		_storage = vol;
		if (storage_pair)
		{
			storage_pair->_storage = _storage;
		}
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

		//if (fix_price || is_consumer && ideal_volume == 0) goto history;

		double ideal_fullness = this->ideal_fullness < 0 ?
			20 * ideal_volume / storage_capacity
			: this->ideal_fullness;
		ideal_fullness = std::min(1.0, ideal_fullness);
		ideal_fullness = std::max(0.0, ideal_fullness);

		double fullness = _storage / storage_capacity;

		double vol_d = 0.1;
		double price_d = 1;

		if (is_consumer)
		{
			if (fullness < ideal_fullness)
			{
				if (free_volume == volume && _storage > 0) // consumed nothing
				{
					// it can't consume on this price (not profitable)
					// don't want to buy more, but it can't reduce the price
				}

				// want to buy more
				else if (free_volume == 0)  // consumed everything
				{
					// it can buy on enough on this price, raise volume
					volume += vol_d;
				}
				else
				{
					// it can't buy ebough on this price, raice price
					price += price_d;
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
			ideal_fullness = 1 - ideal_fullness;
			ideal_fullness = std::min(0.9, ideal_fullness);
			if (fullness < ideal_fullness)
			{
				if (free_volume == volume && _storage > 0) // sold nothing
				{
					// it can't sell on this price
				}

				// want to store more
				else if (free_volume > 0)
				{
					// can't sell enough, reduce volume
					volume -= vol_d;
				}
				else
				{
					// can sell everything, raise price
					price += price_d;
				}
			}
			else
			{
				// want to sell, and reduce storage
				if (free_volume > 0)
				{
					// can't sell enough, reduce price
					price -= price_d;
				}
				else
				{
					// can sell everything, raise volume on the same price
					volume += vol_d;
				}
			}
			volume = std::min(_storage, volume);
		}

		volume = std::max(0.0, volume);

		price = std::max(1.0, price);

	history:
		history_price.push_back(price);
		history_trade.push_back(volume - free_volume);
		if (history_price.size() > history_count) history_price.pop_front();
		if (history_trade.size() > history_count) history_trade.pop_front();
	}

	void Producer::update_storage()
	{
		history_storage.push_back(_storage);
		if (history_storage.size() > history_count) history_storage.pop_front();
	}

	ProductMap::ProductMap(WorldModel& world, int prod_id) :
		_world(world), 
		_production(new std::vector<AreaProd>()), 
		_new_production(new std::vector<AreaProd>()), 
		unique_mode(true),
		update_count(0),
		prod_id(prod_id)
	{
		int n = world.areas().size();
		_production->resize(n);
		_new_production->resize(n);
		_area_consumers.resize(n);
		_area_supplies.resize(n);
		// generate_resources();
		//g = _world.create_g();
	}

	void ProductMap::update()
	{
		if (update_count++ % 10 == 0)
		{
			update_transports();
		}
		update_area_prices();
		update_producer_prices();
		update_producer_storages();
	}

	Transport* ProductMap::get_transport(Producer* src, Producer* dst)
	{
		auto it = std::find_if(_transports.begin(), _transports.end(), [src, dst](const Transport* t) { return t->sup == src && t->dem == dst; });
		if (it == _transports.end())
		{
			Transport* t = new Transport();
			t->route = _world.create_route(src->area, dst->area);
			t->trans_price = t->route->trans_price;
			t->dem = dst;
			t->sup = src;
			t->profit = dst->price - src->price - t->trans_price;
			_transports.push_back(t);
		}
		else
		{
			return *it;
		}
	}

	void ProductMap::update_transports()
	{
		for (Producer* p: _supplies)
		{
			if (p->volume == 0) continue;
			for (Producer* q: _consumers)
			{
				if (p->storage_pair == q) continue;
				if (q->volume == 0) continue;
				auto t = get_transport(p, q);
			}
		}
	}

	void ProductMap::update_trade()
	{
		for (Producer* p : _supplies)
		{
			p->free_volume = p->volume;
			p->ideal_volume = 0;
			p->profit = max_price;
		}
		for (Producer* p : _consumers)
		{
			p->free_volume = p->volume;
			p->ideal_volume = 0;
			p->profit = max_price;
		}

		for (Transport* t : _transports)
		{
			t->profit = t->dem->price - t->sup->price - t->trans_price;
		}

		std::sort(_transports.begin(), _transports.end(), [](Transport* a, Transport* b) {
			return a->profit > b->profit;
		});

		for (Transport* r : _transports)
		{
			if (r->profit <= 0) break;
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

		for (Producer* p : _supplies)
		{
			if (p->profit == max_price) p->profit = 0;
			p->partner_price = p->price + p->profit;
		}
		for (Producer* p : _consumers)
		{
			if (p->profit == max_price) p->profit = 0;
			p->partner_price = std::max(0.0, p->price - p->profit);
		}
	}

	void ProductMap::routes_to_areas(int prod_id)
	{
		for (Road* r: _world.roads())
		{
			r->t[prod_id] = 0;
		}

		for (Transport* route: _transports)
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

	void ProductMap::update_area_prices()
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
	}

	void ProductMap::update_area_prices2(bool full_update)
	{
		if (full_update)
		{
			find_best_producers_for_areas();
		}

		for (Area* a : _world.areas())
		{
			auto& prod = get_new_prod(a);
			prod.p_sup = prod.best_sups.second ? prod.best_sups.second->price + prod.best_sups.first : max_price;
			prod.p_con = prod.best_cons.second ? prod.best_cons.second->price - prod.best_cons.first : 0;
			prod.p = (prod.p_con + prod.p_sup) / 2;
		}

		std::swap(_production, _new_production);
	}

	void ProductMap::find_best_producers_for_areas()
	{
		for (Area* a : _world.areas())
		{
			auto& prod = get_new_prod(a);

			typedef std::pair<double, Producer*> pair_t;

			// find best supplies
			{
				std::vector<pair_t> v;
				for (auto p : _supplies)
				{
					double dist = _world.distance(a, p->area);
					double price = p->price + dist;
					v.push_back(pair_t(price, p));
				}

				if (v.size() == 0)
				{
					prod.best_sups.first = 0;
					prod.best_sups.second = NULL;
				}
				else
				{
					prod.best_sups = *std::min_element(v.begin(), v.end(), [](pair_t& a, pair_t& b) { return a.first < b.first; });
					prod.best_sups.first -= prod.best_sups.second->price;
				}
			}

			// find best consumers
			{
				std::vector<pair_t> v;
				for (auto p : _consumers)
				{
					double dist = _world.distance(a, p->area);
					double price = p->price - dist;
					v.push_back(pair_t(price, p));
				}

				if (v.size() == 0)
				{
					prod.best_cons.first = 0;
					prod.best_cons.second = NULL;
				}
				else
				{
					prod.best_cons = *std::max_element(v.begin(), v.end(), [](pair_t& a, pair_t& b) { return a.first < b.first; });
					prod.best_cons.first = prod.best_sups.second->price - prod.best_cons.first;
				}
			}
		}
	}

	void ProductMap::update_producer_prices()
	{
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

	void ProductMap::before_rules()
	{
		for (Producer* p : _supplies)
		{
			p->ideal_volume = 0;
		}
		for (Producer* p : _consumers)
		{
			p->ideal_volume = 0;
		}
	}

	void ProductMap::update_producer_storages()
	{
		for (Transport* t : _transports)
		{
			double& vol = t->volume;
			if (vol == 0) continue;

			Producer* a = t->sup;
			Producer* b = t->dem;
			
			vol = std::min(a->storage(), vol);
			vol = std::min(b->free_capacity(), vol);

			a->set_storage(a->storage() - t->volume);
			b->set_storage(b->storage() + t->volume);
		}

		for (Producer* p : _supplies)
		{
			p->update_storage();
		}
		for (Producer* p : _consumers)
		{
			p->update_storage();
		}
	}

	Producer* ProductMap::create_prod(Area* area, bool consumer, double volume, double price)
	{
		Producer* p = new Producer();
		p->prod_id = prod_id;
		p->price = price;
		p->is_consumer = consumer;
		p->volume = volume;
		p->area = area;

		auto& v = p->is_consumer ? _consumers : _supplies;
		AreaProd& a = get_prod(area);
		auto it = std::find_if(v.begin(), v.end(), [area](Producer* p) { return p->area == area; });

		if (p->is_consumer)
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

		auto& u = prod->is_consumer ? _consumers : _supplies;
		auto it = std::find(u.begin(), u.end(), prod);
		u.erase(it);

		auto& v = prod->is_consumer ? _area_consumers[id] : _area_supplies[id];
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
		auto& v = prod->is_consumer ? _area_consumers[id] : _area_supplies[id];
		auto it = std::find(v.begin(), v.end(), prod);
		v.erase(it);

		id = new_area->index;
		auto& u = prod->is_consumer ? _area_consumers[id] : _area_supplies[id];
		u.push_back(prod);
	}
}