#include "animals.h"
#include <algorithm>
#include <string>
#include <algorithm>

namespace simciv
{
	const int species_count = 12;

	double ProductionRule::profit(const Prices& prices)
	{
		double profit = 0;
		for (auto& p: input)
		{
			int prod_id = p.first;
			double price = prices.consumption[prod_id];
			double vol = p.second;
			profit -= price * vol;
		}

		for (auto& p : output)
		{
			int prod_id = p.first;
			double price = prices.supply[prod_id];
			double vol = p.second;
			profit += price * vol;
		}

		return profit;
	}

	double ProductionRule::expense(const Prices& prices)
	{
		double result = 0;
		for (auto& p : input)
		{
			int prod_id = p.first;
			double price = prices.consumption[prod_id];
			double vol = p.second;
			result += price * vol;
		}
		return result;
	}

	void Species::find_best_m2m_rule(const Prices& prices, ProductionRule*& rule, double& profit)
	// ProductionRule* Species::find_best_m2m_rule(const Prices& prices)
	{
		double best_profit = 0;
		ProductionRule* best_rule = NULL;
		for (auto& rule : m2m_rules)
		{
			double profit = rule.profit(prices);
			if (profit > best_profit)
			{
				best_profit = profit;
				best_rule = &rule;
			}
		}
		rule = best_rule;
		profit = best_profit;
	}

	Animal::Animal(Species& species) : species(species), money(100000000)
	{
		for (int i = 0; i < material_count; ++i)
		{
			supplies.push_back(NULL);
		}
		for (int i = 0; i < material_count; ++i)
		{
			consumers.push_back(NULL);
		}
	}
	
	void Animal::update()
	{
		//area

		//species.find_best_m2m_rule();
	}

	double Animal::apply_rule(ProductionRule* rule, double profit, double ideal_rate)
	{
		double rate = ideal_rate;
		//if (rate == 0) return 0;
		check_consumption_storage(rule->input, rate);
		//if (rate == 0) return 0;
		double rate2 = rate;
		check_supply_storage(rule->output, rate);
		//if (rate == 0) return 0;

		for (auto& p : rule->input)
		{
			int prod_id = p.first;
			double vol = p.second;
			consumers[prod_id]->modify_storage(ideal_rate * vol, rate * vol);
		}

		for (auto& p : rule->output)
		{
			int prod_id = p.first;
			double vol = p.second;
			//supplies[prod_id]->modify_storage(ideal_rate * vol, rate * vol);
			//supplies[prod_id]->modify_storage(0, rate * vol);
			supplies[prod_id]->modify_storage(rate2 * vol, rate * vol);
		}

		return rate;
	}

	//double Animal::consume_article(int art_ind, Prices& prices, double& volume)
	//{
	//	double full_expense = 0;
	//	double best_expense = max_price;
	//	std::set<ProductionRule*> used;

	//	while (volume > 0)
	//	{
	//		ProductionRule* best_rule = NULL;
	//		double best_rate;
	//		for (auto& rule : species.m2a_rules)
	//		{
	//			double rate = volume;
	//			check_consumption_storage(rule.input, rate);
	//			if (rate > 0)
	//			{
	//				double exp = rule.expense(prices);
	//				if (exp < best_expense)
	//				{
	//					best_expense = exp;
	//					best_rule = &rule;
	//					best_rate = rate;
	//				}
	//			}
	//		}

