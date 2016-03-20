// icongen.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <utility>
#include <string>
#include <vector>


#include "cairo/cairo.h"


class TileSet
{
public:
	TileSet(int cols, int rows, int tile_width, int tile_height, int margin, int space);
	~TileSet();
	virtual void draw() = 0;
	void save(const std::wstring filename);
protected:
	int cols;
	int rows;
	int tile_width;
	int tile_height;
	int margin;
	int space;
	int pixel_width;
	int pixel_height;
	cairo_surface_t* surf;
	cairo_t* g;
};

struct Tile
{
	double w;
	double h;
	virtual void drawto(cairo_t* g, cairo_surface_t* surf) = 0;
};

typedef std::pair<double, double> point;

struct Road : public Tile
{
	point a, b;
	void set_directions(int i, int j);
	virtual void drawto(cairo_t* g, cairo_surface_t* surf) override;
};

class RoadTileSet: public TileSet
{
public:
	RoadTileSet(int cols, int rows, int tile_width, int tile_height, int margin, int space, std::vector<double>& line_width) :
		TileSet(cols, line_width.size() * rows, tile_width, tile_height, margin, space), line_width(line_width) { }
	virtual void draw() override;
	std::vector<double> line_width;
};

int main()
{
	std::vector<double> v;
	v.push_back(3);
	v.push_back(5);
	v.push_back(7);
	v.push_back(9);
	v.push_back(11);

	//for (int i = 0; i < 5; ++i)
	//{
		RoadTileSet map(6, 6, 50, 50, 1, 1, v);
		map.draw();
		map.save(L"../Resources/res/roads4.png");
	//}

	
    return 0;
}

TileSet::TileSet(int cols, int rows, int tile_width, int tile_height, int margin, int space) :
	cols(cols), rows(rows), tile_width(tile_width), tile_height(tile_height), margin(margin), space(space)
{
	pixel_width = 2 * margin + (cols - 1) * space + cols * tile_width;
	pixel_height = 2 * margin + (rows - 1) * space + rows * tile_height;
	surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, pixel_width, pixel_height);
	g = cairo_create(surf);
}

TileSet::~TileSet()
{
	cairo_destroy(g);
	cairo_surface_destroy(surf);
}

cairo_status_t cairo_write_to_file_cb(void *closure, const unsigned char *data, unsigned int length)
{
	FILE* file = (FILE*)closure;
	if (fwrite(data, 1, length, file) != length)
		return CAIRO_STATUS_WRITE_ERROR;
	else
		return CAIRO_STATUS_SUCCESS;
}

void TileSet::save(const std::wstring filename)
{
	FILE *file;
	errno_t err;
	if ((err = _wfopen_s(&file, filename.c_str(), L"wb")) == 0)  
	{
		cairo_surface_flush(surf);
		cairo_surface_write_to_png_stream(surf, cairo_write_to_file_cb, file);
		fclose(file);
	}
}


point operator+ (point& a, point& b)
{
	return point(a.first + b.first, a.second + b.second);
}
point operator- (point& a, point& b)
{
	return point(a.first - b.first, a.second - b.second);
}
point operator* (double d, point& a)
{
	return point(d * a.first, d * a.second);
}
point operator* (point& a, point& b)
{
	return point(a.first * b.first, a.second * b.second);
}

#define _(p) (p).first, (p).second

void Road::set_directions(int i, int j)
{
	const point v[9] = {
		point(-1, 0),
		point(-1, -1),
		point(0, -1),
		point(1, -1),
		point(1, 0),
		point(1, 1),
		point(0, 1),
		point(-1, 1),
		point(0, 0)
	};

	a = point(w / 2 * v[i].first, h / 2 * v[i].second);
	b = point(w / 2 * v[j].first, h / 2 * v[j].second);
}

void Road::drawto(cairo_t* g, cairo_surface_t * surf)
{
	//cairo_set_source_rgba(g, 1, 0, 0, 1);
	//cairo_rectangle(g, - w / 2, - h / 2, w, h);
	//cairo_fill(g);

	point d(0, 0);

	cairo_move_to(g, _(a));
	cairo_curve_to(g
		, _(d * a)
		, _(d * b)
		, _(b));
	cairo_stroke(g);

	//cairo_move_to(g, _(c - i - j));
	//cairo_curve_to(g, _(c), _(c), _(c + i + j));
	//cairo_stroke(g);
}

void RoadTileSet::draw()
{
	//cairo_set_source_rgba(g, 0, 0, 0, 1);
	//cairo_rectangle(g, 0, 0, pixel_width, pixel_height);
	//cairo_fill(g);

	cairo_set_source_rgba(g, 0, 0, 0, 1);
	//cairo_set_antialias(g, CAIRO_ANTIALIAS_GOOD);

	int k = 0;
	for (int level = 0; level < 5; ++level)
	{
		cairo_set_line_width(g, line_width[level]);
		for (int i = 0; i < 9; ++i)
		{
			for (int j = i + 1; j < 9; ++j)
			{
				Road r;
				int row = k / cols;
				int col = k % cols;
				r.w = r.h = 32;
				r.set_directions(i, j);
				cairo_save(g);
				//cairo_translate(g
				//	, 0.5 + margin + col * tile_width + ((col == 0) ? 0 : (col - 1) * space) + tile_width / 2.0
				//	, 0.5 + margin + row * tile_height + ((row == 0) ? 0 : (row - 1) * space) + tile_height / 2.0);

				cairo_translate(g
					, 0.5 + margin + col * (tile_width + space) + tile_width / 2.0
					, 0.5 + margin + row * (tile_height + space) + tile_height / 2.0);

				r.drawto(g, surf);

				cairo_restore(g);

				++k;
			}
		}
	}
}
