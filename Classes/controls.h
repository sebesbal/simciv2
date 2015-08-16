#pragma once
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "cocos2d.h"
#include "economy.h"
#include "animals.h"
#include "ui/UIImageView.h"

USING_NS_CC;

namespace simciv
{
	class MaterialStringView;

	ui::Layout* labelled_cb(std::string text, bool checked, ui::CheckBox::ccCheckBoxCallback cb);

	std::string get_animal_texture(int id);

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


	/// Choose animal
	class SpeciesBrowser: public ui::VBox
	{
	public:
		SpeciesBrowser();
		static SpeciesBrowser* create();
	};


	/// Show one species
	class SpeciesView : public CCLayer
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
		Sprite* _icon;
		ui::VBox* _production_view;
		MaterialStringView* _build_cost;
		MaterialStringView* _maintenance_cost;
		Species* _species;
	};

	///// Show production alternatives. Rows of MaterialStrings
	//class ProductionView : public ui::VBox
	//{
	//public:
	//	ProductionView();
	//	static ProductionView* create();
	//	void add_row(MaterialVec& prod);
	//};

	///// Show one animals properties
	class AnimalView
	{

	};

	class MaterialSprite : public ui::ImageView
	{
	public:
		static MaterialSprite* create(int id, int size);
	};

	class MaterialStringView : public CCLayer
	{
	public:
		static MaterialStringView* create();
		void set_vector(const MaterialVec& v, int size);
	};
}