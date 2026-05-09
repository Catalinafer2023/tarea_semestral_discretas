#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> // Necesario para rand() y srand()

#define MAX_LINE_LENGTH 256


// --- Implementación de funciones ---

void createMatrix(char*** matrix, int size) {
    // Se aloja memoria para la matriz
    *matrix = (char**)malloc(size * sizeof(char*));
    if (*matrix == NULL) {
        perror("Error en malloc para filas");
        exit(1);
    }
    for (int i = 0; i < size; i++) {
        (*matrix)[i] = (char*)malloc(size * sizeof(char));
        if ((*matrix)[i] == NULL) {
            perror("Error en malloc para columnas");
            exit(1);
        }
        for (int j = 0; j < size; j++) {
            (*matrix)[i][j] = 0;
        }
    }
}

void freeMatrix(char** matrix, int size) {
    // Se libera la memoria de la matriz (se libera tambien cada fila para evitar fugas de memoria)
    if (matrix == NULL) {
        return;
    }
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}


void readGraph(const char* fileName, int* nNodes, char*** matrix) {
    // Se lee el grafo del archivo y se guarda en la matriz
    FILE *file;
    char lineV[MAX_LINE_LENGTH];
    char lineE[MAX_LINE_LENGTH];

    // Se valida que el archivo exista y no esté vacio
    file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    if (fgets(lineV, MAX_LINE_LENGTH, file) == NULL || fgets(lineE, MAX_LINE_LENGTH, file) == NULL) {
        fprintf(stderr, "Error: El archivo está incompleto o vacío.\n");
        fclose(file);
        exit(1);
    }
    fclose(file);

    // Se va leyendo las lineas del archivo y se separan por comas los strings
    lineV[strcspn(lineV, "\n\r")] = 0;
    lineE[strcspn(lineE, "\n\r")] = 0;

    char* lineV_copy = strdup(lineV); 
    char* lineE_copy = strdup(lineE);
    
    char* token = strtok(lineV_copy, ",");
    while (token != NULL) {
        (*nNodes)++;
        token = strtok(NULL, ",");
    }

    if (*nNodes == 0) {
        fprintf(stderr, "Error: No se encontraron nodos en el archivo.\n");
        exit(1);
    }

    // Se reserva la memoria de la matriz
    createMatrix(matrix, *nNodes);

    // Se guardan los datos dentro de la matriz
    token = strtok(lineE_copy, ",");
    while (token != NULL) {
        if (strlen(token) >= 2) {
            char valA = token[0] - 'a';
            char valB = token[1] - 'a';
            if (valA >= 0 && valA < *nNodes && valB >= 0 && valB < *nNodes) {
                (*matrix)[valA][valB] = 1;
                (*matrix)[valB][valA] = 1;
            }
        }
        token = strtok(NULL, ",");
    }
    
    free(lineV_copy);
    free(lineE_copy);
}

void crear_matriz_orientada12(char*** matriz_ptr,char*** matrix_salida1,char*** matrix_salida2,int nNodes) {
    char** in = *matriz_ptr;

    // Inicializa ambas en 0
    createMatrix(matrix_salida1, nNodes);
    createMatrix(matrix_salida2, nNodes);

    // Recorre solo mitad superior 
    for (int i = 0; i < nNodes; i++) {
        for (int j = i + 1; j < nNodes; j++) {
            // Hay arista si cualquiera de las dos posiciones está en 1
            if (in[i][j] == 1) {
                // Orientación determinista:
                // salida1: i -> j
                (*matrix_salida1)[i][j] = 1;
                // salida2 (complementaria): j -> i
                (*matrix_salida2)[j][i] = 1;
            }
        }
    }
}
void crear_matriz_orientada34(char*** matriz_ptr,char*** matrix_salida3,char*** matrix_salida4,int nNodes){
    char** in = *matriz_ptr;

    createMatrix(matrix_salida3, nNodes);
    createMatrix(matrix_salida4, nNodes);

    for (int i = 0; i < nNodes; i++) {
        for (int j = i + 1; j < nNodes; j++) {
            if (in[i][j]) {
                if ( ((i + j) & 1) == 0 ) {
                    // par: i -> j en la 3, j -> i en la 4
                    (*matrix_salida3)[i][j] = 1;
                    (*matrix_salida4)[j][i] = 1;
                } else {
                    // impar: j -> i en la 3, i -> j en la 4
                    (*matrix_salida3)[j][i] = 1;
                    (*matrix_salida4)[i][j] = 1;
                }
            }
        }
    }
}
void Imprimir_matriz_adyacencia(char** matrix, int nNodes) {
    // Encabezado de columnas (con espacio inicial para alinear)
    printf("  ");
    for (int j = 0; j < nNodes; j++) {
        printf("%c ", 'a' + j);
    }
    printf("\n");
    // Filas
    for (int i = 0; i < nNodes; i++) {
        printf("%c ", 'a' + i);
        for (int j = 0; j < nNodes; j++) {
            printf("%d ", (int)matrix[i][j]);
        }
        printf("\n");
    }
}


