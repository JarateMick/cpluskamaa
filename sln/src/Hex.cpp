#include "Hex.h"

// hexgatron stuff
bool operator==(Hex a, Hex b)
{
	return a.q == b.q && a.r == b.r && a.s == b.s;
}
bool operator!=(Hex a, Hex b)
{
	return !(a == b);
}

Hex add(Hex a, Hex b)
{
	return Hex(a.q + b.q, a.r + b.r, a.s + b.s);
}

Hex subtract(Hex a, Hex b)
{
	return Hex(a.q - b.q, a.r - b.r, a.s - a.s);
}

Hex multiply(Hex a, int k)
{
	return Hex(a.q * k, a.r * k, a.s * k);
}

int hexLength(Hex hex)
{
	return int((abs(hex.q) + abs(hex.r) + abs(hex.s)) / 2);
}

int hexDistance(Hex a, Hex b)
{
	return hexLength(subtract(a, b));
}

// hex_directions[(6 + (direction % 6)) % 6]
const std::vector<Hex> hexDirections = {
	Hex(1, 0, -1), Hex(1, -1, 0), Hex(0, -1, 1),
	Hex(-1, 0, 1), Hex(-1, 1, 0), Hex(0, 1, -1)
};

Hex hex_direction(int direction /* 0 -> 5*/)
{
	ASSERT(0 <= direction && direction < 6);
	return hexDirections[direction];
}

Hex neighbor(Hex hex, int direction)
{
	return add(hex, hex_direction(direction));
}

struct Orientation
{
	const double f0, f1, f2, f3;
	const double b0, b1, b2, b3;
	const double startAngle; // multiples of 60

	Orientation(double f0, double f1, double f2, double f3,
		double b0, double b1, double b2, double b3, double startAngle) :
		f0(f0), f1(f1), f2(f2), f3(f3),
		b0(b0), b1(b1), b2(b2), b3(b3), startAngle(startAngle) {}
};

const Orientation layout_pointy = Orientation(sqrt(3.0), sqrt(3.0) / 2.0, 0.0, 3.0 / 2.0,
	sqrt(3.0) / 3.0, -1.0 / 3.0, 0.0, 2.0 / 3.0,
	0.5);
const Orientation layout_flat = Orientation(3.0 / 2.0, 0.0, sqrt(3.0) / 2.0, sqrt(3.0),
	2.0 / 3.0, 0.0, -1.0 / 3.0, sqrt(3.0) / 3.0,
	0.0);

struct Point
{
	double x, y;
	Point(double x, double y) : x(x), y(y) { }
};

struct Layout
{
	const Orientation orientation;
	Point size;
	Point origin;

	Layout(Orientation orientation, Point size, Point origin) :
		orientation(orientation), size(size), origin(origin) { }
};

Point hexToPixel(Layout layout, Hex h)
{
	const Orientation& M = layout.orientation;
	double x = (M.f0 * h.q + M.f1 * h.r) * layout.size.x;
	double y = (M.f2 * h.q + M.f3 * h.r) * layout.size.y;
	return Point(x + layout.origin.x, y + layout.origin.y);
}

struct FractionalHex {
	const double q, r, s;
	FractionalHex(double q_, double r_, double s_)
		: q(q_), r(r_), s(s_) {}
};

internal FractionalHex pixelToHex(Layout layout, Point p)
{
	const Orientation& M = layout.orientation;
	Point pt = Point((p.x - layout.origin.x) / layout.size.x,
		(p.y - layout.origin.y) / layout.size.y);
	double q = M.b0 * pt.x + M.b1 * pt.y;
	double r = M.b2 * pt.x + M.b3 * pt.y;
	return FractionalHex(q, r, -q - r);
}

internal Hex hex_round(FractionalHex h)
{
	int q = int(round(h.q));
	int r = int(round(h.r));
	int s = int(round(h.s));
	double q_diff = abs(q - h.q);
	double r_diff = abs(r - h.r);
	double s_diff = abs(s - h.s);
	if (q_diff > r_diff && q_diff > s_diff)
	{
		q = -r - s;
	}
	else if (r_diff > s_diff)
	{
		r = -q - s;
	}
	else
	{
		s = -q - r;
	}
	return Hex(q, r, s);
}

internal Point hexCornerOffset(Layout layout, int corner)
{
	Point size = layout.size;
	double angle = 2.0 * M_PI * (layout.orientation.startAngle + corner) / 6;
	return Point(size.x * cos(angle), size.y * sin(angle));
}

