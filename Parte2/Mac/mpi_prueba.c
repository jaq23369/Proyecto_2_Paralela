/* Verifica comunicacion MPI y cantidad de nodos, no solo lanzamiento por SSH.
 * Uso: mpirun -np 2 ./mpi_prueba <nodos esperados>
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size, len;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int expected = argc == 2 ? atoi(argv[1]) : 1;
    char name[MPI_MAX_PROCESSOR_NAME] = {0};
    MPI_Get_processor_name(name, &len);
    char *names = calloc((size_t)size, sizeof(name));
    if (!names) MPI_Abort(MPI_COMM_WORLD, 1);

    int token = rank == 0 ? 123456789 : 0;
    MPI_Bcast(&token, 1, MPI_INT, 0, MPI_COMM_WORLD);
    long long value = rank + 1, sum = 0;
    MPI_Allreduce(&value, &sum, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allgather(name, sizeof(name), MPI_CHAR,
                  names, sizeof(name), MPI_CHAR, MPI_COMM_WORLD);
    int nodes = 0;
    for (int i = 0; i < size; ++i) {
        int unique = 1;
        for (int j = 0; j < i; ++j)
            if (strcmp(names + i * sizeof(name), names + j * sizeof(name)) == 0)
                unique = 0;
        nodes += unique;
    }
    int ok = expected > 0 && nodes == expected && token == 123456789 &&
             sum == (long long)size * (size + 1) / 2;
    int all_ok;
    MPI_Allreduce(&ok, &all_ok, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
    if (rank == 0) {
        printf("%s: %d procesos, %d nodos; token %d; suma %lld\n",
               all_ok ? "MPI_OK" : "MPI_ERROR", size, nodes, token, sum);
        for (int i = 0; i < size; ++i)
            printf("Proceso %d: %s\n", i, names + i * sizeof(name));
        if (!all_ok) printf("Se esperaban %d nodos distintos.\n", expected);
    }
    free(names);
    MPI_Finalize();
    return all_ok ? 0 : 1;
}
