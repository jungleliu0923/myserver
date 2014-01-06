.PHONY : all
.PHONY : clean

all:
	rm -rf output
	mkdir output
	echo "make server begin"
	make -C src
	echo "make server done"

clean:
	echo "make server clean begin"
	make -C src clean
	echo "make server clean done"
	rm -rf output

