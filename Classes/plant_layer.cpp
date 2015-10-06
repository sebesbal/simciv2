#include "world_ui.h"
#include "economy.h"
#include "controls.h"
#include <map>

namespace simciv
{

USING_NS_CC;
using namespace std;
using namespace ui;

PlantMapLayer* PlantMapLayer::create(WorldModel* model, UIStateData& info)
{
	PlantMapLayer* result = new PlantMapLayer(info);
	result->_model = model;
	if (result && result->init())
	{
		result->autorelease();
		return result;
	}
	else
	{
		CC_SAFE_DELETE(result);
		return nullptr;
	}
}

bool is_map_point(cocos2d::Vec2& p)
{
	//return p.y < 557 || p.x > 366;
	return p.x > 370 || p.y > 222;
}

void PlantMapLayer::onDraw(const Mat4 &transform, uint32_t flags)
{
	// calculate roads
	_model->products()[info.plant_id]->routes_to_areas(info.plant_id);

    Director* director = Director::getInstance();
    director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);
    
    //draw
    CHECK_GL_ERROR_DEBUG();
    
    // draw a simple line
    // The default state is:
    // Line Width: 1
    // color: 255,255,255,255 (white, non-transparent)
    // Anti-Aliased
    //  glEnable(GL_LINE_SMOOTH);
    //DrawPrimitives::drawLine( VisibleRect::leftBottom(), VisibleRect::rightTop() );
	auto b = _map->getBoundingBox();
	//DrawPrimitives::drawLine( Vec2(b.getMinX(), b.getMinY()), Vec2(b.getMaxX(), b.getMaxY()));
	//DrawPrimitives::drawLine( Vec2(b.getMinX(), b.getMaxY()), Vec2(b.getMaxX(), b.getMinY()));

	if (info.show_grid)
	{
		glLineWidth(1);
		float x = b.getMinX();
		for (int i = 0; i <= _model->width(); ++i, x += cs)
		{
			DrawPrimitives::drawLine( Vec2(x, b.getMinY()), Vec2(x, b.getMaxY()));
		}
		float y = b.getMinY();
		for (int i = 0; i <= _model->height(); ++i, y += cs)
		{
			DrawPrimitives::drawLine( Vec2(b.getMinX(), y), Vec2(b.getMaxX(), y));
		}
	}

	// draw_rect(5, 5, 1);

	double min_v = 1000;
	double max_v = 0;
	double min_vol = 1000;
	double max_vol = 0;

