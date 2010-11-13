CC=gcc
CFLAGS=-O2 -march=native

proyecto: build/client build/server build/converter

build/client: common/y.tab.o common/lex.yy.o client/client.o client/octree.o client/font.o client/md2.o common/mymath.o common/map.o client/network.o client/players.o client/drawmap.o client/events.o
	$(CC) $(CFLAGS) -o build/client client/client.o client/octree.o client/font.o client/md2.o client/network.o client/players.o client/events.o client/drawmap.o common/mymath.o common/map.o common/y.tab.o common/lex.yy.o `sdl-config --libs` -lGL -lGLU -lSDL_image;

common/y.tab.o: common/y.tab.c
	$(CC) $(CFLAGS) -o common/y.tab.o -c common/y.tab.c

common/lex.yy.o: common/lex.yy.c
	$(CC) $(CFLAGS) -o common/lex.yy.o -c common/lex.yy.c

common/y.tab.c: common/map_parser.y
	cd common;bison -y -d map_parser.y;cd ..;

common/lex.yy.c: common/map_parser.l
	cd common;flex map_parser.l;cd ..;

client/client.o : client/client.c
	$(CC) $(CFLAGS) -o client/client.o -c client/client.c `sdl-config --cflags`;

client/octree.o : client/octree.c
	$(CC) $(CFLAGS) -o client/octree.o -c client/octree.c;

client/font.o : client/font.c
	$(CC) $(CFLAGS) -o client/font.o -c client/font.c;

client/md2.o : client/md2.c
	$(CC) $(CFLAGS) -o client/md2.o -c client/md2.c;

client/network.o : client/network.c
	$(CC) $(CFLAGS) -o client/network.o -c client/network.c;

client/players.o : client/players.c
	$(CC) $(CFLAGS) -o client/players.o -c client/players.c;

client/drawmap.o : client/drawmap.c
	$(CC) $(CFLAGS) -o client/drawmap.o -c client/drawmap.c;

client/events.o : client/events.c
	$(CC) $(CFLAGS) -o client/events.o -c client/events.c;

common/mymath.o : common/mymath.c
	$(CC) $(CFLAGS) -o common/mymath.o -c common/mymath.c;

common/map.o : common/map.c
	$(CC) $(CFLAGS) -o common/map.o -c common/map.c;

build/server: server/server.o common/mymath.o common/map.o common/collisionsys.o
	$(CC) $(CFLAGS) -o build/server server/server.o common/map.o common/mymath.o common/collisionsys.o -lpthread -lm

server/server.o : server/server.c
	$(CC) $(CFLAGS) -o server/server.o -c server/server.c

common/collisionsys.o : common/collisionsys.c
	$(CC) $(CFLAGS) -o common/collisionsys.o -c common/collisionsys.c;

build/converter: common/y.tab.o common/lex.yy.o converter/convert.o common/map.o common/collisionsys.o common/mymath.o
	$(CC) $(CFLAGS) -o build/converter converter/convert.o common/map.o common/mymath.o common/collisionsys.o common/y.tab.o common/lex.yy.o -lm

converter/convert.o: converter/convert.c
	$(CC) $(CFLAGS) -o converter/convert.o -c converter/convert.c;

clean :
	rm build/server build/client build/converter converter/*.o common/*.o client/*.o server/*.o common/lex.yy.c common/y.tab.h common/y.tab.c

tags: 
	etags `find . -name '*.[ch]'`
