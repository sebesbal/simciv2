#include "stdafx.h"

#include "world_ui.h"
#include <set>
#include <queue>

namespace simciv
{

	USING_NS_CC;
	using namespace std;

	RoadLayer::RoadLayer() : roads(world->areas().size())
	{
		//roads_node = RoadView::create_batch_node("res/roads4.png");
		//addChild(roads_node);

		for (int i = 0; i < 5; ++i)
		{
			tmp_roads_node[i] = RoadView::create_batch_node("res/roads_" + to_string(i) + ".png");
			addChild(tmp_roads_node[i]);
		}
		roads_node = tmp_roads_node[4];
	}

	void RoadLayer::add_road(RoadView * road)
	{
		roads_node->addChild(road);
		roads[road->area->id].roads.emplace(road);
		Area* a = road->area;
		road->setPosition(get_point(a->x, a->y));
	}

	void RoadLayer::add_tmp_road(RoadView * road, int color)
	{
		tmp_roads_node[color]->addChild(road);
		Area* a = road->area;
		road->setPosition(get_point(a->x, a->y));
	}

	void RoadLayer::clear_roadviews(Area * a)
	{
		auto& rs = roads[a->id];
		for (RoadView* r : rs.roads)
		{
			r->removeFromParentAndCleanup(false);
		}
		rs.roads.clear();
	}

	void RoadLayer::add_route(Route* route, int level)
	{
		new_route_level = level;
		clear_new_route();

		auto as = route->areas();
		int n = as.size();
		if (n > 1)
		{
			auto rv = RoadView::create(as[1], as[0], NULL, level);
			add_road(rv); new_route.push_back(rv);

			rv = RoadView::create(as[n - 2], as[n - 1], NULL, level);
			add_road(rv); new_route.push_back(rv);
			
			for (int i = 1; i < n - 1; ++i)
			{
				auto rv = RoadView::create(as[i - 1], as[i], as[i + 1], level);
				add_road(rv); new_route.push_back(rv);
			}
		}
	}

	void RoadLayer::add_tmp_route(Route * route, int level, int color)
	{
		auto as = route->areas();
		int n = as.size();
		if (n > 1)
		{
			auto rv = RoadView::create(as[1], as[0], NULL, level);
			add_tmp_road(rv, color);

			rv = RoadView::create(as[n - 2], as[n - 1], NULL, level);
			add_tmp_road(rv, color);

			for (int i = 1; i < n - 1; ++i)
			{
				auto rv = RoadView::create(as[i - 1], as[i], as[i + 1], level);
				add_tmp_road(rv, color);
			}
		}
	}

	void RoadLayer::finish_route()
	{
		for (RoadView* r : new_route)
		{
			Area* a = r->area;
			a->road_level = max(a->road_level, new_route_level);
		}
		clear_new_route();
		update_roads();
	}

	void RoadLayer::clear_tmp_roads()
	{
		//for (int i = 0; i < tmp_roads_node.size(); ++i)
		//{
		//	tmp_roads_node[i]->removeAllChildren();
		//}
		for (auto& node: tmp_roads_node)
		{
			node->removeAllChildren();
		}
	}

	void RoadLayer::clear_new_route()
	{
		for (auto r : new_route)
		{
			r->removeFromParent();
			roads[r->area->id].roads.erase(r);
		}
		new_route.clear();
	}

	void RoadLayer::add_road(Area * a)
	{
		if (a->road_level == world->max_road_level) return;
		++a->road_level;
		world->area_changed(a);
		auto& da = roads[a->id];
		update_roads();
	}

	void RoadLayer::remove_road(Area * a)
	{
		if (a->road_level == 0) return;
		--a->road_level;
		world->area_changed(a);
		if (a->road_level == 0)
		{
			clear_roadviews(a);
		}
		update_roads();
	}

	using namespace std;

	enum Orientation
	{
		O_NONE,
		O_ORTO,
		O_DIAG,
		O_MIXED
	};

	vector<int> orientations;
	vector<bool> roads2;
	vector<Area*> areas;

#define AREA(a) for (Area* a: world->areas())



	void set_orientation(Area* a)
	{
		if (a->road_level == 0) return;
		auto v = a->sorted_adjs();
		auto f = [&](int i, int j)
		{
			return (v[i] && v[j]) ? (v[i]->road_level + v[j]->road_level) : 0;
			//return (int)(v[i] && v[j]);
		};

		int x = f(0, 4) + f(2, 6);
		int xy = f(1, 5) + f(3, 7);

		if (x > xy)
		{
			orientations[a->id] = O_ORTO;
		}
		else if (x < xy)
		{
			orientations[a->id] = O_DIAG;
		}
	}

	void set_orientation2(Area* a)
	{
		int orto = 0;
		int diag = 0;
		auto f = [&](Area* c)
		{
			if (orientations[c->id] == O_ORTO)
			{
				orto += c->road_level;
			}
			else if (orientations[c->id] == O_DIAG)
			{
				diag += c->road_level;
			}
		};

		//f(a);
		for (Area* b : a->connected_adjs())
		{
			f(b);
		}

		if (diag > orto)
		{
			orientations[a->id] = O_DIAG;
		}
		else if (orto > diag )
		{
			orientations[a->id] = O_ORTO;
		}
		else
		{
			orientations[a->id] = O_MIXED;
		}
	}

