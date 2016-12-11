#pragma once
#include <iomanip>
#include <functional>

#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "ui/UIImageView.h"
#include "ui/UILayout.h"

#include "trade.h"
#include "world.h"

USING_NS_CC;

const Color3B def_bck_color3B(40, 0, 60);
const Color4B def_bck_color4B(40, 0, 60, 255);
const int MARGIN_NORMAL = 5;
const int PAD_NORMAL = 5;

namespace simciv
{
	class ProductStringView;
	class MenuButton;
	class Table;

#define CREATE_FUNC_BODY(__TYPE__, ...) \
{ \
    __TYPE__ *pRet = new(std::nothrow) __TYPE__(); \
    if (pRet && pRet->init(##__VA_ARGS__)) \
    { \
        pRet->autorelease(); \
        return pRet; \
    } \
    else \
    { \
        delete pRet; \
        pRet = NULL; \
        return NULL; \
    } \
}

	ui::Layout* labelled_cb(std::string text, bool checked, ui::CheckBox::ccCheckBoxCallback cb);
	std::string get_product_texture(int id);

	typedef std::function<void(int)> int_cb;
	typedef std::function<void(MenuButton*)> button_cb;

	class RadioBox : public ui::HBox
	{
	public:
		RadioBox(std::vector<std::string> labels, int hh, int marginy);
		void setSelected(int i);
		int getSelected();
		void update();
		static RadioBox* create(std::vector<std::string> labels, int hh, int marginy);
		ui::Layout* labelled_radio(std::string text, ui::CheckBox::ccCheckBoxCallback cb);
		ui::Layout* image_radio(std::string img, ui::CheckBox::ccCheckBoxCallback cb);
		int hh;
		int marginy;
		std::vector<ui::Widget*> items;
		int selected;
		std::function<void(int)> changed;
	};

	class MenuButton : public ui::Button
	{
	public:
		MenuButton();
		CREATE_FUNC(MenuButton)
		static MenuButton* create(std::string texture);
		static MenuButton* create(Size size, std::string image, std::string bck_normal, std::string bck_selected, std::string bck_disabled);
		void set_toggle(bool toggle) { _is_toggle = toggle; }
		void set_selected(bool selected) { if (selected) onPressStateChangedToPressed(); else onPressStateChangedToNormal(); }
	protected:
		bool _is_toggle;
		ui::ImageView* _bck;
		ui::ImageView* _img;
		std::string _bck_normal, _bck_selected, _bck_disabled;
		virtual void onPressStateChangedToNormal() override;
		virtual void onPressStateChangedToPressed() override;
		virtual void onPressStateChangedToDisabled() override;
		virtual void onTouchEnded(Touch *touch, Event *unusedEvent) override;
	};

	class RadioMenu : public ui::VBox
	{
	public:
		CREATE_FUNC(RadioMenu)
		bool init() override;
		MenuButton* get_selected_btn() { return _selected; }
		void set_selected_btn(MenuButton* btn);
		void add_row();
		void add_radio_button(MenuButton* btn);
		void set_on_changed(button_cb cb) { _on_changed = cb; }
		void set_toggle(bool toggle) { _toggle = toggle; }
	protected:
		ui::HBox* _row;
		int _count;
		int _space;
		bool _toggle;
		MenuButton* _selected;
		button_cb _on_changed;
		void on_btn_clicked(Ref* btn, Widget::TouchEventType type);
	};

	class Label;

	class Panel : public ui::Layout
	{
	public:
		CREATE_FUNC(Panel)
		virtual bool init() override;
		ui::Text* create_label(std::string text);
	};

	class Popup : public Panel
	{
	public:
		CREATE_FUNC(Popup)
		virtual bool init() override;
		virtual void doLayout() override;
		Table* table;
	};

	class FactoryPopup : public Popup
	{
	public:
		CREATE_FUNC(FactoryPopup)
		bool init() override;
		void set_profit(double profit) { _profit = profit; }
		void set_cost(double cost) { _cost = cost; }
	protected:
		double _profit;
		double _cost;
	};

	/// Shows one Industry
	class IndustryView : public Panel
	{
	public:
		CREATE_FUNC(IndustryView)
		bool init() override;
		void set_industry(Industry* industry);
	protected:
		void add_prod_row(Products& prod);
		virtual void setContentSize(const Size & var) override;
		Node* _icon;
		ui::Text* _name_label;
		ui::VBox* _production_view;
		ui::Text* _build_cost_label;
		ProductStringView* _build_cost;
		ProductStringView* _maintenance_cost;
		Industry* _industry;
	};

	/// Shows one Factory's properties
	class FactoryView : public Panel
	{
	public:
		CREATE_FUNC(FactoryView)
		bool init() override;
		void set_factory(Factory* Factory);
		Factory* get_factory() { return _factory; }
		virtual void update(float delta) override;
	protected:
		virtual void doLayout() override;
		Factory* _factory;
		ui::Text* _money_txt;
		Label* _money_val;
		ui::HBox* create_producer_view(Trader* p);
		ui::HBox* create_producer_view2(Trader* p);
		ui::VBox* _producer_views;
	};

	class EconomyView : public Panel
	{
	public:
		CREATE_FUNC(EconomyView)
		virtual bool init() override;
		virtual void doLayout() override;
		void add(Product* p);
	protected:
		Table* _table;
	};

	class MilitaryView : public Panel
	{
	public:
		CREATE_FUNC(MilitaryView)
		virtual bool init() override;
		virtual void doLayout() override;
	protected:
		Slider* _global_mil_level;
	};

	class Diagram : public ui::Layout
	{
	public:
		static Diagram* create(history_t* data, int count, double min, double max)
			CREATE_FUNC_BODY(Diagram, data, count, min, max)
		bool init(history_t* data, int count, double min, double max);
	protected:
		double _min, _max;
		int _count;
		history_t* _data;
		CustomCommand _customCommand;
		ui::Text* _text;
		virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
		virtual void onDraw(const Mat4 &transform, uint32_t flags);
	};

	template <typename T>
	std::string to_string_with_precision(const T a_value, const int n = 6)
	{
		if (a_value == (int)a_value) return std::to_string((int)a_value);
		std::ostringstream out;
		out << std::fixed << std::setprecision(n) << a_value;
		return out.str();
	}

	enum LabelSize
	{
		LS_NONE,
		LS_SMALL,
		LS_NORMAL,
		LS_LARGE
	};

	class Label : public ui::Text
	{
	public:
		static Label* create(const std::string& text, const LabelSize& size = LS_NORMAL) CREATE_FUNC_BODY(Label, text, size);
		static Label* create(double* data, const LabelSize& size = LS_NORMAL) CREATE_FUNC_BODY(Label, data, size);
		bool init(const std::string& text, const LabelSize& size = LS_NORMAL);
		bool init(double* data, const LabelSize& size = LS_NORMAL);
		static float font_size(const LabelSize& size);
		virtual void update(float delta) override;
		double* data;
	};

	class Table : public ui::Layout
	{
	public:
		CREATE_FUNC(Table);
		bool init() override;
		// void set_sizes(float cell_height, std::vector<float> col_sizes);
		void add_column(float widht);
		void set_default_row_height(float height) { cell_height = height; }
		void set_cell_size(float width, float height);
		void set_margins(float pad_x, float pad_y, float left, float top, float right, float bottom);
		virtual void doLayout() override;
		Node* create_row(float height = 0);
		std::vector<Node*>& rows() { return _rows; }
	protected:
		std::vector<float> col_sizes;
		std::vector<Node*> _rows;
		float pad_x, pad_y, left, top, right, bottom;
		float cell_height;
	};

	class ProductSprite : public ui::ImageView
	{
	public:
		static ProductSprite* create(Product* p, int size = 20) CREATE_FUNC_BODY(ProductSprite, p, size);
		virtual bool init(Product* p, int size = 20);
	};

	class ProductStringView : public ui::Layout
	{
	public:
		static ProductStringView* create(int size);
		void set_vector(const Products& v, int size);
		void set_map(const ProductMap& m);
		void add_item(Product* p, double volume);
	protected:
		int _size;
	};
}