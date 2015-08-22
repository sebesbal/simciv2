#include "world_ui.h"
#include "economy.h"
#include "controls.h"

namespace simciv
{

USING_NS_CC;
using namespace std;
using namespace ui;


std::string factory_strings[4] = {
	"img/factory_red.png", "img/factory_blue.png", "img/factory_green.png", "img/factory_yellow.png"
};

std::string mine_strings[4] = {
	"img/mine_red.png", "img/mine_blue.png", "img/mine_green.png", "img/mine_yellow.png"
};



ProdView* ProdView::create(WorldModel* model)
{
	ProdView* result = new ProdView();
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

Item* ProdView::add_item(ItemType type, int x, int y)
{
	int ax = x / cs;
	int ay = y / cs;
	if (ax < 0 || ay < 0 || ax >= _model->width() || ay >= _model->height()) return NULL;

	Area* a = _model->get_area(ax, ay);

	switch (type)
	{
	case simciv::IT_MINE:
		{
			auto mine1 = Sprite::create(mine_strings[info.product_id]);
			mine1->setPosition(x, y);
			mine1->setScale(0.05);
			_items->addChild(mine1);
			_model->add_prod(a, info.product_id, 100, 10);
		}
		break;
	case simciv::IT_FACTORY:
		{
			auto factory1 = Sprite::create(factory_strings[info.product_id]);
			factory1->setPosition(x, y);
			factory1->setScale(0.2);
			_items->addChild(factory1);
			_model->add_prod(a, info.product_id, -100, 100);
		}
		break;
	default:
		break;
	}

	return NULL;
}

void ProdView::onTouchEnded(Touch* touch, Event  *event)
{
	//auto s = touch->getStartLocation();
	//auto p = touch->getLocation();
	//if (is_map_point(touch->getLocationInView()) && (p - s).length() < 10)
	//{
	//	p = _items->convertToNodeSpace(p);
	//	add_item(info.mode, p.x, p.y);
	//}
}


void ProdView::onTouchMoved(Touch* touch, Event  *event)
{
	//if (is_map_point(touch->getLocationInView()))
	//{
	//	auto diff = touch->getDelta();
	//	_map->setPosition(_map->getPosition() + diff);
	//	_items->setPosition(_items->getPosition() + diff);
	//}
}

void ProdView::onDraw(const Mat4 &transform, uint32_t flags)
{
	// calculate roads
	_model->products()[info.product_id]->routes_to_areas(info.product_id);

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

	if (info.show_price_vol_mode == 0)
	{
		if (info.show_sup_con_mode == 2)
		{
			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, info.product_id);
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
				auto& p = _model->get_prod(a, info.product_id);
				double v = p.p;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_con + p.v_sup;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
		else if (info.show_sup_con_mode == 0)
		{
			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, info.product_id);
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
				auto& p = _model->get_prod(a, info.product_id);
				double v = p.p_sup;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_sup;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
		else if (info.show_sup_con_mode == 1)
		{
			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, info.product_id);
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
				auto& p = _model->get_prod(a, info.product_id);
				double v = p.p_con;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_con;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
	}
	else if (info.show_price_vol_mode == 2)
	{
		for (Area* a: _model->areas())
		{
			auto& p = _model->get_prod(a, info.product_id);
			draw_rect_green(a->x, a->y, p.resource, 1);
		}
	}

	if (info.show_transport)
	{
		//DrawPrimitives::setDrawColor4F(0, 0, 1, 1);
		glLineWidth(3);
		double scale = 0.1;

		for (Area* a: _model->areas())
		{
			double x, y;
			a->get_trans(info.product_id, x, y);
			Rect r = get_rect(a->x, a->y);
			Vec2 p = Vec2(r.getMidX(), r.getMidY());
			DrawPrimitives::drawLine(p, Vec2(p.x + scale * x, p.y + scale * y));
		}
	}
}

}