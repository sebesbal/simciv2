#include "animals.h"
#include <algorithm>
#include <string>
#include <algorithm>
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include <fstream>
#include <sstream>

#include <string>
#include <iostream>
#include "cocos2d.h"
using namespace std;;

namespace simciv
{
	//const int species_count = 12;
	AnimalWorld _model;
	using namespace std;

	void ProductionRule::load(rapidxml::xml_node<>* node)
	{
		this->input.clear();
		this->output.clear();

		auto item = node->first_node();
		while (item)
		{
			int id = stoi(item->first_attribute("plant")->value());
			double vol = stod(item->first_attribute("vol")->value());
			string name = item->name();
			if (name == "in")
			{
				this->input[id] = vol;
			}
			else if (name == "out")
			{
				this->output[id] = vol;
			}
			item = item->next_sibling();
		}

		//auto output = node->first_node("out");
		//item = output->first_node("dose");
		//while (item)
		//{
		//	int id = stoi(item->first_attribute("plant")->value());
		//	double vol = stod(item->first_attribute("vol")->value());
		//	this->input[id] = vol;
		//	item = item->next_sibling("dose");
		//}
	}

	double ProductionRule::profit(const Prices& prices)
	{
		double profit = 0;
		for (auto& p: input)
		{
			int prod_id = p.first;
			double price = prices.buy[prod_id];
			double vol = p.second;
			profit -= price * vol;
		}

		for (auto& p : output)
		{
			int prod_id = p.first;
			double price = prices.sell[prod_id];
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
			double price = prices.buy[prod_id];
			double vol = p.second;
			result += price * vol;
		}
		return result;
	}

	void Industry::find_best_m2m_rule(const Prices& prices, ProductionRule*& rule, double& profit)
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

	void Industry::find_best_m2a_rule(const Prices& prices, ProductionRule*& rule, double& price)
	{
		price = max_price;
		for (auto& r : m2a_rules)
		{
			double p = r.expense(prices);
			if (p < price)
			{
				price = p;
				rule = &r;
			}
		}
	}

	double Industry::get_build_cost(const Prices& prices)
	{
		double price;
		ProductionRule* rule;
		find_best_m2a_rule(prices, rule, price);
		return price * 100;
	}

	void Industry::load(rapidxml::xml_node<>* node)
	{
		maintenance_cost[0] = 1;
		type = ST_NONE;

		//auto t = node->first_attribute("type");
		//if (t)
		//{
		//	string s = string(t->value());
		//	if		(s == "mine") type = ST_MINE;
		//	else if (s == "Factory") type = ST_factory;
		//	else if (s == "storage") type = ST_STORAGE;
		//}

		auto image = node->first_attribute("image");
		if (image)
		{
			this->icon_file = "img/" + string(image->value());
		}

		auto id = node->first_attribute("id");
		if (id)
		{
			this->name = id->value();
		}

		auto n = node->first_node("produce");
		while (n)
		{
			ProductionRule rule;
			rule.load(n);
			this->m2m_rules.push_back(rule);
			n = n->next_sibling("produce");
		}

		n = node->first_node("eat");
		while (n)
		{
			ProductionRule rule;
			rule.load(n);
			rule.output[0] = 1;
			this->m2a_rules.push_back(rule);
			n = n->next_sibling("eat");
		}

		product = get_product();
	}

	Product* Industry::get_product()
	{
		if (m2m_rules.size() == 1)
		{
			int plant_id = m2m_rules[0].output.begin()->first;
			return _model.get_plants()[plant_id];
		}
		return NULL;
	}

	void Product::load(rapidxml::xml_node<>* node)
	{
		auto name = node->first_attribute("id");
		if (name) this->name = name->value();
		auto image = node->first_attribute("image");
		if (image) this->icon_file = "img/" + string(image->value());
	}

	Factory::Factory(Industry& industry) : industry(industry), money(100000000)
	{
		for (int i = 0; i < product_count; ++i)
		{
			sellers.push_back(NULL);
		}
		for (int i = 0; i < product_count; ++i)
		{
			buyers.push_back(NULL);
		}
	}
	
	void Factory::update()
	{
		//area

		//industry.find_best_m2m_rule();
	}

