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

namespace simciv
{
	class ProductStringView;
	class MenuButton;
	class DataTable;

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

	class DataLabel;

	class Panel : public ui::Layout
	{
	public:
		CREATE_FUNC(Panel)
		virtual bool init() override;
		ui::Text* create_label(std::string text);
	protected:
		virtual void setContentSize(const Size & var) override;
		LayerColor* _bck;
	};

	class Popup : public ui::Layout
	{
	public:
		CREATE_FUNC(Popup)
	protected:
		CustomCommand _customCommand;
		virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
		virtual void onDraw(const Mat4 &transform, uint32_t flags);
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
		virtual void onDraw(const Mat4 &transform, uint32_t flags) override;
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
		virtual void update(float delta) override;
	protected:
		virtual void doLayout() override;
		Factory* _factory;
		ui::Text* _money_txt;
		DataLabel* _money_val;
		ui::HBox* create_producer_view(Trader* p);
		ui::HBox* create_producer_view2(Trader* p);
		ui::VBox* _producer_views;
	};

	class EconomyView : public Panel
	{
	public:
		CREATE_FUNC(EconomyView);
		virtual bool init() override;
		void add(Product* p);
	protected:
		DataTable* _table;
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

	class DataLabel : public ui::Text
	{
	public:
		static DataLabel* create(double* data) CREATE_FUNC_BODY(DataLabel, data);
		bool init(double* data);
		virtual void update(float delta) override
		{
			if (data) this->setText(to_string_with_precision(*data, 1));
		}
		double* data;
	};

	class DataTable : public ui::Layout
	{
	public:
		CREATE_FUNC(DataTable);
		bool init() override;
		void set_sizes(float cell_height, std::vector<float> col_sizes);
		void set_cell_size(float width, float height);
		void set_margins(float pad_x, float pad_y, float left, float top, float right, float bottom);
		virtual void doLayout() override;
		Node* create_row();
	protected:
		std::vector<float> col_sizes;
		std::vector<Node*> _rows;
		float pad_x, pad_y, left, top, right, bottom;
		float cell_width, cell_height;
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