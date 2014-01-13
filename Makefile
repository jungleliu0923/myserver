.PHONY : all
.PHONY : clean
.PHONY : server
.PHONY : client

all:
	rm -rf output
	mkdir output
	echo "make server begin"
	make -C src
	echo "make server done"

server:
	make -C sample_server

client:
	make -C sample_client

clean:
	echo "make server clean begin"
	make -C src clean
	echo "make server clean done"
	
	make -C sample_server clean
	make -C sample_client clean
	rm -rf output


