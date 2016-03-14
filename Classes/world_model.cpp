
#include "world_model.h"
#include <queue>
#include "economy.h"
#include "assert.h"

using namespace std;

namespace simciv
{
	int material_count;

	struct Node
	{
		Node() : area(NULL), parent(NULL), color(0), d(0) {}
		Road* parent;
		Area* area;
		double d;
		int color; // 0 = black, unvisited, 1 = gray, opened, 2 = white, visited
	};

	struct RoadMap
	{
		Node* g;
	};

	class NodeComparator
	{
	public:
		bool operator()(const Node* a, const Node* b)
		{
			return (a->d > b->d);
		}
	};

	void bisect(const MaterialVec& v, MaterialVec& pos, MaterialVec& neg)
	{
		pos.clear();
		neg.clear();
		for (double d : v)
		{
			if (d < 0)
			{
				pos.push_back(0);
				neg.push_back(-d);
			}
			else
			{
				pos.push_back(d);
				neg.push_back(0);
			}
		}
	}

	Area::Area(int index) : index(index), map(NULL)
	{
	}

	Road::Road(double t_price): t_price(t_price)
	{
	}

	Map::Map() : time(0)
	{

	}

	void Map::create_map(int width, int height, int prod_count)
	{
		_width = width;
		_height = height;
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				Area* a = new Area(_areas.size());
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

		for (int i = 0; i < prod_count; ++i)
		{
			_trade_maps.push_back( new TradeMap(i) );
		}
	}

	void Map::add_road(Area* a, Area* b)
	{
		bool orto = a->x == b->x || a->y == b->y;
		Road* r = new Road(orto ? 1.0 : 1.414);
		r->a = a;
		r->b = b;
		_roads.push_back(r);
		a->_roads.push_back(r);
		b->_roads.push_back(r);
	}
	
	const double trans_rate = 1.0;

	Area* Map::get_area(int x, int y)
	{
		return _areas[y * _width + x];
	}

	AreaTrade& Map::get_trade(Area* a, int id)
	{
		return _trade_maps[id]->get_trade(a);
	}

	void Map::update()
	{
		static int k = 0;
		if (k++ % 1 == 0)
		{
			for (TradeMap* product: _trade_maps)
			{
				product->update();
			}
		}
	}

	void Map::create_road_map(Area* a)
	{
		int nn = _areas.size();
		Node* g = new Node[nn];
		a->map = new RoadMap();
		a->map->g = g;
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

		Node* src = &g[a->index];
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

			for (Road* r : a->_roads)
			{
				Area* b = r->other(a);
				Node* m = &g[b->index];
				if (m->color < 2) // if m is not visited yet
				{
					double new_d = n->d + r->t_price;
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

	Route* Map::create_route(Area* src, Area* dst)
	{
		Route* route = new Route();
		RoadMap*& map = src->map;
		if (!map)
		{
			create_road_map(src);
		}

		Node* n = &map->g[dst->index];
		Area* a = dst;
		route->trans_price = 0;

		while (n->parent)
		{
			Road* r = n->parent;
			route->roads.push_back(r);
			route->trans_price += r->t_price;
			a = r->other(a);
			n = &map->g[a->index];
		}

		auto& v = route->roads;
		std::reverse(v.begin(), v.end());
		return route;
	}

	double Map::distance(Area* src, Area* dst)
	{
		double result = 0;
		RoadMap*& map = src->map;
		if (!map)
		{
			create_road_map(src);
		}

		Node* n = &map->g[dst->index];
		Area* a = dst;

		while (n->parent)
		{
			Road* r = n->parent;
			result += r->t_price;
			a = r->other(a);
			n = &map->g[a->index];
		}

		return result;
	}
}