	double Factory::apply_rule(ProductionRule* rule, double profit, double ideal_rate)
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
			buyers[prod_id]->modify_storage(ideal_rate * vol, rate * vol);
		}

		for (auto& p : rule->output)
		{
			int prod_id = p.first;
			double vol = p.second;
			//sellers[prod_id]->modify_storage(ideal_rate * vol, rate * vol);
			//sellers[prod_id]->modify_storage(0, rate * vol);
			sellers[prod_id]->modify_storage(rate2 * vol, rate * vol);
		}

		return rate;
	}

	//double Factory::consume_article(int art_ind, Prices& prices, double& volume)
	//{
	//	double full_expense = 0;
	//	double best_expense = max_price;
	//	std::set<ProductionRule*> used;

	//	while (volume > 0)
	//	{
	//		ProductionRule* best_rule = NULL;
	//		double best_rate;
	//		for (auto& rule : industry.m2a_rules)
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
	//				buyers[prod_id]->modify_storage(volume * vol, best_rate * vol);
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
	//		for (auto& rule : industry.m2a_rules)
	//		{
	//			for (auto& p : rule.input)
	//			{
	//				int prod_id = p.first;
	//				double vol = p.second;
	//				double& v = buyers[prod_id]->volume;
	//				v = std::max(v, volume * vol);
	//			}
	//		}
	//	}

	//	return full_expense;
	//}


	double Factory::consume_article(int art_ind, Prices& prices, double& volume)
	{
		double full_expense = 0;
		typedef std::pair<double, ProductionRule*> pair_t;
		std::vector<pair_t> v;

		for (auto& rule : industry.m2a_rules)
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
				buyers[prod_id]->modify_storage(volume * vol, rate * vol);
			}
			volume -= rate;
			//income(-expense);
			if (volume <= 0) break;
		}

		return full_expense;
	}

	double Factory::consume_articles(Prices& prices)
	{
		double expense = 0;
		auto& vols = industry.maintenance_cost;
		for (auto& p : vols)
		{
			int art_id = p.first;
			double vol = p.second;
			expense += consume_article(art_id, prices, vol);
		}
		return expense;
	}

	void Factory::check_supply_storage(ProductMap& vols, double& rate)
	{
		if (rate == 0) return;
		for (auto& p : vols)
		{
			int prod_id = p.first;
			double vol = p.second;
			auto& producer = sellers[prod_id];
			rate = std::min(rate, (producer->storage_capacity - producer->storage()) / vol);
			if (rate == 0) return;
		}
	}

	void Factory::check_consumption_storage(ProductMap& vols, double& rate)
	{
		if (rate == 0) return;
		for (auto& p : vols)
		{
			int prod_id = p.first;
			double vol = p.second;
			auto& producer = buyers[prod_id];
			rate = std::min(rate, producer->storage() / vol);
			if (rate == 0) return;
		}
	}

	void Factory::check_money(double price, double& rate)
	{
		if (price > money)
		{
			rate = std::min(rate, money / price);;
		}
	}

	Prices Factory::get_prices()
	{
		Prices p;
		for (int i = 0; i < product_count; ++i)
		{
			p.sell[i] = this->sellers[i]->price;
			p.buy[i] = this->buyers[i]->price;
		}
		return p;
	}

	void Factory::income(double money)
	{
		this->money += money;
		if (this->money < 0) this->money = 0;
	}

	void AnimalWorld::create(int width, int height, int prod_count)
	{
		generate_species();
		prod_count = product_count;
		Map::create(width, height, prod_count);
		for (int i = 0; i < prod_count; ++i)
		{
			_trade_maps.push_back(new TradeMap(i));
		}
		generate_factories();
	}

	/*
	void AnimalWorld::generate_species()
	{
		for (int i = 0; i < species_count; ++i)
		{
			Industry s;
			s.id = i;
			for (int j = 0; j < _trade_maps.size(); ++j)
			{
				double d = (double)rand() / RAND_MAX;
				int n = 3;
				int k = std::floor((2 * n + 1) * d - n);
				s.production.push_back(k);
				double e = (double)rand() / RAND_MAX;
				s.maintenance_cost.push_back(e);
			}
			industry.push_back(s);
		}
	}
	*/

	/* 
	materials:
	0 = food
	1 = manpower
	2 = wood
	3 = stone

	factories:
	0 = house:	food --> manpower
	1 = farm:   manpower --> food
	2 = mine:	manpower --> stone
	3 = lodge:	manpower --> wood


	a: _ --> 1
	b: 

	*/

	//void AnimalWorld::generate_species()
	//{
	//	const int cost[4] = { 1, 2, 4, 8 };
	//	const int output[5] = { 1, 3, 5, 8, 13 };
	//	auto id = [](int level, int color) {
	//		return level * color_count + (color + color_count) % color_count;
	//	};
	//	auto next_color = [](int color) {
	//		return (color + 1) % color_count;
	//	};

	//	// generate article rules
	//	std::vector<ProductionRule> art_rules;
	//	for (int i = 0; i < level_count; ++i)
	//	{
	//		ProductionRule rule;
	//		rule.input[id(i, 0)] = pow(0.5, i);
	//		rule.output[0] = 1;
	//		art_rules.push_back(rule);
	//	}

	//	//ProductionRule rule;
	//	//rule.input[0] = 1;
	//	//rule.output[0] = 1;
	//	//art_rules.push_back(rule);

	//	// generate maintenance
	//	ProductMap maintenance;
	//	maintenance[0] = 0.5;

	//	for (int level = 0; level < level_count; ++level)
	//	{
	//		for (int color = 0; color < color_count; ++color)
	//		{
	//			Industry s;
	//			s.level = level;
	//			s.color = color;
	//			s.type = ST_TYPECOLOR;
	//			s.maintenance_cost = maintenance;
	//			s.m2a_rules = art_rules;

	//			ProductionRule r;
	//			if (level > 0)
	//			{
	//				r.input[id(level - 1, color)] = 1;
	//			}
	//			r.output[id(level, color)] = 1;
	//			s.m2m_rules.push_back(r);
	//			s.icon_file = "img/shapes/shape_" + std::to_string(level) + "_" + std::to_string(color) + ".png";

	//			industry.push_back(s);
	//		}
	//	}

	//	Industry s;
	//	s.type = ST_STORAGE;
	//	s.icon_file = "img/shapes/storage.png";
	//	industry.push_back(s);
	//}



