#include "world.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "cocos2d.h"

using namespace std;;

namespace simciv
{
	//const int industry_count = 12;
	World world;
	int max_road_level = 5;
	using namespace std;

	void ProductionRule::load(rapidxml::xml_node<>* node)
	{
		this->input.clear();
		this->output.clear();

		auto item = node->first_node();
		while (item)
		{
			string id = item->first_attribute("product")->value();
			int index = world.get_product(id)->id;
			double vol = stod(item->first_attribute("vol")->value());
			string name = item->name();
			if (name == "in")
			{
				this->input[index] = vol;
			}
			else if (name == "out")
			{
				this->output[index] = vol;
			}
			item = item->next_sibling();
		}

		//auto output = node->first_node("out");
		//item = output->first_node("dose");
		//while (item)
		//{
		//	int id = stoi(item->first_attribute("product")->value());
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

	ProductionRule ProductionRule::multiply(double multiply_input, double multiply_outut)
	{
		ProductionRule rule = *this;
		for (auto& p : rule.input)
		{
			p.second *= multiply_input;
		}
		for (auto& p : rule.output)
		{
			p.second *= multiply_outut;
		}
		return rule;
	}

	Industry::Industry()
	{
		lifetime = 1000;
		buildtime = 1000;
		type = IndustryType::IT_NONE;
	}

	void Industry::find_best_prod_rule(const Prices& prices, Area* area, ProductionRule*& rule, double& profit)
	{
		double best_profit = 0;
		ProductionRule* best_rule = NULL;
		for (auto& rule : prod_rules)
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

		if (product)
		{
			profit *= area->data(product).resource;
		}
	}

	void Industry::find_best_maint_rule(const Prices& prices, ProductionRule*& rule, double& price)
	{
		price = max_price;
		for (auto& r : maint_rules)
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
		find_best_maint_rule(prices, rule, price);
		return price * 100;
	}

	void Industry::load(rapidxml::xml_node<>* node)
	{
		if (auto a = node->first_attribute("image"))
		{
			icon_file = "img/" + string(a->value());
		}

		if (auto a = node->first_attribute("id"))
		{
			name = a->value();
		}

		if (auto a = node->first_attribute("name"))
		{
			display_name = a->value();
		}
		 
		if (auto a = node->first_attribute("lifetime"))
		{
			lifetime = stoi(a->value());
		}

		if (auto a = node->first_attribute("buildtime"))
		{
			buildtime = stoi(a->value());
		}

		auto n = node->first_node("produce");
		while (n)
		{
			ProductionRule rule;
			rule.load(n);
			prod_rules.push_back(rule);
			n = n->next_sibling("produce");
		}

		n = node->first_node("maint");
		while (n)
		{
			ProductionRule rule;
			rule.load(n);
			rule.output[0] = 1;
			maint_rules.push_back(rule);
			n = n->next_sibling("maint");
		}

		n = node->first_node("build");
		if (!n)
		{
			for (auto& r : maint_rules)
			{
				build_total_cost.push_back(r.multiply(lifetime, 1));
			}
		}
		else
		{
			while (n)
			{
				ProductionRule rule;
				rule.load(n);
				rule.output[0] = 1;
				this->build_total_cost.push_back(rule);
				n = n->next_sibling("build");
			}
		}

		for (auto& r : build_total_cost)
		{
			build_rules.push_back(r.multiply(1.0 / buildtime, 1));
		}

		product = get_product();
	}

	Product* Industry::get_product()
	{
		if (prod_rules.size() == 1)
		{
			int product_id = prod_rules[0].output.begin()->first;
			return world.get_products()[product_id];
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

	Factory::Factory(Industry& industry) : industry(industry), money(100000000), efficiency(1), is_under_construction(true)
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

		//industry.find_best_prod_rule();
	}

	double Factory::apply_rule(ProductionRule* rule, double profit, double ideal_rate)
	{
		double rate = ideal_rate;
		//if (rate == 0) return 0;
		check_buyer_storage(rule->input, rate);
		//if (rate == 0) return 0;
		double rate2 = rate;
		check_seller_storage(rule->output, rate);
		//if (rate == 0) return 0;

		for (auto& p : rule->input)
		{
			int prod_id = p.first;
			double vol = p.second;
			buyers[prod_id]->modify_storage(rate * vol);
			buyers[prod_id]->vol_in += ideal_rate * vol;
		}

		for (auto& p : rule->output)
		{
			int prod_id = p.first;
			double vol = p.second;
			sellers[prod_id]->modify_storage(rate * vol);
			sellers[prod_id]->vol_in += rate2 * vol;
		}

		return rate;
	}

	double Factory::consume_articles(Prices& prices)
	{
		double full_expense = 0;
		typedef std::pair<double, ProductionRule*> pair_t;
		std::vector<pair_t> v;
		auto& rules = is_under_construction ? industry.build_rules : industry.maint_rules;

		for (auto& rule : rules)
		{
			v.push_back(pair_t(rule.expense(prices), &rule));
		}

		std::sort(v.begin(), v.end(), [](pair_t& a, pair_t& b) { return a.first < b.first; });

		double volume = 1;
		for (auto& p : v)
		{
			double rate = volume;
			double expense = p.first;
			auto rule = p.second;
			check_buyer_storage(rule->input, rate);
			expense *= rate;
			full_expense += expense;
			for (auto& p : rule->input)
			{
				int prod_id = p.first;
				double vol = p.second;
				buyers[prod_id]->modify_storage(rate * vol);
				buyers[prod_id]->vol_in += volume * vol;
			}
			volume -= rate;
			if (volume <= 0) break;
		}

		return full_expense;
	}

	void Factory::check_seller_storage(ProductMap& vols, double& rate)
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

	void Factory::check_buyer_storage(ProductMap& vols, double& rate)
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

	void Factory::find_best_prod_rule(const Prices& prices, ProductionRule*& rule, double& profit)
	{
		industry.find_best_prod_rule(prices, area, rule, profit);
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

	void World::create(int width, int height, int prod_count)
	{
		generate_industry();
		prod_count = product_count;
		Map::create(width, height, prod_count);
		for (int i = 0; i < prod_count; ++i)
		{
			_trade_maps.push_back(new TradeMap(*products[i]));
		}
		generate_factories();
	}

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

string ExePath() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}

	void World::generate_industry()
	{
		CCLOG("ExePath() %s", ExePath());
		load_from_file("res/mod3.xml");
	}

	void World::generate_factories()
	{
		int x = 12, y = 10;

		auto s1 = get_industry("market");
		auto f = create_factory(get_area(x, y), *s1);
		f->is_under_construction = false;
		f->buyers[world.get_product("food")->id]->set_storage(10000);
	}

	Factory* World::create_factory(Area* a, Industry& industry)
	{
		if (find_factory(a)) return NULL;
		Factory* f = new Factory(industry);
		factories.push_back(f);
		f->area = a;
		if (industry.product)
		{
			f->efficiency = a->data(industry.product).resource;
		}

		// create all producers:
		for (int i = 0; i < product_count; ++i)
		{
			auto p = f->sellers[i] = _trade_maps[i]->create_prod(a, false, 50);
			p->storage_capacity = 10000;
			auto q = f->buyers[i] = _trade_maps[i]->create_prod(a, true, 50);
			q->storage_capacity = 10000;
			p->storage_pair = q;
			q->storage_pair = p;
			p->owner = q->owner = f;
			if (industry.type == IT_STORAGE)
			{
				// p->set_storage(p->storage_capacity / 2);
				q->ideal_fullness = p->ideal_fullness = 0.5;
			}
		}

		return f;
	}

	Factory* World::find_factory(Area* a)
	{
		auto it = find_if(factories.begin(), factories.end(), [a](Factory* f){ return f->area == a; });
		if (it == factories.end())
		{
			return NULL;
		}
		else
		{
			return *it;
		}
	}

	void World::update()
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

		for (Factory* f : factories)
		{
			Prices prices = f->get_prices();

			if (!f->is_under_construction)
			{
				ProductionRule* rule;
				double profit;
				f->find_best_prod_rule(prices, rule, profit);
				if (rule)
				{
					f->apply_rule(rule, profit, f->efficiency);
				}
			}

			double expense = f->consume_articles(prices);
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

	void World::load_from_file(std::string file_name)
	{
		rapidxml::xml_document<> doc; // character type defaults to char
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
			if (name == "industry")
			{
				Industry* s = new Industry();
				s->load(item);
				add_industry(s);
			}
			else if (name == "product")
			{
				Product* product = new Product();
				product->load(item);
				add_product(product);
			}

			item = item->next_sibling();
		}

		product_count = products.size();
	}

	Prices World::get_prices(Area* a)
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

	double World::get_profit(Industry* industry, Area* a)
	{
		Prices prices = get_prices(a);
		double profit;
		ProductionRule* rule;
		industry->find_best_prod_rule(prices, a, rule, profit);
		double price;
		industry->find_best_maint_rule(prices, rule, price);
		profit -= price;
		return profit;
	}

	double World::get_build_cost(Industry* industry, Area* a)
	{
		return industry->get_build_cost(get_prices(a));
	}

	double World::get_resources(Industry* industry, Area* a)
	{
		Product* p = industry->product;
		if (p)
		{
			return get_trade(a, p->id).resource;
		}
		else
		{
			return 1;
		}
	}

	AreaData& World::get_trade(Area* a, int id)
	{
		return _trade_maps[id]->get_trade(a);
	}
}
