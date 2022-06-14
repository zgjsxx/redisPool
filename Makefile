all:demo example
demo:
	g++ RedisPool.cpp  main.cpp ServerLog.cpp -g -o main -std=c++11 -levent -llog4cxx -lpthread -lhiredis \
		-I/usr/local/include/hiredis \
		-Wl,-rpath=/usr/local/lib
example:
	gcc example.c -g -o example -levent -llog4cxx -lpthread -lhiredis \
		-I/usr/local/include/hiredis \
		-Wl,-rpath=/usr/local/lib	
.phony clean:
	rm -rf ./main
