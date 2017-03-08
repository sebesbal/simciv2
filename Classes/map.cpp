#include "stdafx.h"

#include "map.h"

#include "assert.h"
#include <queue>

#include "trade.h"
#include "world.h"

using namespace std;

namespace simciv
{
	int product_count;

	Area::Area(): map(nullptr)
		, road_level(0)
		, mil_level(0)
		, industry(nullptr)
		, has_factory(false)
	{
	}

	AreaData& Area::data(Product* p)
	{
		return world.get_trade(this, p->id);
	}

	AreaData& Area::data(int prod_id)
	{
		return world.get_trade(this, prod_id);
	}

	std::vector<Area*> Area::sorted_adjs()
	{
		std::vector<Area*> result(8, NULL);
		if (road_level == 0) return result;
		for (auto r: roads)
		{
			Area* b = r->other(this);
			if (b->road_level)
			{
				int d = dir(r);
				result[d] = b;
			}
		}
		return result;
	}

	std::vector<Road*> Area::sorted_roads()
	{
		std::vector<Road*> result(8, NULL);
		if (road_level == 0) return result;
		for (auto r : roads)
		{
			Area* b = r->other(this);
			if (b->road_level)
			{
				int d = dir(r);
				result[d] = r;
			}
		}
		return result;
	}

	std::vector<Area*> Area::connected_adjs()
	{
		std::vector<Area*> result;
		if (road_level == 0) return result;
		for (auto r : roads)
		{
			Area* b = r->other(this);
			if (b->road_level)
			{
				result.push_back(b);
			}
		}
		return result;
	}

	#define lofusz(col, w) col.r *= w; col.g *= w; col.b *= w;
	void Area::update_colors()
	{
		color_in.clear();
		color_out.clear();
		if (!industry)
		{
			return;
		}

		//const cocos2d::Color4F colsou[4] = { Color4F(1, 0, 0, 1), Color4F(0, 1, 0, 1), Color4F(0, 0, 1, 1), Color4F(0.7, 0.7, 0.7, 1) };
		ProductionRule& r = industry->prod_rules[0];
		for (auto& p : r.input)
		{
			color_in.push_back(world.colors[p.first]);
		}
		for (auto& p : r.output) 
		{
			color_out.push_back(world.colors[p.first]);
		}

		const float fact_w = 5;
		float r_weight = (color_in.size() + color_out.size()) / 4.0f;
		//float weight = (color_in.size() + color_out.size() + (has_factory ? fact_w : 0)) / (fact_w + 4.0f);
		float weight = has_factory ? 1 : 0.4;
		rad_1 = 0.8 / 2 * r_weight;
		rad_2 = rad_1 + 0.15;

		for (auto& c : color_in)
		{
			lofusz(c, weight);
		}

		for (auto& c : color_out)
		{
			lofusz(c, weight);
		}
	}

	Road* Area::road(Area* b)
	{
		for (auto r : roads)
		{
			if (r->other(this) == b) return r;
		}
		return nullptr;
	}

	int Area::dir(Area * a)
	{
		return dir(road(a));
	}

	int Area::dir(Road * r)
	{
		if (!r) return 8;
		if (r->a == this) return r->dir;
		else return (r->dir + 4) % 8;
	}

	Road * Area::road(int dir)
	{
		for (Road* r : roads)
		{
			if (this->dir(r) == dir) return r;
		}
		return NULL;
	}

	Area * Area::area(int dir)
	{
		for (Road* r : roads)
		{
			if (this->dir(r) == dir) return r->other(this);
		}
		return NULL;
	}

	//// Node for graph algorithms (eg. Dijstra)
	//struct Node
	//{
	//	Node() : area(NULL), parent(NULL), color(0), d(0) {}
	//	Road* parent;
	//	Area* area;
	//	double d;
	//	int color; // 0 = black, unvisited, 1 = gray, opened, 2 = white, visited
	//};

	//// Graph
	//struct RoadMap
	//{
	//	Node* g;
	//	int time;
	//};

	class NodeComparator
	{
	public:
		bool operator()(const Node* a, const Node* b)
		{
			return (a->d > b->d);
		}
	};

	Map::Map() : time(0)
	{

	}

