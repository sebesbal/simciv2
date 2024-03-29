#include "stdafx.h"

#include "world.h"
#include <map>

using namespace std;;

namespace simciv
{
	std::recursive_mutex g_mutex;
	//const int industry_count = 12;
	World* world;
	int max_road_level = 5;
	using namespace std;

	void ProductionRule::end_init()
	{
		for (auto& p: input)
		{
			//int prod = p.first;
			products[p.first] = -p.second;
			//if (find(products.begin(), products.end(), prod) == products.end())
			//{
			//	products.push_back(prod);
			//}
		}
		for (auto& p : output)
		{
			products[p.first] = p.second;
			//if (find(products.begin(), products.end(), prod) == products.end())
			//{
			//	products.push_back(prod);
			//}
		}
	}

	void ProductionRule::load(rapidxml::xml_node<>* node)
	{
		this->input.clear();
		this->output.clear();

		auto item = node->first_node();
		while (item)
		{
			string id = item->first_attribute("product")->value();
			int index = world->get_product(id)->id;
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

		end_init();
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

	Industry::Industry():
		type(IndustryType::IT_NONE),
		base(NULL),
		product(nullptr)
	{
		build_cost.duration = default_buildtime;
		maint_cost.duration = default_lifetime;
	}

	bool Industry::can_upgrade_to(Industry * ind)
	{
		return find(upgrades.begin(), upgrades.end(), ind) != upgrades.end();
	}

	bool Industry::can_create_factory(Area * a)
	{
		return world->find_factories(a).size() == 0;
		//float res = world->get_trade(a, product->id).resource;
		//return res > 0 && world->find_factories(a).size() == 0;
	}

	Factory * Industry::create_factory(Area * a)
	{
		Factory* f = new Factory(this);
		//for (auto& t : f->sellers)
		//{
		//	t->set_storage(start_storage[t->product->id]);
		//}
		return f;
	}

	void Industry::add_rule(const ProductionRule & rule)
	{
		for (auto& p : rule.input)
		{
			buy_products.emplace(p.first);
		}
		for (auto& p : rule.output)
		{
			sell_products.emplace(p.first);
		}
		prod_rules.push_back(rule);
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

		//if (product)
		//{
		//	profit *= area->data(product).resource;
		//}
	}

	void Industry::find_best_maint_rule(const Prices& prices, ProductionRule*& rule, double& price)
	{
		price = max_price;
		for (auto& r : maint_cost.per_turn)
		{
			double p = r.expense(prices);
			if (p < price)
			{
				price = p;
				rule = &r;
			}
		}
	}

	bool Industry::is_infra()
	{
		return prod_rules.size() == 0;
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
		// always buy fuel
		buy_products.emplace(7); // TODO: 7 = _fuel_id

		if (auto a = node->first_attribute("id"))
		{
			name = a->value();
			product = world->get_product(name);
		}

		if (auto a = node->first_attribute("gid"))
		{
			group = a->value();
		}

		if (auto a = node->first_attribute("image"))
		{
			icon_file = "img/" + string(a->value());
		}
		else if (product)
		{
			icon_file = product->icon_file;
		}

		if (auto a = node->first_attribute("name"))
		{
			display_name = a->value();
		}
		else if (product)
		{
			display_name = product->display_name;
		}

		if (auto a = node->first_attribute("base"))
		{
			base = world->get_industry(a->value());
			auto i = base;
			while (i)
			{
				i->upgrades.push_back(this);
				i = i->base;
			}
		}
		 
		if (auto a = node->first_attribute("maint_cost.duration"))
		{
			maint_cost.duration = default_lifetime * stoi(a->value()) / 100.0;
		}

		if (auto a = node->first_attribute("build_cost.duration"))
		{
			build_cost.duration = default_buildtime * stoi(a->value()) / 100.0;
		}

		auto process_rule = [this](ProductionRule& rule) {
			for (auto& p : rule.input)
			{
				buy_products.emplace(p.first);
			}
			for (auto& p : rule.output)
			{
				sell_products.emplace(p.first);
			}
		};

		auto n = node->first_node("produce");
		while (n)
		{
			ProductionRule rule;
			rule.load(n);
			prod_rules.push_back(rule);
			process_rule(rule);
			n = n->next_sibling("produce");
		}

		// build_cost.total = maint * maint_cost.duration
		// build_cost.per_turn = build_cost.total / build_cost.duration
		n = node->first_node("maint");
		while (n)
		{
			ProductionRule rule;
			rule.load(n);
			//rule.output[0] = 1;
			process_rule(rule);
			maint_cost.per_turn.push_back(rule);
			n = n->next_sibling("maint");
		}

		n = node->first_node("build");
		if (!n)
		{
			for (auto& r : maint_cost.per_turn)
			{
				build_cost.total.push_back(r.multiply(maint_cost.duration, 1));
			}
		}
		else
		{
			while (n)
			{
				ProductionRule rule;
				rule.load(n);
				//rule.output[0] = 1;
				process_rule(rule);
				this->build_cost.total.push_back(rule);
				n = n->next_sibling("build");
			}
		}

		build_cost.calc_per_turn();
		//for (auto& r : build_cost.total)
		//{
		//	build_cost.per_turn.push_back(r.multiply(1.0 / build_cost.duration, 1));
		//}

		product = get_product();
	}

	Industry * Industry::create(rapidxml::xml_node<>* node)
	{
		Industry* result;
		if (node)
		{
			if (auto a = node->first_attribute("id"))
			{
				if (strcmp(a->value(), "explorer") == 0)
				{
					result = new Explorer();
				}
				else
				{
					result = new Industry();
				}
			}
			result->load(node);
		}
		else
		{
			result = new Industry();
		}
		return result;
	}

	Product* Industry::get_product()
	{
		if (prod_rules.size() == 1)
		{
			int product_id = prod_rules[0].output.begin()->first;
			return world->get_products()[product_id];
		}
		return NULL;
	}

	void Product::load(rapidxml::xml_node<>* node)
	{
		if (auto a = node->first_attribute("id"))
		{
			name = a->value();
		}

		if (auto a = node->first_attribute("gid"))
		{
			group = a->value();
		}

		if (auto a = node->first_attribute("image"))
		{
			icon_file = "img/" + string(a->value());
		}

		if (auto a = node->first_attribute("name"))
		{
			display_name = a->value();
		}

		if (auto a = node->first_attribute("is_resource"))
		{
			is_resource = string(a->value()) == "true";
		}
		else
		{
			is_resource = false;
		}
	}

	Factory::Factory(Industry* industry) :
		money(100000000), 
		efficiency(1), 
		state(FS_UNDER_CONTRUCTION),
		health(0),
		marked_as_deleted(false),
		utilization(0),
		_profit(0),
		best_rule(nullptr)
	{
		set_industry(industry);
		for (int i = 0; i < product_count; ++i)
		{
			sellers.push_back(NULL);
		}
		for (int i = 0; i < product_count; ++i)
		{
			buyers.push_back(NULL);
		}
	}

	Factory::~Factory()
	{
		for (int i = 0; i < product_count; ++i)
		{
			//world->trade_maps()[i]->remove_prod(sellers[i]);
			//world->trade_maps()[i]->remove_prod(buyers[i]);
			if (sellers[i]) --sellers[i]->ref_count;
			if (buyers[i]) --buyers[i]->ref_count;
		}
	}
	
	void Factory::set_state(FactoryState state)
	{
		switch (state)
		{
		case simciv::FS_NONE:
			break;
		case simciv::FS_UNDER_CONTRUCTION:
			current_healing_cost = industry->build_cost;
			break;
		case simciv::FS_RUN:
			current_healing_cost = industry->build_cost;
			break;
		case simciv::FS_UPGRADE:
			break;
		case simciv::FS_DEAD:
			break;
		default:
			break;
		}

		if (state != this->state)
		{
			auto old_state = state;
			this->state = state;
			if (on_state_changed)
			{
				on_state_changed(this, old_state, this->state);
			}
		}
	}

	void Factory::set_industry(Industry * industry)
	{
		this->industry = industry;
		current_healing_cost = industry->build_cost;
	}

	void Factory::start_upgrade_to(Industry * industry)
	{
		set_state(FS_UPGRADE);
		set_industry(industry);
		current_healing_cost = industry->build_cost;
		health = 0.5;
	}

	void Factory::update()
	{
		Prices prices = get_prices();
		double profit;
		find_best_prod_rule(prices, best_rule, profit);

		auto stor_func = [](double dstor) {
			if (-ideal_storage / 2 < dstor && dstor < ideal_storage / 2) return 0.0;
			else return dstor;
		};

		if (best_rule)
		{
			// compute utilization
			vector<double> delta_stores(product_count, 0.0);
			double sum_g_pvol = 0;
			double min_utilization = 1;
			for (auto& p: best_rule->products)
			{
				int prod = p.first;
				double storage = sellers[prod]->storage();				// current storage
				double import = buyers[prod]->vol - sellers[prod]->vol;	// import (export) volume
				double delta_storage = stor_func(ideal_storage - storage) * 0.1; // we want to increase (decrease) the storage by this volume
				delta_stores[prod] = delta_storage;
				double goal_consumption = import - delta_storage;		// We need to consume (produce) around goal_cons. (import - goal_cons = delta_stor)
				bool is_buyer = p.second < 0;
				if (!is_buyer)
				{
					goal_consumption = -goal_consumption; // it is production actually
				}
				goal_consumption = max(0.0, goal_consumption);
				double utilization = abs(goal_consumption / p.second);	// We need this utilization for this product
				min_utilization = min(min_utilization, utilization);
			}
			utilization = min_utilization;

			double profit = 0;
			double profit_coef = 1;
			if (profit < 0) profit_coef = 0;

			for (auto& p : best_rule->products)
			{
				int prod = p.first;
				double rule_consumption = -p.second;
				bool is_buyer = rule_consumption > 0;
				double delta_storage = delta_stores[prod];
				double consumption = rule_consumption * utilization;	// actual consumption
				double goal_import = delta_storage + profit_coef * consumption;
				double new_goal_vol = is_buyer ? goal_import : -goal_import;
				new_goal_vol = max(0.0, new_goal_vol);
				double& goal_vol = (is_buyer ? buyers[prod]->goal_vol : sellers[prod]->goal_vol);
				goal_vol = alpha * goal_vol + beta * new_goal_vol;
			}
		}
	}

	double Factory::apply_rule(ProductionRule* rule, double profit, double max_rate)
	{
		// TODO:
		// buyers->vol_in should be: the volume what we want to buy. it depends on seller->vol_out
		
		double rate = max_rate; // ideal_rate == 1 always, TODO
		//if (rate == 0) return 0;
		//check_buyer_storage(rule->input, rate);
		check_storage(true, buyers, rule->input, rate);
		//if (rate == 0) return 0;
		double rate2 = rate;
		check_storage(false, sellers, rule->output, rate);
		//if (rate == 0) return 0;

		for (auto& p : rule->input)
		{
			int prod_id = p.first;
			double vol = p.second;
			buyers[prod_id]->modify_storage(rate * vol);
			//buyers[prod_id]->vol_in += max_rate * vol;
		}

		for (auto& p : rule->output)
		{
			int prod_id = p.first;
			double vol = p.second;
			sellers[prod_id]->modify_storage(rate * vol);
			//sellers[prod_id]->vol_in += rate2 * vol;
		}

		smooth_change(utilization, rate);
		//smooth_change(_profit, rate * profit);
		return rate;
	}

	double Factory::consume_articles(Prices& prices)
	{
		double full_expense = 0;

		//if (health < 1 && (state == simciv::FS_UNDER_CONTRUCTION || _profit > 0))
		if (health < 1)
		{
			// Build / Repair / Upgrade
			double volume = min((1 - health) * current_healing_cost.duration, 1.0);
			double unconsumed_volume = consume_articles(prices, current_healing_cost.per_turn, volume, full_expense);
			health += (volume - unconsumed_volume) / current_healing_cost.duration;
		}

		switch (state)
		{
		case simciv::FS_UPGRADE:
		case simciv::FS_UNDER_CONTRUCTION:
		{
			if (health >= 1)
			{
				health = 1;
				set_state(FS_RUN);
			}
			break;
		}
		case simciv::FS_RUN:
		{
			// Maintenance
			double unconsumed_volume = _profit < 0 ? 1 : consume_articles(prices, industry->maint_cost.per_turn, 1, full_expense);
			health -= unconsumed_volume / industry->maint_cost.duration;
			if (health <= 0)
			{
				health = 0;
				//set_state(FS_DEAD);
			}
			break;
		}
		}
		return full_expense;
	}

	double Factory::consume_articles(Prices & prices, std::vector<ProductionRule>& rules, double volume, double& full_expense)
	{
		return simciv::consume_articles(buyers, prices, rules, volume, full_expense);
	}

	void Factory::check_money(double price, double& rate)
	{
		if (price > money)
		{
			rate = std::min(rate, money / price);
		}
	}

	void Factory::find_best_prod_rule(const Prices& prices, ProductionRule*& rule, double& profit)
	{
		industry->find_best_prod_rule(prices, area, rule, profit);
	}

	Prices Factory::get_prices()
	{
		Prices p;
		for (int i = 0; i < product_count; ++i)
		{
			if (sellers[i]) p.sell[i] = sellers[i]->price;
			if (buyers[i]) p.buy[i] = buyers[i]->price;
		}
		return p;
	}

	void Factory::income(double money)
	{
		this->money += money;
		if (this->money < 0) this->money = 0;
	}

	World * World::create_from_file(const std::string & file_name, int width, int height)
	{
		World* result = nullptr;
		std::ifstream file(file_name);
		if (!file.is_open())
		{
			throw "File not found: " + file_name;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();
		std::string content(buffer.str());
		rapidxml::xml_document<> doc;
		//auto& doc = result->m_settings;
		doc.parse<0>(doc.allocate_string(&content[0]));  // without allocate_string, it takes over the ownership of the string
		auto *root = doc.first_node("simciv");
		auto mode = root->first_attribute("mode");
		string mode_str = mode ? mode->value() : "default";
		if (mode_str == "default")
		{
			result = new World();

		}
		else if (mode_str == "mvp")
		{
			result = new MvpWorld();
		}

		result->m_settings.parse<0>(doc.allocate_string(&content[0])); // this is ugly, but we have to parse again as doc doesn't have copy constructor
		world = result;
		result->init(width, height);
		return result;
	}

	//void World::create(int width, int height)
	//{
	//	generate_industry();
	//	// prod_count = product_count;
	//	Map::create(width, height);

	//	// create trade maps
	//	for (int i = 0; i < product_count; ++i)
	//	{
	//		auto tm = new TradeMap(*products[i]);
	//		_trade_maps.push_back(tm);
	//		if (products[i]->name == "fuel_1")
	//		{
	//			_fuel_map = tm;
	//			_fuel_id = i;
	//		}
	//	}
	//	//generate_factories();
	//}

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

	//void World::generate_industry()
	//{
	//	CCLOG("ExePath() %s", ExePath());
	//	load_from_file("res/mod4.xml");
	//	init_col_industries();
	//}

	//void World::generate_factories()
	//{
	//	int x = 12, y = 10;

	//	auto a = get_area(x, y);
	//	ProductionRule r, b;
	//	b.input[0] = 0;
	//	a->industry = new Industry();
	//	for (int i = 0; i < 4; ++i)
	//	{
	//		r.output[i] = 0.1;
	//		a->industry->sell_products.emplace(i);
	//	}
	//	a->industry->prod_rules.push_back(r);
	//	a->industry->buy_products.emplace(_fuel_id);
	//	a->industry->build_cost.total.push_back(b);
	//	a->industry->build_cost.calc_per_turn();
	//	a->has_factory = true;
	//	//a->update_colors();

	//	auto g = [this](Factory* f, string s) {
	//		int id = world->get_product(s)->id;
	//		f->buyers[id] = _trade_maps[id]->create_prod(f->area, true, 50);
	//		f->sellers[id] = _trade_maps[id]->create_prod(f->area, false, 50);
	//		_trade_maps[id]->sync_area_traders(f->area);
	//		f->buyers[id]->set_storage(10);

	//	};

	//	auto fact = [&](int x, int y) {
	//		Area* a = get_area(x, y);
	//		if (!a->industry) return;
	//		auto f = create_factory(a, a->industry);
	//		a->has_factory = true;
	//		//a->update_colors();
	//		f->set_state(FS_RUN);
	//		f->health = 1;

	//		g(f, "prod_1");
	//		g(f, "prod_2");
	//		g(f, "prod_3");
	//		g(f, "fuel_1");
	//	};

	//	//if (!s1) throw("Industry not found!");
	//	//auto f = create_factory(a, a->industry);
	//	//f->set_state(FS_RUN);
	//	//f->health = 1;



	//	//g(f, "prod_1");
	//	//g(f, "prod_2");
	//	//g(f, "prod_3");
	//	//g(f, "fuel");

	//	//f->buyers[world->get_product("food_1")->id]->set_storage(1000);
	//	//f->buyers[world->get_product("manpow")->id]->set_storage(1000);
	//	//f->buyers[world->get_product("wood_1")->id]->set_storage(1000);
	//	//f->buyers[world->get_product("stone_1")->id]->set_storage(1000);
	//	//f->buyers[world->get_product("fuel_1")->id]->set_storage(1000);

	//	// some other starting factory

	//	fact(x, y);
	//	fact(x - 2, y + 1);
	//	fact(x + 2, y + 1);
	//	fact(x - 2, y );
	//	fact(x - 1, y + 1);
	//}

	Factory* World::create_factory(Area* a, Industry* industry)
	{
		if (!industry->can_create_factory(a)) return nullptr;

		Factory* f = industry->create_factory(a);
		f->area = a;
		if (industry->product && industry->product->is_resource)
		{
			f->efficiency = a->data(industry->product).resource;
		}
		else
		{
			f->efficiency = 1;
		}

		// create all producers:
		for (int i = 0; i < product_count; ++i)
		{
			if (industry->sell_products.find(i) != industry->sell_products.end())
			{
				auto p = f->sellers[i] = _trade_maps[i]->create_prod(a, false, 50);
				++p->ref_count;
				p->storage_capacity = 10000;
			}
			if (industry->buy_products.find(i) != industry->buy_products.end())
			{
				auto q = f->buyers[i] = _trade_maps[i]->create_prod(a, true, 50);
				++q->ref_count;
				q->storage_capacity = 10000;
			}
			
			_trade_maps[i]->sync_area_traders(a);
		}

		factories.push_back(f);
		a->has_factory = true;
		return f;
	}

	void World::delete_factory(Factory * f)
	{
		//for (auto t : f->buyers) if (t) --t->ref_count;
		//for (auto t : f->sellers) if (t) --t->ref_count;
		f->marked_as_deleted = true;
	}

	Factory * World::create_explorer(Area * a, Industry * industry)
	{
		//create_factory(a, );
		return nullptr;
	}

	vector<Factory*> World::find_factories(Area* a)
	{
		// ting::shared_lock<ting::upgrade_mutex> lock(factories_mutex);
		vector<Factory*> result;
		for (auto f : factories)
		{
			if (f->area == a)
			{
				result.push_back(f);
			}
		}
		return result;
	}

	void World::update()
	{
		static bool working = false;
		if (working) return;

		working = true;

		static int k = 0;
		++time;

		factories.erase(remove_if(factories.begin(), factories.end(), [](Factory* f) {
			if (f->marked_as_deleted)
			{
				delete f;
				return true;
			}
			return false;
		}), factories.end());

		for (auto& map : _trade_maps)
		{
			auto& v = map->_transports;
			v.erase(remove_if(v.begin(), v.end(), [](Transport* t) {
				if (t->seller->ref_count == 0 || t->buyer->ref_count == 0)
				{
					t->marked_as_deleted = true;
					return true;
				}
				return false;
			}), v.end());

			auto& b = map->_buyers;
			b.erase(remove_if(b.begin(), b.end(), [map](Trader* t) { 
				if (t->ref_count == 0)
				{
					auto& x = map->_area_buyers[t->area->id];
					x.erase(find(x.begin(), x.end(), t));
					return true;
				}
				return false;
			}), b.end());

			auto& s = map->_buyers;
			s.erase(remove_if(s.begin(), s.end(), [map](Trader* t) {
				if (t->ref_count == 0)
				{
					auto& x = map->_area_sellers[t->area->id];
					x.erase(find(x.begin(), x.end(), t));
					return true;
				}
				return false;
			}), s.end());
		}

		update_road_maps();

		for (TradeMap* product : _trade_maps)
		{
			product->before_rules();
		}

		// create/updates trade routes. (profits, fuel prices, route)
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

		// find best seller and buyer prices
		if (k % 1 == 0)
		for (TradeMap* product : _trade_maps)
		{
			product->update_area_prices2(! (k % 10));
		}

		{
			for (Factory* f : factories)
			{
				Prices prices = f->get_prices();

				if (f->state == FS_RUN || f->state == FS_UNDER_CONTRUCTION)
				{
					ProductionRule* rule;
					double profit;
					f->find_best_prod_rule(prices, rule, profit);
					if (rule)
					{
						f->_profit = profit;
						f->apply_rule(rule, profit, f->efficiency * f->health);
					}
					else
					{
						f->_profit = -1;
					}
				}

				if (f->state == FS_RUN || f->state == FS_UNDER_CONTRUCTION || f->state == FS_UPGRADE)
				//if (f->state == FS_UNDER_CONTRUCTION || f->state == FS_UPGRADE)
				{
					double expense = f->consume_articles(prices);
				}
			}
		}

		//if (k % 5 == 0)
		{
			for (TradeMap* product : _trade_maps)
			{
				product->update_trade_volume();
			}
		}

		for (TradeMap* product : _trade_maps)
		{
			product->update_producer_storages();
			product->update_producer_prices();

			product->update_data();
		}

		++k;

		working = false;
	}

	using namespace rapidxml;
	using namespace std;

	void World::init(int width, int height)
	{
		rapidxml::xml_document<> a, b;
		Map::create(width, height);
		xml_node<> *root = m_settings.first_node("simciv");
		xml_node<> *item = root->first_node();
		_fuel_id = -1;
		while (item)
		{
			string name = item->name();
			if (name == "industry")
			{
				Industry* s = Industry::create(item);
				add_industry(s);
				auto e = dynamic_cast<Explorer*>(s);
				if (e)
				{
					_explorer = e;
				}
			}
			else if (name == "product")
			{
				Product* product = new Product();
				product->load(item);
				product->is_resource = true;
				add_product(product);

				auto is_fuel = item->first_attribute("is_fuel");
				if (is_fuel && string(is_fuel->value()) == "true")
				{
					_fuel_id = product->id;
				}
			}

			item = item->next_sibling();
		}

		product_count = products.size();

		// create trade maps
		for (int i = 0; i < product_count; ++i)
		{
			auto tm = new TradeMap(*products[i]);
			_trade_maps.push_back(tm);
			if (i == _fuel_id)
			{
				_fuel_map = tm;
			}
		}
	}

	void World::add_product(Product * product)
	{
		product->id = products.size();
		products.push_back(product);
		product->tile_res[AT_NONE] = 1;
		product->tile_res[AT_SEA] = 0;
		product->tile_res[AT_PLAIN] = 1;
		product->tile_res[AT_MOUNTAIN] = 0.5;
	}

	void World::add_industry(Industry * industry)
	{
		industries.push_back(industry);
		industry->product = industry->get_product();
		industry->build_cost.calc_per_turn();
		industry->buy_products.emplace(_fuel_id);
		for (const auto& rule: industry->maint_cost.total)
		{
			for (auto& p : rule.input)
			{
				industry->buy_products.emplace(p.first);
			}
		}
		for (const auto& rule : industry->build_cost.total)
		{
			for (auto& p : rule.input)
			{
				industry->buy_products.emplace(p.first);
			}
		}
	}

	void World::init_col_industries()
	{
		// simciv::product_count = product_count;
		const cocos2d::Color4F c[7] = {
			Color4F(1, 0, 0, 1), 
			Color4F(0, 1, 0, 1), 
			Color4F(0, 0, 1, 1), 
			Color4F(0.7, 0.7, 0.7, 1),
			Color4F(1, 1, 0, 1),
			Color4F(1, 0, 1, 1),
			Color4F(0, 1, 1, 1)
		};
		for (int i = 0; i < product_count; ++i)
		{
			colors.push_back(c[i]);
		}
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
		if (a->type == AT_SEA) return -1;
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

	void World::update_road_maps()
	{
		static int k = 0;
		int n = _areas.size();
		int old_k = k;
		clock_t start = clock();

		for (int l = 0; l < 20;)
		{
			Area* a = _areas[k];
			RoadMap* m = a->map;
			if (m)
			{
				int t = time - m->time;
				if (m->invalidated && t > 5
					|| !m->invalidated && t > 20)
				{
					create_road_map(a);
					++l;
				}
			}
			k = (k + 1) % n;
			if (k == old_k) break;

			if (clock() - start > 100) break;
		}
	}
	void World::area_changed(Area * a)
	{
		int m = 5;
		int x = a->x, y = a->y;
		for (int i = max(0, x - m); i < min(x + m, width()); ++i)
		{
			for (int j = max(0, y - m); j < min(y + m, height()); ++j)
			{
				Area* b = get_area(i, j);
				if (is_used(b) && b->map)
				{
					b->map->invalidated = true;
				}
			}
		}
	}
	bool World::is_used(Area * a)
	{
		for (auto t : trade_maps())
		{
			if (t->is_used(a)) return true;
		}
		return false;
	}
	double World::transport_cost(Area * a, Area * b)
	{
		double d = distance(a, b);
		//double fuel_price = _fuel_map->get_trade(a).p_buy;
		auto& seller = _fuel_map->_area_buyers[a->id];
		double fuel_price = seller.size() > 0 ? seller[0]->price : _fuel_map->get_trade(a).p_buy;
		return distance_cost * d * fuel_price;
	}

	void World::get_transports(Area * a, std::vector<Transport*>& in, std::vector<Transport*>& out)
	{
		for (auto m : _trade_maps)
		{
			m->get_transports(a, in, out);
		}
	}

	//double World::fuel_price(Area * a)
	//{
	//	return _fuel_map->get_trade(a).p_buy;
	//}

	Trader * World::fuel_buyer(Area * a)
	{

		return _fuel_map->_area_buyers[a->id][0];
	}

	void World::set_mil_state(Area * a, MilitaryState state)
	{

	}

	void World::set_explored(Area * a)
	{
		a->mil_state = MILS_EXPLORED;
		on_area_changed(a);
		for (Area* b : a->adjs)
		{
			if (b->mil_state == MILS_UNEXPLORED)
			{
				b->mil_state = MILS_EXPLORABLE;
				on_area_changed(b);
			}
		}
	}

	//void World::generate_resources()
	//{
	//	for (TradeMap* m : _trade_maps)
	//	{
	//		m->generate_resources();
	//	}

	//	const int n = 3;
	//	const cocos2d::Color3B cols[n] = { Color3B(255, 0, 0), Color3B(0, 255, 0), Color3B(0, 0, 255) };

	//	vector<TradeMap*> tms;

	//	tms.push_back(get_product("food_1")->map);
	//	tms.push_back(get_product("wood_1")->map);
	//	tms.push_back(get_product("fuel_1")->map);

	//	for (auto a : _areas)
	//	{
	//		auto it = max_element(tms.begin(), tms.end(),
	//			[a](TradeMap* ta, TradeMap* tb) { return ta->get_trade(a).resource <= tb->get_trade(a).resource; });
	//		auto tm = *it;
	//		auto p = tm->product;
	//		//if (p.id < n)
	//		{
	//			int id = find(tms.begin(), tms.end(), tm) - tms.begin();
	//			Color3B c = cols[id];
	//			double r = tm->get_trade(a).resource;
	//			double& rr = tm->get_trade(a).resource;
	//			if (r < 0.3)
	//			{
	//				r = 0; rr = 0;
	//			}
	//			else if (r < 0.35)
	//			{
	//				r = 0.2;
	//				rr = 0.5;
	//			}
	//			else if (r < 0.4)
	//			{
	//				r = 0.6;
	//				rr = 0.75;
	//			}
	//			else
	//			{
	//				r = 1;
	//				rr = 1;
	//			}

	//			a->color = Color3B(r * c.r, r * c.g, r * c.b);
	//		}
	//		//else
	//		//{
	//		//	a->color = Color3B(0, 0, 0);
	//		//}
	//	}
	//}

	void World::generate_resources_rand_ind()
	{
		for (TradeMap* m : _trade_maps)
		{
			m->generate_resources();
		}

		const int n = 4;
		float a = 0.6;
		// const cocos2d::Color4F colsin[n] = { Color4F(a, 0, 0, 1), Color4F(0, a, 0, 1), Color4F(0, 0, a, 1) };
		//const cocos2d::Color4F colsou[n] = { Color4F(1, 0, 0, 1), Color4F(0, 1, 0, 1), Color4F(0, 0, 1, 1), Color4F(0.7, 0.7, 0.7, 1) };

		vector<TradeMap*> tms;

		tms.push_back(get_product("prod_1")->map);
		tms.push_back(get_product("prod_2")->map);
		tms.push_back(get_product("prod_3")->map);
		tms.push_back(get_product("fuel_1")->map);

		for (auto a : _areas)
		{
			if (a->type == AT_SEA) continue;

			auto get_n = [](const int p[5]) {
				int x = 100 * random() / RAND_MAX;
				int a = 0;
				for (int i = 0; i < 4; ++i)
				{
					a += p[i];
					if (a > x) return i;
				}
				return 4; // never happens
			};

			//const int in_p[5] = { 65, 20, 10, 5, 0 };
			const int in_p[5] = { 0, 50, 50, 0, 0 };
			//const int out_p[5] = { 70, 3, 6, 9, 12 };
			const int out_p[5] = { 90, 5, 5, 0, 0 };
			int out_n = get_n(out_p);
			if (out_n == 0) continue;
			int in_n = get_n(in_p);

			vector<int> s;
			for (int i = 0; i < 4; ++i) s.push_back(i);

			auto f = [](vector<int>& s, int n) {
				set<int> result;
				int k = s.size();
				for (int i = 0; i < n && k > 0; ++i)
				{
					int id = random(0, k - 1);
					auto it = s.begin() + id;
					result.emplace(*it);
					s.erase(it);
					--k;
				}
				return result;
			};

			auto out = f(s, out_n);
			auto in = f(s, in_n);

			ProductionRule r, b, m;
			a->industry = new Industry();
			for (int i : in)
			{
				r.input[i] = 1;
				b.input[i] = 1;
				a->industry->buy_products.emplace(i);
				//a->color_in.push_back(colsou[i]);
			}

			double x[4] = {1.0, 2.0, 4.0, 8.0};

			for (int i : out)
			{
				r.output[i] = 0.5 * x[in.size()] / x[out.size() - 1];
				a->industry->sell_products.emplace(i);
				//a->color_out.push_back(colsou[i]);
			}
			a->industry->buy_products.emplace(_fuel_id);
			a->industry->prod_rules.push_back(r);
			a->industry->build_cost.duration = 10;
			a->industry->build_cost.total.push_back(b);
			a->industry->build_cost.calc_per_turn();

			m.input[0] = 0.1;
			a->industry->maint_cost.per_turn.push_back(m);
			a->industry->buy_products.emplace(0);
			//a->update_colors();
		}

		//generate_factories(doc);
	}

	void World::generate_resources()
	{
		for (TradeMap* m : _trade_maps)
		{
			m->generate_resources();
		}
	}

	void smooth_change(double & smooth_value, const double current_value, double a)
	{
		smooth_value = a * smooth_value + (1 - a) * current_value;
	}

	void update_var(double & moving_average, const double & current_value)
	{
		moving_average = alpha * moving_average + beta * current_value;
	}

	double consume_articles(std::vector<Trader*>& storage, Prices & prices, std::vector<ProductionRule>& rules, double volume, double & full_expense)
	{
		if (rules.size() == 0) return 0;

		typedef std::pair<double, ProductionRule*> pair_t;
		std::vector<pair_t> v;

		for (auto& rule : rules)
		{
			v.push_back(pair_t(rule.expense(prices), &rule));
		}

		std::sort(v.begin(), v.end(), [](pair_t& a, pair_t& b) { return a.first < b.first; });

		for (auto& p : v)
		{
			double rate = volume;
			double expense = p.first;
			auto rule = p.second;
			check_storage(true, storage, rule->input, rate);
			expense *= rate;
			full_expense += expense;
			for (auto& p : rule->input)
			{
				int prod_id = p.first;
				double vol = p.second;
				storage[prod_id]->modify_storage(rate * vol);
				storage[prod_id]->vol_in += volume * vol;
			}
			volume -= rate;
			if (volume <= 0) break;
		}

		volume = max(volume, 0.0);
		return volume;
	}

	void check_storage(bool is_buyer, std::vector<Trader*>& storage, ProductMap & vols, double & rate)
	{
		if (rate == 0) return;
		for (auto& p : vols)
		{
			int prod_id = p.first;
			double vol = p.second;
			auto& producer = storage[prod_id];
			if (is_buyer)
			{
				rate = std::min(rate, producer->storage() / vol);
			}
			else
			{
				rate = std::min(rate, (producer->storage_capacity - producer->storage()) / vol);
			}
			if (rate == 0) return;
		}
	}
	bool Explorer::can_create_factory(Area * a)
	{
		return a->mil_state == MILS_EXPLORABLE
			&& world->find_factories(a).size() == 0;
	}
	void ProductionCost::calc_per_turn()
	{
		for (auto& r : total)
		{
			per_turn.push_back(r.multiply(1.0 / duration, 1));
		}
	}
	void MvpWorld::init(int width, int height)
	{
		Map::create(width, height);
		_fuel_id = -1;
		int brick_id = -1;
		vector<string> color_names;
		map<int, int> shape_id;  // prod.id -> id (shape_1_<id>.png)
		xml_node<> *root = m_settings.first_node("simciv");
		auto col_node = root->first_node("color");
		int i = 0;
		while (col_node)
		{
			color_names.push_back(col_node->first_attribute("name")->value());
			shape_id[i++] = stoi(col_node->first_attribute("shape_id")->value());
			col_node = col_node->next_sibling("color");
		}
		int colors = color_names.size();

		int levels = stoi(root->first_attribute("levels")->value());
		map<int, map<int, Product*>> prods;
		map<int, map<int, int>> ids;
		map<int, int> ep_ids;

		auto icon_file = [](int color, int level){
			return "img/shapes/shape_" + to_string(level) + "_" + to_string(color) + ".png";
		};

		auto name = [&](int color, int level) {
			return color_names[color] + "_" + to_string(level);
		};

		// commodities
		for (int color = 0; color < colors; ++color)
		{
			for (int level = 0; level < levels; ++level)
			{
				Product* product = new Product();
				add_product(product);
				product->level = level;
				prods[color][level] = product;
				ids[color][level] = product->id;
				product->name = name(color, level);
				product->display_name = product->name;
				product->group = to_string(color + 1);
				product->icon_file = icon_file(shape_id[color], level);
				product->is_resource = level == 0;
				product->is_end_product = false;
			}
		}
		 
		// end products
		auto ep_node = root->first_node("end_product");
		i = 0;
		string ep_group = to_string(colors + 1);
		while (ep_node)
		{
			Product* product = new Product();
			add_product(product);
			product->level = 0;
			ep_ids[i] = product->id;
			product->name = ep_node->first_attribute("name")->value();
			product->display_name = product->name;
			product->group = to_string(colors + i + 1);
			shape_id[colors + i] = stoi(ep_node->first_attribute("shape_id")->value());
			// product->icon_file = icon_file(colors + i, 0);
			product->icon_file = icon_file(shape_id[colors + i], 0);
			product->is_resource = false;
			product->is_end_product = true;


			auto is_fuel = ep_node->first_attribute("is_fuel");
			if (is_fuel && string(is_fuel->value()) == "true")
			{
				_fuel_id = product->id;
			}

			auto is_brick = ep_node->first_attribute("is_brick");
			if (is_brick && string(is_brick->value()) == "true")
			{
				brick_id = product->id;
			}

			ep_node = ep_node->next_sibling("end_product");
			i += 1;
		}

		ProductionRule defaultBuild;
		defaultBuild.input[brick_id] = 0.01;
		defaultBuild.end_init();

		// Industries, commodities
		for (int color = 0; color < colors; ++color)
		{
			string color_name = color_names[color];
			
			Industry* s = Industry::create();
			//s->icon_file = icon_file(color, 0);
			s->icon_file = icon_file(shape_id[color], 0);
			Product* prod = products[ids[color][0]];
			s->level = prod->level;
			s->name = prod->name;
			s->display_name = s->name;
			s->group = prod->group;
			s->maint_cost.total.push_back(defaultBuild);
			s->build_cost.total.push_back(defaultBuild);
			ProductionRule rule;
			rule.output[ids[color][0]] = 1;
			rule.end_init();
			s->add_rule(rule);
			add_industry(s);

			for (int level = 1; level < levels; ++level)
			{
				Industry* s = Industry::create();
				s->level = prod->level;
				//s->icon_file = icon_file(color, level);
				s->icon_file = icon_file(shape_id[color], level);
				Product* prod = products[ids[color][level]];
				s->name = prod->name;
				s->display_name = s->name;
				s->group = prod->group;
				s->maint_cost.total.push_back(defaultBuild);
				s->build_cost.total.push_back(defaultBuild);
				ProductionRule rule;
				rule.input[ids[color][level - 1]] = 0.5;
				rule.input[ids[(color + 1) % colors][level - 1]] = 0.5;
				rule.output[ids[color][level]] = 1;
				rule.end_init();
				s->add_rule(rule);
				add_industry(s);
			}
		}

		// Industries, end products
		for (auto p : ep_ids)
		{
			int ep_id = p.first;
			int prod_id = p.second;
			for (int level = 0; level < levels; ++level)
			{
				Industry* s = Industry::create();
				s->level = level;
				//s->icon_file = icon_file(colors + ep_id, level);
				//shape_id[colors + ep_id] = stoi(ep_node->first_attribute("icon_group")->value());
				s->icon_file = icon_file(shape_id[colors + ep_id], level);
				Product* prod = products[ep_ids[ep_id]];
				s->name = prod->name;
				s->display_name = s->name;
				s->group = prod->group;
				s->maint_cost.total.push_back(defaultBuild);
				s->build_cost.total.push_back(defaultBuild);
				for (int level_input = 0; level_input <= level; ++level_input)
				{
					for (int color = 0; color < colors; ++color)
					{
						ProductionRule rule;
						rule.input[ids[color][level_input]] = 1;
						rule.output[prod_id] = pow(2, level);
						rule.end_init();
						s->add_rule(rule);
					}
				}
				add_industry(s);
			}
		}
		
		product_count = products.size();

		// capital
		{
			Industry* s = Industry::create();
			s->level = 0;
			s->icon_file = "img/shapes/circle_white.png";
			s->name = "Capital";
			s->display_name = s->name;
			s->group = to_string(colors + ep_ids.size() + 1);
			
			//s->maint_cost.total.push_back(defaultBuild);
			//s->build_cost.total.push_back(defaultBuild);
			for (int color = 0; color < colors; ++color)
			{
				s->start_storage[ids[color][0]] = 50;
				ProductionRule rule;
				//rule.input[ids[color][level_input]] = 1;
				rule.output[ids[color][0]] = 0.2;
				rule.end_init();
				s->add_rule(rule);
			}
			for (auto& p : ep_ids)
			{
				ProductionRule rule;
				//rule.input[ids[color][level_input]] = 1;
				rule.output[p.second] = 0.2;
				rule.end_init();
				s->add_rule(rule);
			}
			add_industry(s);
		}

		// create trade maps
		for (int i = 0; i < product_count; ++i)
		{
			auto tm = new TradeMap(*products[i]);
			_trade_maps.push_back(tm);
			if (i == _fuel_id)
			{
				_fuel_map = tm;
			}
		}
	}
	void MvpWorld::generate_factories()
	{
		auto *root = m_settings.first_node("simciv");
		auto fact_node = root->first_node("factory");
		while (fact_node)
		{
			int col = stoi(fact_node->first_attribute("col")->value());
			int row = stoi(fact_node->first_attribute("row")->value());
			int level = stoi(fact_node->first_attribute("level")->value());
			string group = fact_node->first_attribute("industry")->value();
			auto industry = std::find_if(industries.begin(), industries.end(), [level, group](Industry* a) {
				return a->group == group && a->level == level;
			});

			this->create_factory(get_area(col, row), *industry);

			fact_node = fact_node->next_sibling("factory");
		}
	}
}
