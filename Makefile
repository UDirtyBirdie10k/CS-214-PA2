CC = gcc -Wall -Werror

spchk: spchk.c 
    $(CC) -o spchk spchk.c