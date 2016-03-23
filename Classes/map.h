#pragma once
#include <vector>
#include <map>

namespace simciv
{
	struct Road;
	struct RoadMap;
	struct AreaData;
	struct Product;

	/// One cell of the map
	struct Area
	{
		int index;
		int x;
		int y;
		std::vector<Road*> roads;
		std::vector<Area*> adjs;
		int road_level;
		RoadMap* map; ///< routes from this
		AreaData& data(Product* p);
		AreaData& data(int prod_id);
		int dir(Area * a);
		int dir(Road * r);
		Road* road(int dir);
		Area* area(int dir);
		Road* road(Area* b);
		std::vector<Area*> sorted_adjs();
		std::vector<Road*> sorted_roads();
		std::vector<Area*> connected_adjs();
	};

	/// Road between two adjacent Area
	struct Road
	{
		int id;
		Area* a;				///< one end of the road
		Area* b;				///< other end of the road (adjacent to "a")
		double cost;			///< cost, modifed by road_level
		double base_cost;		///< cost based only the terrain 
		Area* other(Area* a) { return a == this->a ? b : this->a; }
		int dir;				///< direction from 'a' to 'b'. 0 = W, 1 = NW, 2 = N, 3 = NE ..., 7 = SW
		static int direction(int x, int y); ///< eg.: W = 1 = direction(-1, 0); NE = 3 = direction(1, 1)
	};

	/// List of roads between two distant Area
	struct Route
	{
		std::vector<Road*> roads;
		double cost;
	};

	/// Graph of the areas and roads.
	class Map
	{
	public:
		Map();
		virtual void create(int width, int height, int prod_count);
		const std::vector<Road*>& roads() { return _roads; }
		const std::vector<Area*>& areas() { return _areas; }
		virtual void update();
		Area* get_area(int x, int y);
		int width() { return _width; }
		int height() { return _height; }
		void create_road_map(Area* a);
		Route* create_route(Area* src, Area* dst);
		double distance(Area* src, Area* dst);
		int time;
	protected:
		std::vector<Road*> _roads;
		std::vector<Area*> _areas;
		std::map<int, std::map<int, Route*>> _routes;
		int _width;
		int _height;
		void add_road(Area* a, Area* b);
	};
}