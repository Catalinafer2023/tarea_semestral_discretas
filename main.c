#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_STREETS   50
#define MAX_DISTANCIA 2000
#define MAX_TOURISTS  100
#define MAX_NOMBRE    128
#define MAX_NEIGHBORS 150
#define EPS_COORD  1e-3   // Para comparar coordenadas y distancias
#define EPS_DENOM  1e-7   // Para el determinante en interseccionSegmentos

// 1. Estructuras
typedef struct {
    float x, y;
    int id;
} Nodo;

typedef struct {
    float t;
    int id;
} NodoEnCalle;

typedef struct {
    char nombre[MAX_NOMBRE];
    float x1, y1, x2, y2;
    char sentido;
} Calle;

typedef struct {
    char descripcion[MAX_NOMBRE];
    char calle[MAX_NOMBRE];
    float posicion;
    int idNodo;
} PuntoTuristico;

// 2. Función auxiliar para evitar duplicar nodos
int obtenerIdNodo(Nodo* arregloNodos, int* totalNodos, float x, float y) {
    for (int i = 0; i < *totalNodos; i++) {
        if (fabs(arregloNodos[i].x - x) < EPS_COORD &&
            fabs(arregloNodos[i].y - y) < EPS_COORD) {
            return i;
        }
    }
    if (*totalNodos >= MAX_DISTANCIA) {
        printf("Error fatal: se superó el límite de %d nodos.\n", MAX_DISTANCIA);
        exit(1);
    }
    arregloNodos[*totalNodos].x = x;
    arregloNodos[*totalNodos].y = y;
    arregloNodos[*totalNodos].id = *totalNodos;
    (*totalNodos)++;
    return (*totalNodos) - 1;
}

// 3. Matriz de distancias
float** crearMatrizDistancias(int numNodos) {
    float** matriz = (float**)malloc(numNodos * sizeof(float*));
    if (matriz == NULL) { printf("Error: malloc fallido.\n"); exit(1); }
    for (int i = 0; i < numNodos; i++) {
        matriz[i] = (float*)malloc(numNodos * sizeof(float));
        if (matriz[i] == NULL) { printf("Error: malloc fallido.\n"); exit(1); }
        for (int j = 0; j < numNodos; j++) {
            matriz[i][j] = (i == j) ? 0.0f : -1.0f;
        }
    }
    return matriz;
}

// 4. Intersección de segmentos
int interseccionSegmentos(float x1, float y1, float x2, float y2,
                          float x3, float y3, float x4, float y4,
                          float* ix, float* iy) {
    float d1x = x2 - x1, d1y = y2 - y1;
    float d2x = x4 - x3, d2y = y4 - y3;
    float denom = d1x * d2y - d1y * d2x;

    if (fabs(denom) < EPS_DENOM) return 0;

    float t = ((x3 - x1) * d2y - (y3 - y1) * d2x) / denom;
    float u = ((x3 - x1) * d1y - (y3 - y1) * d1x) / denom;

    if (t >= -EPS_COORD && t <= 1.0f + EPS_COORD &&
        u >= -EPS_COORD && u <= 1.0f + EPS_COORD) {
        *ix = x1 + t * d1x;
        *iy = y1 + t * d1y;
        return 1;
    }
    return 0;
}

int cmpNodoEnCalle(const void* a, const void* b) {
    float diff = ((NodoEnCalle*)a)->t - ((NodoEnCalle*)b)->t;
    return (diff > 0) - (diff < 0);
}

