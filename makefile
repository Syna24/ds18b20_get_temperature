SRC=$(filter-out $(SRC_DIR)server.c $(SRC_DIR)client.c,$(wildcard $(SRC_DIR)*.c))
OBJ=$(patsubst %.c,%.o,$(SRC))
SRC_DIR=./source/

all:install client server
	@make clear

libobj:
	gcc -fPIC -c $(SRC) -I./include
	cp *.o ./source/	

mylib:libobj
	gcc -shared -o libmylib.so $(OBJ)
	cp ./*.so ./install/

client:mylib
	gcc ./source/client.c -o client -I./include -L./install -lzlog -lsqlite3 -lmylib 

server:mylib
	gcc ./source/server.c -o server -I./include -L./install -lzlog -lsqlite3 -lmylib
 
install:
	mkdir install
	cp ./zlog/lib/* ./install

clear:
	rm -f *.o ./source/*.o
	rm *.so

clean:
	rm client server
	rm -rf install
	rm  *.db
	rm  *.log

