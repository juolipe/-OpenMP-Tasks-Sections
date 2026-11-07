#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define NUM_PARTICLES (1024 * 1024 * 10)
#define DT 0.01 // Time step
typedef struct
{
    double x, y, z;    // Posição
    double vx, vy, vz; // Velocidade
} Particle;
int main()
{
    Particle *particles = (Particle *)malloc(NUM_PARTICLES * sizeof(Particle));
    if (!particles)
    {
        perror("malloc falhou");
        return 1;
    }
    long long i;
    // Inicializa partículas
    printf("Inicializando partículas...\n");
#pragma omp parallel for
    for (i = 0; i < NUM_PARTICLES; i++)
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
    start_time = omp_get_wtime();
    // --- Bloco a ser otimizado ---
    printf("Simulando um passo(versão inicial)...\n");
    const double GRAVITY = -9.81;
    // 1. Atualiza Posições

#pragma omp parallel for
    for (i = 0; i < NUM_PARTICLES; i++)
    {
        particles[i].x += particles[i].vx * DT;
        particles[i].y += particles[i].vy * DT;
        particles[i].z += particles[i].vz * DT;
    }
    printf(" Posições atualizadas.\n");
// 2. Atualiza Velocidades (baseado em força externa simples)
#pragma omp parallel for
    for (i = 0; i < NUM_PARTICLES; i++)
    {
        // Exemplo: Gravidade agindo em Y
        particles[i].vy += GRAVITY * DT;
        // Outras forças poderiam ser calculadas aqui
    }
    printf(" Velocidades atualizadas.\n");
    // --- Fim do Bloco ---
    end_time = omp_get_wtime();
    printf("\nSimulação concluída.\n");
    // Verificar uma partícula (opcional)
    // printf("Partícula 0: Pos(%.2f, %.2f, %.2f) Vel(%.2f, %.2f, %.2f)\n",
    // particles[0].x, particles[0].y, particles[0].z,
    // particles[0].vx, particles[0].vy, particles[0].vz);
    printf("Tempo de execução(simulação) : % .4f segundos\n", end_time - start_time);
    free(particles);
    return 0;
}