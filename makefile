build:
	clang++ *.cpp *.hpp dep/imgui/*.cpp -Idep -Idep/imgui -Ishaders -Ldep/lua544/lib -l:liblua.a -lGL -lGLEW -lglfw -std=c++17