string ExePath() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}

	void AnimalWorld::generate_species()
	{
		CCLOG("ExePath() %s", ExePath());
		load_from_file("res\\mod2.xml");
		//load_from_file("C:\\dev\\simciv2\\proj.win32\\Debug.win32\\res\\mod1.xml");
	}

	//void AnimalWorld::generate_factories()
	//{
	//	int x = 10, y = 10;

	//	auto s1 = get_species("1");
	//	create_factory(get_area(x + 4, y), *s1);

	//	auto s0 = get_species("0");
	//	create_factory(get_area(x, y), *s0);

	//	////create_factory(get_area(x, y+1), *s0);

	//	//

	//	auto s2 = get_species("2");
	//	create_factory(get_area(x + 2, y + 3), *s2);

	//	////auto storage = get_storage_species();
	//	////create_factory(get_area(x + 2, y - 3), *storage);
	//}

	void AnimalWorld::generate_factories()
	{
		int x = 12, y = 10;

		auto s1 = get_species("3");
		create_factory(get_area(x, y), *s1);
	}

	Factory* AnimalWorld::create_factory(Area* a, Industry& industry)
	{
		if (find_factory(a)) return NULL;
		Factory* ani = new Factory(industry);
		factories.push_back(ani);
		ani->area = a;

		// create all producers:
		for (int i = 0; i < product_count; ++i)
		{
			auto p = ani->sellers[i] = _trade_maps[i]->create_prod(a, false, 0, 50);
			p->storage_capacity = 10000;
			auto q = ani->buyers[i] = _trade_maps[i]->create_prod(a, true, 0, 50);
			q->storage_capacity = 10000;
			p->storage_pair = q;
			q->storage_pair = p;
			p->owner = q->owner = ani;
			if (industry.type == ST_STORAGE)
			{
				// p->set_storage(p->storage_capacity / 2);
				q->ideal_fullness = p->ideal_fullness = 0.5;
			}
		}

		return ani;
	}

	Factory* AnimalWorld::find_factory(Area* a)
	{
		auto it = find_if(factories.begin(), factories.end(), [a](Factory* ani){ return ani->area == a; });
		if (it == factories.end())
		{
			return NULL;
		}
		else
		{
			return *it;
		}
	}

	//Industry* AnimalWorld::get_species(int level, int color)
	//{
	//	for (auto& s : industry)
	//	{
	//		if (s.level == level && s.color == color) return &s;
	//	}
	//	return NULL;
	//}

	//void AnimalWorld::update()
	//{
	//	// update product maps
	//	Map::update();

	//	for (Factory* ani : factories)
	//	{
	//		Area* area = ani->area;
	//		Prices prices = get_prices(area);
	//		auto rule = ani->industry.find_best_m2m_rule(prices);
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
		++time;

		for (TradeMap* product : _trade_maps)
		{
			product->before_rules();
		}

		//if (k % 100 == 0)
		{
			for (TradeMap* product : _trade_maps)
			{
				product->update_transports();
			}
		}

		//if (k % 10 == 0)
		//for (TradeMap* product : _trade_maps)
		//{
		//	product->update_area_prices();
		//}

		if (k % 10 == 0)
		for (TradeMap* product : _trade_maps)
		{
			product->update_area_prices2(k % 100);
		}

		for (Factory* ani : factories)
		{
			Prices prices = ani->get_prices();
			ProductionRule* rule;
			double profit;
			ani->industry.find_best_m2m_rule(prices, rule, profit);
			if (rule)
			{
				ani->apply_rule(rule, profit, 1);
			}

			double expense = ani->consume_articles(prices);
		}

		//if (k % 5 == 0)
		{
			for (TradeMap* product : _trade_maps)
			{
				product->update_trade();
			}
		}

		for (TradeMap* product : _trade_maps)
		{
			product->update_producer_storages();
			product->update_producer_prices();
		}

		++k;
	}

	using namespace rapidxml;
	using namespace std;

	void AnimalWorld::load_from_file(std::string file_name)
	{
		rapidxml::xml_document<> doc;												// character type defaults to char
		std::ifstream file(file_name);
		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();
		std::string content(buffer.str());
		doc.parse<0>(&content[0]);
		xml_node<> *root = doc.first_node("simciv");

		xml_node<> *item = root->first_node();
		while (item)
		{
			string name = item->name();
			if (name == "species")
			{
				Industry* s = new Industry();
				s->load(item);
				add_species(s);
			}
			else if (name == "plant")
			{
				Product* plant = new Product();
				plant->load(item);
				add_plant(plant);
			}

			item = item->next_sibling();
		}

		product_count = plants.size();
	}

	Prices AnimalWorld::get_prices(Area* a)
	{
		Prices p;
		for (int i = 0; i < product_count; ++i)
		{
			auto& prod = this->get_trade(a, i);
			p.sell[i] = prod.p_sell;
			p.buy[i] = prod.p_buy;
		}
		return p;
	}

	double AnimalWorld::get_profit(Industry* industry, Area* a)
	{
		Prices prices = get_prices(a);
		double profit;
		ProductionRule* rule;
		industry->find_best_m2m_rule(prices, rule, profit);
		double price;
		industry->find_best_m2a_rule(prices, rule, price);
		profit -= price;
		return profit;
	}

	double AnimalWorld::get_build_cost(Industry* industry, Area* a)
	{
		return industry->get_build_cost(get_prices(a));
	}

	double AnimalWorld::get_resources(Industry* industry, Area* a)
	{
		Product* p = industry->product;
		if (p)
		{
			return get_trade(a, p->id).resource;
		}
		else
		{
			return 0;
		}
	}

	AreaTrade& AnimalWorld::get_trade(Area* a, int id)
	{
		return _trade_maps[id]->get_trade(a);
	}

	void AnimalWorld::move_factory(Factory* ani, Area* new_area)
	{
		for (size_t i = 0; i < _trade_maps.size(); ++i)
		{
			_trade_maps[i]->move_prod(ani->sellers[i], new_area);
			_trade_maps[i]->move_prod(ani->buyers[i], new_area);
		}
		ani->area = new_area;
	}

	//double check_supply_storage(Factory* ani, ProductMap& vols)
	//{
	//	double rate = 1;
	//	for (auto& p : vols)
	//	{
	//		int prod_id = p.first;
	//		double vol = p.second;
	//		auto& producer = ani->sellers[prod_id];
	//		double d = producer->storage_capacity - producer->storage;
	//		if (d == 0) return 0;
	//		rate = std::min(rate, d / vol);
	//	}
	//	return rate;
	//}

	//double check_consumption_storage(Factory* ani, ProductMap& vols)
	//{
	//	double rate = 1;
	//	for (auto& p : vols)
	//	{
	//		int prod_id = p.first;
	//		double vol = p.second;
	//		auto& producer = ani->buyers[prod_id];
	//		double d = producer->storage;
	//		if (d == 0) return 0;
	//		rate = std::min(rate, d / vol);
	//	}
	//	return rate;
	//}

	//void AnimalWorld::update_factory_m2m(Factory* ani, ProductionRule* rule)
	//{
	//	//double rate = check_consumption_storage(ani, rule->input);
	//	//if (rate == 0) return;
	//	//rate = std::min(rate, check_supply_storage(ani, rule->output));
	//	//ani->apply_rule(rule, rate);

	//	double rate = 1;
	//	ani->apply_rule(rule, rate);
	//}

	//void AnimalWorld::update_factory_m2a(Factory* ani, ProductMap& vols, Prices& prices)
	//{
	//	auto& s = ani->industry;
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

	//Prices AnimalWorld::get_prices(Area* a)
	//{
	//	Prices result;
	//	for (int i = 0; i < product_count; ++i)
	//	{
	//		auto& info = get_trade(a, i);
	//		result.supply[i] = info.p_sell;
	//		result.consumption[i] = info.p_buy;
	//	}
	//	return result;
	//}
}