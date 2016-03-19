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
		std::vector<Road*> _roads;
		RoadMap* map; ///< routes from this
		AreaData& data(Product* p);
		AreaData& data(int prod_id);
		Road* get_road(Area* b);
	};

	/// Road between two adjacent Area
	struct Road
	{
		Area* a;
		Area* b;
		double cost;
		Area* other(Area* a) { return a == this->a ? b : this->a; }
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