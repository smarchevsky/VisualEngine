if not exist .\build64 (
	mkdir build64
)
cd build64
cmake -DCMAKE_GENERATOR_PLATFORM=x64 ..	