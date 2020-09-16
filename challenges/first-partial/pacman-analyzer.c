#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FECHA_DE_BUSQUEDA 10
#define ACCION_DE_BUSQUEDA 11
#define ACCION_DE_EXTRACCION 12
#define NOMBRE_DE_BUSQUEDA 13
#define BUSQUEDA_NUEVA_LINEA 14

#define INSTALLED 0
#define REMOVED 1
#define UPGRADED 2

#define REPORT "packages_report.txt"

struct Paquete
{
    char nombre[50];
    char ifecha[17];
    char lupdate[17];
    int actualizaciones;
    char rdate[17];
    int status;
};

struct Hashtable
{
    int tamaño;
    int nelements;
    struct Paquete array[1000];
};

void analizar(char *logFile, char *report);

bool esAccion(char c1, char c2, char c3);

void agregarAlHashtable(struct Hashtable *ht, struct Paquete *p);

int codigoHash(char s[]);

bool buscarEnHash(struct Hashtable *ht, char key[]);

struct Paquete *get(struct Hashtable *ht, char key[]);

void printHashtable(struct Hashtable *ht);

void htToString(char string[], struct Hashtable *ht);

void printPaquete(struct Paquete *p);

void pToString(char string[], struct Paquete *ht);

void generarReporte(char *reportS, int iPaquetes, int rPaquetes, int uPaquetes, int cInstalados, struct Hashtable *ht);

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        return 1;
    }

    analizar(argv[1], REPORT);

    return 0;
}

void analizar(char *logFile, char *report)
{
    printf("Generando reporte de: [%s]\n", logFile);

    struct Hashtable ht = {1000, 0};
    int iPaquetes = 0;
    int rPaquetes = 0;
    int uPaquetes = 0;
    int cInstalados = 0;

    int fd = open(logFile, O_RDONLY);
    if (fd == -1)
    {
        printf("No se pudo abrir el archivo.\n");
        return;
    }
    int tamaño = lseek(fd, sizeof(char), SEEK_END);
    close(fd);
    fd = open(logFile, O_RDONLY);
    if (fd == -1)
    {
        printf("No se pudo abrir el archivo.\n");
        return;
    }
    char buf[tamaño];
    read(fd, buf, tamaño);
    close(fd);
    buf[tamaño - 1] = '\0';

    int i = 0;
    int j = 0;
    int estado = FECHA_DE_BUSQUEDA;
    char date[17];
    char nombre[50];
    char accion[10];
    bool banderaLinea = false;
    while (i < tamaño)
    {
        switch (estado)
        {
        case FECHA_DE_BUSQUEDA:
            if (buf[i] != 'f')
            {
                i++;
                j = 0;
                while (buf[i] != ']')
                {
                    date[j] = buf[i];
                    j++;
                    i++;
                }
                date[j] = '\0';
                i = i + 2;
                estado = ACCION_DE_BUSQUEDA;
            }
            else
            {
                estado = ACCION_DE_BUSQUEDA;
            }
            break;

        case ACCION_DE_BUSQUEDA:
            while (buf[i] != ' ')
            {
                i++;
            }
            i++;
            estado = ACCION_DE_EXTRACCION;
            break;

        case ACCION_DE_EXTRACCION:
            j = 0;
            if (esAccion(buf[i], buf[i + 1], buf[i+2]))
            {
                banderaLinea = true;
                while (buf[i] != ' ')
                {
                    accion[j] = buf[i];
                    i++;
                    j++;
                }
                accion[j] = '\0';
                i++;
                estado = NOMBRE_DE_BUSQUEDA;
            }
            else
            {
                estado = BUSQUEDA_NUEVA_LINEA;
            }
            break;

        case NOMBRE_DE_BUSQUEDA:
            j = 0;
            while (buf[i] != ' ')
            {
                nombre[j] = buf[i];
                i++;
                j++;
            }
            nombre[j] = '\0';
            i++;
            estado = BUSQUEDA_NUEVA_LINEA;
            break;

        case BUSQUEDA_NUEVA_LINEA:
            while (!(buf[i] == '\n' || buf[i] == '\0'))
            {
                i++;
            }
            i++;
            if (banderaLinea)
            {
                if (!buscarEnHash(&ht, nombre))
                {
                    struct Paquete p = {"", "", "", 0, "-", INSTALLED};
                    strcpy(p.nombre, nombre);
                    strcpy(p.ifecha, date);
                    agregarAlHashtable(&ht, &p);

                    iPaquetes++;
                }
                else
                {
                    struct Paquete *p1 = get(&ht, nombre);
                    if (strcmp(accion, "installed") == 0)
                    {
                        if (p1->status == REMOVED)
                        {
                            p1->status = INSTALLED;
                            strcpy(p1->rdate, "-");
                            rPaquetes--;
                        }
                    }
                    else if (strcmp(accion, "removed") == 0)
                    {
                        if (p1->status == INSTALLED || p1->status == UPGRADED)
                        {
                            p1->status = REMOVED;
                            strcpy(p1->rdate, date);
                            strcpy(p1->lupdate, date);
                            p1->actualizaciones = p1->actualizaciones + 1;
                            rPaquetes++;
                        }
                    }
                    else if (strcmp(accion, "upgraded") == 0)
                    {
                        if (p1->status == INSTALLED)
                        {
                            p1->status = UPGRADED;
                            strcpy(p1->lupdate, date);
                            p1->actualizaciones = p1->actualizaciones + 1;
                            uPaquetes++;
                        }
                        else if (p1->status == UPGRADED)
                        {
                            strcpy(p1->lupdate, date);
                            p1->actualizaciones = p1->actualizaciones + 1;
                        }
                    }
                }
            }
            banderaLinea = false;
            estado = FECHA_DE_BUSQUEDA;
            if (i >= tamaño - 1)
            {
                i = i + 1;
            }
            break;
        }
    }
    cInstalados = iPaquetes - rPaquetes;
    char reportS[100000];
    generarReporte(reportS, iPaquetes, rPaquetes, uPaquetes, cInstalados, &ht);
    fd = open(report, O_CREAT | O_WRONLY, 0600);
    if (fd == -1)
    {
        printf("Failed to open the file.\n");
        return;
    }
    write(fd, reportS, strlen(reportS));
    close(fd);

    printf("Reporte generado en: [%s]\n", report);
}

