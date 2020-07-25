CC=mpicc

mpi : src/mpi.c
	$(CC) -o prog src/mpi.c

clean : 
	rm prog