	void Map::create(int width, int height)
	{
		_width = width;
		_height = height;
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				Area* a = new Area();
				a->id = _areas.size();
				a->x = x;
				a->y = y;
				_areas.push_back(a);

				if (x > 0)
				{
					add_road(a, get_area(x - 1, y));
					if (y > 0)
					{
						add_road(a, get_area(x - 1, y - 1));
					}
				}
				if (y > 0)
				{
					add_road(a, get_area(x, y - 1));
					if (x < width - 1)
					{
						add_road(a, get_area(x + 1, y - 1));
					}
				}
			}
		}
	}

	void Map::add_road(Area* a, Area* b)
	{
		bool orto = a->x == b->x || a->y == b->y;
		Road* r = new Road();
		r->cost = orto ? 1.0 : 1.414;
		r->a = a;
		r->b = b;
		r->dir = Road::direction(b->x - a->x, b->y - a->y);
		r->id = _roads.size();
		_roads.push_back(r);
		a->roads.push_back(r);
		b->roads.push_back(r);
		a->adjs.push_back(b);
		b->adjs.push_back(a);
	}
	
	const double trans_rate = 1.0;

	Area* Map::get_area(int x, int y)
	{
		return _areas[y * _width + x];
	}

	//AreaData& Map::get_trade(Area* a, int id)
	//{
	//	return _trade_maps[id]->get_trade(a);
	//}

	void Map::update()
	{
		//static int k = 0;
		//if (k++ % 1 == 0)
		//{
		//	for (TradeMap* product: _trade_maps)
		//	{
		//		product->update();
		//	}
		//}
	}

	void Map::create_road_map(Area* a)
	{
		int nn = _areas.size();
		Node* g = new Node[nn];
		if (a->map)
		{
			delete a->map;
		}
		a->map = new RoadMap();
		a->map->time = time;
		a->map->g = g;
		a->map->invalidated = false;
		for (int i = 0; i < nn; ++i)
		{
			g[i].area = _areas[i];
		}

		for (int i = 0; i < nn; ++i)
		{
			Node& n = g[i];
			n.color = 0;
			n.d = std::numeric_limits<double>::max();
			n.parent = NULL;
		}

		Node* src = &g[a->id];
		std::vector<Node*> Q;
		// std::make_heap(Q.begin(), Q.end());
		src->color = 1;
		src->d = 0;

		Q.push_back(src);
		// std::push_heap(Q.begin(), Q.end());

		auto pr = [](Node* a, Node* b) {
			return a->d > b->d;
		};

		while (Q.size() > 0)
		{
			//Node* n = Q.front();
			std::pop_heap(Q.begin(), Q.end(), pr);
			Node* n = Q.back();
			Q.pop_back();

			n->color = 2;
			Area* a = n->area;

			for (Road* r : a->roads)
			{
				Area* b = r->other(a);
				Node* m = &g[b->id];
				if (m->color < 2) // if m is not visited yet
				{
					double new_d = n->d + r->cost;
					if (new_d < m->d)
					{
						m->d = new_d;
						m->parent = r;
						if (m->color == 0)
						{
							m->color = 1;
							Q.push_back(m);
							std::push_heap(Q.begin(), Q.end(), pr);
						}
						else
						{
							std::make_heap(Q.begin(), Q.end(), pr);
						}
					}
				}
			}
		}
	}

	Route* Map::create_route(Area* start, Area* dest)
	{
		Route* route = new Route();
		RoadMap*& map = start->map;
		if (!map)
		{
			create_road_map(start);
		}
		if (!dest->map)
		{
			create_road_map(dest);
		}

		Node* n = &map->g[dest->id];
		Area* a = dest;
		route->a = start;
		route->b = dest;
		route->cost = 0;

		while (n->parent)
		{
			Road* r = n->parent;
			route->roads.push_back(r);
			route->cost += r->cost;
			a = r->other(a);
			n = &map->g[a->id];
		}

		auto& v = route->roads;
		std::reverse(v.begin(), v.end());
		return route;
	}

	double Map::distance(Area* start, Area* dest)
	{
		double result = 0;
		RoadMap*& map = start->map;
		if (!map)
		{
			create_road_map(start);
		}

		Node* n = &map->g[dest->id];
		Area* a = dest;

		while (n->parent)
		{
			Road* r = n->parent;
			result += r->cost;
			a = r->other(a);
			n = &map->g[a->id];
		}

		return result;
	}

	void Map::update_roads()
	{
		for (Road* r : _roads)
		{
			double level = (r->a->road_level + r->b->road_level) / 2.0;
			if (r->a->road_level > 0 && r->b->road_level > 0) level = 20;
			else level = 0;
			r->cost = r->base_cost * pow(0.5, level);
		}
	}

	int Road::direction(int x, int y)
	{
#define dir(i, j, d) if (x == i && y == - j) return d; else

		dir(-1, 0, 0) \
		dir(-1, -1, 1) \
		dir(0, -1, 2) \
		dir(1, -1, 3) \
		dir(1, 0, 4) \
		dir(1, 1, 5) \
		dir(0, 1, 6) \
		dir(-1, 1, 7) \
		dir(0, 0, 8);

#undef dir
	}
	std::vector<Area*> Route::areas()
	{
		std::vector<Area*> result;
		Area* a = this->a;
		result.push_back(a);
		for (Road* r : roads)
		{
			a = r->other(a);
			result.push_back(a);
		}
		return result;
	}
}