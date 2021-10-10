#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma pack(1)

typedef struct _Endereco Endereco;

struct _Endereco
{
    char logradouro[72];
    char bairro[72];
    char cidade[72];
    char uf[72];
    char sigla[2];
    char cep[8];
    char lixo[2];
};

int compara(const void *e1, const void *e2)
{
    return strncmp(((Endereco *)e1)->cep, ((Endereco *)e2)->cep, 8);
}

void intercala(char *arq1, char *arq2, char *arqSaida);

int main()
{
    FILE *f, *saida;
    Endereco *e;
    long posicao, qtdRegistros, qtdRegPorBloco, qtdMaxRegPorBloco;
    int ultimoArquivo = 0, qtdBlocos;
    char *arquivoCep = "cep.dat";

    qtdMaxRegPorBloco = 50000;

    f = fopen(arquivoCep, "r"); // Abre o Arquivo Original Completo.
    if (f == NULL)
    {
        fprintf(stderr, "Erro ao abrir o %s.\n", arquivoCep);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    posicao = ftell(f);
    qtdRegistros = posicao / sizeof(Endereco);
    printf("Quantidade de Registros no %s: %d registros\n", arquivoCep, qtdRegistros);

    qtdBlocos = pow(2, ceil(log2(qtdRegistros / qtdMaxRegPorBloco)));
    printf("Quantidade de Blocos: %d\n", qtdBlocos);
    qtdRegPorBloco = (int)ceil((float)qtdRegistros / qtdBlocos);
    printf("Quantidade aproximada de Reg por Bloco: %d registros\n\n", (qtdRegPorBloco));

    rewind(f);
    e = (Endereco *)malloc(qtdRegPorBloco * sizeof(Endereco));
    for (int i = 0; i < qtdBlocos; i++)
    {
        if (i == qtdBlocos - 1)
            qtdRegPorBloco = qtdRegistros;

        if (fread(e, sizeof(Endereco), qtdRegPorBloco, f) == qtdRegPorBloco)
            printf("Bloco %d -> Lido\n", i);

        qsort(e, qtdRegPorBloco, sizeof(Endereco), compara);
        printf("Bloco %d -> Ordenado\n", i);

        char nomeBloco[20];
        sprintf(nomeBloco, "cep_bloco_%d.dat", i);

        saida = fopen(nomeBloco, "w");

        fwrite(e, sizeof(Endereco), qtdRegPorBloco, saida);
        fclose(saida);
        printf("Bloco %d -> Escrito\n", i);
        qtdRegistros -= qtdRegPorBloco;
        printf("- Falta ler %d registros do arquivo original.\n\n", qtdRegistros);
        ultimoArquivo++;
    }
    free(e);
    fclose(f);

    int arqIndex = 0, aux;
    for (int etapa = log2(qtdBlocos); etapa > 0; etapa--)
    {
        printf("Etapa %d:\n", etapa);
        aux = ultimoArquivo;
        qtdBlocos = qtdBlocos / 2;

        for (int i = 0; i < qtdBlocos; i++)
        {
            char arquivo1[30], arquivo2[30], arqSaida[30];

            sprintf(arquivo1, "cep_bloco_%d.dat", arqIndex++);
            sprintf(arquivo2, "cep_bloco_%d.dat", arqIndex++);

            (etapa == 1) ? sprintf(arqSaida, "cep_ordenado.dat") : sprintf(arqSaida, "cep_bloco_%d.dat", ultimoArquivo++);

            printf("Intercalando: '%s' + '%s' ---> '%s'\n", arquivo1, arquivo2, arqSaida);

            intercala(arquivo1, arquivo2, arqSaida);
            remove(arquivo1);
            remove(arquivo2);

            if (i == qtdBlocos - 1)
                arqIndex = aux;
        }

        printf("\n\n");
    }

    return 0;
}

void intercala(char *arq1, char *arq2, char *arqSaida)
{
    FILE *a, *b, *saida;
    Endereco ea, eb;

    a = fopen(arq1, "r");
    b = fopen(arq2, "r");
    saida = fopen(arqSaida, "w");

    fread(&ea, sizeof(Endereco), 1, a);
    fread(&eb, sizeof(Endereco), 1, b);

    while (!feof(a) && !feof(b))
    {
        if (compara(&ea, &eb) < 0)
        {
            fwrite(&ea, sizeof(Endereco), 1, saida);
            fread(&ea, sizeof(Endereco), 1, a);
        }
        else
        {
            fwrite(&eb, sizeof(Endereco), 1, saida);
            fread(&eb, sizeof(Endereco), 1, b);
        }
    }

    while (!feof(a))
    {
        fwrite(&ea, sizeof(Endereco), 1, saida);
        fread(&ea, sizeof(Endereco), 1, a);
    }
    while (!feof(b))
    {
        fwrite(&eb, sizeof(Endereco), 1, saida);
        fread(&eb, sizeof(Endereco), 1, b);
    }

    fclose(a);
    fclose(b);
    fclose(saida);
}