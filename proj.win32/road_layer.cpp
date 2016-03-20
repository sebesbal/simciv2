#include "world_ui.h"
#include <set>

namespace simciv
{

	USING_NS_CC;
	using namespace std;

	RoadLayer::RoadLayer() : road_index(0), roads(world.areas().size())
	{
		roads_node = RoadView::create_batch_node("res/roads4.png");
		addChild(roads_node);
	}

	void RoadLayer::add_road(Area * a, Area * b, Area * c, int level)
	{
		Vec2 u(a->x - b->x, a->y - b->y);
		Vec2 v(c->x - b->x, c->y - b->y);
		auto road = RoadView::create(level, u, v);
		road->level = level;
		add_road(b, road);
	}

	void RoadLayer::add_road(Area* a, RoadView * road)
	{
		//road->setContentSize(Size(cs, cs));
		road->setAnchorPoint(Vec2(0.5, 0.5));
		road->setPosition(get_point(a->x, a->y));
		roads_node->addChild(road);
	}

	void RoadLayer::add_road(Area * a)
	{
		if (a->road_level == 5) return;
		++a->road_level;
		auto& da = roads[a->index];
		if (da.id == 0) da.id = ++road_index;
		update_roads(a);
		for (auto r : a->_roads)
		{
			update_roads(r->other(a));
		}
	}

	void RoadLayer::remove_road(Area * a)
	{
	}

	void RoadLayer::update_roads()
	{
		for (Area* a : world.areas())
		{
			update_roads(a);
		}
	}

	void RoadLayer::update_roads(Area * a)
	{
		auto& da = roads[a->index];

		for (Sprite* s : da.roads)
		{
			s->removeFromParentAndCleanup(false);
		}

		da.roads.clear();

		int alevel = a->road_level;
		if (alevel == 0) return;

		struct road_t
		{
			int dir;
			int level;
			int id;
		};
		vector<road_t> v;

		for (auto r : a->_roads)
		{
			Area* b = r->other(a);
			int blevel = b->road_level;
			if (blevel == 0) continue;

			auto& db = roads[b->index];
			road_t rr;
			rr.dir = RoadView::get_dir(dir(a, b));
			//rr.level = std::min(alevel, blevel);
			rr.level = (alevel + blevel) / 2;
			rr.id = db.id;
			v.push_back(rr);
		}

		if (v.size() == 0) return;

		//auto it = max_element(v.begin(), v.end(), [&](const road_t& a, const road_t& b)
		//{
		//	return a.level < b.level
		//		|| a.level == b.level && (a.dir % 2) > (b.dir % 2);
		//		// || a.level == b.level && a.id > b.id;
		//});

		auto it = max_element(v.begin(), v.end(), [&](const road_t& a, const road_t& b)
		{
			return (a.dir % 2) >(b.dir % 2);
		});

		if (it == v.end()) return;

		int root = it->dir;
		if (v.size() == 1)
		{
			auto r = RoadView::create(it->level, root);
			add_road(a, r);
			da.roads.push_back(r);
		}
		else
		{
			// set<int> s;
			road_t* s[8], *f[8];
			for (int i = 0; i < 8; ++i) s[i] = f[i] = NULL;
			f[root] = &*it;

			for (auto& rd : v)
			{
				// s.emplace(rd.dir);
				s[rd.dir] = &rd;
			}

			const int g[8] = { 0, 2, 4, 6, 1, 3, 5, 7 };
			const int h[8] = { 1, 3, 5, 7, 0, 2, 4, 6 };

			for (int k = 0; k < 8; ++k)
			{
				//int i = (root % 2 == 0) ? g[k] : h[k];
				int i = g[k];
				if (i == root || !s[i]) continue;

				int ad = (i + 7) % 8;
				int bd = (i + 1) % 8;

				//if ((root % 2 == 0) && (f[ad] || f[bd]))
				if ( (f[ad] || f[bd]))
				{
					continue;
				}

				f[i] = s[i];
				s[ad] = s[bd] = NULL;

				auto r = RoadView::create(s[i]->level, root, i);
				add_road(a, r);
				da.roads.push_back(r);
			}

			if (da.roads.size() == 0)
			{
				auto r = RoadView::create(it->level, root);
				add_road(a, r);
				da.roads.push_back(r);
			}


			//for (auto& rd : v)
			//{
			//	if (rd.id == it->id) continue;

			//	int dir = rd.dir;
			//	int ad = (dir - 1) % 8;
			//	int bd = (dir + 1) % 8;

			//	if (s.find(ad) != s.end() || s.find(bd) != s.end()) continue;

			//	auto r = RoadView::create(root, dir);
			//	r->level = rd.level;
			//	add_road(a, r);
			//	da.roads.push_back(r);
			//}
		}
	}
}