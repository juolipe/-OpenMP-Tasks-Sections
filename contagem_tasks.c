#define _GNU_SOURCE
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <omp.h>

// Contador global de linhas
long long total_lines = 0;

// Função para contar linhas em um arquivo
long long count_lines_in_file(const char *filepath)
{
    FILE *fp = fopen(filepath, "r");

    if (!fp)
    {
        fprintf(stderr, "Erro ao abrir arquivo %s: %s\n",
                filepath, strerror(errno));
        return 0;
    }

    long long count = 0;
    int ch;

    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch == '\n')
        {
            count++;
        }
    }

    fclose(fp);

    return count;
}

// Função recursiva para atravessar diretórios
void traverse_directory(const char *dirpath)
{
    struct dirent **namelist;
    int n;

    n = scandir(dirpath, &namelist, NULL, alphasort);

    if (n < 0)
    {
        fprintf(stderr, "Erro ao escanear diretório %s: %s\n",
                dirpath, strerror(errno));
        return;
    }

    // Agrupa as tarefas criadas neste diretório
    #pragma omp taskgroup
    {
        for (int i = 0; i < n; i++)
        {
            // Ignora "." e ".."
            if (strcmp(namelist[i]->d_name, ".") == 0 ||
                strcmp(namelist[i]->d_name, "..") == 0)
            {
                free(namelist[i]);
                continue;
            }

            // Monta o caminho completo
            char *fullpath = NULL;

            if (asprintf(&fullpath, "%s/%s",
                         dirpath, namelist[i]->d_name) == -1)
            {
                fprintf(stderr, "Erro de alocação em asprintf\n");
                free(namelist[i]);
                continue;
            }

            struct stat path_stat;

            if (stat(fullpath, &path_stat) != 0)
            {
                fprintf(stderr, "Erro no stat para %s: %s\n",
                        fullpath, strerror(errno));

                free(fullpath);
                free(namelist[i]);
                continue;
            }

            // Se for um diretório, cria uma task
            if (S_ISDIR(path_stat.st_mode))
            {
                #pragma omp task firstprivate(fullpath)
                {
                    traverse_directory(fullpath);
                    free(fullpath);
                }
            }
            // Se for um arquivo regular
            else if (S_ISREG(path_stat.st_mode))
            {
                size_t len = strlen(namelist[i]->d_name);

                // Verifica se termina com .c ou .h
                if (len > 2 &&
                    (strcmp(namelist[i]->d_name + len - 2, ".c") == 0 ||
                     strcmp(namelist[i]->d_name + len - 2, ".h") == 0))
                {
                    // Cria uma task para contar as linhas
                    #pragma omp task firstprivate(fullpath)
                    {
                        long long lines = count_lines_in_file(fullpath);

                        // Atualiza o contador global com segurança
                        #pragma omp atomic update
                        total_lines += lines;

                        free(fullpath);
                    }
                }
                else
                {
                    free(fullpath);
                }
            }
            else
            {
                free(fullpath);
            }

            free(namelist[i]);
        }
    }

    free(namelist);
}

int main(int argc, char *argv[])
{
    const char *start_dir = ".";

    if (argc > 1)
    {
        start_dir = argv[1];
    }

    printf("Contando linhas em .c/.h a partir de: %s "
           "(versão paralela com tasks)\n", start_dir);

    double start_time, end_time;

    start_time = omp_get_wtime();

    // Região paralela
    #pragma omp parallel
    {
        // Apenas uma thread inicia a exploração
        #pragma omp single
        {
            traverse_directory(start_dir);
        }
    }

    end_time = omp_get_wtime();

    printf("\n--- Resultado ---\n");
    printf("Total de linhas encontradas: %lld\n", total_lines);
    printf("Tempo de execução: %.4f segundos\n",
           end_time - start_time);

    return 0;
}