	void set_orientations()
	{
		deque<Area*> q, w;

		AREA(a)
		{
			if (a->road_level > 0)
			{
				areas.push_back(a);
				set_orientation(a);
				if (orientations[a->id] != O_NONE)
				{
					w.push_back(a);
				}
			}
		}

		while (!w.empty())
		{
			Area* a = w.front();
			w.pop_front();
			for (Area* b : a->connected_adjs())
			{
				if (orientations[b->id] == O_NONE && find(q.begin(), q.end(), b) == q.end())
				{
					q.push_back(b);
				}
			}
		}

		while (!q.empty())
		{
			Area* a = q.front();
			q.pop_front();
			set_orientation2(a);
			for (Area* b : a->connected_adjs())
			{
				if (orientations[b->id] == O_NONE && find(q.begin(), q.end(), b) == q.end())
				{
					q.push_back(b);
				}
			}
		}
	}

	void update_roads2(Area * a)
	{
		auto ori = orientations[a->id];
		auto v = a->sorted_adjs();
		if (ori == O_ORTO)
		{
			//for (int i = 0; i < 8; i += 2) if (v[i]) roads2[a->road(i)->id] = true;
			if (v[0] && v[4])
			{
				roads2[a->road(0)->id] = roads2[a->road(4)->id] = true;
			}
			if (v[2] && v[6])
			{
				roads2[a->road(2)->id] = roads2[a->road(6)->id] = true;
			}
		}
		else if (ori == O_DIAG)
		{
			//for (int i = 1; i < 8; i += 2) if (v[i]) roads2[a->road(i)->id] = true;

			if (v[1] && v[5])
			{
				roads2[a->road(1)->id] = roads2[a->road(5)->id] = true;
			}
			if (v[3] && v[7])
			{
				roads2[a->road(3)->id] = roads2[a->road(7)->id] = true;
			}
		}
	}

	void update_roads3(Area * a)
	{
		auto ori = orientations[a->id];
		auto v = a->sorted_adjs();
		for (int i = 0; i < 8; ++i)
		{
			if (v[i] && !roads2[a->road(i)->id])
			{
				int j = (i + 1) % 8, k = (i + 7) % 8;
				if ( (v[j] && !roads2[a->road(j)->id])
				  && (v[k] && !roads2[a->road(k)->id])
				||	(!v[j] && !v[k])
				)
				{
					roads2[a->road(i)->id] = true;
				}
			}
		}
	}

	void RoadLayer::update_roads()
	{
		world->update_roads();
		orientations.assign(world->areas().size(), O_NONE);
		roads2.assign(world->roads().size(), false);
		areas.clear();
		set_orientations();

		for (Area* a : areas)
		{
			update_roads2(a);
		}

		for (Area* a : areas)
		{
			update_roads3(a);
		}

		deque<Area*> q;
		for (Area* a : areas)
		{
			clear_roadviews(a);
			q.push_back(a);
		}

		while (!q.empty())
		{
			Area* a = q.front();
			q.pop_front();

			auto v = a->sorted_roads();
			set<int> s;
			auto marked = [&](int i) { return v[i] && roads2[v[i]->id]; };
			auto drawed = [&](int i) { return s.find(i) != s.end(); };
			auto lofusz = [&](int dif, int n)
			{
				for (int i = 0; i < n; ++i)
				{
					int j = (i + dif) % 8;
					if (marked(i) && marked(j) && (!drawed(i) || !drawed(j)))
					{
						//add_road(a, i, j);
						add_road(RoadView::create(a, i, j));
						s.emplace(i);
						s.emplace(j);
					}
				}
			};

			lofusz(4, 4);
			lofusz(3, 8);
			lofusz(2, 8);

			for (int i = 0; i < 8; ++i)
			{
				if (marked(i) && !drawed(i))
				{
					add_road(RoadView::create(a, i, 8));
					s.emplace(i);
				}
			}
		}
	}

	cocos2d::SpriteFrame* RoadView::frames[5][9][9];

	cocos2d::SpriteBatchNode * RoadView::create_batch_node(std::string file)
	{
		SpriteBatchNode* result = SpriteBatchNode::create(file);
		int m = 1;
		int s = 1;
		int w = 50;
		const int cols = 6;
		int k = 0;
		Sprite* tmp = Sprite::create(file);
		auto size = tmp->getContentSize();
		world->max_road_level = (int)((size.height - 2*m + s) / (cols * (w + s)));

		for (int level = 0; level < world->max_road_level; ++level)
		{
			for (int i = 0; i < 9; ++i)
			{
				for (int j = i + 1; j < 9; ++j)
				{
					Road r;
					int row = k / cols;
					int col = k % cols;
					Rect rect(m + col * (w + s), m + row * (w + s), w, w);
					SpriteFrame* f = SpriteFrame::create(file, rect);
					f->retain();
					frames[level][i][j] = f;
					++k;
				}
			}
		}
		return result;
	}

	RoadView * RoadView::create(Area* a, const int & ad, const int & bd, int level)
	{
		if (level == -1)
		{
			level = road_level(a->area(ad), a, a->area(bd));
		}
		SpriteFrame* f = ad < bd ? frames[level - 1][ad][bd] : frames[level - 1][bd][ad];
		RoadView* rw = RoadView::create();
		rw->area = a;
		rw->level = level;
		rw->setSpriteFrame(f);
		rw->setAnchorPoint(Vec2(0.5, 0.5));
		return rw;
	}

	RoadView * RoadView::create(Area * a, Area * b, Area * c, int level)
	{
		if (level == -1)
		{
			level = road_level(a, b, c);
		}
		return RoadView::create(b, b->dir(a), b->dir(c), level);
	}

	int RoadView::road_level(Area * a, Area * b, Area * c)
	{
		if (c)
		{
			return (2 * b->road_level + a->road_level + c->road_level) / 4;
		}
		else
		{
			return (a->road_level + b->road_level) / 2;
		}
	}
}