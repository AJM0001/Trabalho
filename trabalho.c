#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct dado{
    int n;
    char observationdate[11], province[50], country[25], lu[20];
    float conf, death, rec;
}DADO;


// Criação de dados em formato binário.
char* modificaNome(char nome[], int n){
    int i;
    for(i = 0; i < n; i++){
        if(nome[i] == '\0'){
            nome[i] = ' ';
        }
    }
    nome[i-1] = '\0';
}

char* reformata(char nome[], int n){
    int i;
    for(i = 0; i < n; i++){
        if(nome[i] == ' ' && (nome[i+1] == ' ' || nome[i+1] == '\0')){
           if(nome[i+1] == '\0'){
            break;
           }
            nome[i] = '\0';
        }
    }
}

void cria_arqbin(){
    FILE *p = fopen("covid_19_data - covid_19_data2.csv", "r");
    FILE *s = fopen("saida.bin", "wb");
    DADO d;

    while(!feof(p)){
        memset(d.province, '\0', 50);
        memset(d.country, '\0', 25);

        fscanf(p, "%d", &d.n);
        fseek(p, +1, SEEK_CUR);
        fscanf(p, "%[^,]", d.observationdate);
        fseek(p, +1, SEEK_CUR);
        fscanf(p, "%[^,]", d.province);
        modificaNome(d.province, 50);
        fseek(p, +1, SEEK_CUR);
        fscanf(p, "%[^,]", d.country);
        modificaNome(d.country, 25);
        fseek(p, +1, SEEK_CUR);
        fscanf(p, "%[^,]", d.lu);
        fseek(p, +1, SEEK_CUR);
        fscanf(p, "%f", &d.conf);
        fseek(p, +1, SEEK_CUR);
        fscanf(p, "%f", &d.death);
        fseek(p, +1, SEEK_CUR);
        fscanf(p, "%f\n", &d.rec);

        fwrite(&d, sizeof(d), 1, s);
    }
    fclose(p);
    fclose(s);
}

void mostra(DADO d){
    printf("====================================\n");
    printf("Inxed :%d\nObservation Date: %s\n"
            "Province: %s\nCountry: %s\nLast Update: %s\n"
            "Confirmed: %.0f\nDeath: %.0f\nRecovered: %.0f\n", d.n, d.observationdate, d.province, d.country, d.lu, d.conf, d.death, d.rec);
}

void mostra_10(FILE *s){
    int i;
    DADO d;
    for(i = 0; i < 10; i++){
        fread(&d, sizeof(d), 1, s);
        reformata(d.province, 50);
        reformata(d.country, 25);
        mostra(d);
    }
}

void mostra_todos(FILE *s){
    DADO d;
    while(!feof(s)){
        fread(&d, sizeof(d), 1, s);
        reformata(d.province, 50);
        reformata(d.country, 25);
        mostra(d);
    }
}

//2.2===============================================================================================================
//Arquivo de índice para chave 1.
typedef struct ind1{
    int chave, pos;
}IND;

void cria_ind1(FILE *p){
    FILE *ind = fopen("indice1.bin", "wb");
    int i;
    DADO d;
    IND index;

    while(!feof(p)){
        index.pos = ftell(p);
        fread(&d, sizeof(d), 1, p);
        index.chave = d.n;
        fwrite(&index, sizeof(index), 1, ind);
    }
    fclose(ind);
}

