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

	RoadView* RoadLayer::add_road(Area * a, Area * b, Area * c, int level)
	{
		auto road = RoadView::create(level, b->dir(a), b->dir(c));
		road->level = level;
		add_road(b, road);
		return road;
	}

	void RoadLayer::add_road(Area* a, RoadView * road)
	{
		road->setAnchorPoint(Vec2(0.5, 0.5));
		road->setPosition(get_point(a->x, a->y));
		roads_node->addChild(road);
		roads[a->index].roads.push_back(road);
	}

	RoadView* RoadLayer::add_road(Area * a, int i, int j)
	{
		int level;
		if (j == 8)
		{
			level = (a->road_level + a->area(i)->road_level) / 2;
		}
		else
		{
			level = (2 * a->road_level + a->area(i)->road_level + a->area(j)->road_level) / 4;
		}

		auto r = RoadView::create(level, i, j);
		add_road(a, r);
		return r;
	}

	void RoadLayer::add_route(Route* route)
	{
		static vector<RoadView*> tmp_rws;
		for (auto rw : tmp_rws) rw->removeFromParent();
		tmp_rws.clear();

		auto as = route->areas();
		int n = as.size();
		if (n > 1)
		{
			tmp_rws.push_back(add_road(as[0], as[1], 3));
			tmp_rws.push_back(add_road(as[n - 1], as[n - 2], 3));
			for (int i = 1; i < n - 1; ++i)
			{
				tmp_rws.push_back(add_road(as[i - 1], as[i], as[i + 1], 3));
			}
		}
	}

	//void RoadLayer::add_road(Area * a, int i, int j)
	//{
	//	int level;
	//	if (j == 8)
	//	{
	//		level = min(a->road_level, a->area(i)->road_level);
	//	}
	//	else
	//	{
	//		level = min(min(a->road_level, a->area(i)->road_level), a->area(j)->road_level);
	//	}

	//	auto r = RoadView::create(level, i, j);
	//	add_road(a, r);
	//}

	void RoadLayer::add_road(Area * a)
	{
		if (a->road_level == world.max_road_level) return;
		++a->road_level;
		auto& da = roads[a->index];
		if (da.id == 0) da.id = ++road_index;
		update_roads();
		//update_roads(a);
		//for (auto r : a->roads)
		//{
		//	update_roads(r->other(a));
		//}
	}

	void RoadLayer::remove_road(Area * a)
	{
	}

	//void RoadLayer::update_roads()
	//{
	//	for (Area* a : world.areas())
	//	{
	//		update_roads(a);
	//	}
	//}

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

#define AREA(a) for (Area* a: world.areas())



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

		//int x = 0, xy = 0;
		//for (int i = 0; i < 8; ++i)
		//{
		//	if (v[i])
		//	{
		//		if (i % 2 == 0)
		//		{
		//			x += v[i]->road_level;
		//		}
		//		else
		//		{
		//			xy += v[i]->road_level;
		//		}
		//	}
		//}

		if (x > xy)
		{
			orientations[a->index] = O_ORTO;
		}
		else if (x < xy)
		{
			orientations[a->index] = O_DIAG;
		}
	}

	void set_orientation2(Area* a)
	{
		int orto = 0;
		int diag = 0;
		auto f = [&](Area* c)
		{
			if (orientations[c->index] == O_ORTO)
			{
				orto += c->road_level;
			}
			else if (orientations[c->index] == O_DIAG)
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
			orientations[a->index] = O_DIAG;
		}
		else if (orto > diag )
		{
			orientations[a->index] = O_ORTO;
		}
		else
		{
			orientations[a->index] = O_MIXED;
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
				if (orientations[a->index] != O_NONE)
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
				if (orientations[b->index] == O_NONE && find(q.begin(), q.end(), b) == q.end())
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
				if (orientations[b->index] == O_NONE && find(q.begin(), q.end(), b) == q.end())
				{
					q.push_back(b);
				}
			}
		}
	}

	void update_roads2(Area * a)
	{
		auto ori = orientations[a->index];
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
		auto ori = orientations[a->index];
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
		orientations.assign(world.areas().size(), O_NONE);
		roads2.assign(world.roads().size(), false);
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
			auto& da = roads[a->index];

			for (Sprite* s : da.roads)
			{
				s->removeFromParentAndCleanup(false);
			}

			da.roads.clear();
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
						add_road(a, i, j);
						s.emplace(i);
						s.emplace(j);
					}
				}
			};

			lofusz(4, 4);
			lofusz(3, 8);
			lofusz(2, 8);

			//for (int i = 0; i < 4; ++i)
			//{
			//	int j = (i + 4) % 8;
			//	if (marked(i) && marked(j))
			//	{
			//		add_road(a, i, j);
			//		s.emplace(i);
			//		s.emplace(j);
			//	}
			//}

			//for (int i = 0; i < 4; ++i)
			//{
			//	int j = (i + 3) % 8;
			//	if (marked(i) && marked(j) && (!drawed(i) || !drawed(j)))
			//	{
			//		add_road(a, i, j);
			//		s.emplace(i);
			//		s.emplace(j);
			//	}
			//}

			//for (int i = 0; i < 4; ++i)
			//{
			//	int j = (i + 5) % 8;
			//	if (marked(i) && marked(j) && (!drawed(i) || !drawed(j)))
			//	{
			//		add_road(a, i, j);
			//		s.emplace(i);
			//		s.emplace(j);
			//	}
			//}

			for (int i = 0; i < 8; ++i)
			{
				if (marked(i) && !drawed(i))
				{
					add_road(a, i, 8);
					s.emplace(i);
				}
			}
		}
	}

	RoadView * simciv::RoadLayer::add_road(Area * a, Area * b, int level)
	{
		auto road = RoadView::create(level, a->dir(b), 8);
		road->level = level;
		add_road(a, road);
		return road;
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
		Sprite* tmp = Sprite::create(file);
		auto size = tmp->getContentSize();
		world.max_road_level = (int)((size.height - 2*m + s) / (cols * (w + s)));

		for (int level = 0; level < world.max_road_level; ++level)
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