#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <locale.h>
#include <queue>

using namespace std;

int** generate_adjacency_matrix(int size, int edge_percent, bool weighted, bool directed) {
    int **G = (int **)malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++) {
        G[i] = (int *)malloc(size * sizeof(int));
    }

    for (int i = 0; i < size; i++) {
        G[i][i] = 0;
        for (int j = (directed ? 0 : i + 1); j < size; j++) {
            if (i == j) continue;
            int val = 0;
            if (rand() % 100 < edge_percent) {
                val = weighted ? rand() % 10 + 1 : 1;
            }
            G[i][j] = val;
            if (!directed) G[j][i] = val;
        }
    }
    return G;
}

void free_matrix(int **G, int size) {
    for (int i = 0; i < size; i++) {
        free(G[i]);
    }
    free(G);
}

void print_matrix(int **G, int size) {
    printf("    ");
    for (int j = 0; j < size; j++) {
        printf("v%-3d", j);
    }
    printf("\n");

    for (int i = 0; i < size; i++) {
        printf("v%-2d ", i);
        for (int j = 0; j < size; j++) {
            printf("%-4d", G[i][j]);
        }
        printf("\n");
    }
}

void bfs_distance_weighted(int start, int size, int **G, int *DIST, bool print_order) {
    queue<int> Q;
    for (int i = 0; i < size; i++) DIST[i] = -1;
    Q.push(start);
    DIST[start] = 0;

    while (!Q.empty()) {
        int v = Q.front();
        Q.pop();
        if (print_order) printf("%d ", v);

        for (int i = 0; i < size; i++) {
            if (G[v][i] > 0 && (DIST[i] == -1 || DIST[i] > DIST[v] + G[v][i])) {
                Q.push(i);
                DIST[i] = DIST[v] + G[v][i];
            }
        }
    }
}

void reset_dist(int *DIST, int size) {
    for (int i = 0; i < size; i++) DIST[i] = -1;
}

void print_dist(int *DIST, int size, int start) {
    printf("Расстояния от вершины %d:\n", start);
    for (int i = 0; i < size; i++) {
        if (DIST[i] == -1) printf("  до v%d: недостижима\n", i);
        else printf("  до v%d: %d\n", i, DIST[i]);
    }
}

int** build_distance_matrix(int size, int **G) {
    int **D = (int **)malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++) {
        D[i] = (int *)malloc(size * sizeof(int));
        reset_dist(D[i], size);
        bfs_distance_weighted(i, size, G, D[i], false);
    }
    return D;
}

void print_distance_matrix(int **D, int size) {
    printf("    ");
    for (int j = 0; j < size; j++) {
        printf("v%-3d", j);
    }
    printf("\n");

    for (int i = 0; i < size; i++) {
        printf("v%-2d ", i);
        for (int j = 0; j < size; j++) {
            if (D[i][j] == -1) printf("%-4s", "-");
            else printf("%-4d", D[i][j]);
        }
        printf("\n");
    }
}

int* find_eccentricities(int **D, int size) {
    int *ecc = (int *)malloc(size * sizeof(int));
    if (!ecc) { fprintf(stderr, "Недостаточно памяти.\n"); exit(EXIT_FAILURE); }
    for (int i = 0; i < size; i++) {
        ecc[i] = 0;
        for (int j = 0; j < size; j++) {
            if (D[i][j] == -1) {
                ecc[i] = -1;
                break;
            }
            if (D[i][j] > ecc[i]) ecc[i] = D[i][j];
        }
    }
    return ecc;
}

void print_graph_characteristics(int *ecc, int size) {
    int radius = -1, diameter = 0;
    bool infinite_diameter = false;
    printf("Эксцентриситеты вершин:\n");
    for (int i = 0; i < size; i++) {
        if (ecc[i] == -1) {
            printf("  e(v%d) = бесконечность\n", i);
            infinite_diameter = true;
        } else {
            printf("  e(v%d) = %d\n", i, ecc[i]);
            if (radius == -1 || ecc[i] < radius) radius = ecc[i];
            if (ecc[i] > diameter) diameter = ecc[i];
        }
    }
    if (radius == -1) printf("Радиус графа r(G) = бесконечность\n");
    else printf("Радиус графа r(G) = %d\n", radius);
    if (infinite_diameter) printf("Диаметр графа D(G) = бесконечность\n");
    else printf("Диаметр графа D(G) = %d\n", diameter);

    if (radius == -1) {
        printf("Нет вершины, из которой достижим весь граф; конечный центр отсутствует.\n");
        printf("Центр и периферию по бесконечному эксцентриситету не выделяем.\n");
        return;
    }
    printf("Центральные вершины: ");
    for (int i = 0; i < size; i++) {
        if (ecc[i] == radius) printf("v%d ", i);
    }
    if (infinite_diameter) {
        printf("\nПериферийные вершины с бесконечным эксцентриситетом: ");
    } else printf("\nПериферийные вершины: ");
    for (int i = 0; i < size; i++) {
        if (ecc[i] == (infinite_diameter ? -1 : diameter)) printf("v%d ", i);
    }
    printf("\n");
}

