#pragma once
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "ui/UICheckBox.h"
#include "ui/UILayout.h"
#include "ui/UIHBox.h"
#include "ui/UIVBox.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/CocosGUI.h"
#include "base/ccTypes.h"

#include "map.h"
#include "controls.h"

namespace simciv
{

USING_NS_CC;

class Item;
class TransportAnimation;
class RoadView;

const Color3B def_bck_color3B(40, 0, 60);
const Color4B def_bck_color4B(40, 0, 60, 255);

#define defvec(vec, ...) \
	static const string arr ## vec[] = { __VA_ARGS__ }; \
	vector<string> vec (arr ## vec, arr ## vec + sizeof(arr ## vec) / sizeof(arr ## vec[0]) );

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
	static const int cs = 32; // cell size
};

class TileMapView : public MapView
{
public:
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
	CustomCommand _customCommand;
	virtual void onDraw(const Mat4 &transform, uint32_t flags);

	void draw_rect(int x, int y, double rate, double alpha);
	void draw_rect_green(int x, int y, double rate, double alpha);
	void draw_triangles(int x, int y, double a, double b);
	void draw_circles(int x, int y, double a, double b);
};

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
	MM_ROAD
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
};

struct RoadInfo
{
	RoadInfo() : id(0) {}
	int id;
	std::vector<cocos2d::Sprite*> roads;
};

class RoadLayer: public MapView
{
public:
	RoadLayer();
	CREATE_FUNC(RoadLayer)
	void add_road(Area* a);
	void remove_road(Area* a);
	void update_roads();
	RoadView* add_road(Area* a, Area* b, int level);
	RoadView* add_road(Area* a, Area* b, Area* c, int level);
	void add_road(Area* a, RoadView* road);
	RoadView* add_road(Area* a, int i, int j);
	void add_route(Route* route);
protected:
	SpriteBatchNode* roads_node;
	int road_index;
	void update_roads(Area* a);
	std::vector<RoadInfo> roads;
};

/// Draws colored cells
class ColorMapLayer : public TileMapView
{
public:
	ColorMapLayer(UIStateData& info);
	static ColorMapLayer* create(UIStateData& info);
	void update(float delta);
	void add_road(Road* r, int level);
protected:
	UIStateData& info;
	virtual void onDraw(const Mat4 &transform, uint32_t flags) override;
	std::map<Transport*, TransportAnimation*> transports;
	
};

/// Draws factories and other sprites
class FactoryMapLayer : public MapView
{
public:
	CREATE_FUNC(FactoryMapLayer)
	virtual bool init() override;
	Factory* create_factory(Area* a, Industry& industry);
	Sprite* create_sprite(Factory* f);
	void create_sprites_from_model();
protected:
	Node* _factories;
};

class RoadView : public cocos2d::Sprite
{
public:
	RoadView(): road(NULL), level(1), direction(0) {  }
	CREATE_FUNC(RoadView)
	int level;
	int direction;
	Road* road;
	static cocos2d::SpriteBatchNode* create_batch_node(std::string file);
	static RoadView* create(int level, const int& a);
	static RoadView* create(int level, const int& a, const int& b);
protected:
	static cocos2d::SpriteFrame* frames[5][9][9];
};

enum UIState
{
	UIS_NONE,
	UIS_FACTORY,
	UIS_PRODUCT,
	UIS_ROAD_AREA,
	UIS_ROAD_ROUTE
};

/// The main ui
class WorldUI : public cocos2d::Layer
{
public:
	WorldUI();
	static cocos2d::Scene* createScene();
    void menuCloseCallback(Ref* sender);
	virtual void onEnter() override;
    CREATE_FUNC(WorldUI)
protected:
	Popup* _popup;
	bool _paused;
	int _speed;
	UIState _state;
	UIStateData info;
	Size _menu_size;
	int _menu_space;
	int view_mode, new_view_mode;
	std::vector<Node*> views;
	cocos2d::Node* _map;
	bool _drag_start;
	cocos2d::Vec2 _mouse_down_pos;
	Area* _drag_start_area;

	RadioMenu* _main_menu;
	RadioMenu* _industry_browser;
	RadioMenu* _products_browser;
	RadioMenu* _roads_menu;

	IndustryView* _industry_view;
	FactoryView* _factory_view;
	ColorMapLayer* _color_layer;
	RoadLayer* _road_layer;
	FactoryMapLayer* _factory_layer;
	ui::VBox* _factory_layers_panel;
	ui::VBox* _color_layers_panel;
	ui::HBox* _play_panel;
	Sprite* _cursor;

	std::function<void()> _on_state_product;
	std::function<void()> _on_state_build;
	std::function<void()> _on_state_factory;

	void tick(float f);
	void load_from_tmx(std::string tmx);
	virtual bool onTouchBegan(Touch* touch, Event *event);
	virtual void onTouchEnded(Touch* touch, Event *event);
	virtual void onTouchMoved(Touch* touch, Event *event);
	virtual void onMouseMove(Event  *event);

	bool is_inside_cell(Vec2& p, Area* a = NULL);

	RadioMenu* create_left_menu();
	void create_play_panel();
	RadioMenu* create_industry_browser();
	RadioMenu* create_products_browser();
	RadioMenu* create_roads_menu();
	void create_factory_layers_panel();
	void create_color_layers_panel();
	virtual void setContentSize(const Size & var) override;
	void set_state(UIState state);
	void update_popup(const Vec2& p);
	void WorldUI::find_child(const cocos2d::Node* n, const Vec2& wp, cocos2d::Node*& child, int& z_order);
	cocos2d::Node* WorldUI::find_child(const cocos2d::Node* node, const Vec2& wp);
};

/// Animation for draw transports
class TransportAnimation
{
public:
	TransportAnimation();
	void set_route(int prod_id, Transport* transport, MapView* map);
	void stop();
	void start();
private:
	Transport* transport;
	Sprite* sprite;
};

}