
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define NUM_PARTICLES (1024 * 1024 * 10)
#define DT 0.01

typedef struct
{
    double x, y, z;
    double vx, vy, vz;
} Particle;

int main()
{
    Particle *particles = (Particle *)malloc(NUM_PARTICLES * sizeof(Particle));

    if (!particles)
    {
        perror("malloc falhou");
        return 1;
    }

    // Inicializa partículas
    printf("Inicializando partículas...\n");

    #pragma omp parallel for
    for (long long i = 0; i < NUM_PARTICLES; i++)
    {
        particles[i].x = (double)rand() / RAND_MAX * 100.0;
        particles[i].y = (double)rand() / RAND_MAX * 100.0;
        particles[i].z = (double)rand() / RAND_MAX * 100.0;
        particles[i].vx = (double)rand() / RAND_MAX * 10.0 - 5.0;
        particles[i].vy = (double)rand() / RAND_MAX * 10.0 - 5.0;
        particles[i].vz = (double)rand() / RAND_MAX * 10.0 - 5.0;
    }

    printf("Partículas inicializadas.\n");

    double start_time, end_time;
    const double GRAVITY = -9.81;

    start_time = omp_get_wtime();

    printf("Simulando um passo (versão sections)...\n");

    #pragma omp parallel
    {
        #pragma omp sections
        {
            // 1. Atualiza posições
            #pragma omp section
            {
                for (long long i = 0; i < NUM_PARTICLES; i++)
                {
                    particles[i].x += particles[i].vx * DT;
                    particles[i].y += particles[i].vy * DT;
                    particles[i].z += particles[i].vz * DT;
                }

                printf("Posições atualizadas.\n");
            }

            // 2. Atualiza velocidades
            #pragma omp section
            {
                for (long long i = 0; i < NUM_PARTICLES; i++)
                {
                    particles[i].vy += GRAVITY * DT;
                }

                printf("Velocidades atualizadas.\n");
            }
        }
    }

    end_time = omp_get_wtime();

    printf("\nSimulação concluída.\n");
    printf("Tempo de execução (simulação): %.4f segundos\n",
           end_time - start_time);

    free(particles);

    return 0;
}
