# 指定编译器
CC=g++
#指定编译选项
CFLAGS=-Wall -g -std=c++17 -march=armv8.2-a
Target=main
Src:=$(wildcard ./*.cpp)
Objs:=$(patsubst %.cpp,%.o, $(Src))
#指定头文件位置
INCLUDES = -I ./include


$(Target):$(Objs)
	echo $(Objs)
	@mkdir -p output
	$(CC) $(Objs) -o output/$(Target)

%.o:%.cpp
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	rm $(Objs)
