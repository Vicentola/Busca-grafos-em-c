#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define MAX_ESTACOES 200
#define MAX_NOME 50
#define INFINITO INT_MAX

typedef struct Aresta {
    int destino;
    int peso;
    struct Aresta* proxima;
} Aresta;

typedef struct Estacao {
    char nome[MAX_NOME];
    Aresta* lista_adjacencia;
    int visitado;
} Estacao;

typedef struct Grafo {
    Estacao estacoes[MAX_ESTACOES];
    int num_estacoes;
} Grafo;

typedef struct Caminho {
    int estacoes[MAX_ESTACOES];
    int tamanho;
    int custo_total;
} Caminho;

// Funções básicas do grafo
int encontrar_estacao(Grafo* grafo, const char* nome);
int adicionar_estacao(Grafo* grafo, const char* nome);
void adicionar_aresta(Grafo* grafo, const char* origem, const char* destino, int peso);
void remover_aresta(Grafo* grafo, const char* origem, const char* destino);
void inicializar_grafo(Grafo* grafo);

// Algoritmos de busca
Caminho dijkstra(Grafo* grafo, const char* origem, const char* destino);
int bfs_conectividade(Grafo* grafo, const char* origem, const char* destino);
void encontrar_rotas_redundantes(Grafo* grafo, const char* origem, const char* destino);

// Funções do sistema
int carregar_csv(Grafo* grafo, const char* filename);
void aplicar_rotas_bloqueadas(Grafo* grafo);

// Utilitárias
void imprimir_caminho(Caminho caminho, Grafo* grafo);
void liberar_grafo(Grafo* grafo);

int main() {
    clock_t inicio_total, fim_total;           //medir o tempo de execução dos algortimos
    inicio_total = clock();
    
    Grafo grafo;
    inicializar_grafo(&grafo);
    
    printf("=== MISSAO DE RESGATE ESPACIAL ===\n");
    printf("Carregando rede de estacoes...\n");
    
    clock_t inicio_carregamento = clock();
    
    // tenta primeiro o nome fornecido pelo user
    int carregado = carregar_csv(&grafo, "arquivo.csv");
    
    //  não encontrou, tenta outros nomes
    if (!carregado) {
        printf("Tentando arquivo alternativo...\n");
        carregado = carregar_csv(&grafo, "estacoeserotas.csv");
    }
    if (!carregado) {
        printf("Tentando terceira opcao...\n");
        carregado = carregar_csv(&grafo, "estacoeserotas (1).csv");
    }
    
    clock_t fim_carregamento = clock();
    
    if (!carregado) {
        printf("ERRO: Nao foi possivel carregar o arquivo CSV!\n");
        printf("Certifique-se de que o arquivo esta na mesma pasta do executavel.\n");
        printf("Nomes tentados: arquivo.csv, estacoeserotas.csv, estacoeserotas (1).csv\n");
        
        // listar arquivos CSV na pasta para ajudar no diagnóstico
        printf("\nArquivos na pasta atual:\n");
        system("dir /b *.csv 2>nul");
        
        return 1;
    }
    
    printf("Rede carregada com %d estacoes\n", grafo.num_estacoes);
    
    // aplica rotas bloqueadas
    aplicar_rotas_bloqueadas(&grafo);
    
    int terra = encontrar_estacao(&grafo, "Terra");
    int centauri = encontrar_estacao(&grafo, "Centauri");
    
    if (terra == -1 || centauri == -1) {
        printf("ERRO: Estacoes Terra ou Centauri nao encontradas!\n");
        printf("Estacoes disponiveis:\n");
        for (int i = 0; i < grafo.num_estacoes; i++) {
            printf("- %s\n", grafo.estacoes[i].nome);
        }
        liberar_grafo(&grafo);
        return 1;
    }
    
    printf("\n1. BUSCANDO CAMINHO MAIS CURTO Terra -> Centauri\n");
    clock_t inicio_dijkstra = clock();
    Caminho caminho_curto = dijkstra(&grafo, "Terra", "Centauri");
    clock_t fim_dijkstra = clock();
    
    if (caminho_curto.tamanho > 0) {
        printf("Caminho encontrado! Custo total: %d\n", caminho_curto.custo_total);
        imprimir_caminho(caminho_curto, &grafo);
    } else {
        printf("Nenhum caminho seguro encontrado!\n");
    }
    
    printf("\n2. VERIFICANDO CONECTIVIDADE DA REDE\n");
    clock_t inicio_conectividade = clock();
    int conectado = bfs_conectividade(&grafo, "Terra", "Centauri");
    clock_t fim_conectividade = clock();
    
    if (conectado) {
        printf("Terra e Centauri estao conectadas\n");
    } else {
        printf("Terra e Centauri NAO estao conectadas\n");
    }
    
    printf("\n3. IDENTIFICANDO ROTAS REDUNDANTES\n");
    clock_t inicio_redundancias = clock();
    encontrar_rotas_redundantes(&grafo, "Terra", "Centauri");
    clock_t fim_redundancias = clock();
    
    // tempo execução
    printf("\n=== TEMPOS DE EXECUCAO ===\n");
    printf("Carregamento: %.4f ms\n", ((double)(fim_carregamento - inicio_carregamento) * 1000 / CLOCKS_PER_SEC)); //1000 pois converte segundos para milisegundos, para contar os ticks
    printf("Dijkstra: %.4f ms\n", ((double)(fim_dijkstra - inicio_dijkstra) * 1000 / CLOCKS_PER_SEC));
    printf("Conectividade: %.4f ms\n", ((double)(fim_conectividade - inicio_conectividade) * 1000 / CLOCKS_PER_SEC));
    printf("Redundancias: %.4f ms\n", ((double)(fim_redundancias - inicio_redundancias) * 1000 / CLOCKS_PER_SEC));
    
    fim_total = clock();
    printf("Tempo total: %.4f ms\n", ((double)(fim_total - inicio_total) * 1000 / CLOCKS_PER_SEC));
    
    liberar_grafo(&grafo);
    return 0;
}

