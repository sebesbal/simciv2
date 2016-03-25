#include "trade.h"

#include <algorithm>
#include <assert.h>

#include "world.h"

namespace simciv
{
	AreaData::AreaData(): 
		p(-1),
		p_buy(max_price),
		p_sell(0),
		v_buy(0),
		v_sell(0),
		v(0)
	{
	}

	Trader::Trader() :
		_storage(0),
		storage_last(0),
		storage_capacity(10000),
		storage_pair(NULL),
		ideal_fullness(-1),
		price(0),
		worst_profit(0),
		worst_price(0),
		owner(NULL),
		_d_storage(0)
	{
	}

	void Trader::modify_storage(double vol)
	{
		if (is_buyer)
		{
			set_storage(_storage - vol);
		}
		else
		{
			set_storage(_storage + vol);
		}
	}

	void Trader::set_storage(double vol)
	{
		_storage = vol;
		if (storage_pair)
		{
			storage_pair->_storage = _storage;
		}
	}

	double Trader::money()
	{
		return owner->money;
	}

void Trader::update_price()
{
	double price_d = 1;

	if (is_buyer)
	{
		price += price_d * (volume - vol_out - 0.01 * _storage);
	}
	else
	{
		price += price_d * (vol_out - volume - 0.01 * _storage);
	}

	price = std::max(1.0, price);

history:
	history_price.push_back(price);
	history_trade.push_back(volume - free_volume);
	if (history_price.size() > history_count) history_price.pop_front();
	if (history_trade.size() > history_count) history_trade.pop_front();
}

void Trader::update_volume()
{
	if (is_buyer)
	{
		if (vol_in == 0)
		{
			volume = 0;
		}
		else if (vol_out > vol_in)
		{
			if (_storage < 50.0)
			{
				volume = vol_out;
			}
			else
			{
				volume = vol_in;
			}
		}
		else
		{
			volume = vol_in;
		}
		volume = std::min(volume, free_capacity());
	}
	else
	{
		if (vol_out > vol_in)
		{
			if (_storage > 50.0)
			{
				volume = vol_out;
			}
			else
			{
				volume = vol_in;
			}
		}
		else
		{
			volume = vol_in;
		}
		volume = std::min(volume, _storage);
	}
}

	void Trader::synchronize_price()
	{
		if (storage_pair)
		{
			double p = pow(this->price * storage_pair->price, 0.5);
			const double a = 1.05;
			if (is_buyer)
			{
				this->price = p / a;
				storage_pair->price = p * a;
			}
			else 
			{
				this->price = p * a;
				storage_pair->price = p / a;
			}
		}
	}

	void Trader::update_storage()
	{
		_storage = std::max(0.0, _storage);
		history_storage.push_back(_storage);
		if (history_storage.size() > history_count) history_storage.pop_front();
	}

	TradeMap::TradeMap(Product& p) :
		_production(new std::vector<AreaData>()), 
		_new_production(new std::vector<AreaData>()), 
		unique_mode(true),
		update_count(0),
		product(p)
	{
		int n = world.areas().size();
		_production->resize(n);
		_new_production->resize(n);
		_area_buyers.resize(n);
		_area_sellers.resize(n);
		generate_resources();
	}

	void TradeMap::update()
	{
		if (update_count++ % 10 == 0)
		{
			update_transports();
		}
		update_area_prices();
		update_producer_prices();
		update_producer_storages();
	}

	Transport* TradeMap::get_transport(Trader* src, Trader* dst)
	{
		auto it = std::find_if(_transports.begin(), _transports.end(), [src, dst](const Transport* t) { return t->seller == src && t->buyer == dst; });
		if (it == _transports.end())
		{
			Transport* t = new Transport();
			t->route = world.create_route(src->area, dst->area);
			t->cost = t->route->cost;
			t->buyer = dst;
			t->seller = src;
			t->profit = dst->price - src->price - t->cost;
			_transports.push_back(t);
		}
		else
		{
			return *it;
		}
	}

	void TradeMap::update_transports()
	{
		for (Trader* p: _sellers)
		{
			//if (p->volume == 0) continue;
			for (Trader* q: _buyers)
			{
				//if (p->storage_pair == q) continue;
				// if (q->volume == 0) continue;
				auto t = get_transport(p, q);
			}
		}
	}