	if (info.price_vol_mode == 0)
	{
		if (info.produce_consume_mode == 2)
		{
			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, info.plant_id);
				double v = p.p;
				min_v = std::min(min_v, v);
				max_v = std::max(max_v, v);
				double vol = p.v_con + p.v_sup;
				min_vol = std::min(min_vol, vol);
				max_vol = std::max(max_vol, vol);
			}
			double d = max_v - min_v;
			double d_vol = max_vol - min_vol;

			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, info.plant_id);
				double v = p.p;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_con + p.v_sup;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
		else if (info.produce_consume_mode == 0)
		{
			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, info.plant_id);
				double v = p.p_sup;
				min_v = std::min(min_v, v);
				max_v = std::max(max_v, v);
				double vol = p.v_sup;
				min_vol = std::min(min_vol, vol);
				max_vol = std::max(max_vol, vol);
			}
			double d = max_v - min_v;
			double d_vol = max_vol - min_vol;

			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, info.plant_id);
				double v = p.p_sup;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_sup;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
		else if (info.produce_consume_mode == 1)
		{
			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, info.plant_id);
				double v = p.p_con;
				min_v = std::min(min_v, v);
				max_v = std::max(max_v, v);
				double vol = p.v_con;
				min_vol = std::min(min_vol, vol);
				max_vol = std::max(max_vol, vol);
			}
			double d = max_v - min_v;
			double d_vol = max_vol - min_vol;

			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, info.plant_id);
				double v = p.p_con;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_con;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
	}
	else if (info.price_vol_mode == 2)
	{
		for (Area* a: _model->areas())
		{
			auto& p = _model->get_prod(a, info.plant_id);
			draw_rect_green(a->x, a->y, p.resource, 1);
		}
	}

	//if (info.show_transport)
	//{
	//	//DrawPrimitives::setDrawColor4F(0, 0, 1, 1);
	//	glLineWidth(3);
	//	double scale = 5; // 0.1;

	//	for (Area* a: _model->areas())
	//	{
	//		double x, y;
	//		a->get_trans(info.plant_id, x, y);
	//		Rect r = get_rect(a->x, a->y);
	//		Vec2 p = Vec2(r.getMidX(), r.getMidY());
	//		DrawPrimitives::drawLine(p, Vec2(p.x + scale * x, p.y + scale * y));
	//	}
	//}

	if (info.show_transport)
	{
		//static bool first_run = true;
		//if (!first_run
		ProductMap* prod = _model->products()[info.plant_id];
		auto& v = prod->routes();
		for (auto route : v)
		{
			auto r = route->route;
			auto it = routes.find(r);
			if (it == routes.end())
			{
				RouteAnimation* ani = new RouteAnimation();
				ani->set_route(info.plant_id, r, this);
				routes[r] = ani;
			}
			else
			{

			}
		}


		//CCFiniteTimeAction* actionMove =
		//	CCMoveTo::create((float)actualDuration,
		//	ccp(0 - target->getContentSize().width / 2, actualY));
		//CCFiniteTimeAction* actionMoveDone =
		//	CCCallFuncN::create(this,
		//	callfuncN_selector(HelloWorld::spriteMoveFinished));
		//target->runAction(CCSequence::create(actionMove,
		//	actionMoveDone, NULL));

		//DrawPrimitives::setDrawColor4F(0, 0, 1, 1);
		//glLineWidth(3);
		//double scale = 5; // 0.1;

		//for (Area* a : _model->areas())
		//{
		//	double x, y;
		//	a->get_trans(info.plant_id, x, y);
		//	Rect r = get_rect(a->x, a->y);
		//	Vec2 p = Vec2(r.getMidX(), r.getMidY());
		//	DrawPrimitives::drawLine(p, Vec2(p.x + scale * x, p.y + scale * y));
		//}
	}
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}

void RouteAnimation::set_route(int prod_id, Route* route, MapView* map)
{
	auto it = route->roads.begin();
	Area* a = (*it)->a;
	//Sprite* sprite = Sprite::create(get_animal_texture(ani->species.id));
	Sprite* sprite = Sprite::create(get_plant_texture(prod_id));
	Rect r = map->get_rect(a->x, a->y);
	//sprite->setPosition(r.getMidX(), r.getMidY());
	sprite->setScale(0.02f);
	map->addChild(sprite);

	int cs = map->cell_size();
	this->route = route;
	float time = 0;

	
	Vector<FiniteTimeAction*> v;

	CCFiniteTimeAction* actionMove = CCMoveTo::create(0, ccp(a->x * cs + cs / 2, a->y * cs + cs / 2));
	v.pushBack(actionMove);

	for (auto r : route->roads)
	{
		Area* b = r->other(a);
		//Area* a = r->a;
		//Area* b = r->b;

		CCFiniteTimeAction* actionMove = CCMoveTo::create(3, ccp(b->x * cs + cs / 2, b->y * cs + cs / 2));
		v.pushBack(actionMove);
		
		

		// time += 0.05;
		a = b;
	}
	//CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create(this, callfuncN_selector(RouteAnimation::spriteMoveFinished));
	CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create([](CCNode* sender){});
	v.pushBack(actionMoveDone);
	// v.pushBack(NULL);
	
	sprite->runAction(CCRepeatForever::create(CCSequence::create(v)));
}
//
//void RouteAnimation::spriteMoveFinished(CCNode* sender)
//{
//	//CCSprite *sprite = (CCSprite *)sender;
//	//this->removeChild(sprite, true);
//}

}