void pesquisaBin(int chave, FILE *s){
    FILE *ind = fopen("indice1.bin", "rb");
    int meio, tam, inicio, fim;
    IND c;
    inicio = 0;
    fseek(ind, 0, SEEK_END);
    fim = ftell(ind);

    fseek(ind, 0, SEEK_SET);
    // calcula o numero de registros do arquivo e cálcula o meio do arquivo.
    tam = (fim - inicio) / 8;
    meio = (tam / 2 ) * 8;


    while(1){
        fseek(ind, meio, SEEK_SET);
        fread(&c, sizeof(c), 1, ind);
        printf("Comparando chave: %d, com a chave: %d\n", c.chave, chave);

        if(c.chave == chave){
            DADO d;
            fseek(s, c.pos, SEEK_SET);
            fread(&d, sizeof(d), 1, s);
            reformata(d.province, 50);
            reformata(d.country, 25);
            mostra(d);
            return;
        }else if(c.chave > chave && tam != 1){
            fim = meio;
        }else if(c.chave < chave && tam != 1){
            inicio = meio;
        }else{
            printf("Chave %d nao foi encontrada.\n", chave);
            break;
        }

        tam = tam = (fim - inicio) / 8;
        meio = ((tam / 2 ) * 8) + inicio;
    }
}

//2.2===============================================================================================================

/*typedef struct pal{
    char palavra[25];
    int ini, fim;
}P;

int testa(char country[25]){
    FILE *tl = fopen("ind_nome.bin", "rb");
    P pal;
    fseek(tl, 0, SEEK_SET);
    while(!feof(tl)){
            fread(&pal, sizeof(pal), 1, tl);
            //printf("%s, %s\n", country, pal.palavra);
            if(strcmp(pal.palavra, country) == 0){
                //printf("%s, %s\n", country, pal.palavra);
                return 1;
            }
        }
    return 0;
}

void busca(P *pa, char country[25]){
    DADO d;
    int pos;
    FILE *t = fopen("indtest.txt", "w");
    FILE *p = fopen("saida.bin", "rb");

    fseek(p, 0, SEEK_SET);
    fseek(t, 0, SEEK_END);
    pa->ini = ftell(t);
    //printf("entrei na busca\n");
    while(!feof(p)){
        pos = ftell(p);
        fread(&d, sizeof(d), 1, p);
        //printf("%s\n", d.country);
        //printf("entrei no laco\n");
        if(strcmp(d.country, country) == 0){
            //printf("entrei no if\n");
            fprintf(t, "%d\n", pos);
            //fwrite(&pos, sizeof(int), 1, t);
        }
    }
}

void cria_ind_test(FILE *p){
    DADO d, f;
    P pa;
    int corrente;
    FILE *te = fopen("ind_nome.bin", "wb");

    //FILE *t = fopen("indtest.bin", "wb");

    fread(&d, sizeof(d), 1, p);
    strcpy(pa.palavra, d.country);
    fseek(p, 0, SEEK_SET);
    //fwrite(&pa, sizeof(pa), 1, te);
    fseek(te, 0, SEEK_SET);


    while(!feof(p)){
        fread(&d, sizeof(d), 1, p);
        corrente = ftell(p);
        int flag = testa(d.country);
        printf("%d\n", flag);

        if(flag == 0){
            //printf("entrei\n");
            fseek(te, 0, SEEK_END);
            strcpy(pa.palavra, d.country);
            busca(&pa, d.country);
            fwrite(&pa, sizeof(pa), 1, te);

        }
    fseek(p, corrente, SEEK_SET);
    }
}*/
typedef struct ind2{
    char country[25];
    int pos, rep;
}IND2;

void cria_ind_2(FILE *p){
    DADO d;
    IND2 ind;
    int corrente, ini;
    FILE *te = fopen("ind_nome.bin", "wb");

    fread(&d, sizeof(d), 1, p);
    strcpy(ind.country, d.country);
    fseek(p, 0, SEEK_SET);
    ind.pos = ftell(p);
    ind.rep = 0;


    while(!feof(p)){
        corrente = ftell(p);
        fread(&d, sizeof(d), 1, p);
        if(strcmp(d.country, ind.country) == 0){
            ind.rep++;
        }else{
            fwrite(&ind, sizeof(ind), 1, te);
            strcpy(ind.country, d.country);
            ind.pos = corrente;
            ind.rep = 1;
        }
    }
    ind.rep = 1;
    fwrite(&ind, sizeof(ind), 1, te);
    fclose(te);
}

