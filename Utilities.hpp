#pragma once
#include <iostream>
#include <string>

void StringReplace(std::string* _str, std::string _find, std::string _insert);

class Random
{
public:

	Random(uint32_t _seed);

	uint32_t LehmerSeed;

	uint32_t Lehmer32();

	uint32_t Lehmer32(uint32_t _min, uint32_t _max);

};