// 5. Dijkstra
int* calcularDijkstra(float** matrix, int nNodes, int startNode, int endNode, int* largoRuta) {
    float dist[nNodes];
    int prev[nNodes];
    int visited[nNodes];

    for (int i = 0; i < nNodes; i++) {
        dist[i] = INFINITY;
        prev[i] = -1;
        visited[i] = 0;
    }
    dist[startNode] = 0.0f;

    for (int count = 0; count < nNodes; count++) {
        float minDist = INFINITY;
        int currentNode = -1;
        for (int v = 0; v < nNodes; v++) {
            if (!visited[v] && dist[v] <= minDist) {
                minDist = dist[v];
                currentNode = v;
            }
        }
        if (currentNode == -1 || currentNode == endNode) break;

        visited[currentNode] = 1;

        for (int neighbor = 0; neighbor < nNodes; neighbor++) {
            if (!visited[neighbor] && matrix[currentNode][neighbor] > 0.0f) {
                float alt = dist[currentNode] + matrix[currentNode][neighbor];
                if (alt < dist[neighbor]) {
                    dist[neighbor] = alt;
                    prev[neighbor] = currentNode;
                }
            }
        }
    }

    if (dist[endNode] == INFINITY) {
        *largoRuta = 0;
        return NULL;
    }

    int pathLength = 0;
    int curr = endNode;
    while (curr != -1) { pathLength++; curr = prev[curr]; }

    int* path = (int*)malloc(pathLength * sizeof(int));
    if (path == NULL) { printf("Error: malloc fallido.\n"); exit(1); }

    curr = endNode;
    for (int i = pathLength - 1; i >= 0; i--) {
        path[i] = curr;
        curr = prev[curr];
    }
    *largoRuta = pathLength;
    return path;
}

// 6. Construir grafo
float** construirGrafoCalles(Calle* calles, int numCalles, PuntoTuristico* turistas,
                              int numTuristas, Nodo* arregloNodos, int* totalNodos) {
    *totalNodos = 0;

    // Paso A: extremos de calles
    for (int i = 0; i < numCalles; i++) {
        obtenerIdNodo(arregloNodos, totalNodos, calles[i].x1, calles[i].y1);
        obtenerIdNodo(arregloNodos, totalNodos, calles[i].x2, calles[i].y2);
    }

    // Paso B: intersecciones
    for (int i = 0; i < numCalles; i++) {
        for (int j = i + 1; j < numCalles; j++) {
            float ix, iy;
            if (interseccionSegmentos(calles[i].x1, calles[i].y1, calles[i].x2, calles[i].y2,
                                      calles[j].x1, calles[j].y1, calles[j].x2, calles[j].y2,
                                      &ix, &iy)) {
                obtenerIdNodo(arregloNodos, totalNodos, ix, iy);
            }
        }
    }

    // Paso B2: puntos turísticos como nodos
    for (int t = 0; t < numTuristas; t++) {
        turistas[t].idNodo = -1;
        for (int c = 0; c < numCalles; c++) {
            if (strcmp(turistas[t].calle, calles[c].nombre) == 0) {
                float px, py;
                if (calles[c].sentido == 'X') {
                    px = turistas[t].posicion;
                    if (fabs(calles[c].x2 - calles[c].x1) > EPS_COORD) {
                        py = calles[c].y1 + ((calles[c].y2 - calles[c].y1) /
                             (calles[c].x2 - calles[c].x1)) * (px - calles[c].x1);
                    } else {
                        py = calles[c].y1;
                    }
                } else {
                    py = turistas[t].posicion;
                    if (fabs(calles[c].y2 - calles[c].y1) > EPS_COORD) {
                        px = calles[c].x1 + ((calles[c].x2 - calles[c].x1) /
                             (calles[c].y2 - calles[c].y1)) * (py - calles[c].y1);
                    } else {
                        px = calles[c].x1;
                    }
                }
                turistas[t].idNodo = obtenerIdNodo(arregloNodos, totalNodos, px, py);
                break;
            }
        }
        if (turistas[t].idNodo == -1) {
            printf("Advertencia: La calle '%s' del punto '%s' no existe en el mapa. "
                   "Se omitirá ese punto.\n",
                   turistas[t].calle, turistas[t].descripcion);
        }
    }

    // Paso C: crear matriz
    float** matrizDistancias = crearMatrizDistancias(*totalNodos);

    // Paso D: conectar nodos en la misma calle
    for (int c = 0; c < numCalles; c++) {
        float cx = calles[c].x2 - calles[c].x1;
        float cy = calles[c].y2 - calles[c].y1;
        float len2 = cx * cx + cy * cy;

        NodoEnCalle sobre[MAX_DISTANCIA];
        int cnt = 0;

        for (int n = 0; n < *totalNodos; n++) {
            float nx = arregloNodos[n].x - calles[c].x1;
            float ny = arregloNodos[n].y - calles[c].y1;
            float t_val = (nx * cx + ny * cy) / len2;

            if (t_val >= -EPS_COORD && t_val <= 1.0f + EPS_COORD) {
                float px = calles[c].x1 + t_val * cx;
                float py = calles[c].y1 + t_val * cy;
                float distSq = (arregloNodos[n].x - px) * (arregloNodos[n].x - px) +
                               (arregloNodos[n].y - py) * (arregloNodos[n].y - py);
                if (distSq < EPS_COORD) {
                    sobre[cnt].t  = t_val;
                    sobre[cnt].id = n;
                    cnt++;
                }
            }
        }

        qsort(sobre, cnt, sizeof(NodoEnCalle), cmpNodoEnCalle);

        for (int k = 0; k < cnt - 1; k++) {
            int id1 = sobre[k].id;
            int id2 = sobre[k + 1].id;
            float dx = arregloNodos[id1].x - arregloNodos[id2].x;
            float dy = arregloNodos[id1].y - arregloNodos[id2].y;
            float dist = sqrtf(dx * dx + dy * dy);
            matrizDistancias[id1][id2] = dist;
            matrizDistancias[id2][id1] = dist;
        }
    }
    return matrizDistancias;
}

