CFLAGS=-O3

proyecto: client server converter

client: parser client.o octree.o font.o md2.o mymath.o map.o network.o players.o drawmap.o events.o
	gcc $(CFLAGS) -o build/client client/client.o client/octree.o client/font.o client/md2.o client/network.o client/players.o client/events.o client/drawmap.o common/mymath.o common/map.o common/y.tab.o common/lex.yy.o `sdl-config --libs` -lGL -lGLU -lSDL_image;

parser: 
	cd common;bison -y -d map_parser.y; flex map_parser.l; gcc $(CFLAGS) -c y.tab.c lex.yy.c;cd ..;

client.o : 
	cd client;gcc $(CFLAGS) -c client.c `sdl-config --cflags`;cd ..;

octree.o :
	cd client;gcc $(CFLAGS) -c octree.c;cd ..;

font.o : 
	cd client;gcc $(CFLAGS) -c font.c;cd ..;

md2.o : 
	cd client;gcc $(CFLAGS) -c md2.c;cd ..;

mymath.o : 
	cd common;gcc $(CFLAGS) -c mymath.c;cd ..;

map.o : 
	cd common;gcc $(CFLAGS) -c map.c;cd ..;

network.o :
	cd client;gcc $(CFLAGS) -c network.c;cd ..

players.o :
	cd client;gcc $(CFLAGS) -c players.c;cd ..

drawmap.o :
	cd client;gcc $(CFLAGS) -c drawmap.c;cd ..

events.o :
	cd client;gcc $(CFLAGS) -c events.c;cd ..

server: server.o mymath.o map.o collisionsys.o
	gcc $(CFLAGS) -o build/server server/server.o common/map.o common/mymath.o common/collisionsys.o -lpthread -lm

server.o :
	cd server;gcc $(CFLAGS) -c server.c;cd ..;

collisionsys.o :
	cd common;gcc $(CFLAGS) -c collisionsys.c;cd ..;

converter: parser convert.o map.o collisionsys.o mymath.o
	gcc $(CFLAGS) -o build/converter converter/convert.o common/map.o common/mymath.o common/collisionsys.o common/y.tab.o common/lex.yy.o -lm

convert.o: 
	cd converter;gcc $(CFLAGS) -c convert.c;cd ..;

clean :
	rm build/server build/client build/converter converter/*.o common/*.o client/*.o server/*.o common/lex.yy.c common/y.tab.h common/y.tab.c

tags: 
	etags `find . -name '*.[ch]'`
