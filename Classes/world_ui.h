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

class TransportAnimation;
class RoadView;

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
	cocos2d::Size _menu_size;
	int _menu_space;
	int view_mode, new_view_mode;
	std::vector<cocos2d::Node*> views;
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
	cocos2d::ui::VBox* _factory_layers_panel;
	cocos2d::ui::VBox* _color_layers_panel;
	cocos2d::ui::HBox* _play_panel;
	cocos2d::Sprite* _cursor;

	std::function<void()> _on_state_product;
	std::function<void()> _on_state_build;
	std::function<void()> _on_state_factory;

	void tick(float f);
	void load_from_tmx(std::string tmx);
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
	void create_factory_layers_panel();
	void create_color_layers_panel();
	virtual void setContentSize(const cocos2d::Size & var) override;
	void set_state(UIState state);
	void update_popup(const cocos2d::Vec2& p);
	void WorldUI::find_child(const cocos2d::Node* n, const cocos2d::Vec2& wp, cocos2d::Node*& child, int& z_order);
	cocos2d::Node* WorldUI::find_child(const cocos2d::Node* node, const cocos2d::Vec2& wp);
};

}