// 7. Dado dos nodos consecutivos, devuelve el nombre de la calle que los conecta
const char* obtenerNombreCalle(Nodo* arregloNodos, Calle* calles, int numCalles, int id1, int id2) {
    for (int c = 0; c < numCalles; c++) {
        float cx = calles[c].x2 - calles[c].x1;
        float cy = calles[c].y2 - calles[c].y1;
        float len2 = cx * cx + cy * cy;
        if (len2 < EPS_COORD) continue;

        // Verificar que id1 esté sobre la calle
        float nx1 = arregloNodos[id1].x - calles[c].x1;
        float ny1 = arregloNodos[id1].y - calles[c].y1;
        float t1  = (nx1 * cx + ny1 * cy) / len2;
        if (t1 < -EPS_COORD || t1 > 1.0f + EPS_COORD) continue;
        float d1 = (arregloNodos[id1].x - (calles[c].x1 + t1*cx)) *
                   (arregloNodos[id1].x - (calles[c].x1 + t1*cx)) +
                   (arregloNodos[id1].y - (calles[c].y1 + t1*cy)) *
                   (arregloNodos[id1].y - (calles[c].y1 + t1*cy));
        if (d1 >= EPS_COORD) continue;

        // Verificar que id2 esté sobre la misma calle
        float nx2 = arregloNodos[id2].x - calles[c].x1;
        float ny2 = arregloNodos[id2].y - calles[c].y1;
        float t2  = (nx2 * cx + ny2 * cy) / len2;
        if (t2 < -EPS_COORD || t2 > 1.0f + EPS_COORD) continue;
        float d2 = (arregloNodos[id2].x - (calles[c].x1 + t2*cx)) *
                   (arregloNodos[id2].x - (calles[c].x1 + t2*cx)) +
                   (arregloNodos[id2].y - (calles[c].y1 + t2*cy)) *
                   (arregloNodos[id2].y - (calles[c].y1 + t2*cy));
        if (d2 >= EPS_COORD) continue;

        return calles[c].nombre;
    }
    return "desconocida";
}

