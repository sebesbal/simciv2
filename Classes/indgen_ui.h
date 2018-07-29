#pragma once
#include "cocos2d.h"
#include "ui/UIHBox.h"
#include "ui/UIVBox.h"

#include "map.h"
#include "controls.h"
#include "sprites.h"
#include "map_view.h"

#include "indgen.h"

namespace indgen
{

	class IndGenUI : public cocos2d::ui::Layout
	{
	public:
		CREATE_FUNC(IndGenUI)
		virtual bool init() override;
		static cocos2d::Scene* createScene();
	protected:
		Economy* economy;
	};
}