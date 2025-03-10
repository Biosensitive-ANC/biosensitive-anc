files = 

all: main.o ancMixing.o whiteNoise.o oled.o rec_uart.o EEG.o i2c_manager.o
	g++ -o adaptiveNoise main.o ancMixing.o whiteNoise.o oled.o rec_uart.o EEG.o i2c_manager.o -lasound -lpthread

main.o:
	g++ -c main.cpp -o main.o -std=c++11

ancMixing.o: ancMixing.h ancMixing.cpp
	g++ -c ancMixing.cpp -o ancMixing.o -std=c++11

whiteNoise.o: whiteNoise.cpp *.h
	g++ -c whiteNoise.cpp -o whiteNoise.o -std=c++11

oled.o: oled.cpp *.h
	g++ -c oled.cpp -o oled.o -std=c++11

rec_uart.o: rec_uart.cpp *.h
	g++ -c rec_uart.cpp -o rec_uart.o -std=c++11

EEG.o: EEG.cpp *.h
	g++ -c EEG.cpp -o EEG.o -std=c++11

i2c_manager.o: i2c_manager.cpp *.h
	g++ -c i2c_manager.cpp -o i2c_manager.o -std=c++11

clean:
	rm *.o
	rm adaptiveNoise