void inicializar_grafo(Grafo* grafo) {
    grafo->num_estacoes = 0;
    for (int i = 0; i < MAX_ESTACOES; i++) {
        grafo->estacoes[i].lista_adjacencia = NULL;
        grafo->estacoes[i].visitado = 0;
        strcpy(grafo->estacoes[i].nome, ""); // copia nomes para dentro da estrutura do grafo para acessas dps
    }
}

int encontrar_estacao(Grafo* grafo, const char* nome) {
    for (int i = 0; i < grafo->num_estacoes; i++) {
        if (strcmp(grafo->estacoes[i].nome, nome) == 0) { //permite encontrar estacoes pelo nome ao invez do indice
            return i;
        }
    }
    return -1;
}

int adicionar_estacao(Grafo* grafo, const char* nome) {
    if (grafo->num_estacoes >= MAX_ESTACOES) return -1;
    
    int idx = encontrar_estacao(grafo, nome);
    if (idx != -1) return idx;
    // Copia nome para nova estação e inicializa lista vazia
    strcpy(grafo->estacoes[grafo->num_estacoes].nome, nome);
    grafo->estacoes[grafo->num_estacoes].lista_adjacencia = NULL;
    return grafo->num_estacoes++;
}

void adicionar_aresta(Grafo* grafo, const char* origem, const char* destino, int peso) {
    int idx_origem = adicionar_estacao(grafo, origem);
    int idx_destino = adicionar_estacao(grafo, destino);
    
    // Adicionar aresta origem -> destino
    Aresta* nova_aresta = malloc(sizeof(Aresta));
    nova_aresta->destino = idx_destino;
    nova_aresta->peso = peso;
    // Insere no início da lista de adjacência
    nova_aresta->proxima = grafo->estacoes[idx_origem].lista_adjacencia;
    grafo->estacoes[idx_origem].lista_adjacencia = nova_aresta;
    
    // Adicionar aresta destino -> origem (grafo não direcionado)
    nova_aresta = malloc(sizeof(Aresta));
    nova_aresta->destino = idx_origem;
    nova_aresta->peso = peso;
    //add no início da lista de adjacencia
    //cria a conexão de volta
    nova_aresta->proxima = grafo->estacoes[idx_destino].lista_adjacencia;
    grafo->estacoes[idx_destino].lista_adjacencia = nova_aresta;
}

