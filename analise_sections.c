#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>           // Para INT_MAX, INT_MIN
#include <math.h>             // Para sqrt
#define N (1024 * 1024 * 100) // 100 Milhões de inteiros

int main(){
    int *data = (int *)malloc(N * sizeof(int));
    if (!data)
    {
        perror("malloc falhou");
        return 1;
    }
    // Inicializa com dados (exemplo)
    printf("Inicializando dados...\n");
    #pragma omp parallel for
        for (long long i = 0; i < N; i++)
        {
            data[i] = (i % 1000) - 500; // Valores entre -500 e 499
        }
        printf("Dados inicializados.\n");
        int min_val = INT_MAX;
        int max_val = INT_MIN;
        long long sum = 0;
        long long sum_sq = 0; // Soma dos quadrados
        double start_time, end_time;
        start_time = omp_get_wtime();
        // --- Bloco a ser otimizado ---
        printf("Calculando métricas(versão inicial)...\n");
    #pragma omp parallel reduction(min:min_val) reduction(max:max_val) reduction(+:sum,sum_sq)
    {
        #pragma omp sections
        {
        // Section 1: Calcula o mínimo
            #pragma omp section
            {
                for (long long i = 0; i < N; i++) {
                    if (data[i] < min_val) {
                        min_val = data[i];
                    }
                }
            }

        // Section 2: Calcula o máximo
            #pragma omp section
            {   
                for (long long i = 0; i < N; i++) {
                    if (data[i] > max_val) {
                        max_val = data[i];
                    }
                }
            }

        // Section 3: Calcula a soma e a soma dos quadrados
            #pragma omp section
            {
                for (long long i = 0; i < N; i++) {
                    sum += data[i];
                    sum_sq += (long long)data[i] * data[i];
                }
            }
        }
    }   
    // --- Fim do Bloco ---
    end_time = omp_get_wtime();
    double average = (double)sum / N;
    double variance = ((double)sum_sq / N) - (average * average);
    double std_dev = sqrt(variance);
    printf("\n-- - Resultados-- -\n");
    printf("Min : % d\n", min_val);
    printf("Max : % d\n", max_val);
    printf("Soma : % lld\n", sum);
    printf("Média : % .2f\n", average);
    printf("Desvio Padrão : % .2f\n", std_dev);
    printf("Tempo de execução(cálculo) : % .4f segundos\n", end_time - start_time);
    free(data);
    return 0;
}