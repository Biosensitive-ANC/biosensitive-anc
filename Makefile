cpp_files := $(wildcard *.cpp)
o_files := $(patsubst %.cpp,%.o,$(cpp_files))

adaptiveNoise: $(o_files)
	g++ -o adaptiveNoise $(o_files) -lasound -lpthread

%.o: %.cpp
	g++ -c $< -o $@ -std=c++11

clean:
	rm *.o
	rm adaptiveNoise