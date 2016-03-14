#pragma once
#include <vector>
#include <map>

namespace simciv
{
	struct Road;
	struct AreaTrade;
	struct Trader;
	class TradeMap;
	struct Route;


	struct RoadMap;

	struct Area
	{
		Area(int index);
		int index;
		int x;
		int y;
		std::vector<Road*> _roads;
		RoadMap* map; ///< routes from this
	};

	struct Road
	{
		Road(double cost);
		Area* a;
		Area* b;
		double cost;
		Area* other(Area* a) { return a == this->a ? b : this->a; }
	};

	struct Route
	{
		std::vector<Road*> roads;
		double cost;
	};

	/// The map. Graph of areas and roads.
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