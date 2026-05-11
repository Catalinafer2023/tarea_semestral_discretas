#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STREETS    50   
#define MAX_DISTANCIA      2000 
#define MAX_TOURISTS   100   
#define MAX_NOMBRE       128   
#define MAX_NEIGHBORS  150   
// Generar una matriz de 0
int** crearMatrizAdyacencia(int numNodos) {
    int** matriz = (int**)malloc(numNodos * sizeof(int*));
    if (matriz == NULL) {
        printf("Error: No se pudo asignar memoria para las filas de la matriz.\n");
        exit(1);
    }

    for (int i = 0; i < numNodos; i++) {
        matriz[i] = (int*)malloc(numNodos * sizeof(int));
        if (matriz[i] == NULL) {
            printf("Error: No se pudo asignar memoria para las columnas de la fila %d.\n", i);
            exit(1);
        }

        for (int j = 0; j < numNodos; j++) {
            matriz[i][j] = 0;
        }
    }

    return matriz;
}

typedef struct {
    char nombre[MAX_NOMBRE];
    float x1, y1, x2, y2;
    char sentido;
} Calle;

typedef struct {
    char descripcion[MAX_NOMBRE];
    char calle[MAX_NOMBRE];
    float posicion;
} PuntoTuristico;

int main() {
    int puntos_de_interseción_de_calles = 0;
    char nombreArchivo[100];
    FILE *archivo = NULL;
    
    // El programa debe hacer ver el error y volver a preguntar sin caerse
    while (archivo == NULL) {
        printf("Ingresa el nombre del archivo del mapa: ");
        scanf("%s", nombreArchivo);

        archivo = fopen(nombreArchivo, "r");
        if (archivo == NULL) {
            printf("Error: No se pudo abrir el archivo '%s'. Intenta nuevamente.\n", nombreArchivo);
        }
    }

    // LECTURA DE CALLES
    int numCalles;
    fscanf(archivo, "%d", &numCalles);
    puntos_de_interseción_de_calles += numCalles;
    if (numCalles > MAX_STREETS) {
        printf("Error: El mapa tiene %d calles, superando el maximo permitido de %d.\n", numCalles, MAX_STREETS);
        fclose(archivo);
        return 1;
    }
    
    Calle calles[MAX_STREETS];
    
    for (int i = 0; i < numCalles; i++) {
        fscanf(archivo, "%s %f %f %f %f %c", 
               calles[i].nombre, 
               &calles[i].x1, &calles[i].y1, 
               &calles[i].x2, &calles[i].y2, 
               &calles[i].sentido);
               
        if (calles[i].x1 < 0 || calles[i].x1 > 2000 || calles[i].y1 < 0 || calles[i].y1 > 2000 ||
            calles[i].x2 < 0 || calles[i].x2 > 2000 || calles[i].y2 < 0 || calles[i].y2 > 2000) {
            printf("Error: Las coordenadas de la calle %s no estan en el rango de 0 a 2000.\n", calles[i].nombre);
            fclose(archivo);
            return 1;
        }
        
        if (calles[i].sentido != 'X' && calles[i].sentido != 'Y') {
            printf("Error: El sentido de la calle %s debe ser 'X' o 'Y'. Se leyo '%c'.\n", calles[i].nombre, calles[i].sentido);
            fclose(archivo);
            return 1;
        }
        if (calles[i].sentido == 'X' && calles[i].x1 == calles[i].x2){
            printf("Error: No puedes decir qué el sentido de una calle es un sentido contrario a lo qué realmente es %s debe ser 'X' o 'Y'. Se leyo '%c'.\n", calles[i].nombre, calles[i].sentido);

        }
        if (calles[i].sentido == 'Y' && calles[i].y1 == calles[i].y2){
            printf("Error: No puedes decir qué el sentido de una calle es un sentido contrario a lo qué realmente es %s debe ser 'X' o 'Y'. Se leyo '%c'.\n", calles[i].nombre, calles[i].sentido);
        }
    }

    // puntos turisticos
    int numPuntos;
    fscanf(archivo, "%d", &numPuntos);
    
    if (numPuntos > MAX_TOURISTS) {
        printf("Error: El numero de puntos turisticos excede el maximo soportado de %d.\n", MAX_TOURISTS);
        fclose(archivo);
        return 1;
    }
    
    PuntoTuristico puntos[MAX_TOURISTS]; 
    
    for (int i = 0; i < numPuntos; i++) {
        fscanf(archivo, "%s %s %f", 
               puntos[i].descripcion, 
               puntos[i].calle, 
               &puntos[i].posicion);
    }

    fclose(archivo);

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

    return 0;
}
void liberarMatrizAdyacencia(int** matriz, int numNodos) {
    if (matriz == NULL) return;
    
    for (int i = 0; i < numNodos; i++) {
        free(matriz[i]);
    }
    free(matriz);
}
