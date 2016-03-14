#include "world_ui.h"
#include "economy.h"
#include "controls.h"
#include "animals.h"
#include <map>

namespace simciv
{

USING_NS_CC;
using namespace std;
using namespace ui;

PlantMapLayer* PlantMapLayer::create(WorldModel* model, UIStateData& info)
{
	PlantMapLayer* result = new PlantMapLayer(info);
	//result->_model = model;
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

#define DRAW_AREAS(area, exp) \
{ \
	double min = std::numeric_limits<double>::max(); \
	double max = std::numeric_limits<double>::min(); \
	vector<double> u(_model.areas().size()); \
	int i = 0; \
	for (Area* area : _model.areas()) \
	{ \
		double v = exp; \
		min = std::min(min, v); \
		max = std::max(max, v); \
		u[i++] = v; \
	} \
	double d = max - min; \
	i = 0; \
	if (d == 0) \
		{ \
		for (Area* a : _model.areas()) \
		{ \
			draw_rect(a->x, a->y, min, 1); \
		} \
	} \
	else \
	{ \
		for (Area* a : _model.areas()) \
		{ \
			draw_rect(a->x, a->y, (u[i++] - min) / d, 1); \
		} \
	} \
}

#define DRAW_AREAS_2(area, exp1, exp2) \
{ \
	double min1 = std::numeric_limits<double>::max(); \
	double max1 = std::numeric_limits<double>::min(); \
	double min2 = std::numeric_limits<double>::max(); \
	double max2 = std::numeric_limits<double>::min(); \
	vector<double> u1(_model.areas().size()); \
	vector<double> u2(_model.areas().size()); \
	int i = 0; \
	for (Area* area : _model.areas()) \
		{ \
		double v1 = exp1; \
		double v2 = exp2; \
		min1 = std::min(min1, v1); \
		max1 = std::max(max1, v1); \
		min2 = std::min(min2, v2); \
		max2 = std::max(max2, v2); \
		u1[i] = v1; \
		u2[i] = v2; \
		++i; \
	} \
	double d1 = max1 - min1; \
	double d2 = max2 - min2; \
	i = 0; \
	if (d1 == 0) \
	{ \
		if (d2 == 0)	for (Area* a : _model.areas()) draw_circles(a->x, a->y, min1, min2); \
		else			for (Area* a : _model.areas()) draw_circles(a->x, a->y, min1, (u2[i++] - min2) / d2); \
	} \
	else for (Area* a : _model.areas()) draw_circles(a->x, a->y, (u1[i] - min1) / d1, (u2[i++] - min2) / d2); \
}

void PlantMapLayer::onDraw(const Mat4 &transform, uint32_t flags)
{
	// calculate roads
	// if (info.plant) _model.products()[info.plant->id]->routes_to_areas();

    Director* director = Director::getInstance();
    director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);
    
    //draw
    CHECK_GL_ERROR_DEBUG();
	auto b = _map->getBoundingBox();

	if (info.show_grid)
	{
		glLineWidth(1);
		float x = b.getMinX();
		for (int i = 0; i <= _model.width(); ++i, x += cs)
		{
			DrawPrimitives::drawLine( Vec2(x, b.getMinY()), Vec2(x, b.getMaxY()));
		}
		float y = b.getMinY();
		for (int i = 0; i <= _model.height(); ++i, y += cs)
		{
			DrawPrimitives::drawLine( Vec2(b.getMinX(), y), Vec2(b.getMaxX(), y));
		}
	}

	double min_v = 1000;
	double max_v = 0;
	double min_vol = 1000;
	double max_vol = 0;

	switch (info.mode)
	{
	case MM_PRICE_SELL:
		if (info.plant) DRAW_AREAS(area, _model.get_prod(area, info.plant->id).p_sell);
		break;
	case MM_PRICE_BUY:
		if (info.plant) DRAW_AREAS(area, _model.get_prod(area, info.plant->id).p_buy);
		break;
	case MM_RESOURCES:
		if (info.plant) DRAW_AREAS(area, _model.get_prod(area, info.plant->id).resource);
		break;
	case MM_PROFIT:
		if (info.species) DRAW_AREAS(area, _model.get_profit(info.species, area));
		break;
	case MM_BUILD_COST:
		if (info.species) DRAW_AREAS(area, _model.get_build_cost(info.species, area));
		break;
	case MM_PROFIT_RES:
		if (info.species) DRAW_AREAS_2(area,
			_model.get_prod(area, info.plant->id).resource,
			_model.get_profit(info.species, area));
		break;
	default:
		break;
	}
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}

void PlantMapLayer::update(float delta)
{
	if (info.show_transport)
	{
		for (int prod_id = 0; prod_id < material_count; ++prod_id)
		{
			ProductMap* prod = _model.products()[prod_id];
			auto& v = prod->transports();
			for (auto transport : v)
			{
				auto it = transports.find(transport);
				if (it == transports.end())
				{
					if (transport->route->roads.size() > 0)
					{
						RouteAnimation* ani = new RouteAnimation();
						ani->set_route(prod_id, transport, this);
						transports[transport] = ani;
					}
				}
				else
				{
					Transport* t = it->first;
					if (t->volume == 0 && t->active_time + 50 < _model.time)
					{
						RouteAnimation* ani = it->second;
						ani->stop();
						transports.erase(it);
						delete ani;
					}
					else
					{
						//RouteAnimation* ani = it->second;
						//ani->start();

					}
				}
			}
		}
	}
}

RouteAnimation::RouteAnimation() : sprite(NULL), transport(NULL)
{
}

void RouteAnimation::set_route(int prod_id, Transport* transport, MapView* map)
{
	if (sprite) return;

	// auto it = route->roads.begin();
	Area* a = transport->seller->area;
	//Sprite* sprite = Sprite::create(get_animal_texture(ani->species.id));
	sprite = Sprite::create(get_plant_texture(prod_id));
	Rect r = map->get_rect(a->x, a->y);
	//sprite->setPosition(r.getMidX(), r.getMidY());
	//sprite->setScale(0.04f);
	sprite->setScale(0.1f);
	map->addChild(sprite);

	int cs = map->cell_size();
	this->transport = transport;
	float time = 0;

	
	Vector<FiniteTimeAction*> v;

	CCFiniteTimeAction* actionMove = CCMoveTo::create(0, ccp(a->x * cs + cs / 2, a->y * cs + cs / 2));
	v.pushBack(actionMove);

	for (auto r : transport->route->roads)
	{
		Area* b = r->other(a);
		//Area* a = r->a;
		//Area* b = r->b;

		int i = abs(b->x - a->x) + abs(b->y - a->y);

		CCFiniteTimeAction* actionMove = CCMoveTo::create(i == 2 ? sqrt(2) : 1, ccp(b->x * cs + cs / 2, b->y * cs + cs / 2));
		v.pushBack(actionMove);
		
		a = b;

		// time += 0.05;
		//a = b;
	}
	//CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create(this, callfuncN_selector(RouteAnimation::spriteMoveFinished));
	CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create([](CCNode* sender){});
	v.pushBack(actionMoveDone);
	// v.pushBack(NULL);
	
	sprite->runAction(CCRepeatForever::create(CCSequence::create(v)));
}

void RouteAnimation::stop()
{
	if (sprite)
	{
		sprite->removeFromParentAndCleanup(true);
		sprite = NULL;
	}
}

void RouteAnimation::start()
{
	if (sprite)
	{
		sprite->resume();
		sprite->setVisible(true);
		//sprite->getParent()->removeChild(sprite, true);
		//sprite = NULL;
	}
}

//
//void RouteAnimation::spriteMoveFinished(CCNode* sender)
//{
//	//CCSprite *sprite = (CCSprite *)sender;
//	//this->removeChild(sprite, true);
//}

}