	void TradeMap::update_trade()
	{
		for (Trader* p : _sellers)
		{
			p->update_volume();
			p->vol_out = 0;
			p->volume = std::max(0.0, p->volume);
			p->free_volume = p->volume;
			p->worst_profit = max_price;
		}
		for (Trader* p : _buyers)
		{
			p->update_volume();
			p->vol_out = 0;
			p->free_volume = p->volume;
			p->worst_profit = max_price;
		}

		for (Transport* t : _transports)
		{
			t->profit = t->buyer->price - t->seller->price - t->cost;
			t->demand_volume = t->volume = 0;
		}

		std::sort(_transports.begin(), _transports.end(), [](Transport* a, Transport* b) {
			return a->profit > b->profit;
		});

		for (Transport* r : _transports)
		{
			if (r->profit < 0) break;
			if (r->buyer->money() <= 0) continue;
			double& v_sell = r->seller->free_volume;
			double& v_buy = r->buyer->free_volume;
			double v = std::min(v_sell, v_buy);
			double u = std::max(v_sell, v_buy);

			r->seller->vol_out += r->buyer->free_volume;
			r->buyer->vol_out += r->seller->free_volume;

			r->demand_volume = u;
			if (v > 0)
			{
				v_sell -= v;
				v_buy -= v;
				r->volume = v;
				r->active_time = world.time;
				auto& seller = r->seller->worst_profit;
				seller = std::min(seller, r->profit);
				auto& con = r->buyer->worst_profit;
				con = std::min(con, r->profit);
			}
		}

		for (Trader* p : _sellers)
		{
			if (p->worst_profit == max_price) p->worst_profit = 0;
			p->worst_price = p->price + p->worst_profit;
		}
		for (Trader* p : _buyers)
		{
			if (p->worst_profit == max_price) p->worst_profit = 0;
			p->worst_price = std::max(0.0, p->price - p->worst_profit);
		}
	}

	void TradeMap::generate_resources()
	{
		auto& v = *_production;

		for (size_t i = 0; i < world.areas().size(); ++i)
		{
			//AreaData& ap = (*_production)[i];
			//(*_new_production)[i].resource = (*_production)[i].resource = pow( (double)rand() / RAND_MAX, 3);
			v[i].resource = pow((double)rand() / RAND_MAX, 3);
		}

		for (Area* a : world.areas())
		{
			int i = a->id;
			int k = 1;
			double sum = v[i].resource;
			for (Road* r : a->roads)
			{
				sum += v[r->other(a)->id].resource;
				++k;
			}
			(*_new_production)[i].resource = v[i].resource = sum / k;
		}
	}

	void TradeMap::update_area_prices()
	{
		for (Area* a : world.areas())
		{
			auto& p = get_new_prod(a);
			double new_supply_price = 0; // the highest price in this area what can a supplier use (to sell the product).
			auto& v = _area_buyers[a->id];
			if (v.size() > 0)
			{
				auto it = std::max_element(v.begin(), v.end(), [](Trader* a, Trader* b){ return a->worst_price < b->worst_price; });
				new_supply_price = (*it)->worst_price;
			}

			for (Road* r: a->roads)
			{
				Area* b = r->other(a);
				auto& bp = get_trade(b);
				new_supply_price = std::max(new_supply_price, bp.p_sell - r->cost);
			}
			p.p_sell = new_supply_price;

			double new_cons_price = max_price; // the lowest price in this area what can a supplier use (to sell the product).
			auto& u = _area_sellers[a->id];
			if (u.size() > 0)
			{
				auto it = std::min_element(u.begin(), u.end(), [](Trader* a, Trader* b){ return a->worst_price < b->worst_price; });
				new_cons_price = (*it)->worst_price;
			}

			for (Road* r: a->roads)
			{
				Area* b = r->other(a);
				auto& bp = get_trade(b);
				new_cons_price = std::min(new_cons_price, bp.p_buy + r->cost);
			}
			p.p_buy = new_cons_price;

			p.p = (p.p_buy + p.p_sell) / 2;
		}

		//*_production = *_new_production;
		std::swap(_production, _new_production);
	}

