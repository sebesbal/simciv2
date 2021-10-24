#pragma once

#include "cocos2d.h"
#include "sprites.h"

namespace simciv
{
	enum UIMapMode
	{
		MM_NONE,
		MM_PRICE_SELL,
		MM_PRICE_BUY,
		MM_PROFIT,
		MM_PLANT_RESOURCES,
		MM_BUILD_COST,
		MM_PROFIT_RES,
		MM_SPECIES_RESOURCES,
		MM_ROAD,
		MM_MIL_LEVEL
	};

	enum UIAction
	{
		UIA_NONE,
		UIA_FACT_CREATE,
		UIA_FACT_INFO,
		UIA_MIL_PLUS,
		UIA_MIL_MINUS,
		UIA_ROAD_PLUS,
		UIA_ROAD_MINUS,
		UIA_ROAD_ROUTE,
		UIA_EXPLORE
	};

	struct UIStateData
	{
		UIStateData() :
			industry(NULL),
			product(NULL),
			mode(MM_NONE),
			show_grid(false),
			show_transport(true),
			show_products(true)
		{

		}
		UIMapMode mode;
		Industry* industry;
		Product* product;
		bool show_grid;
		bool show_transport;
		bool show_products;
		UIAction action;
		//MilitaryImportance mil_importance;
	};

	/// draws tiles, map background, routes
	class MapView : public cocos2d::Node
	{
	public:
		CREATE_FUNC(MapView)
		Rect get_rect(int x, int y);
		Vec2 get_point(Area* a);
		Vec2 get_point(int x, int y);
		Area* get_area(Vec2 p);
		int cell_size() { return cs; }
		Vec2 dir(Area* a, Area* b);
	protected:
		//static const int cs = 32; // cell size
	};

	struct RoadInfo
	{
		//RoadInfo() : id(0) {}
		RoadInfo() {}
		//int id;
		std::set<RoadView*> roads;
	};

	class TileMapView : public MapView
	{
	public:
		virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) override;
		cocos2d::CustomCommand _customCommand;
		virtual void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);

		void draw_rect(int x, int y, float rate, float alpha);
		void draw_rect(int x, int y, Color3B& col);
		void draw_rect_green(int x, int y, float rate, float alpha);
		void draw_triangles(int x, int y, float a, float b);
		void draw_triangle(int x, int y, float a, float alpha);
		void draw_circles(int x, int y, float a, float b);
		void draw_circles2(int x, int y, float a, float alpha);
		void draw_circles(Area* a);
	};

	class RoadLayer : public MapView
	{
	public:
		RoadLayer();
		CREATE_FUNC(RoadLayer)
		void add_road(Area* a);
		void remove_road(Area* a);
		void update_roads();
		void add_road(RoadView* rw);
		void add_tmp_road(RoadView* rw, int color);
		void clear_roadviews(Area* a);
		void add_route(Route* route, int level);
		void add_tmp_route(Route* route, int level, int color);
		void finish_route();
		void clear_tmp_roads();
	protected:
		void clear_new_route();
		cocos2d::SpriteBatchNode* roads_node;
		cocos2d::SpriteBatchNode* tmp_roads_node[5];
		std::vector<RoadInfo> roads;
		std::vector<RoadView*> new_route; ///< temporal route's RoadViews
		int new_route_level;
	};

	/// Draws colored cells
	class ColorMapLayer : public TileMapView
	{
	public:
		ColorMapLayer(UIStateData& info);
		static ColorMapLayer* create(UIStateData& info);
		void update(float delta);
	protected:
		UIStateData& info;
		virtual void onDraw(const cocos2d::Mat4 &transform, uint32_t flags) override;
		std::map<Transport*, TransportAnimation*> transports;

	};

	/// Draws factories and other sprites
	class FactoryMapLayer : public MapView
	{
	public:
		CREATE_FUNC(FactoryMapLayer)
		virtual bool init() override;
		virtual void update(float dt) override;
		Factory* create_factory(Area* a, Industry* industry);
		Factory* update_or_create_factory(Area* a, Industry* industry);
		cocos2d::Sprite* create_sprite(Factory* f, Industry* ind = nullptr, Area* a = nullptr);
		void create_sprites_from_model();
		void delete_factory(Factory* f);
		FactorySprite* get_sprite(Area* a);
	protected:
		cocos2d::Node* _factories;
		std::vector<FactorySprite*> _factory_sprites;
	};

	extern FactoryMapLayer* g_factory_layer;
}