void remover_aresta(Grafo* grafo, const char* origem, const char* destino) {
    int idx_origem = encontrar_estacao(grafo, origem);
    int idx_destino = encontrar_estacao(grafo, destino);
    
    if (idx_origem == -1 || idx_destino == -1) return;
    
    // Remover origem -> destino
    //** ponteiro qeu aponta par o ponteiro de uma aresta + facil de remover os elementos da lista 
    Aresta** atual = &grafo->estacoes[idx_origem].lista_adjacencia;
    while (*atual) {
        if ((*atual)->destino == idx_destino) {
            Aresta* temp = *atual;
            *atual = (*atual)->proxima;
            free(temp);
            break;
        }
        atual = &(*atual)->proxima;
    }
    
    // Remover destino -> origem
    atual = &grafo->estacoes[idx_destino].lista_adjacencia;
    while (*atual) {
        if ((*atual)->destino == idx_origem) {
            Aresta* temp = *atual;
            *atual = (*atual)->proxima;
            free(temp);
            break;
        }
        atual = &(*atual)->proxima;
    }
}

int carregar_csv(Grafo* grafo, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return 0;
    }
    
    printf("Carregando arquivo: %s\n", filename);
    
    char linha[1024];
    while (fgets(linha, sizeof(linha), file)) {
        // limpa as newlines antes do processamento
        linha[strcspn(linha, "\r\n")] = 0;
        
        char* token = strtok(linha, ",");
        if (!token) continue;
        
        char origem[MAX_NOME];
        strcpy(origem, token);
        
        // Processar pares (destino, peso)
        while ((token = strtok(NULL, ",")) != NULL) { // strtok divide as strings em tokens para facilitar o processamento
            char destino[MAX_NOME];
            strcpy(destino, token);
            
            token = strtok(NULL, ",");
            if (!token) break;
            
            int peso = atoi(token); // converte string para int , Converte "1", "3", "6" do CSV para pesos das rotas, dijkstra n ia funcionar
            adicionar_aresta(grafo, origem, destino, peso);
        }
    }
    
    fclose(file);
    return 1;
}

void aplicar_rotas_bloqueadas(Grafo* grafo) {
    printf("Aplicando rotas bloqueadas...\n");
    remover_aresta(grafo, "Elysium", "Idris");
    remover_aresta(grafo, "Idris", "Rethor");
    remover_aresta(grafo, "Rethor", "Croshaw");
    remover_aresta(grafo, "Croshaw", "Nul");
    printf("Rotas bloqueadas removidas\n");
}

Caminho dijkstra(Grafo* grafo, const char* origem, const char* destino) {
    Caminho caminho = { .tamanho = 0, .custo_total = 0 };
    int idx_origem = encontrar_estacao(grafo, origem);
    int idx_destino = encontrar_estacao(grafo, destino);
    
    if (idx_origem == -1 || idx_destino == -1) return caminho;
    
    int dist[MAX_ESTACOES];
    int anterior[MAX_ESTACOES];
    int visitado[MAX_ESTACOES] = {0};
    
    for (int i = 0; i < grafo->num_estacoes; i++) {
        dist[i] = INFINITO;
        anterior[i] = -1;
    }
    
    dist[idx_origem] = 0;
    
    for (int count = 0; count < grafo->num_estacoes - 1; count++) {
        int min_dist = INFINITO;
        int u = -1;
        
        for (int v = 0; v < grafo->num_estacoes; v++) {
            if (!visitado[v] && dist[v] < min_dist) {
                min_dist = dist[v];
                u = v;
            }
        }
        
        if (u == -1 || u == idx_destino) break;
        
        visitado[u] = 1;
        
        Aresta* aresta = grafo->estacoes[u].lista_adjacencia;
        while (aresta) {
            int v = aresta->destino;
            int peso = aresta->peso;
             // RELAXAMENTO: atualiza distancia se encontrou caminho melhor
            if (!visitado[v] && dist[u] != INFINITO && dist[u] + peso < dist[v]) {
                dist[v] = dist[u] + peso;
                anterior[v] = u;
            }
            aresta = aresta->proxima;
        }
    }
    
    if (dist[idx_destino] != INFINITO) { //se n for infinito tem um caminho
        int atual = idx_destino;
        while (atual != -1) {
            caminho.estacoes[caminho.tamanho++] = atual;
            atual = anterior[atual]; //ta no destino e volta pelo caminho ate chegar na origem
        }
        
        for (int i = 0; i < caminho.tamanho / 2; i++) {
            int temp = caminho.estacoes[i];
            caminho.estacoes[i] = caminho.estacoes[caminho.tamanho - 1 - i];
            caminho.estacoes[caminho.tamanho - 1 - i] = temp;
        } // inverte o caminho pq foi construido de tras pra frente pois dijkstra faz a menor dist da origem ate cada no e n do no ate a origem
        
        caminho.custo_total = dist[idx_destino];
    }
    
    return caminho;
}