	void TradeMap::update_area_prices2(bool full_update)
	{
		if (full_update)
		{
			find_best_producers_for_areas();
		}

		for (Area* a : world.areas())
		{
			auto& prod = get_new_prod(a);
			prod.p_buy = prod.best_seller.second ? prod.best_seller.second->price + prod.best_seller.first : max_price;
			prod.p_sell = prod.best_buyer.second ? prod.best_buyer.second->price - prod.best_buyer.first : 0;

			prod.p = (prod.p_buy + prod.p_sell) / 2;
		}

		std::swap(_production, _new_production);
	}

	void TradeMap::find_best_producers_for_areas()
	{
		for (Area* a : world.areas())
		{
			auto& prod = get_new_prod(a);

			typedef std::pair<double, Trader*> pair_t;

			// find best sellers
			{
				std::vector<pair_t> v;
				for (auto p : _sellers)
				{
					double dist = world.distance(a, p->area);
					double price = p->price + dist;
					v.push_back(pair_t(price, p));
				}

				if (v.size() == 0)
				{
					prod.best_seller.first = 0;
					prod.best_seller.second = NULL;
				}
				else
				{
					prod.best_seller = *std::min_element(v.begin(), v.end(), [](pair_t& a, pair_t& b) { return a.first < b.first; });
					prod.best_seller.first -= prod.best_seller.second->price;
				}
			}

			// find best buyers
			{
				std::vector<pair_t> v;
				for (auto p : _buyers)
				{
					double dist = world.distance(a, p->area);
					double price = p->price - dist;
					v.push_back(pair_t(price, p));
				}

				if (v.size() == 0)
				{
					prod.best_buyer.first = 0;
					prod.best_buyer.second = NULL;
				}
				else
				{
					prod.best_buyer = *std::max_element(v.begin(), v.end(), [](pair_t& a, pair_t& b) { return a.first < b.first; });
					prod.best_buyer.first = prod.best_seller.second->price - prod.best_buyer.first;
				}
			}
		}
	}

	void TradeMap::update_producer_prices()
	{
		// update prices in the producers
		for (Trader* p : _sellers)
		{
			p->update_price();
		}
		for (Trader* p : _buyers)
		{
			p->update_price();
			//p->synchronize_price();
		}
	}

	void TradeMap::before_rules()
	{
		for (Trader* p : _sellers)
		{
			p->vol_in = 0;
			
			// p->volume = p->_storage - p->storage_last + p->volume - p->free_volume;
			//p->volume = p->_d_storage;

			//p->volume = std::max(0.0, p->volume);
			p->storage_last = p->_storage;
			p->_d_storage = 0;
		}
		for (Trader* p : _buyers)
		{
			p->vol_in = 0;
			p->storage_last = p->_storage;
		}
	}

	void TradeMap::update_producer_storages()
	{
		for (Transport* t : _transports)
		{
			double& vol = t->volume;
			if (vol == 0 && t->demand_volume == 0) continue;

			Trader* a = t->seller;
			Trader* b = t->buyer;
			
			vol = std::min(a->storage(), vol);
			vol = std::min(b->free_capacity(), vol);

			a->set_storage(a->storage() - t->volume);
			b->set_storage(b->storage() + t->volume);

			a->owner->income(vol * a->price);
			b->owner->income(- vol * b->price);
		}

		for (Trader* p : _sellers)
		{
			p->update_storage();
		}
		for (Trader* p : _buyers)
		{
			p->update_storage();
		}
	}

	Trader* TradeMap::create_prod(Area* area, bool consumer, double price)
	{
		Trader* p = new Trader();
		p->product = &product;
		p->price = price;
		p->is_buyer = consumer;
		p->area = area;

		auto& v = p->is_buyer ? _buyers : _sellers;
		AreaData& a = get_trade(area);
		auto it = std::find_if(v.begin(), v.end(), [area](Trader* p) { return p->area == area; });

		if (p->is_buyer)
		{
			_buyers.push_back(p);
			_area_buyers[area->id].push_back(p);
		}
		else
		{
			// p->volume *= a.resource;
			_sellers.push_back(p);
			_area_sellers[area->id].push_back(p);
		}

		return p;
	}
}