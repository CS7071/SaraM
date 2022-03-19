.PHONY: test

test: test_assign2_1

test_assign2_1: test_assign2_1.o storage_mgr.o dberror.o buffer_mgr.o
	gcc -o test_assign2_1 test_assign2_1.o storage_mgr.o dberror.o buffer_mgr.o
	./test_assign2_1

buffer_mgr.o: buffer_mgr.c buffer_mgr.h
	gcc -c -o buffer_mgr.o buffer_mgr.c

test_assign1_1.o: test_assign1_1.c test_helper.h
	gcc -c -o test_assign1_1.o test_assign1_1.c

storage_mgr.o: storage_mgr.c storage_mgr.h
	gcc -c -o storage_mgr.o storage_mgr.c

dberror.o: dberror.c dberror.h
	gcc -c -o dberror.o dberror.c
