#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <getopt.h>
#include <string.h>

// Tama�o m�ximo de caracteres de la ruta (nombre) de los ficheros
#define MAX_CAMINO 4096
// Tama�o de cada clase del histograma de ficheros
#define C10KB 10240

/*
 * El programa genera un histograma de los directorios dentro de una
 * jeraqu�a seg�n la cantidad de ficheros que cuelguen directamente de
 * �l (opci�n -n) o un histograma de los ficheros dentro de la misma
 * jerarqu�a seg�n el tama�o en bytes (opcion -t)
 */

char *diract = "."; // Si no se especifica directorio,
// el programa se aplica al directorio actual

int totalf = 0;     // Total de ficheros
int totald = 1;     // Total de directorios
int tam[11], num[11];   // Tablas de histograma segun opcion -t u opcion-n


int opciont=0, opcionn=0;

// Recorre la parte de la jerarqu�a de ficheros que comienza en
// el directorio dir y le aplica la funci�n de contabilidad f
// especificada como argumento de esta funci�n
int recorrer_arbol(char *dir, int (*f)()) {
    DIR *dirp;
    struct dirent *dp;
    struct stat buf;
    char fichero[256];
    int ok;

    // Abre el directorio dir, y obtiene un descriptor dirp de tipo DIR
    if ((dirp = opendir(dir)) == NULL) {
        perror(dir);
        return -1;
    }

    // Aplica la funci�n de contabilidad al directorio dir
    if (f(dir) == -1) return -1;

    // Recorre recursivamente los directorios por debajo del actual


    // Comienza leyendo una entrada del directorio
    while ((dp = readdir(dirp)) != NULL) {
        //  prescinde de las entradas "." y ".." que producir�an un
        //  bucle sin fin si se consideraran
        if (!strcmp(".", dp->d_name)) continue;
        if (!strcmp("..", dp->d_name)) continue;
        // Construye el nombre del fichero a partir del directorio dir
        sprintf(fichero, "%s/%s", dir, dp->d_name);
        // Averigua si la entrada corresponde a un sub-directorio
        ok = lstat(fichero, &buf);
        // Si lo es, le aplica recursivamente esta misma funci�n
        //  y actualiza el total de directorios recorridos
        if ((ok != 1) && ((buf.st_mode & S_IFMT)==S_IFDIR))  {
            totald++;
            if (recorrer_arbol(fichero, f) == -1) break;
        }
    }
    return 0;
}

/* Funci�n que construye el histograma de ficheros seg�n tama�os
 * Examina los ficheros contenidos en el directorio dir,
 *  consulta su tama�o y seg�n el valor incrementa el acumulador
 *  de su clase.
 * Tambi�n actualiza el contador del total de ficheros
 */
int cnt_tam(char *dir) {
    DIR *dirp;
    struct dirent *dp;
    struct stat buf;
    char fichero[256];
    int n, ok;

    if ((dirp = opendir(dir)) == NULL) {
        perror(dir);
        return -1;
    }

    while ((dp = readdir(dirp)) != NULL) {
        if (!strcmp(".", dp->d_name)) continue;
        if (!strcmp("..", dp->d_name)) continue;
        sprintf(fichero, "%s/%s", dir, dp->d_name);
        ok = lstat(fichero, &buf);
        if ((ok != 1) && ((buf.st_mode & S_IFMT)==S_IFREG)) {
            totalf++;
            if ((n = buf.st_size / C10KB) < 0)
                continue;
            else if (n < 10)
                tam[n]++;
            else
                tam[10]++;
        }
    }
    return 0;
}

/* Funci�n que construye el histograma de directorios seg�n n�mero de
 * ficheros que contienen
 * Contabiliza el n�mero de ficheros contenidos en el directorio dir,
 *  y seg�n el valor obtenido incrementa el acumulador de su clase.
 */
int cnt_num(char *dir) {
    DIR *dirp;
    struct dirent *dp;
    int n, nf=0;

    if ((dirp = opendir(dir)) == NULL) {
        perror(dir);
        return -1;
    }
    while ((dp = readdir(dirp)) != NULL) {
        if (!strcmp(".", dp->d_name)) continue;
        if (!strcmp("..", dp->d_name)) continue;
        nf++;
    }
    if ((n = (nf/10)) < 10)
        num[n]++;
    else
        num[10]++;
    return 0;
}

/* Funcion que presenta en pantalla los resultados del ca�culo
 * de los histograma de ficheros por tama�o (opcion -t)
 * o de directorios segun n�emro de ficheros (opcion -n
 */

void presentar_resultados(char *dir) {
    int i;

    printf("Resultados para el directorio %s.\n", dir);
    if (opciont) {
        printf("Rango\t\tTotal\tPorcentaje\n");
        for (i=0; i<10; i++)
            printf("[%dK, %dK]\t%d\t%f%%\n",
                   10*i, 10*i+10, tam[i], (tam[i]*100.0)/totalf);
        printf("Mayores\t\t%d\t%f%%\n", tam[10], (tam[10]*100.0)/totalf);
        printf("=======\t\t=====\t=======\n");
        printf("TOTAL FICHEROS\t%d\t%f%%\n\n", totalf, 100.00);
    }
    if (opcionn) {
        printf("Rango\t\tTotal\tPorcentaje\n");
        for (i=0; i<10; i++)
            printf("[%2d, %3d]\t%d\t%f%%\n",
                   10*i, 10*i+10, num[i], (num[i]*100.0)/totald);
        printf("Mas de 100\t%d\t%f%%\n", num[10], (num[10]*100.0)/totald);
        printf("=======\t\t=====\t=======\n");
        printf("TOTAL DIRECT.\t%d\t%f%%\n\n", totald, 100.00);
    }
}

int main(int argc, char *argv[]) {
    char *dir;
    char opcion;
    int primera,i;

    for (i=0; i<argc; i++)
        printf("argc[%d]=%s\n", i, argv[i]);

    // Examina las opciones en l�nea de orden
    while ((opcion = getopt(argc, argv, "tn")) != EOF)  {
        switch (opcion)  {
        case 't':   // Histograma de ficheros por tama�o
            opciont = 1;
            break;
        case 'n':     // Histograma de directorios por n�mero de ficheros
            opcionn = 1;
            break;
        default:
            fprintf(stderr, "Error, opcion %c ilegal\n", opcion);
        }
    }

    for (i=0; i<argc; i++)
        printf("argc[%d]=%s\n", i, argv[i]);

    // Por defecto se aplica la opci�n -t
    if (!opciont && !opcionn) opciont=1;

    primera=1;
    for (; (optind<argc)||primera ; optind++) {
        primera = 0;
        dir = (optind < argc) ? argv[optind] : diract;

        if (opciont)
            // recorre el �rbol de directorios aplicando la funci�n cnt_tam
            recorrer_arbol(dir, cnt_tam);
        if (opcionn)
            // recorre el �rbol de directorios aplicando la funci�n cnt_num
            recorrer_arbol(dir, cnt_num);
        presentar_resultados(dir);
        //printf("%d: directorio %s [%d,%d]\n", optind, dir, opciont, opcionn);
    }

    return 0;
}
