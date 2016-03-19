// icongen.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <utility>
#include <string>


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
	int w;
	int h;
	virtual void drawto(cairo_t* g, cairo_surface_t* surf) = 0;
};

struct Road : public Tile
{
	virtual void drawto(cairo_t* g, cairo_surface_t* surf) override;
};

class RoadTileSet: public TileSet
{
public:
	RoadTileSet(int cols, int rows, int tile_width, int tile_height, int margin, int space) :
		TileSet(cols, rows, tile_width, tile_height, margin, space) { }
	virtual void draw() override;
};

int main()
{
	RoadTileSet map(6, 6, 50, 50, 1, 1);
	map.draw();
	map.save(L"map.png");
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

typedef std::pair<double, double> point;
point operator+ (point& a, point& b)
{
	return point(a.first + b.first, a.second + b.second);
}
point operator- (point& a, point& b)
{
	return point(a.first - b.first, a.second - b.second);
}

#define _(p) (p).first, (p).second

void Road::drawto(cairo_t* g, cairo_surface_t * surf)
{
	point i(w / 2, 0);
	point j(0, h / 2);
	point c(0, 0);
	cairo_move_to(g, _(c - i - j));
	cairo_curve_to(g, _(c), _(c), _(c + i + j));
	cairo_stroke(g);
}

void RoadTileSet::draw()
{
	// cairo_set_source_rgba(g, 1, 1, 1, 1);
	// cairo_rectangle(g, 0, 0, pixel_width, pixel_height);
	// cairo_fill(g);
	cairo_set_source_rgba(g, 0, 0, 0, 1);
	cairo_set_antialias(g, CAIRO_ANTIALIAS_GOOD);
	cairo_set_line_width(g, 4);


	int k = 0;
	for (int i = 0; i < 9; ++i)
	{
		for (int j = i + 1; j < 9; ++j)
		{
			Road r;
			int row = k / rows;
			int col = k % cols;
			r.w = r.h = 32;
			cairo_save(g);
			cairo_translate(g
				, margin + col * tile_width + ((col == 0) ? 0 : (col - 1) * space) + tile_width / 2
				, margin + row * tile_height + ((row == 0) ? 0 : (row - 1) * space) + tile_height / 2);

			r.drawto(g, surf);

			cairo_restore(g);

			++k;
		}
	}
}
