#pragma once
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "cocos2d.h"
#include "trade.h"
#include "world.h"
#include "ui/UIImageView.h"
#include "ui/UILayout.h"
#include <iomanip>
#include <functional>

USING_NS_CC;

namespace simciv
{
	class MaterialStringView;
	class MenuButton;

	ui::Layout* labelled_cb(std::string text, bool checked, ui::CheckBox::ccCheckBoxCallback cb);

	std::string get_factory_texture(int id);

	std::string get_product_texture(int id);

	// typedef void(*int_cb)(int id);
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

	/// Show prices, profits, volumens etc.
	class MapLayersPanel
	{

	};


	class MenuButton : public ui::Button
	{
	public:
		MenuButton();
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
		RadioMenu();
		static RadioMenu* create();
		MenuButton* get_selected_btn() { return _selected; }
		void set_selected_btn(MenuButton* btn);
		//void set_selected_btn(int id);
		// int find_btn(MenuButton* btn);
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

	class DebugLabel;

	class MyPanel : public ui::Layout
	{
	public:
		MyPanel();
		ui::Text* create_label(std::string text);
		DebugLabel* create_data_label(double* data);
	protected:
		virtual void setContentSize(const Size & var) override;
		LayerColor* _bck;
	};

	class MyPopup : public ui::Layout
	{
	public:
		// MyPopup();
	protected:
		CustomCommand _customCommand;
		virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
		virtual void onDraw(const Mat4 &transform, uint32_t flags);
	};

	class FactoryPopup : public MyPopup
	{
	public:
		FactoryPopup();
		void set_profit(double profit) { _profit = profit; }
		void set_cost(double cost) { _cost = cost; }
	protected:
		double _profit;
		double _cost;
		virtual void onDraw(const Mat4 &transform, uint32_t flags) override;
	};

	/// Show one industry
	class IndustryView : public MyPanel
	{
	public:
		IndustryView();
		static IndustryView* create();
		bool init() override;
		void set_industry(Industry* industry);
	protected:
		void add_prod_row(Products& prod);
		virtual void setContentSize(const Size & var) override;
		
		ui::ImageView* _icon;
		ui::Text* _name_label;
		ui::VBox* _production_view;
		ui::Text* _build_cost_label;
		MaterialStringView* _build_cost;
		MaterialStringView* _maintenance_cost;

		Industry* _industry;
	};

	/// Show one factories properties
	class FactoryView : public MyPanel
	{
	public:
		FactoryView();
		static FactoryView* create();
		bool init() override;
		void set_factory(Factory* Factory);
		virtual void update(float delta) override;
	protected:
		virtual void setContentSize(const Size & var) override;
		virtual void doLayout() override;
		Factory* _factory;
		ui::Text* _money_txt;
		DebugLabel* _money_val;
		ui::HBox* create_producer_view(Trader* p);
		ui::HBox* create_producer_view2(Trader* p);
		ui::VBox* _producer_views;
	};

	class Diagram : public ui::Layout
	{
	public:
		Diagram() :_data(NULL), _min(0), _max(100) { }
		static Diagram* create();
		void set_data(history_t* data)
		{
			_data = data; 
		}
		void set_range(int count, double min, double max) { _count = count; _min = min; _max = max; }
	protected:
		double _min, _max;
		int _count;
		history_t* _data;
		CustomCommand _customCommand;
		ui::Text* _text;
		virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
		virtual void onDraw(const Mat4 &transform, uint32_t flags);
		//void draw(Renderer *renderer, const kmMat4& transform, bool transformUpdated) override;
	};

	template <typename T>
	std::string to_string_with_precision(const T a_value, const int n = 6)
	{
		if (a_value == (int)a_value) return std::to_string((int)a_value);
		std::ostringstream out;
		out << std::fixed << std::setprecision(n) << a_value;
		return out.str();
	}

	class DebugLabel : public ui::Text
	{
	public:
		DebugLabel() : data(NULL) { init(); autorelease(); scheduleUpdate(); }
		virtual void update(float delta) override { 
			//if (data) this->setText(std::to_string((int)(*data)));
			//if (data) this->setText(std::to_string((*data)));
			if (data) this->setText(to_string_with_precision(*data, 1));
		}
		double* data;
	};

	//class TraderView : public ui::Layout
	//{

	//};

	class MaterialSprite : public ui::ImageView
	{
	public:
		static MaterialSprite* create(int id, int size);
	};

	class MaterialStringView : public ui::Layout
	{
	public:
		static MaterialStringView* create(int size);
		void set_vector(const Products& v, int size);
		void set_map(const ProductMap& m);
		void add_item(int prod_id, double volume);
	protected:
		int _size;
	};
}