	//		if (best_rule)
	//		{
	//			full_expense += best_expense;
	//			for (auto& p : best_rule->input)
	//			{
	//				int prod_id = p.first;
	//				double vol = p.second;
	//				consumers[prod_id]->modify_storage(volume * vol, best_rate * vol);
	//			}
	//			volume -= best_rate;
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}

	//	if (volume > 0)
	//	{
	//		for (auto& rule : species.m2a_rules)
	//		{
	//			for (auto& p : rule.input)
	//			{
	//				int prod_id = p.first;
	//				double vol = p.second;
	//				double& v = consumers[prod_id]->volume;
	//				v = std::max(v, volume * vol);
	//			}
	//		}
	//	}

	//	return full_expense;
	//}


	double Animal::consume_article(int art_ind, Prices& prices, double& volume)
	{
		double full_expense = 0;
		typedef std::pair<double, ProductionRule*> pair_t;
		std::vector<pair_t> v;

		for (auto& rule : species.m2a_rules)
		{
			v.push_back(pair_t(rule.expense(prices), &rule));
		}

		std::sort(v.begin(), v.end(), [](pair_t& a, pair_t& b) { return a.first < b.first; });

		for (auto& p : v)
		{
			double rate = volume;
			double expense = p.first;
			auto rule = p.second;
			check_consumption_storage(rule->input, rate);
			expense *= rate;
			//check_money(expense, rate);
			full_expense += expense;
			for (auto& p : rule->input)
			{
				int prod_id = p.first;
				double vol = p.second;
				consumers[prod_id]->modify_storage(volume * vol, rate * vol);
			}
			volume -= rate;
			//income(-expense);
			if (volume <= 0) break;
		}

		return full_expense;
	}

	double Animal::consume_articles(Prices& prices)
	{
		double expense = 0;
		auto& vols = species.maintenance_cost;
		for (auto& p : vols)
		{
			int art_id = p.first;
			double vol = p.second;
			expense += consume_article(art_id, prices, vol);
		}
		return expense;
	}

	void Animal::check_supply_storage(MaterialMap& vols, double& rate)
	{
		if (rate == 0) return;
		for (auto& p : vols)
		{
			int prod_id = p.first;
			double vol = p.second;
			auto& producer = supplies[prod_id];
			rate = std::min(rate, (producer->storage_capacity - producer->storage()) / vol);
			if (rate == 0) return;
		}
	}

	void Animal::check_consumption_storage(MaterialMap& vols, double& rate)
	{
		if (rate == 0) return;
		for (auto& p : vols)
		{
			int prod_id = p.first;
			double vol = p.second;
			auto& producer = consumers[prod_id];
			rate = std::min(rate, producer->storage() / vol);
			if (rate == 0) return;
		}
	}

	void Animal::check_money(double price, double& rate)
	{
		if (price > money)
		{
			rate = std::min(rate, money / price);;
		}
	}

	Prices Animal::get_prices()
	{
		Prices p;
		for (int i = 0; i < material_count; ++i)
		{
			p.supply[i] = this->supplies[i]->price;
			p.consumption[i] = this->consumers[i]->price;
		}
		return p;
	}

	void Animal::income(double money)
	{
		this->money += money;
		if (this->money < 0) this->money = 0;
	}

	void AnimalWorld::create_map(int width, int height, int prod_count)
	{
		WorldModel::create_map(width, height, prod_count);
		generate_species();
		generate_animals();
	}

	/*
	void AnimalWorld::generate_species()
	{
		for (int i = 0; i < species_count; ++i)
		{
			Species s;
			s.id = i;
			for (int j = 0; j < _products.size(); ++j)
			{
				double d = (double)rand() / RAND_MAX;
				int n = 3;
				int k = std::floor((2 * n + 1) * d - n);
				s.production.push_back(k);
				double e = (double)rand() / RAND_MAX;
				s.maintenance_cost.push_back(e);
			}
			species.push_back(s);
		}
	}
	*/

	/* 
	materials:
	0 = food
	1 = manpower
	2 = wood
	3 = stone

	animals:
	0 = house:	food --> manpower
	1 = farm:   manpower --> food
	2 = mine:	manpower --> stone
	3 = lodge:	manpower --> wood


	a: _ --> 1
	b: 

	*/

	void AnimalWorld::generate_species()
	{
		const int cost[4] = { 1, 2, 4, 8 };
		const int output[5] = { 1, 3, 5, 8, 13 };
		auto id = [](int level, int color) {
			return level * color_count + (color + color_count) % color_count;
		};
		auto next_color = [](int color) {
			return (color + 1) % color_count;
		};

		// generate article rules
		std::vector<ProductionRule> art_rules;
		for (int i = 0; i < level_count; ++i)
		{
			ProductionRule rule;
			rule.input[id(i, 0)] = pow(0.5, i);
			rule.output[0] = 1;
			art_rules.push_back(rule);
		}

		//ProductionRule rule;
		//rule.input[0] = 1;
		//rule.output[0] = 1;
		//art_rules.push_back(rule);

		// generate maintenance
		MaterialMap maintenance;
		maintenance[0] = 0.5;

		for (int level = 0; level < level_count; ++level)
		{
			for (int color = 0; color < color_count; ++color)
			{
				Species s;
				s.level = level;
				s.color = color;
				s.type = ST_TYPECOLOR;
				s.maintenance_cost = maintenance;
				s.m2a_rules = art_rules;

				ProductionRule r;
				if (level > 0)
				{
					r.input[id(level - 1, color)] = 1;
				}
				r.output[id(level, color)] = 1;
				s.m2m_rules.push_back(r);
				s.icon_file = "img/shapes/shape_" + std::to_string(level) + "_" + std::to_string(color) + ".png";

				species.push_back(s);
			}
		}

		Species s;
		s.type = ST_STORAGE;
		s.icon_file = "img/shapes/storage.png";
		species.push_back(s);
	}

	void AnimalWorld::generate_animals()
	{
		int x = 10, y = 10;

		auto s1 = get_species(1, 0);
		create_animal(get_area(x + 4, y), *s1);

		auto s0 = get_species(0, 0);
		create_animal(get_area(x, y), *s0);

		create_animal(get_area(x, y+1), *s0);

		

		//auto s2 = get_species(2, 0);
		//create_animal(get_area(x + 2, y + 3), *s2);

		//auto storage = get_storage_species();
		//create_animal(get_area(x + 2, y - 3), *storage);
	}

	Animal* AnimalWorld::create_animal(Area* a, Species& species)
	{
		if (find_animal(a)) return NULL;
		Animal* ani = new Animal(species);
		animals.push_back(ani);
		ani->area = a;

		// create all producers:
		for (int i = 0; i < material_count; ++i)
		{
			auto p = ani->supplies[i] = _products[i]->create_prod(a, false, 0, 50);
			p->storage_capacity = species.type == ST_TYPECOLOR ? 10000 : 200;
			auto q = ani->consumers[i] = _products[i]->create_prod(a, true, 0, 50);
			q->storage_capacity = species.type == ST_TYPECOLOR ? 10000 : 200;
			p->storage_pair = q;
			q->storage_pair = p;
			p->owner = q->owner = ani;
			if (species.type == ST_STORAGE)
			{
				// p->set_storage(p->storage_capacity / 2);
				q->ideal_fullness = p->ideal_fullness = 0.5;
			}
		}

		return ani;
	}

	Animal* AnimalWorld::find_animal(Area* a)
	{
		auto it = find_if(animals.begin(), animals.end(), [a](Animal* ani){ return ani->area == a; });
		if (it == animals.end())
		{
			return NULL;
		}
		else
		{
			return *it;
		}
	}

	Species* AnimalWorld::get_species(int level, int color)
	{
		for (auto& s : species)
		{
			if (s.level == level && s.color == color) return &s;
		}
		return NULL;
	}

	//void AnimalWorld::update()
	//{
	//	// update product maps
	//	WorldModel::update();

	//	for (Animal* ani : animals)
	//	{
	//		Area* area = ani->area;
	//		Prices prices = get_prices(area);
	//		auto rule = ani->species.find_best_m2m_rule(prices);
	//		if (rule)
	//		{
	//			double rate = 1;
	//			ani->apply_rule(rule, rate);
	//		}

	//		double expense = ani->consume_articles(prices);
	//	}
	//}


	void AnimalWorld::update()
	{
		static int k = 0;

		for (ProductMap* product : _products)
		{
			product->before_rules();
		}

		//if (k % 100 == 0)
		{
			for (ProductMap* product : _products)
			{
				product->update_transports();
			}
		}

		//if (k % 10 == 0)
		//for (ProductMap* product : _products)
		//{
		//	product->update_area_prices();
		//}

		if (k % 10 == 0)
		for (ProductMap* product : _products)
		{
			product->update_area_prices2(k % 100);
		}

		for (Animal* ani : animals)
		{
			if (ani->species.type == ST_TYPECOLOR)
			{
				Prices prices = ani->get_prices();
				ProductionRule* rule;
				double profit;
				ani->species.find_best_m2m_rule(prices, rule, profit);
				if (rule)
				{
					ani->apply_rule(rule, profit, 1);
				}

				double expense = ani->consume_articles(prices);
			}
		}

		//if (k % 5 == 0)
		{
			for (ProductMap* product : _products)
			{
				product->update_trade();
			}
		}

		for (ProductMap* product : _products)
		{
			product->update_producer_storages();
			product->update_producer_prices();
		}

		++k;
	}

	void AnimalWorld::move_animal(Animal* ani, Area* new_area)
	{
		for (size_t i = 0; i < _products.size(); ++i)
		{
			_products[i]->move_prod(ani->supplies[i], new_area);
			_products[i]->move_prod(ani->consumers[i], new_area);
		}
		ani->area = new_area;
	}

	//double check_supply_storage(Animal* ani, MaterialMap& vols)
	//{
	//	double rate = 1;
	//	for (auto& p : vols)
	//	{
	//		int prod_id = p.first;
	//		double vol = p.second;
	//		auto& producer = ani->supplies[prod_id];
	//		double d = producer->storage_capacity - producer->storage;
	//		if (d == 0) return 0;
	//		rate = std::min(rate, d / vol);
	//	}
	//	return rate;
	//}

	//double check_consumption_storage(Animal* ani, MaterialMap& vols)
	//{
	//	double rate = 1;
	//	for (auto& p : vols)
	//	{
	//		int prod_id = p.first;
	//		double vol = p.second;
	//		auto& producer = ani->consumers[prod_id];
	//		double d = producer->storage;
	//		if (d == 0) return 0;
	//		rate = std::min(rate, d / vol);
	//	}
	//	return rate;
	//}

	//void AnimalWorld::update_animal_m2m(Animal* ani, ProductionRule* rule)
	//{
	//	//double rate = check_consumption_storage(ani, rule->input);
	//	//if (rate == 0) return;
	//	//rate = std::min(rate, check_supply_storage(ani, rule->output));
	//	//ani->apply_rule(rule, rate);

	//	double rate = 1;
	//	ani->apply_rule(rule, rate);
	//}

	//void AnimalWorld::update_animal_m2a(Animal* ani, MaterialMap& vols, Prices& prices)
	//{
	//	auto& s = ani->species;
	//	auto& rules = s.m2a_rules;

	//	for (auto& p : vols)
	//	{
	//		int art_id = p.first;
	//		double vol = p.second;
	//		double best_expense = 100000;
	//		bool rule_found = true;

	//		while (vol > 0 && rule_found)
	//		for (auto& r : rules)
	//		{
	//			if ()
	//			double e = r.expense(prices);
	//		}

	//	}
	//}

	Prices AnimalWorld::get_prices(Area* a)
	{
		Prices result;
		for (int i = 0; i < material_count; ++i)
		{
			auto& info = get_prod(a, i);
			result.supply[i] = info.p_sup;
			result.consumption[i] = info.p_con;
		}
		return result;
	}
}