int bfs_conectividade(Grafo* grafo, const char* origem, const char* destino) {
    int idx_origem = encontrar_estacao(grafo, origem);
    int idx_destino = encontrar_estacao(grafo, destino);
    
    if (idx_origem == -1 || idx_destino == -1) return 0;
    
    int visitado[MAX_ESTACOES] = {0}; // estacoes ja visitadas
    int fila[MAX_ESTACOES]; //fila para bfs
    int frente = 0, tras = 0; //ponteiuros
    
    fila[tras++] = idx_origem; //enfileira origem
    visitado[idx_origem] = 1; // marca como visitada
    
    while (frente < tras) {
        int atual = fila[frente++];
        
        if (atual == idx_destino) return 1;
        
        Aresta* aresta = grafo->estacoes[atual].lista_adjacencia;
        while (aresta) {
            int vizinho = aresta->destino;
            if (!visitado[vizinho]) { //se n foi visitado
                visitado[vizinho] = 1; //marca como visitado
                fila[tras++] = vizinho; //enfileira para visitar dps
            }
            aresta = aresta->proxima;
        }
    }
    
    return 0;
}

void encontrar_rotas_redundantes(Grafo* grafo, const char* origem, const char* destino) {
    int idx_origem = encontrar_estacao(grafo, origem);
    int idx_destino = encontrar_estacao(grafo, destino);
    
    if (idx_origem == -1 || idx_destino == -1) return;
    
    Caminho principal = dijkstra(grafo, origem, destino);
    
    if (principal.tamanho == 0) {
        printf("Nenhum caminho principal encontrado\n");
        return;
    }
    
    printf("Caminho principal: ");
    imprimir_caminho(principal, grafo);
    
    printf("\nAnalisando redundancias:\n");
    
    for (int i = 1; i < principal.tamanho - 1; i++) {
        int estacao_testada = principal.estacoes[i];
        char nome_estacao[MAX_NOME];
        strcpy(nome_estacao, grafo->estacoes[estacao_testada].nome);
        
        printf("Testando sem a estacao: %s - ", nome_estacao);
        
        // Salvar conexões
        int destinos_salvos[MAX_ESTACOES];
        int pesos_salvos[MAX_ESTACOES];
        int num_conexoes = 0;
        
        Aresta* aresta = grafo->estacoes[estacao_testada].lista_adjacencia;
        while (aresta && num_conexoes < MAX_ESTACOES) {
            destinos_salvos[num_conexoes] = aresta->destino;
            pesos_salvos[num_conexoes] = aresta->peso;
            num_conexoes++;
            aresta = aresta->proxima;
        }
        
        // Remover todas as conexões
        aresta = grafo->estacoes[estacao_testada].lista_adjacencia;
        while (aresta) {
            Aresta* temp = aresta;
            aresta = aresta->proxima;
            remover_aresta(grafo, nome_estacao, grafo->estacoes[temp->destino].nome);
        }
        
        // Verificar conectividade
        if (bfs_conectividade(grafo, origem, destino)) {
            printf("REDUNDANTE\n");
        } else {
            printf("CRITICA\n");
        }
        
        // Restaurar conexões
        for (int j = 0; j < num_conexoes; j++) {
            adicionar_aresta(grafo, nome_estacao, 
                           grafo->estacoes[destinos_salvos[j]].nome,
                           pesos_salvos[j]);
        }
    }
}

void imprimir_caminho(Caminho caminho, Grafo* grafo) {
    for (int i = 0; i < caminho.tamanho; i++) {
        printf("%s", grafo->estacoes[caminho.estacoes[i]].nome);
        if (i < caminho.tamanho - 1) printf(" -> ");
    }
    printf("\n");
}

void liberar_grafo(Grafo* grafo) {
    for (int i = 0; i < grafo->num_estacoes; i++) {
        Aresta* aresta = grafo->estacoes[i].lista_adjacencia;
        while (aresta) {
            Aresta* temp = aresta;
            aresta = aresta->proxima;
            free(temp);
        }
    }
}