bool process_graph(int size, int edge_percent, bool weighted, bool directed) {
    int **G = generate_adjacency_matrix(size, edge_percent, weighted, directed);

    printf("\n================================================\n");
    printf("Граф: %s, %s\n", weighted ? "взвешенный" : "невзвешенный",
        directed ? "ориентированный" : "неориентированный");
    printf("================================================\n");

    printf("\nМатрица смежности G (%d вершин, вероятность ребра - %d%%):\n", size, edge_percent);
    print_matrix(G, size);

    int start;
    printf("\nВведите начальную вершину (от 0 до %d): ", size - 1);
    if (scanf("%d", &start) != 1 || start < 0 || start >= size) {
        printf("Некорректная начальная вершина.\n");
        free_matrix(G, size);
        return false;
    }

    int *DIST = (int *)malloc(size * sizeof(int));
    if (!DIST) { free_matrix(G, size); return false; }
    reset_dist(DIST, size);
    printf("\nПоиск расстояний\n");
    printf("Порядок обхода вершин: ");
    bfs_distance_weighted(start, size, G, DIST, true);
    printf("\n");
    print_dist(DIST, size, start);

    int **D = build_distance_matrix(size, G);
    printf("\nМатрица расстояний (\"-\" - вершина недостижима):\n");
    print_distance_matrix(D, size);

    int *ecc = find_eccentricities(D, size);
    printf("\n");
    print_graph_characteristics(ecc, size);

    free(ecc);
    free_matrix(D, size);
    free(DIST);
    free_matrix(G, size);
    return true;
}

bool parse_flag(const char *value, bool *result) {
    if (strcmp(value, "1") == 0 || strcmp(value, "yes") == 0) {
        *result = true;
        return true;
    }
    if (strcmp(value, "0") == 0 || strcmp(value, "no") == 0) {
        *result = false;
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    bool weighted = true, directed = false;
    bool directed_set = false;

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-weighted") == 0 || strcmp(argv[i], "-directed") == 0)
                && i + 1 == argc) {
            printf("Не указано значение параметра %s.\n", argv[i]);
            return 1;
        }
        if (strcmp(argv[i], "-weighted") == 0 && i + 1 < argc) {
            if (!parse_flag(argv[++i], &weighted)) {
                printf("Некорректное значение параметра -weighted: %s\n", argv[i]);
                return 1;
            }
        } else if (strcmp(argv[i], "-directed") == 0 && i + 1 < argc) {
            if (!parse_flag(argv[++i], &directed)) {
                printf("Некорректное значение параметра -directed: %s\n", argv[i]);
                return 1;
            }
            directed_set = true;
        } else {
            printf("Неизвестный параметр: %s\n", argv[i]);
            printf("Использование: %s [-weighted 0|1|yes|no] [-directed 0|1|yes|no]\n", argv[0]);
            return 1;
        }
    }

    int size, edge_percent;

    printf("Введите количество вершин графа: ");
    if (scanf("%d", &size) != 1 || size <= 0) {
        printf("Некорректное количество вершин.");
        return 1;
    }

    printf("Введите вероятность появления ребра в процентах (0-100): ");
    if (scanf("%d", &edge_percent) != 1 || edge_percent < 0 || edge_percent > 100) {
        printf("Некорректная вероятность.");
        return 1;
    }

    if (directed_set) {
        if (!process_graph(size, edge_percent, weighted, directed)) return 1;
    } else {
        if (!process_graph(size, edge_percent, weighted, false)) return 1;
        if (!process_graph(size, edge_percent, weighted, true)) return 1;
    }

    return 0;
}
