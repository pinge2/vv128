FILE := collisions.cpp

all: debug


debug:
	@clear
	@g++ $(FILE) -DDEBUG -O1 -o main && ./main


max:
	@clear
	@g++ $(FILE) -O3 -flto -o main && ./main


clear:
	@rm main