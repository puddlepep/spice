#include "Utilities.hpp"

void StringReplace(std::string* _str, std::string _find, std::string _insert)
{
	int found = _str->find(_find);
	while (found >= 0) {
		if (found >= 0)
			_str->replace(found, _find.size(), _insert);
		found = _str->find(_find);
	}
}


Random::Random(uint32_t _seed)
{
	LehmerSeed = _seed;
}

uint32_t Random::Lehmer32()
{
	LehmerSeed += 0xe120fc15;
	uint64_t tmp;
	tmp = (uint64_t)LehmerSeed * 0x4a39b70d;
	uint32_t m1 = (tmp >> 32) ^ tmp;
	tmp = (uint64_t)m1 * 0x12fad5c9;
	uint32_t m2 = (tmp >> 32) ^ tmp;
	return m2;
}

uint32_t Random::Lehmer32(uint32_t _min, uint32_t _max)
{
	return Lehmer32() % (_max - _min + 1) + _min;
}
