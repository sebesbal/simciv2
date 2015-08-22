#pragma once
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "cocos2d.h"
#include "economy.h"
#include "animals.h"
#include "ui/UIImageView.h"
#include "ui/UILayout.h"

USING_NS_CC;

namespace simciv
{
	class MaterialStringView;

	ui::Layout* labelled_cb(std::string text, bool checked, ui::CheckBox::ccCheckBoxCallback cb);

	std::string get_animal_texture(int id);

	std::string get_plant_texture(int id);

	// typedef void(*int_cb)(int id);
	typedef std::function<void(int)> int_cb;

	class RadioBox : public ui::HBox
	{
	public:
		RadioBox(int* data, std::vector<std::string> labels, int hh, int marginy);
		void setSelected(int i);
		static RadioBox* create(int* data, std::vector<std::string> labels, int hh, int marginy);
		ui::Layout* labelled_radio(std::string text, ui::CheckBox::ccCheckBoxCallback cb);
		ui::Layout* image_radio(std::string img, ui::CheckBox::ccCheckBoxCallback cb);
		int hh;
		int marginy;
		std::vector<ui::Widget*> items;
		int* data;
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
		void set_selected_btn(int id);
		int find_btn(MenuButton* btn);
		void add_radio_button(MenuButton* btn);
		void set_on_changed(int_cb cb) { _on_changed = cb; }
		void set_toggle(bool toggle) { _toggle = toggle; }
	protected:
		int _space;
		bool _toggle;
		MenuButton* _selected;
		int_cb _on_changed;
		void on_btn_clicked(Ref* btn, Widget::TouchEventType type);
	};

	/// Show one species
	class SpeciesView : public ui::Layout
	{
	public:
		SpeciesView();
		static SpeciesView* create();
		bool init() override;
		void set_species(Species* species);
	protected:
		void add_prod_row(MaterialVec& prod);
		virtual void setContentSize(const Size & var) override;
		CCLayerColor* _bck;
		ui::ImageView* _icon;
		ui::Text* _name_label;
		ui::VBox* _production_view;
		ui::Text* _build_cost_label;
		MaterialStringView* _build_cost;
		MaterialStringView* _maintenance_cost;

		Species* _species;
	};

	///// Show one animals properties
	class AnimalView
	{

	};

	class MaterialSprite : public ui::ImageView
	{
	public:
		static MaterialSprite* create(int id, int size);
	};

	class MaterialStringView : public ui::Layout
	{
	public:
		static MaterialStringView* create();
		void set_vector(const MaterialVec& v, int size);
	};
}