internal std::vector<Point> polygonCorners(Layout layout, Hex h)
{
	vector<Point> corners = {};
	Point center = hexToPixel(layout, h);
	for (int i = 0; i < 6; i++)
	{
		Point offset = hexCornerOffset(layout, i);
		corners.push_back(Point(center.x + offset.x, center.y + offset.y));
	}
	return corners;
}

void SaveHexMap(std::unordered_set<Hex> &map, char* fileName)
{
	// SDL_RWops* file = SDL_RWFromFile(fileName, "w+b");

	ALLEGRO_FILE* file = al_fopen(fileName, "wb");

	if (file == nullptr)
	{
		// file = SDL_RWFromFile(fileName, "w+b");
		printf("couldn't not open file");
		ASSERT(false);
	}

	printf("new file created");

	int count = 0;
	for (auto &hex : map)
	{
		// SDL_RWwrite(file, &hex, sizeof(Hex), 1);
		al_fwrite(file, &hex, sizeof(Hex));
		count++;
	}

	printf("writed: %i objects\n", count);
	al_fclose(file);
}

void ReadHexMaq(std::unordered_set<Hex> &map, char* fileName)
{
	ALLEGRO_FILE* file = al_fopen(fileName, "rb");

	if (file)
	{
		al_fseek(file, 0, ALLEGRO_SEEK_END);
		int64_t fileSize = al_ftell(file);
		al_fseek(file, 0, ALLEGRO_SEEK_SET);

		// int64_t res_size = SDL_RWsize(file);
		int64_t nb_read_total = 0, nb_read = 1;

		while (nb_read_total < fileSize && nb_read != 0)
		{
			Hex hex;
			nb_read = al_fread(file, &hex, sizeof(Hex));
			nb_read_total += nb_read;
			map.insert(hex);
		}

		al_fclose(file);
	}
	else
	{
		printf("couldn't open file");
	}
}

inline Hex getHexUnderCursor(game_state* gameState, EngineCore* core)
{
	static Layout layout = Layout(layout_pointy, { 20, 20 }, { 0, 0 }); // todo: nuke this
	return (hex_round(pixelToHex(layout, { (double)core->input->mouseX + gameState->cameraX, (double)core->input->mouseY + gameState->cameraY })));
}

// *********************
// some hex init code

//		Layout layout = Layout(layout_pointy, { 20, 20 }, { 0, 0 });
//		static std::vector<Point> poi; // = polygonCorners(layout, Hex(1, 1, -2));
//		int hexWidth = 20;
//		int hexHeight = 10;
//		int hexCount = hexWidth * hexHeight;
//
//		for (int i = 0; i < hexHeight; ++i)
//		{
//			for (int y = 0; y < hexWidth; y++)
//			{
//				std::vector<Point> tmp = polygonCorners(layout, Hex(i, y, -y - i));
//				poi.insert(poi.end(), tmp.begin(), tmp.end());
//			}
//		}
//
//		for (int i = 0; i < hexCount * 6; ++i)
//		{
//			// points[i].x = poi[i].x;
//			// points[i].y = poi[i].y;
//		}
//		gameState->pointcount = hexCount;
//		// gameState->points = points;
//
//		// std::unordered_map<Hex, float> heights;
//		// init memory hexes
//
//		// rectangle shape
//	//	int q1 = 5, q2 = 10;
//	//	int r1 = 5, r2 = 10;
//	//	for (int q = q1; q <= q2; q++)
//	//	{
//	//		for (int r = r1; r <= r2; r++)
//	//		{
//	//			heights.emplace(Hex(r, q, -q - -r));
//	//		}
//	// Position -= gameState->cameraX
//	//	}
//
//		// triangle 
//// 		int map_size = 10;
//// 		for (int q = 2; q <= map_size; q++)
//// 		{
//// 			for(int r = 2; r <= map_size - q; r++)
//// 			{
//// 				heights.emplace(Hex(q, r, -q-r));
//// 			}
//// 		}
////  
//		// hexagonal shape
//		int map_radius = 10;
//		for (int q = -map_radius; q <= map_radius; q++)
//		{
//			int r1 = std::max(-map_radius, -q - map_radius);
//			int r2 = std::min(map_radius, -q + map_radius);
//			for (int r = r1; r <= r2; r++)
//			{
//				heights.insert(Hex(q, r, -q - r));
//			}
//		}
