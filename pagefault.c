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

//Obtiene la dirección del marco que contiene la página que tiene más tiempo sin ser usada
struct PROCESSPAGETABLE* getLastUsed()
{
    long time = 1<<32 - 1;
    struct PROCESSPAGETABLE* page_pointer;

    for(int i = 0; i < ptlr; i++)
    {
        if(((ptbr + i)->tlastaccess) < time && (ptbr + i)->presente == 1)   
        {           
            time = (ptbr + i)->tlastaccess;
            page_pointer = (ptbr + i);
        }
    }

    return page_pointer;

}

int get_offset(int frame)
{
    return frame - framesbegin;
}

// Writes in swap pagepagetable info from a pointer
void write_in_swap(struct PROCESSPAGETABLE * page_pointer)
{
    int offset = get_offset(page_pointer->framenumber);
    FILE * swap = fopen("swap", "r+");
    fseek(swap, offset * sizeof(struct PROCESSPAGETABLE), SEEK_SET);
    fwrite(page_pointer, sizeof(struct PROCESSPAGETABLE), 1, swap);
    fclose(swap);
}

// Check if page is any virtual frame
int is_not_in_virtual(struct PROCESSPAGETABLE * page)
{
    return page->framenumber == -1;
}

int get_free_vframe()
{
    int i;
    for(i = framesbegin + systemframetablesize - 1; i < framesbegin + systemframetablesize * 2; i++)
    {
        if (!systemframetable[i].assigned) {
            break;
        }
    }
    if (i < framesbegin + systemframetablesize * 2)
        systemframetable[i].assigned = 1;
    else
        i = -1; // This conditions will never execute
    return i;
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
        struct PROCESSPAGETABLE* last_used_page = getLastUsed();

        struct PROCESSPAGETABLE* new_page = ptbr + pag_del_proceso;

        if(is_not_in_virtual(new_page))
            (new_page)->framenumber = get_free_vframe();

        // Make not present last used page
        last_used_page->presente = 0;

        if (last_used_page->modificado)
            last_used_page->modificado = 0;

        // Swap last_used_page and new_page
        int temp_frame = last_used_page->framenumber;
        last_used_page->framenumber = new_page->framenumber;
        new_page->framenumber = temp_frame;

        // Make present new_page
        new_page->presente = 1;

        write_in_swap(last_used_page);
        write_in_swap(new_page);
        return 1;
    }


    (ptbr+pag_del_proceso)->presente=1;
    (ptbr+pag_del_proceso)->framenumber=frame;
    write_in_swap(ptbr + pag_del_proceso);

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