//  MAIN 
int main() {
    char nombreArchivo[100];
    FILE* archivo = NULL;

    while (archivo == NULL) {
        printf("Ingresa el nombre del archivo del mapa: ");
        scanf("%99s", nombreArchivo);  
        archivo = fopen(nombreArchivo, "r");
        if (archivo == NULL) {
            printf("Error: No se pudo abrir '%s'. Intenta nuevamente.\n", nombreArchivo);
        }
    }

    int numCalles;
    fscanf(archivo, "%d", &numCalles);

    if (numCalles > MAX_STREETS) {
        printf("Error: El mapa tiene %d calles, superando el máximo de %d.\n", numCalles, MAX_STREETS);
        fclose(archivo);
        return 1;
    }

    Calle calles[MAX_STREETS];

    for (int i = 0; i < numCalles; i++) {

        fscanf(archivo, "%127s %f %f %f %f %c",
               calles[i].nombre,
               &calles[i].x1, &calles[i].y1,
               &calles[i].x2, &calles[i].y2,
               &calles[i].sentido);        

        if (calles[i].x1 < 0 || calles[i].x1 > 2000 ||
            calles[i].y1 < 0 || calles[i].y1 > 2000 ||
            calles[i].x2 < 0 || calles[i].x2 > 2000 ||
            calles[i].y2 < 0 || calles[i].y2 > 2000) {
            printf("Error: Coordenadas de '%s' fuera del rango 0-2000.\n", calles[i].nombre);
            fclose(archivo);
            return 1;
        }

        if (calles[i].sentido != 'X' && calles[i].sentido != 'Y') {
            printf("Error: Sentido de '%s' debe ser 'X' o 'Y'. Se leyó '%c'.\n",
                   calles[i].nombre, calles[i].sentido);
            fclose(archivo);
            return 1;
        }

        if (calles[i].sentido == 'X' && calles[i].x1 == calles[i].x2) {
            printf("Error: Calle '%s' declarada en X pero x1 == x2.\n", calles[i].nombre);
            fclose(archivo); 
        }

        if (calles[i].sentido == 'Y' && calles[i].y1 == calles[i].y2) {
            printf("Error: Calle '%s' declarada en Y pero y1 == y2.\n", calles[i].nombre);
            fclose(archivo); 
            return 1;
        }
    }

    int numPuntos;
    fscanf(archivo, "%d", &numPuntos);

    if (numPuntos > MAX_TOURISTS) {
        printf("Error: Puntos turísticos exceden el máximo de %d.\n", MAX_TOURISTS);
        fclose(archivo);
        return 1;
    }

    PuntoTuristico puntos[MAX_TOURISTS];

    for (int i = 0; i < numPuntos; i++) {
        fscanf(archivo, "%127s %127s %f",
               puntos[i].descripcion,
               puntos[i].calle,
               &puntos[i].posicion);
    }

    fclose(archivo);

    /* --- DEBUG: verificación de lectura ---*/
    printf("\n--- Calles Leidas ---\n");
    for (int i = 0; i < numCalles; i++) {
        printf("Calle: %s | Inicio: (%.1f, %.1f) | Fin: (%.1f, %.1f) | Eje: %c\n",
               calles[i].nombre, calles[i].x1, calles[i].y1,
               calles[i].x2, calles[i].y2, calles[i].sentido);
    }

    printf("\n--- Puntos Turisticos Leidos ---\n");
    for (int i = 0; i < numPuntos; i++) {
        printf("Lugar: %s | Ubicacion: calle %s, pos %.1f\n",
               puntos[i].descripcion, puntos[i].calle, puntos[i].posicion);
    }
    /*-- fin DEBUG --- */

    Nodo arregloNodos[MAX_DISTANCIA];
    int totalNodos = 0;

    float** matriz = construirGrafoCalles(calles, numCalles, puntos, numPuntos,
                                          arregloNodos, &totalNodos);

    /* --- DEBUG: grafo generado --- */
    printf("\n=== LISTA DE VERTICES (NODOS) ===\n");
    printf("Total de nodos generados: %d\n", totalNodos);
    for (int i = 0; i < totalNodos; i++) {
        printf("Nodo %2d: (%.1f, %.1f)\n", i, arregloNodos[i].x, arregloNodos[i].y);
    }

    printf("\n=== MATRIZ DE DISTANCIAS ===\n");
    printf("     ");
    for (int i = 0; i < totalNodos; i++) printf("N%02d   ", i);
    printf("\n");

    for (int i = 0; i < totalNodos; i++) {
        printf("N%02d: ", i);
        for (int j = 0; j < totalNodos; j++) {
            if (matriz[i][j] < 0)  printf("  --  ");
            else                   printf("%5.1f ", matriz[i][j]);
        }
        printf("\n");
    }
   /* --- fin DEBUG --- */

    //  Ruta turística 
    if (numPuntos == 0 || puntos[0].idNodo == -1) {
        printf("Error: El punto de partida no tiene una calle válida.\n");
        for (int i = 0; i < totalNodos; i++) free(matriz[i]);
        free(matriz);
        return 1;
    }

    int visitados[MAX_TOURISTS] = {0};
    int nodoActual = puntos[0].idNodo;
    visitados[0] = 1;

    printf("\n=== INICIANDO RUTA TURISTICA ===\n");
    printf("Punto de partida: %s (Nodo %d)\n", puntos[0].descripcion, nodoActual);

    for (int i = 1; i < numPuntos; i++) {
        if (visitados[i]) continue;

        if (puntos[i].idNodo == -1) {
            printf("\n[OMITIDO] %s no tiene calle válida.\n", puntos[i].descripcion);
            visitados[i] = 1;
            continue;
        }

        int nodoDestino = puntos[i].idNodo;
        int largoRuta = 0;
        int* ruta = calcularDijkstra(matriz, totalNodos, nodoActual, nodoDestino, &largoRuta);

        if (ruta != NULL) {
            printf("\nViajando hacia: %s...\n", puntos[i].descripcion);

            /* --- anterior: imprimía IDs de nodos --- */
            printf("Ruta: ");
            for (int p = 0; p < largoRuta; p++) {
                int nodoEnRuta = ruta[p];
                printf("%d%s", nodoEnRuta, (p == largoRuta - 1) ? "" : " -> ");
                for (int t = 0; t < numPuntos; t++) {
                    if (puntos[t].idNodo != -1 &&
                        nodoEnRuta == puntos[t].idNodo && !visitados[t]) {
                        visitados[t] = 1;
                        if (t != i)
                            printf("\n  [!] Visitaste de pasada: %s", puntos[t].descripcion);
                    }
                }
            }
           /* --- fin anterior --- */

            // Instrucciones por calle: agrupa tramos consecutivos del mismo nombre
            const char* calleActual = NULL;
            int idInicioTramo = ruta[0];

            for (int p = 0; p < largoRuta; p++) {
                int nodoEnRuta = ruta[p];

                // Detectar puntos turísticos de pasada
                for (int t = 0; t < numPuntos; t++) {
                    if (puntos[t].idNodo != -1 &&
                        nodoEnRuta == puntos[t].idNodo && !visitados[t]) {
                        visitados[t] = 1;
                        if (t != i)
                            printf("  [!] Visitaste de pasada: %s\n", puntos[t].descripcion);
                    }
                }

                // Determinar la calle del tramo actual → siguiente
                const char* calleSiguiente = (p < largoRuta - 1)
                    ? obtenerNombreCalle(arregloNodos, calles, numCalles, ruta[p], ruta[p + 1])
                    : NULL;

                // Si la calle cambia (o llegamos al final), imprimimos la instrucción del tramo
                if (calleActual != NULL &&
                    (calleSiguiente == NULL || strcmp(calleActual, calleSiguiente) != 0)) {
                    printf("  Seguir por calle %s hasta (%.1f, %.1f)\n",
                           calleActual,
                           arregloNodos[nodoEnRuta].x,
                           arregloNodos[nodoEnRuta].y);
                    idInicioTramo = nodoEnRuta;
                }
                calleActual = calleSiguiente;
            }
            printf("\n");
            nodoActual = nodoDestino;
            free(ruta);
        } else {
            printf("\n[ERROR] No hay camino a %s. Saltando...\n", puntos[i].descripcion);
        }
    }

    printf("\n=== RUTA FINALIZADA ===\n");

    for (int i = 0; i < totalNodos; i++) free(matriz[i]);
    free(matriz);
    return 0;
}
