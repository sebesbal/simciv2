#include "world_ui.h"
#include <set>
#include <queue>

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
		auto road = RoadView::create(level, b->dir(a), b->dir(c));
		road->level = level;
		add_road(b, road);
	}

	void RoadLayer::add_road(Area* a, RoadView * road)
	{
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
		for (auto r : a->roads)
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

	using namespace std;

	enum Orientation
	{
		O_NONE,
		O_ORTO,
		O_DIAG
	};

	vector<int> orientations;

#define AREA(a) for (Area* a: world.areas())

	void set_orientation(Area* a)
	{
		//for (auto r : a->roads)
		//{

		//}

		AREA(a)
		{
			for (Area* b : a->adjs)
			{

			}
		}

		
	}

	//void RoadLayer::update_roads()
	//{
	//	auto& av = world.areas();
	//	queue<Area*> q;
	//	// q.push();

	//	for (Area* a : av)
	//	{

	//	}
	//}

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

		for (auto b : a->adjs)
		{
			int blevel = b->road_level;
			if (blevel == 0) continue;

			auto& db = roads[b->index];
			road_t rr;
			rr.dir = a->dir(b);
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

	cocos2d::SpriteFrame* RoadView::frames[5][9][9];

	cocos2d::SpriteBatchNode * RoadView::create_batch_node(std::string file)
	{
		SpriteBatchNode* result = SpriteBatchNode::create(file);
		int m = 1;
		int s = 1;
		int w = 50;
		const int cols = 6;
		int k = 0;

		for (int level = 0; level < 5; ++level)
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

	RoadView * RoadView::create(int level, const int & a)
	{
		return create(level, a, 8);
	}

	RoadView * RoadView::create(int level, const int & ad, const int & bd)
	{
		SpriteFrame* f = ad < bd ? frames[level - 1][ad][bd] : frames[level - 1][bd][ad];
		RoadView* s = RoadView::create();
		s->level = level;
		s->setSpriteFrame(f);
		return s;
	}
}