int codigoHash(char s[])
{
    int n = strlen(s);
    int hashValue = 0;

    for (int i = 0; i < n; i++)
    {
        hashValue = hashValue * 31 + s[i];
    }

    hashValue = hashValue & 0x7fffffff;
    return hashValue;
}

void agregarAlHashtable(struct Hashtable *ht, struct Paquete *p)
{
    for (int i = 0; i < ht->nelements + 1; i++)
    {
        int hashValue = codigoHash(p->nombre) + i;
        int index = hashValue % ht->tamaño;
        if (strcmp(ht->array[index].nombre, "") == 0)
        {
            ht->array[index] = *p;
            break;
        }
    }
    ht->nelements += 1;
}

bool esAccion(char c1, char c2, char c3)
{
    if (c1 == 'i' && c2 == 'n' && c3 == 's')
    {
        return true;
    }
    else if (c1 == 'u')
    {
        return true;
    }
    else if (c1 == 'r' && c2 == 'e')
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool buscarEnHash(struct Hashtable *ht, char key[])
{
    for (int i = 0; i < ht->nelements + 1; i++)
    {
        int hashValue = codigoHash(key) + i;
        int index = hashValue % ht->tamaño;
        if (strcmp(ht->array[index].nombre, key) == 0)
        {
            return true;
        }
        else if (strcmp(ht->array[index].nombre, "") == 0)
        {
            return false;
        }
    }
    return false;
}

struct Paquete *get(struct Hashtable *ht, char key[])
{
    for (int i = 0; i < ht->nelements + 1; i++)
    {
        int hashValue = codigoHash(key) + i;
        int index = hashValue % ht->tamaño;
        if (strcmp(ht->array[index].nombre, key) == 0)
        {
            return &ht->array[index];
        }
        else if (strcmp(ht->array[index].nombre, "") == 0)
        {
            return NULL;
        }
    }
    return NULL;
}

void printHashtable(struct Hashtable *ht)
{
    printf("ht.tamaño: %d\n", ht->tamaño);
    printf("ht.nelements: %d\n", ht->nelements);
    printf("ht.array: \n");
    for (int i = 0; i < ht->tamaño; i++)
    {
        if (strcmp(ht->array[i].nombre, "") != 0)
        {
            printPaquete(&ht->array[i]);
            printf("\n");
        }
    }
}

void htToString(char string[], struct Hashtable *ht)
{
    for (int i = 0; i < ht->tamaño; i++)
    {
        if (strcmp(ht->array[i].nombre, "") != 0)
        {
            pToString(string, &ht->array[i]);
            strcat(string, "\n\n");
        }
    }
}

void printPaquete(struct Paquete *p)
{
    printf("- Paquete nombre        : %s\n", p->nombre);
    printf("  - Install date      : %s\n", p->ifecha);
    printf("  - Last update date  : %s\n", p->lupdate);
    printf("  - Numero de actualizaciones  : %d\n", p->actualizaciones);
    printf("  - Removal date      : %s\n", p->rdate);
}

void pToString(char string[], struct Paquete *p)
{
    strcat(string, "- Nombre        : ");
    strcat(string, p->nombre);
    strcat(string, "\n");
    strcat(string, "  - Fecha de instalacion      : ");
    strcat(string, p->ifecha);
    strcat(string, "\n");
    strcat(string, "  - Last update date  : ");
    strcat(string, p->lupdate);
    strcat(string, "\n");
    strcat(string, "  - Cuantas actualizaciones  : ");
    char numBuf[20];
    sprintf(numBuf, "%d\n", p->actualizaciones);
    strcat(string, numBuf);
    strcat(string, "  - Eliminado en      : ");
    strcat(string, p->rdate);
}

void generarReporte(char *reportS, int iPaquetes, int rPaquetes, int uPaquetes, int cInstalados, struct Hashtable *ht)
{
    strcat(reportS, "Reporte de Paquetes\n");
    strcat(reportS, "----------------------\n");
    char numBuf[120];
    sprintf(numBuf, "- Paquetes Instalados : %d\n- Paquetes Eliminados   : %d\n- Paquetes Mejorados  : %d\n- Actualmente instalados  : %d\n\n", iPaquetes, rPaquetes, uPaquetes, cInstalados);
    strcat(reportS, numBuf);
    htToString(reportS, ht);
}