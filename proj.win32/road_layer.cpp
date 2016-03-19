#include "world_ui.h"

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
		auto road = RoadView::create(u, v);
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
			Vec2 dir;
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
			rr.dir = dir(a, b);
			rr.level = std::min(alevel, blevel);
			rr.id = db.id;
			v.push_back(rr);
		}

		auto it = max_element(v.begin(), v.end(), [&](const road_t& a, const road_t& b)
		{
			return a.level < b.level || a.level == b.level && a.id > b.id;
		});

		if (it == v.end()) return;

		Vec2 root = it->dir;

		if (v.size() == 1)
		{
			auto r = RoadView::create(root);
			r->level = it->level;
			add_road(a, r);
			da.roads.push_back(r);
		}
		else
		{
			for (auto& rd : v)
			{
				if (rd.id == it->id) continue;
				auto r = RoadView::create(root, rd.dir);
				r->level = rd.level;
				add_road(a, r);
				da.roads.push_back(r);
			}
		}
	}
}