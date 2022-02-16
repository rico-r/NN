
CC=gcc -Wunused -g -ggdb

output.bin: main.o dataset.o nn.o layer.o mat.o vec.o cmdargs.o
	$(CC) -o nn -lm main.o dataset.o nn.o layer.o mat.o vec.o cmdargs.o

main.o: main.c nn_cmd.c ae_cmd.c ds_cmd.c \
     dataset.h nn.h layer.h mat.h vec.h
	$(CC) -c main.c 

dataset.o: dataset.c dataset.h vec.h
	$(CC) -c dataset.c 

nn.o: nn.c nn.h layer.h mat.h vec.h
	$(CC) -c nn.c 

layer.o: layer.c layer.h mat.h vec.h
	$(CC) -c layer.c 

mat.o: mat.c mat.h vec.h
	$(CC) -c mat.c 

vec.o: vec.c vec.h
	$(CC) -c vec.c

cmdargs.o: cmdargs.c cmdargs.h
	$(CC) -c cmdargs.c

.PHONY: output.bin