#include "stdafx.h"
#include "indgen_ui.h"
#include "sprites.h"

namespace indgen
{
	bool IndGenUI::init()
	{
		if (!cocos2d::ui::Layout::init()) return false;

		auto c = simciv::CircleShape::create(Color4F(1, 0, 0, 1), Size(100, 100));
		c->setPosition(200, 200);
		addChild(c);

		EconomyType type;
		//type.economy = new Economy();

		return true;
	}

	cocos2d::Scene * IndGenUI::createScene()
	{
		auto scene = Scene::create();
		IndGenUI *layer = IndGenUI::create();
		layer->setContentSize(scene->getContentSize());
		scene->addChild(layer);
		return scene;
	}

}