cpp_files := $(wildcard *.cpp)
h_files := $(wildcard *.h)
o_files := $(patsubst %.cpp,%.o,$(cpp_files))

adaptiveNoise: $(o_files)
	g++ -o adaptiveNoise $(o_files) -lasound -lpthread

%.o: %.cpp $(h_files)
	g++ -c $< -o $@ -std=c++11

clean:
	rm *.o
	rm adaptiveNoise