#pragma once
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

#include "cocos2d.h"
#include "world_model.h"
#include "ui\UICheckBox.h"
#include "ui\UILayout.h"
#include "ui\UIHBox.h"
#include "ui\UIVBox.h"
#include "ui\UIText.h"
#include "ui\UIButton.h"

#include "base\ccTypes.h"
#include "controls.h"
namespace simciv
{

USING_NS_CC;

class Item;
class RouteAnimation;

const Color3B def_bck_color3B(40, 0, 60);
const Color4B def_bck_color4B(40, 0, 60, 255);

#define defvec(vec, ...) \
	static const string arr ## vec[] = { __VA_ARGS__ }; \
	vector<string> vec (arr ## vec, arr ## vec + sizeof(arr ## vec) / sizeof(arr ## vec[0]) );

/// draw tiles, map background, routes
class MapView : public cocos2d::Layer
{
public:
	static MapView* create(WorldModel* model);
	virtual bool init() override;
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
	CustomCommand _customCommand;
    virtual void onDraw(const Mat4 &transform, uint32_t flags);
	virtual bool onTouchBegan(Touch* touch, Event  *event);
	virtual void onTouchEnded(Touch* touch, Event  *event);
	virtual void onTouchMoved(Touch* touch, Event  *event);

	void draw_rect(int x, int y, double rate, double alpha);
	void draw_rect_green(int x, int y, double rate, double alpha);
	void draw_triangles(int x, int y, double a, double b);
	void draw_circles(int x, int y, double a, double b);
	void draw_vec(Vec2 a, Vec2 v);
    Rect get_rect(int x, int y);
	Area* get_area(Vec2 p);
	int cell_size() { return cs; }
	void draw_areas(std::vector<double>& v);
protected:
	static const int cs = 33; // cell size
	Size _table;
	cocos2d::Node* _map;
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
	MM_SPECIES_RESOURCES
};

//struct UIMapData
//{
//	UIMapMode mode;
//	Species* species;
//	Plant* plant;
//};

struct UIStateData
{
	UIStateData() : 
		species(NULL),
		plant(NULL),
		mode(MM_NONE),
		// price_vol_mode(0),
		// produce_consume_mode(2),
		show_grid(false),
		show_transport(true),
		show_plants(true)
	{

	}
	UIMapMode mode;
	Species* species;
	Plant* plant;
	// int price_vol_mode;
	// int produce_consume_mode;
	bool show_grid;
	bool show_transport;
	bool show_plants;
};

/// Draw mines and factories
class PlantMapLayer : public MapView
{
public:
	PlantMapLayer(UIStateData& info) : info(info) { }
	static PlantMapLayer* create(WorldModel* model, UIStateData& info);
	void update(float delta);
protected:
	UIStateData& info;
	virtual void onDraw(const Mat4 &transform, uint32_t flags) override;
	
	void set_price_vol_mode(int i);
	void set_sup_buy_mode(int i);
	std::map<Transport*, RouteAnimation*> transports;
};

/// Render animals
class AnimalMapLayer : public MapView
{
public:
	static AnimalMapLayer* create(AnimalWorld* model);
	virtual bool init() override;
	//bool onTouchBegan(Touch* touch, Event  *event) override;
	//void onTouchEnded(Touch* touch, Event  *event) override;
	//void onTouchMoved(Touch* touch, Event  *event) override;
	Animal* create_animal(Area* a, Species& species);
	Sprite* create_sprite(Animal* ani);
	void create_sprites_from_model();
protected:
	// AnimalWorld& model() { return *(AnimalWorld*)_model; }
	bool is_map_point(cocos2d::Vec2& p);
	virtual void onDraw(const Mat4 &transform, uint32_t flags) override;
	Node* _animals;
};

enum UIState
{
	UIS_NONE,
	UIS_ANIMAL,
	UIS_PLANTS
};

/// The main ui
class WorldUI : public cocos2d::Layer
{
public:
	WorldUI();

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
    
    // a selector callback
    void menuCloseCallback(Ref* sender);
	virtual void onEnter() override;
    // implement the "static node()" method manually
    CREATE_FUNC(WorldUI);
protected:
	MyPopup* _popup;
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

	RadioMenu* _main_menu;
	RadioMenu* _species_browser;
	RadioMenu* _plants_browser;

	SpeciesView* _species_view;
	AnimalView* _animal_view;
	PlantMapLayer* _plant_layer;
	AnimalMapLayer* _animal_layer;
	ui::VBox* _animal_layers_panel;
	ui::VBox* _plant_layers_panel;
	ui::HBox* _play_panel;

	std::function<void()> _on_state_plant;
	std::function<void()> _on_state_build;
	std::function<void()> _on_state_animal;

	void tick(float f);
	void load_from_tmx(std::string tmx);
	virtual bool onTouchBegan(Touch* touch, Event  *event);
	virtual void onTouchEnded(Touch* touch, Event  *event);
	virtual void onTouchMoved(Touch* touch, Event  *event);
	// virtual void onMouseDown(Event  *event);
	virtual void onMouseMove(Event  *event);


	RadioMenu* create_left_menu();
	void create_play_panel();
	RadioMenu* create_species_browser();
	RadioMenu* create_plants_browser();
	void create_animal_layers_panel();
	void create_plant_layers_panel();
	virtual void setContentSize(const Size & var) override;
	void update_panels(bool animal, bool plants);
	void set_state(UIState state);
	void update_popup(const Vec2& p);
	void WorldUI::find_child(const cocos2d::Node* n, const Vec2& wp, cocos2d::Node*& child, int& z_order);
	cocos2d::Node* WorldUI::find_child(const cocos2d::Node* node, const Vec2& wp);
};

class RouteAnimation
{
public:
	RouteAnimation();
	// ~RouteAnimation() { if (sprite) sprite-> }
	void set_route(int prod_id, Transport* transport, MapView* map);
	void stop();
	void start();
private:
	Transport* transport;
	Sprite* sprite;
	//void spriteMoveFinished(CCNode* sender);
};

}