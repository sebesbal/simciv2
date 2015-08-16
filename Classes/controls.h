#pragma once
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "cocos2d.h"
#include "economy.h"
#include "animals.h"

USING_NS_CC;

namespace simciv
{
	class ProductionView;

	ui::Layout* labelled_cb(std::string text, bool checked, ui::CheckBox::ccCheckBoxCallback cb);

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
	class SpeciesBrowser
	{
	};


	/// Show one species
	class SpeciesView : ui::HBox
	{
	public:
		SpeciesView();
		void set_species(Species* species);
	protected:
		ui::VBox* _left;
		ProductionView* _production_view;
		Species* _species;
	};

	/// Show production alternatives. Rows of MaterialStrings
	class ProductionView : ui::VBox
	{
	public:
		void add_row(MaterialVec& prod);
	};

	///// Show one animals properties
	//class AnimalMapLayer
	//{

	//};

	class MaterialSprite: Sprite
	{

	};

	class MaterialStringView
	{

	};
}