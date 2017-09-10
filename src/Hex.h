#pragma once

struct Entity;

struct Hex // cube coordinates
{
	int q, r, s;
	Entity* OnTop = nullptr;
	int region = -1;

	Hex() = default;
	Hex(int q, int r, int s) : q(q), r(r), s(s)
	{
		// q + r + s == 0
		// ASSERT(q + r + s == 0);
	}
};

namespace std {
	template <> struct hash<Hex> {
		size_t operator()(const Hex& h) const {
			hash<int> int_hash;
			size_t hq = int_hash(h.q);
			size_t hr = int_hash(h.r);
			return hq ^ (hr + 0x9e3779b9 + (hq << 6) + (hq >> 2));
		}
	};
}

