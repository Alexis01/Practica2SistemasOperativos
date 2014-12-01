#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <getopt.h>
#include <string.h>

// Tamaño máximo de caracteres de la ruta (nombre) de los ficheros
#define MAX_CAMINO 4096
// Tamaño de cada clase del histograma de ficheros
#define C10KB 10240

/*
 * El programa genera un histograma de los directorios dentro de una
 * jeraquía según la cantidad de ficheros que cuelguen directamente de
 * él (opción -n) o un histograma de los ficheros dentro de la misma 
 * jerarquía según el tamaño en bytes (opcion -t)
 */
 
char *diract = ".";	// Si no se especifica directorio, 
					// el programa se aplica al directorio actual

int totalf;			// Total de ficheros
int totald;			// Total de directorios
int tam[11], num[11]; 	// Tablas de histograma segun opcion -t u opcion-n


int opciont=0, opcionn=0;

// Recorre la parte de la jerarquía de ficheros que comienza en 
// el directorio dir y le aplica la función de contabilidad f 
// especificada como argumento de esta función
int recorrer_arbol(char *dir, int (*f)()) {
	DIR *dirp;
	struct dirent *dp;
	struct stat buf;
	char fichero[256];
	int ok;

	// Abre el directorio dir, y obtiene un descriptor dirp de tipo DIR
	if ((dirp = opendir(dir)) == NULL) { perror(dir); return -1; }

	// Aplica la función de contabilidad al directorio dir
	if (f(dir) == -1){
		closedir(dirp);
		return -1;
	}
	
	// Recorre recursivamente los directorios por debajo del actual
	
	
	// Comienza leyendo una entrada del directorio
	while ((dp = readdir(dirp)) != NULL) {
		//	prescinde de las entradas "." y ".." que producirían un
		//	bucle sin fin si se consideraran
		if (!strcmp(".", dp->d_name)) continue;
		if (!strcmp("..", dp->d_name)) continue;
		// Construye el nombre del fichero a partir del directorio dir
		sprintf(fichero, "%s/%s", dir, dp->d_name);
		// Averigua si la entrada corresponde a un sub-directorio
		ok = lstat(fichero, &buf);
		// Si lo es, le aplica recursivamente esta misma función
		//	y actualiza el total de directorios recorridos
		if ((ok != 1) && ((buf.st_mode & S_IFMT)==S_IFDIR))  {
			totald++;
			if (recorrer_arbol(fichero, f) == -1) break;
		}
	}
        closedir(dirp);
	return 0;
}

/* Función que construye el histograma de ficheros según tamaños
 * Examina los ficheros contenidos en el directorio dir, 
 * 	consulta su tamaño y según el valor incrementa el acumulador 
 *	de su clase. 
 * También actualiza el contador del total de ficheros
 */
int cnt_tam(char *dir) {
	DIR *dirp;
	struct dirent *dp;
	struct stat buf;
	char fichero[256];
	int n, ok;

	if ((dirp = opendir(dir)) == NULL) { perror(dir); return -1; }

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
        closedir(dirp);
	return 0;
}

/* Función que construye el histograma de directorios según número de
 * ficheros que contienen
 * Contabiliza el número de ficheros contenidos en el directorio dir, 
 * 	y según el valor obtenido incrementa el acumulador de su clase. 
 */
int cnt_num(char *dir) {
	DIR *dirp;
	struct dirent *dp;
	int n, nf=0;

	if ((dirp = opendir(dir)) == NULL) { perror(dir); return -1; }
	while ((dp = readdir(dirp)) != NULL) {
		if (!strcmp(".", dp->d_name)) continue;
		if (!strcmp("..", dp->d_name)) continue;
		nf++;
	}
	if ((n = (nf/10)) < 10)
		num[n]++;
	else
		num[10]++;
        closedir(dirp);
	return 0;
}

/* Funcion que presenta en pantalla los resultados del caáculo
 * de los histograma de ficheros por tamaño (opcion -t)
 * o de directorios segun núemro de ficheros (opcion -n
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
	int primera;

	// Examina las opciones en línea de orden 
	while ((opcion = getopt(argc, argv, "tn")) != EOF)  {
		switch (opcion)  {
		case 't':		// Histograma de ficheros por tamaño
			opciont = 1; break;
		case 'n': 		// Histograma de directorios por número de ficheros
			opcionn = 1; break;
		default:
			fprintf(stderr, "Error, opcion %c ilegal\n", opcion);
		}
	}
	// Por defecto se aplica la opción -t
	if (!opciont && !opcionn) opciont=1;
	
	primera=1;
	for (; (optind<argc)||primera ; optind++) {
		primera = 0;
		dir = (optind < argc) ? argv[optind] : diract;

		if (opciont)
			totalf = 0; totald = 1;
			// recorre el árbol de directorios aplicando la función cnt_tam
			recorrer_arbol(dir, cnt_tam);
		if (opcionn)
			totald = 1;
			// recorre el árbol de directorios aplicando la función cnt_num
			recorrer_arbol(dir, cnt_num);
		presentar_resultados(dir);
		//printf("%d: directorio %s [%d,%d]\n", optind, dir, opciont, opcionn);
	}

	return 0;
}
