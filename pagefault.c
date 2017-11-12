#include <stdio.h>
#include <stdlib.h>
#include "mmu.h"

#define RESIDENTSETSIZE 3

extern char *base;
extern int framesbegin;
extern int idproc;
extern int systemframetablesize;
extern int ptlr;

extern struct SYSTEMFRAMETABLE *systemframetable;
extern struct PROCESSPAGETABLE *ptbr;
extern struct PROCESSPAGETABLE *gprocesspagetable;

int getfreeframe();

void printPageTable()
{
    printf("---------Inicio--------\n");
    printf("%X\n", ptbr);
    for(int i = 0; i < ptlr; i++)
    {
        printf("Frame %d\n", (ptbr+i)->framenumber);
    }
    printf("--------Fin---------\n");
}

//Obtiene la dirección del marco que contiene la página que tiene más tiempo sin ser usada
int getLeastUsed()
{
    long time = 0;
    int frame = 0;

    for(int i = 0; i < ptlr; i++)
    {
        //printf("%ld\n", (ptbr + i)->tlastaccess);
        if(((ptbr + i)->tlastaccess) > time && (ptbr + i)->presente == 1)   
        {           
            time = (ptbr + i)->tlastaccess;
            frame = (ptbr + i)->framenumber;
        }
    }

    return frame;

}

// Rutina de fallos de página

int pagefault(char *vaddress)
{

    int i;
    int frame;
    long pag_a_expulsar;
    long pag_del_proceso;

    // Calcula la página del proceso
    pag_del_proceso=(long) vaddress>>12;

    // Cuenta los marcos asignados al proceso
    i=countframesassigned();

    // Busca un marco libre en el sistema
    frame=getfreeframe();


    // Aqui se hace el algoritmo de remplazamiento
    if(frame==-1 || i == RESIDENTSETSIZE)
    {
       
        printf("Último usado %d\n", getLeastUsed());
        return(-1); // Regresar indicando error de memoria insuficiente
    }


    (ptbr+pag_del_proceso)->presente=1;
    (ptbr+pag_del_proceso)->framenumber=frame;

    //printPageTable();
    
    return(1); // Regresar todo bien
}


int getfreeframe()
{
    int i;
    // Busca un marco libre en el sistema
    for(i=framesbegin;i<systemframetablesize+framesbegin;i++)
        if(!systemframetable[i].assigned)
        {
            systemframetable[i].assigned=1;
            break;
        }
    if(i<systemframetablesize+framesbegin)
        systemframetable[i].assigned=1;
    else
        i=-1;
    return(i);
}




