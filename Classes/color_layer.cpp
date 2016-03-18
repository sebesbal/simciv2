#include "world_ui.h"

#include <map>

#include "trade.h"
#include "controls.h"
#include "world.h"

namespace simciv
{

USING_NS_CC;
using namespace std;
using namespace ui;

ColorMapLayer* ColorMapLayer::create(Map* model, UIStateData& info)
{
	ColorMapLayer* result = new ColorMapLayer(info);
	//result->world = model;
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

#define DRAW_AREAS(area, exp) \
{ \
	double min = std::numeric_limits<double>::max(); \
	double max = std::numeric_limits<double>::min(); \
	vector<double> u(world.areas().size()); \
	int i = 0; \
	for (Area* area : world.areas()) \
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
		for (Area* a : world.areas()) \
		{ \
			draw_rect(a->x, a->y, min, 1); \
		} \
	} \
	else \
	{ \
		for (Area* a : world.areas()) \
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
	vector<double> u1(world.areas().size()); \
	vector<double> u2(world.areas().size()); \
	int i = 0; \
	for (Area* area : world.areas()) \
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
		if (d2 == 0)	for (Area* a : world.areas()) draw_circles(a->x, a->y, min1, min2); \
		else			for (Area* a : world.areas()) draw_circles(a->x, a->y, min1, (u2[i++] - min2) / d2); \
	} \
	else for (Area* a : world.areas()) draw_circles(a->x, a->y, (u1[i] - min1) / d1, (u2[i++] - min2) / d2); \
}

void ColorMapLayer::onDraw(const Mat4 &transform, uint32_t flags)
{
	// calculate roads
	// if (info.product) world.trade_maps()[info.product->id]->routes_to_areas();

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
		for (int i = 0; i <= world.width(); ++i, x += cs)
		{
			DrawPrimitives::drawLine( Vec2(x, b.getMinY()), Vec2(x, b.getMaxY()));
		}
		float y = b.getMinY();
		for (int i = 0; i <= world.height(); ++i, y += cs)
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
		if (info.product) DRAW_AREAS(area, world.get_trade(area, info.product->id).p_sell);
		break;
	case MM_PRICE_BUY:
		if (info.product) DRAW_AREAS(area, world.get_trade(area, info.product->id).p_buy);
		break;
	case MM_PLANT_RESOURCES:
		if (info.product) DRAW_AREAS(area, world.get_trade(area, info.product->id).resource);
		break;
	case MM_PROFIT:
		if (info.industry) DRAW_AREAS(area, world.get_profit(info.industry, area));
		break;
	case MM_BUILD_COST:
		if (info.industry) DRAW_AREAS(area, world.get_build_cost(info.industry, area));
		break;
	case MM_SPECIES_RESOURCES:
		if (info.industry) DRAW_AREAS(area, world.get_resources(info.industry, area));
		break;
	case MM_PROFIT_RES:
		if (info.industry) DRAW_AREAS_2(area,
			world.get_trade(area, info.product->id).resource,
			world.get_profit(info.industry, area));
		break;
	default:
		break;
	}
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}

void ColorMapLayer::update(float delta)
{
	if (info.show_transport)
	{
		for (int prod_id = 0; prod_id < product_count; ++prod_id)
		{
			TradeMap* prod = world.trade_maps()[prod_id];
			auto& v = prod->transports();
			for (auto transport : v)
			{
				if (transport->volume == 0) continue;
				auto it = transports.find(transport);
				if (it == transports.end())
				{
					if (transport->route->roads.size() > 0)
					{
						TransportAnimation* f = new TransportAnimation();
						f->set_route(prod_id, transport, this);
						transports[transport] = f;
					}
				}
				else
				{
					Transport* t = it->first;
					if (t->volume == 0 && t->active_time + 50 < world.time)
					{
						TransportAnimation* f = it->second;
						f->stop();
						transports.erase(it);
						delete f;
					}
				}
			}
		}
	}
}

TransportAnimation::TransportAnimation() : sprite(NULL), transport(NULL)
{
}

void TransportAnimation::set_route(int prod_id, Transport* transport, MapView* map)
{
	if (sprite) return;

	Area* a = transport->seller->area;
	sprite = Sprite::create(get_product_texture(prod_id));
	Rect r = map->get_rect(a->x, a->y);
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
	//CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create(this, callfuncN_selector(TransportAnimation::spriteMoveFinished));
	CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create([](CCNode* sender){});
	v.pushBack(actionMoveDone);
	// v.pushBack(NULL);
	
	sprite->runAction(CCRepeatForever::create(CCSequence::create(v)));
}

void TransportAnimation::stop()
{
	if (sprite)
	{
		sprite->removeFromParentAndCleanup(true);
		sprite = NULL;
	}
}

void TransportAnimation::start()
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
//void TransportAnimation::spriteMoveFinished(CCNode* sender)
//{
//	//CCSprite *sprite = (CCSprite *)sender;
//	//this->removeChild(sprite, true);
//}

}