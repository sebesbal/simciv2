#pragma once
#include "cocos2d.h"
#include "ui/UIHBox.h"
#include "ui/UIVBox.h"

#include "map.h"
#include "controls.h"
#include "sprites.h"
#include "map_view.h"

namespace simciv
{

#define RUNUI(f) Director::getInstance()->getScheduler()->performFunctionInCocosThread(f)

class TransportAnimation;
class RoadView;

enum UIState
{
	UIS_NONE,
	UIS_BROWSING,
	UIS_FACTORY,
	UIS_PRODUCT,
	UIS_ROAD_AREA,
	UIS_ROAD_ROUTE,
	UIS_MILITARY 
};

/// The main ui
class WorldUI : public cocos2d::ui::Layout
{
public:
	CREATE_FUNC(WorldUI)
	~WorldUI() { if (_worker.joinable()) _worker.join(); }
	virtual bool init() override;
	static cocos2d::Scene* createScene();
    void menuCloseCallback(Ref* sender);
protected:
	std::thread _worker;
	Popup* _popup;
	bool _paused;
	int _speed;
	UIState _state;
	UIStateData info;
	cocos2d::Size _menu_size;
	int _menu_space;
	std::vector<cocos2d::Node*> views;
	// cocos2d::Node* _map;
	bool _drag_start;
	cocos2d::Vec2 _mouse_down_pos;
	Area* _drag_start_area;

	// left menu
	RadioMenu* _main_menu;
	RadioMenu* _industry_browser;
	RadioMenu* _products_browser;
	RadioMenu* _roads_menu;
	RadioMenu* _military_menu;

	// right panels
	IndustryView* _industry_view;
	FactoryView* _factory_view;
	MilitaryView* _military_view;

	// top panels
	EconomyView* _economy_view;
	Panel* _factory_layers_options;
	Panel* _color_layers_options;

	// map layers
	TMXTiledMap* _map;
	Size _grid_size;
	ColorMapLayer* _color_layer;
	RoadLayer* _road_layer;
	FactoryMapLayer* _factory_layer;
	Vec2 get_tile(Area* a);

	// pause, play
	cocos2d::ui::HBox* _play_panel;
	cocos2d::Sprite* _cursor;

	std::function<void()> _on_state_product;
	std::function<void()> _on_state_build;
	std::function<void()> _on_state_factory;

	void tick(float f);
	void load_from_tmx(std::string tmx);
	// Vec2 
	virtual bool onTouchBegan(cocos2d::Touch* touch, Event *event);
	virtual void onTouchEnded(cocos2d::Touch* touch, Event *event);
	virtual void onTouchMoved(cocos2d::Touch* touch, Event *event);
	virtual void onMouseMove(cocos2d::Event  *event);

	bool is_inside_cell(cocos2d::Vec2& p, Area* a = NULL);

	RadioMenu* create_left_menu();
	void create_play_panel();
	RadioMenu* create_industry_browser();
	RadioMenu* create_products_browser();
	RadioMenu* create_roads_menu();
	RadioMenu * create_military_menu();
	void create_option_panels();
	virtual void doLayout() override;
	void set_state(UIState state);
	void update_ui();
	void update_popup(const cocos2d::Vec2& p);
	cocos2d::Node* find_child(const cocos2d::Node* node, const Vec2& wp);
	void find_child(const cocos2d::Node* n, const cocos2d::Vec2& wp, cocos2d::Node*& child, int& z_order);
};

}