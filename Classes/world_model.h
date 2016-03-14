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
	const double max_price = 100000000000;

	extern int material_count;

	int mat_id(int level, int color);

	class MaterialVec : public std::vector < double >
	{
	public:
		MaterialVec() { for (int i = 0; i < material_count; ++i) push_back(0); }
	};

	struct Prices
	{
		MaterialVec sell;
		MaterialVec buy;
	};

	typedef std::map<int, double> MaterialMap;

	// typedef std::vector<double> MaterialVec;

	void bisect(const MaterialVec& v, MaterialVec& pos, MaterialVec& neg);

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
		Road(double t_price);
		Area* a;
		Area* b;
		double t_price;
		Area* other(Area* a) { return a == this->a ? b : this->a; }
	};

	struct Route
	{
		std::vector<Road*> roads;
		double trans_price;
	};

	/// The map. Graph of areas and roads.
	class Map
	{
	public:
		Map();
		virtual void create_map(int width, int height, int prod_count);
		const std::vector<Road*>& roads() { return _roads; }
		const std::vector<Area*>& areas() { return _areas; }
		virtual void update();
		Area* get_area(int x, int y);
		int width() { return _width; }
		int height() { return _height; }
		AreaTrade& get_trade(Area* a, int id);
		void create_road_map(Area* a);
		Route* create_route(Area* src, Area* dst);
		double distance(Area* src, Area* dst);
		std::vector<TradeMap*>& trade_maps() { return _trade_maps; }
		int time;
	protected:
		std::vector<TradeMap*> _trade_maps;
		std::vector<Road*> _roads;
		std::vector<Area*> _areas;
		std::map<int, std::map<int, Route*>> _routes;
		int _width;
		int _height;
		void add_road(Area* a, Area* b);
	};
}