#pragma once
#include <vector>
#include <map>

namespace simciv
{
	struct Area;
	struct Road;
	struct RoadMap;
	struct AreaData;
	struct Product;

	enum AreaType
	{
		AT_NONE,
		AT_PLAIN,
		AT_MOUNTAIN,
		AT_SEA
	};

	enum MilitaryImportance
	{
		MILIMP_NONE,
		MILIMP_LOW,
		MILIMP_NORMAL,
		MILIMP_HIGH
	};

	enum MilitaryState
	{
		MILS_UNEXPLORED,	
		MILS_EXPLORABLE,	// it's adjacent to explored areas
		MILS_EXPLORED

		// MILS_CONTROLLED,	// my territory
		// MILS_ALLIED,		// trade + military use
		//MILS_FRIENDLY,		// trad
		//MILS_UNFRIENDLY,	// no trade
		//MILS_ENEMY
	};

	struct MilData
	{
		MilitaryImportance importance;
		double health;

	};

	enum PlayerState
	{
		PLAYS_NONE,
		PLAYS_ME,
		PLAYS_ALLIED,		// trade + military use
		PLAYS_FRIENDLY,		// trade
		PLAYS_UNFRIENDLY,	// no trade
		PLAYS_ENEMY
	};

	struct Player
	{
		int id;
		std::string name;
		PlayerState state;
		//std::vector<Area*> areas;
	};

	/// One cell of the map
	struct Area
	{
		int id;						///< index in Map._areas
		int x;						///< x coordinate
		int y;						///< y coordinate
		int tile_gid;				///< TileMap GID (can be 0 when unexplored)
		int ori_tile_gid;			///< Original TileMap GID
		AreaType type;
		std::vector<Road*> roads;	///< connected roads
		std::vector<Area*> adjs;	///< unsorted adjacent areas
		int road_level;				///< level of road infrastructure on this area. Road->cost is calculated from this.
		int mil_level;				///< military importance of the area. 0=nothing, 1=explore, 2=low, 3=medium, 4=high
		MilitaryState mil_state;	///< friendly, enemy etc.
		Player* owner;
		RoadMap* map;				///< routes from this road
		AreaData& data(Product* p); ///< trade data for every Product
		AreaData& data(int prod_id);  ///< trade data for every Product
		int dir(Area * a);			///< get the direction for a given area
		int dir(Road * r);			///< get the direction for a given road
		Road* road(int dir);		///< get the road for a given direction
		Area* area(int dir);		///< get the area for a given direction
		Road* road(Area* b);		///< get the road to a given adjacent Area
		std::vector<Area*> sorted_adjs();	///< adjacent areas sorted in direction, filtered by road_level > 0
		std::vector<Road*> sorted_roads();	///< sorted and filtered roads to the sorted_adjs() Areas
		std::vector<Area*> connected_adjs(); ///< unsorted filtered adjacendt Areas
		bool is_explored() { return mil_state == MILS_EXPLORED; }
	};

	/// Road between two adjacent Area
	struct Road
	{
		int id;
		Area* a;				///< one end of the road
		Area* b;				///< other end of the road (adjacent to "a")
		double cost;			///< cost, modified by road_level
		double base_cost;		///< cost based only on Area.tile_gid
		Area* other(Area* a) { return a == this->a ? b : this->a; }
		int dir;				///< direction from 'a' to 'b'. 0 = W, 1 = NW, 2 = N, 3 = NE ..., 7 = SW
		static int direction(int x, int y); ///< eg.: W = 1 = direction(-1, 0); NE = 3 = direction(1, 1)
	};

	/// List of roads between two distant Area
	struct Route
	{
		std::vector<Road*> roads;	
		Area* a;					///< start of the route
		Area* b;					///< destination of the route
		double cost;				///< total cost
		std::vector<Area*> areas();
	};

	// Node for graph algorithms (eg. Dijstra)
	struct Node
	{
		Node() : area(NULL), parent(NULL), color(0), d(0) {}
		Road* parent;
		Area* area;
		double d;
		int color; // 0 = black, unvisited, 1 = gray, opened, 2 = white, visited
	};

	// Graph
	struct RoadMap
	{
		RoadMap(): invalidated(false) {}
		Node* g;
		int time;
		bool invalidated;
	};

	/// Graph of Areas and Roads
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
		Route* create_route(Area* start, Area* dest);
		double distance(Area* start, Area* dest);
		int time;
		void update_roads(); ///< update Road::cost
	protected:
		std::vector<Road*> _roads;
		std::vector<Area*> _areas;
		std::map<int, std::map<int, Route*>> _routes;
		int _width;
		int _height;
		void add_road(Area* a, Area* b);
	};
}