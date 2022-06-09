#include "Spice.hpp"
#include <filesystem>

int main()
{

	if (!std::filesystem::exists("source")) std::filesystem::create_directory("source");
	if (!std::filesystem::exists("source/scripts")) std::filesystem::create_directory("source/scripts");
	if (!std::filesystem::exists("save")) std::filesystem::create_directory("save");

	if (!std::filesystem::exists("save/data.lua"))
	{
		std::ofstream data_file("save/data.lua");
		data_file.close();
	}

	Spice::Engine* engine = new Spice::Engine();
	engine->Run(1600, 900, "Spice Game");

}