void findShortestPath(char** matrix, int nNodes, char v1, char v2) {
    int startNode = v1 - 'a';
    int endNode = v2 - 'a';

    // Fila 0: distancia desde el inicio; Fila 1: nodo predecesor en el camino
    int pathInfo[2][nNodes];
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < nNodes; j++) {
            pathInfo[i][j] = -1; // -1 representa infinito/no visitado
        }
    }

    int queue[nNodes]; // Cola para los nodos a visitar
    int visited[nNodes]; // Para marcar nodos ya visitados y en la cola
    for(int i = 0; i < nNodes; i++) visited[i] = 0;

    int head = 0, tail = 0;

    // 1. Iniciar el algoritmo
    pathInfo[0][startNode] = 0; // Distancia a sí mismo es 0
    queue[tail++] = startNode;
    visited[startNode] = 1;

    // 2. Búsqueda
    while (head < tail) {
        int currentNode = queue[head++]; // Sacar el primer nodo de la cola

        if (currentNode == endNode) {
            break; // Se encontró el destino
        }

        // Explorar vecinos
        for (int neighbor = 0; neighbor < nNodes; neighbor++) {
            if (matrix[currentNode][neighbor] == 1 && !visited[neighbor]) {
                visited[neighbor] = 1;
                pathInfo[0][neighbor] = pathInfo[0][currentNode] + 1; // Distancia es 1 más que el padre
                pathInfo[1][neighbor] = currentNode;                  // Guardar quién lo encontró
                queue[tail++] = neighbor;                             // Añadir vecino a la cola
            }
        }
    }

    // 3. Reconstruir y mostrar el camino
    if (pathInfo[0][endNode] == -1) {
        printf("No se encontró un camino entre %c y %c.\n", v1, v2);
    } else {
        int pathLength = pathInfo[0][endNode] + 1;
        printf("El camino mas corto tiene %d nodos y es:\n", pathLength);

        int path[pathLength];
        int currentNode = endNode;
        for (int i = pathLength - 1; i >= 0; i--) {
            path[i] = currentNode;
            currentNode = pathInfo[1][currentNode];
        }

        for (int i = 0; i < pathLength; i++) {
            printf("%c%s", 'a' + path[i], (i == pathLength - 1) ? "" : " -> ");
        }
        printf("\n");
    }
}

int main(int argc, char const *argv[]) {
    if (argc != 4) {
        printf("Error, uso del programa: %s v1 v2 grafo.txt\n", argv[0]);
        printf("Lista de grafos:\n");
        printf("-eulerian.txt\n");
        printf("-plane.txt\n");
        printf("-tree.txt\n");
        return 1;
    }
    
    // Inicializar el generador de números aleatorios
    srand(time(NULL));

    const char v1 = argv[1][0];
    const char v2 = argv[2][0];
    const char* fileName = argv[3];

    int nNodes = 0;
    char** matrix = NULL;
    char** matrix_orientada1 = NULL;
    char** matrix_orientada2 = NULL;
    char** matrix_orientada3 = NULL;
    char** matrix_orientada4 = NULL;

    readGraph(fileName, &nNodes, &matrix);
    crear_matriz_orientada12(&matrix, &matrix_orientada1,&matrix_orientada2, nNodes);
    crear_matriz_orientada34(&matrix,&matrix_orientada3,&matrix_orientada4, nNodes);


    // Imprimir matriz de adyacencia original
    printf("Matriz de adyacencia de grafo no orientado:\n");
    Imprimir_matriz_adyacencia(matrix,nNodes);
  // Camino en el grafo original
    printf(" Camino en Grafo no orientado\n");
    findShortestPath(matrix, nNodes, v1, v2);
    // Imprimir matriz de adyacencia nueva (orientada)
    printf("matriz orientada 1:\n");
    Imprimir_matriz_adyacencia(matrix_orientada1,nNodes);
// Camino en el grafo orientado 1
    printf("\n Camino del grafo orientado 1\n");
    findShortestPath(matrix_orientada1, nNodes, v1, v2);
//grafo orientado 2
    printf("Matriz de adyaciencia de grafo orientado 2: \n");
    Imprimir_matriz_adyacencia(matrix_orientada2,nNodes);

    printf("\n Camino del grafo orientado 2\n");
    findShortestPath(matrix_orientada2, nNodes, v1, v2);
//grafo orientado 3
    printf("Matriz de grafo orientado 3:\n");
    Imprimir_matriz_adyacencia(matrix_orientada3,nNodes);

    printf("\n Camino del grafo orientado 3\n");
    findShortestPath(matrix_orientada3, nNodes, v1, v2);
//grafo orientado 4
    printf("Matriz del grafo orientado 4: \n");
    Imprimir_matriz_adyacencia(matrix_orientada4,nNodes);

    printf("\nCamino del grafo orientado 4\n");
    findShortestPath(matrix_orientada4, nNodes, v1, v2);
    
    // Liberar ambas matrices
freeMatrix(matrix, nNodes);
freeMatrix(matrix_orientada1, nNodes);
freeMatrix(matrix_orientada2, nNodes);
freeMatrix(matrix_orientada3, nNodes);
freeMatrix(matrix_orientada4, nNodes);

    return 0;
}
