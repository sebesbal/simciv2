#pragma once
#include <vector>
#include <map>

namespace simciv
{
	struct Road;
	struct AreaProd;
	struct Producer;
	class ProductMap;
	struct Route;
	const double max_price = 100000000000;

	const int level_count = 4;
	const int color_count = 3;
	// const int material_count = level_count * color_count;
	extern int material_count;
	const int article_count = 3;

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

	class ArticleVec : public std::vector < double >
	{
	public:
		ArticleVec() { for (int i = 0; i < article_count; ++i) push_back(0); }
	};

	typedef std::map<int, double> MaterialMap;

	// typedef std::vector<double> MaterialVec;

	void bisect(const MaterialVec& v, MaterialVec& pos, MaterialVec& neg);

	struct RoadMap;

	struct Area
	{
		Area(int index, int pc);
		void get_trans(int id, double& x, double& y);
		int index;
		int x;
		int y;
		std::vector<Road*> _roads;
		RoadMap* map; ///< routes from this
		double data1; ///< used for drawing
		double data2; ///< used for drawing
	};

	struct Road
	{
		Road(int pc, double t_price);
		Area* a;
		Area* b;
		MaterialVec t; // transport
		double t_price;
		Area* other(Area* a) { return a == this->a ? b : this->a; }
	};

	struct Node
	{
		Node(): area(NULL), parent(NULL), color(0), d(0) {}
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

	struct Route
	{
		std::vector<Road*> roads;
		double trans_price;
	};

	/// The map. Graph of areas and roads.
	class WorldModel
	{
	public:
		WorldModel();
		virtual void create_map(int width, int height, int prod_count);
		const std::vector<Road*>& roads() { return _roads; }
		const std::vector<Area*>& areas() { return _areas; }
		virtual void update();
		//virtual void add_prod(Area* area, int prod_id, double volume, double price);
		//virtual void remove_prod(Area* area, int prod_id, double volume, double price);
		Area* get_area(int x, int y);
		int width() { return _width; }
		int height() { return _height; }
		AreaProd& get_prod(Area* a, int id);
		void create_road_map(Area* a);
		Route* create_route(Area* src, Area* dst);
		double distance(Area* src, Area* dst);
		std::vector<ProductMap*>& products() { return _products; }
		int time;
	protected:
		MaterialVec empty_mat_vec();
		std::vector<ProductMap*> _products;
		std::vector<Road*> _roads;
		std::vector<Area*> _areas;
		std::map<int, std::map<int, Route*>> _routes;
		int _pc; ///< Product count
		int _width;
		int _height;
		void add_road(Area* a, Area* b);
	};
}