all: libmy1.so libmy2.so static_part dynamic_part

libmy1.so: lib1.c lib.h
	gcc -fPIC -shared -o libmy1.so lib1.c -lm

libmy2.so: lib2.c lib.h
	gcc -fPIC -shared -o libmy2.so lib2.c

static_part: static_part.c lib1.c lib.h
	gcc static_part.c lib1.c -o static_part -lm

dynamic_part: dynamic_part.c
	gcc dynamic_part.c -o dynamic_part -ldl

clean:
	rm -f *.so static_part dynamic_part