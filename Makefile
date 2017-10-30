all:
	gcc machine.c pra.c -lm -o machine
	gcc sort.c -g -lm -o sort

	# Warning!  This line creates a file that is read by machine.c
	# Depending on your system, this file can end up being quite large (e.g., ~200MB)
	valgrind --tool=lackey --trace-mem=yes --log-fd=9 9>instructions.txt ./sort 

debug:
	gcc -g -D DEBUG machine.c pra.c -lm -o machine
	gcc sort.c -lm -o sort

final:
	gcc machine.c pra.c -lm -o machine