void pesquisa_ind_2(FILE *p, char country[25]){
    FILE *t = fopen("ind_nome.bin", "rb");
    IND2 ind;
    DADO d;
    char c[25];
    memset(c, '\0', 25);
    strcpy(c, country);
    modificaNome(c, 25);
    int flag = 0;
    while(!feof(t)){
        fread(&ind, sizeof(ind), 1, t);
        if(strcmp(ind.country, c) == 0){
            flag = 1;
            int i;
            fseek(p, ind.pos, SEEK_SET);
            for(i = 0; i < ind.rep; i++){
                fread(&d, sizeof(d), 1, p);
                if(d.conf > 0){
                    reformata(d.province, 50);
                    reformata(d.country, 25);
                    mostra(d);
                    fclose(t);
                    return;
                }
            }
        }
    }
    if(flag == 0){
        printf("Pais não encontrado\n");
    }
    fclose(t);
}


//2.3===============================================================================================================
// Indice em memória para o campo ObservationDate

typedef struct nodo{
    char observationdate[11];
    int ini, fim;
    struct nodo *prox;
}NODO;

void cria_list(NODO **inicio, char od[8], int ini, int fim){
    NODO *novo;

    novo = (NODO *)malloc(sizeof(NODO));
    strcpy(novo->observationdate, od);
    novo->ini = ini;
    novo->fim = fim;
    novo->prox = NULL;

    if(*inicio == NULL){
        *inicio = novo;
    }else{
        NODO *aux;
        aux = *inicio;
        while(aux->prox != NULL){
            aux = aux->prox;
        }
        aux->prox = novo;
    }
}

void modifica_od(char d[], char c[]){
    int flag = 0;
    int cont = 0;
    int i;

    for(i = 0; d[i] != '\0'; i++){
        if(d[i] != '/' && flag != 1){
            c[cont] = d[i];
            cont++;
        }else if(d[i] == '/' && flag == 0){
            c[cont] = '/';
            cont++;
            flag++;
        }else if(d[i] == '/'){
            flag++;
        }
    }
    c[cont] = '\0';
}

void cria_ind_list(NODO *inicio, FILE *p){
    int ini, iniatual, inianterior;
    DADO d;
    char data[11];

    char catual[8] = "01/2020";
    ini = ftell(p);

    while(!feof(p)){
        iniatual = ftell(p);
        fread(&d, sizeof(d), 1, p);

        char c[8];
        if(strcmp(data, d.observationdate) != 0){
            modifica_od(d.observationdate, c);

            if(strcmp(catual, c) != 0){
                cria_list(&(*inicio), catual, ini, inianterior);
                ini = iniatual;
                strcpy(catual, c);
            }
        }

    inianterior = iniatual;
    }
    cria_list(&(*inicio), catual, ini, inianterior);

}

void pesquisa_list(NODO *ini, char data[11], FILE *p){
    char c[8];

    modifica_od(data, c);

    while(ini != NULL){
        if(strcmp(c, ini->observationdate) != 0){
            ini = ini->prox;
        }else{
            fseek(p, ini->ini, SEEK_SET);
            DADO d, k;
            int flag = 0;
            float rec = 0;
            while(ftell(p) < ini->fim){
                fread(&d, sizeof(d), 1, p);
                if(strcmp(d.observationdate, data) == 0){
                    flag = 1;
                    if(d.rec > rec){
                        rec = d.rec;
                        k = d;
                    }
                }
            }
            if(flag == 1){
                reformata(k.province, 50);
                reformata(k.country, 25);
                mostra(k);
                break;
            }else{
                printf("data nao encontrada\n");
                break;
            }
        }
    }
}

void escreve(NODO *inicio){
    NODO *aux;
    aux = inicio;
    while(aux != NULL){
        printf("%s %d %d\n", aux->observationdate, aux->ini, aux->fim);
        aux = aux->prox;
    }
}

//2.3===============================================================================================================
// Índice em memória utilizando uma AVL com o índice LastUpdate
typedef struct arv{
    char lastupdate[8];
    int inicio, fim;
    struct arv *dir, *esq;
    int altura;
}ARV;

// Calcula a altura
int altura(ARV *n) {
    if(n == NULL){
        return 0;
    }
    return n->altura;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

// Rotação a direita
ARV *RotacionaDir(ARV *y) {
    ARV *x = y->esq;
    ARV *T2 = x->dir;

    x->dir = y;
    y->esq = T2;

    y->altura = max(altura(y->esq), altura(y->dir)) + 1;
    x->altura = max(altura(x->esq), altura(x->dir)) + 1;

    return x;
}

// Rotação a esquerda
ARV *RotacionaEsq(ARV *x) {
    ARV *y = x->dir;
    ARV *T2 = y->esq;

    y->esq = x;
    x->dir = T2;

    x->altura = max(altura(x->esq), altura(x->dir)) + 1;
    y->altura = max(altura(y->esq), altura(y->dir)) + 1;

    return y;
}

// Calcula o fator de balanceamento
int pegaBal(ARV *n) {
    if (n == NULL)
        return 0;
    return altura(n->esq) - altura(n->dir);
}

// Insere o nodo
void insere_nodo(ARV **raiz, char key[], int ini, int fim) {

    ARV *novo;
    novo = (ARV *)malloc(sizeof(ARV));
    strcpy(novo->lastupdate, key);
    novo->inicio = ini;
    novo->fim = fim;
    novo->esq = NULL;
    novo->dir = NULL;
    novo->altura = 1;

    if (*raiz == NULL){
        *raiz = novo;
        return;
    }

    if(strcmp((*raiz)->lastupdate, key) > 0){
        insere_nodo(&(*raiz)->esq, key, ini, fim);
    }else if (strcmp((*raiz)->lastupdate, key) < 0){
        insere_nodo(&(*raiz)->dir, key, ini, fim);
    }else{
        return;
    }

  //Atualiza o fator de balanceamento de cada nodo e balanceia a árvore.
    (*raiz)->altura = 1 + max(altura((*raiz)->esq), altura((*raiz)->dir));

    int balancemaneto = pegaBal(*raiz);
    if (balancemaneto > 1 && strcmp(((*raiz)->esq)->lastupdate, key) > 0)
        return;

    if (balancemaneto < -1 && strcmp(((*raiz)->dir)->lastupdate, key) < 0)
        return;

    if (balancemaneto > 1 && strcmp(((*raiz)->esq)->lastupdate, key) > 0) {
        (*raiz)->esq = RotacionaEsq((*raiz)->esq);
        return;
    }

    if (balancemaneto < -1 && strcmp(((*raiz)->dir)->lastupdate, key) < 0) {
        (*raiz)->dir = RotacionaDir((*raiz)->dir);
        return;
    }
}

void modificaData(char data[], char c[]){
    int i;
    int cont = 0;
    int flag = 0;
    for(i = 0; data[i] != '\0'; i++){
        if(data[i] != '-'){
            c[cont] = data[i];
            cont++;
        }else if(data[i] == '-' && flag == 0){
            c[cont] = data[i];
            cont++;
            flag++;
        }else if(data[i] == '-' && flag == 1){
            break;
        }
    }
    c[cont] = '\0';
}

void cria_ind_arv(ARV *raiz, FILE *p){
    int ini, iniatual, inianterior;
    DADO d;
    char catual[8] = "2020-01";
    char lastupdate[20];
    int k = 0;
    ini = ftell(p);

    while(!feof(p)){
        iniatual = ftell(p);
        fread(&d, sizeof(d), 1, p);

        char c[8];
        if(strcmp(lastupdate, d.lu) != 0){
            modificaData(d.lu, c);

            if(strcmp(catual, c) != 0){
                insere_nodo(&(*raiz), catual, ini, inianterior);
                ini = iniatual;
                strcpy(catual, c);
            }
        }
    inianterior = iniatual;
    }
    insere_nodo(&(*raiz), catual, ini, inianterior);
}

void mod(char lu[], char aux[]){
    int i;
    for(i = 0;  lu[i] != ' '; i++){
        aux[i] = lu[i];
    }
    aux[i] = '\0';
}

void pesquisa_arv(ARV *raiz, char data[11], FILE *p){
    char c[8];
    char k[11];
    strcpy(k, data);
    modificaData(k, c);

    while(raiz != NULL){
        if(strcmp(c, raiz->lastupdate) > 0){
            raiz = raiz->dir;
        }else if(strcmp(c, raiz->lastupdate) < 0){
            raiz = raiz->esq;
        }else{
            fseek(p, raiz->inicio, SEEK_SET);
            DADO d, k;
            int flag = 0;
            float death = 0;
            while(ftell(p) < raiz->fim){
                fread(&d, sizeof(d), 1, p);
                char aux[11];
                mod(d.lu, aux);
                if(strcmp(aux, data) == 0){
                    flag = 1;
                    if(d.death > death){
                        death = d.death;
                        k = d;
                    }
                }
            }
            if(flag == 1){
                reformata(k.province, 50);
                reformata(k.country, 25);
                mostra(k);
                break;
            }else{
                printf("data nao encontrada\n");
                break;
            }
        }
    }
}

void escreve_arv(ARV *raiz){
    if(raiz != NULL){
        printf("%s %d %d\n", raiz->lastupdate, raiz->inicio, raiz->fim);
        escreve_arv(raiz->esq);

        escreve_arv(raiz->dir);
    }
}

void menu(){
    printf("1 - Mostrar os 10 primeiros dados\n");
    printf("2 - Mostrar todos os dados\n");
    printf("3 - Pesquisar por chave primaria [inteiro]\n");
    printf("4 - Pesquisar primeiro registro de Caso confirmado de um Pais\n");
    printf("5 - Pesquisar o pais que teve maior numero de recuperados em uma data de observacao\n");
    printf("6 - Pesquisar o pais que teve maior numero de mortos em uma data atualizacao\n");
    printf("0 - SAIR\n");
}

int main(){
    FILE *p = fopen("covid_19_data - covid_19_data2.csv", "r");

    cria_arqbin();

    fclose(p);

    FILE *s = fopen("saida.bin", "rb");

    int escolha = 0;
    int x;
    char pais[25], data[11];
    NODO *inicio = NULL;
    ARV *arv = NULL;

    cria_ind1(s);
    fseek(s, 0, SEEK_SET);
    cria_ind_2(s);
    fseek(s, 0, SEEK_SET);
    cria_ind_list(&inicio, s);
    fseek(s, 0, SEEK_SET);
    cria_ind_arv(&arv, s);
    fseek(s, 0, SEEK_SET);

    escreve_arv(arv);


    while(1){
        menu();
        printf("Escolha: ");
        scanf("%d", &escolha);

        if(escolha == 0){
            printf("ENCERRANDO...");
            break;
        }

        switch(escolha){
            case 1:
                mostra_10(s);
                break;
            case 2:
                mostra_todos(s);
                break;
            case 3:
                printf("Insira uma chave [int]: ");
                scanf("%d", &x);
                pesquisaBin(x, s);
                break;
            case 4:
                printf("Insira o nome do pais: ");
                scanf("%s", pais);
                pesquisa_ind_2(s, pais);
                break;
            case 5:
                printf("Insira uma data [mm/dd/aaaa]:");
                scanf("%s", data);
                pesquisa_list(inicio, data, s);
                break;
            case 6:
                printf("Insira uma data [aaaa-mm-dd]:");
                scanf("%s", data);
                pesquisa_arv(arv, data, s);
                break;

        }
    }

    fclose(s);

    return 0;
}
