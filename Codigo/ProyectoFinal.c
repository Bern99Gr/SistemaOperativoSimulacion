//
// ProyectoFinal.c
//
// Creado por: Mauricio, Bernardo y Samantha.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <curses.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#define MBR_SIZE 512 //particiones 512 bytes
#define numPartitions 4 //cuatro particiones

// Donde empieza y acaba el MBR
#define MBRstart 0x1BE
#define MBRend 0x1FE

//Tamaño de particion/sector es 512 bytes
static uint8_t mbr[512]; //El sector debe ser de ese tamaño

void valoresCHS(uint32_t);
void valorSector(uint32_t);
void verParticion(unsigned int c,unsigned int h,unsigned int s,unsigned int sector);
void cursesInit();

int main(int argc, char **argv){
	int x=0, y=1, j=0, z=0, Sam=20;
	unsigned int c, h, s, sector;
	int Caracter=0;
	cursesInit();
		if (argc < 2) {
			printw("usage: %s (filename)\n", argv[0]);
			exit(0);
		}

		// open & read primer particion en mbr 
		FILE *file = fopen(argv[1], "r");
		if (!file) {
			printw("No se pudo abrir %s.\n", argv[1]);
			exit(0);
		}
		fread(&mbr, MBR_SIZE, 1, file);

		
	do{
		clear();
		//Checar que tenga MBR ROOT signature
		if (mbr[MBRend + 1] == 0xaa && mbr[MBRend] == 0x55)
			printw("MBR");
		else {
			printw("No hay MBR ROOT signature");
			exit(0);
		}

		//Ya imprimimos
		printw("\t\tC\tH\tS\tSector\n");

		//Leyendo las particiones una por una
		for (uint8_t i = 0; i < numPartitions; i++) 
		{
			uint32_t Partition = MBRstart + i * 0x10;
			valoresCHS(Partition + 0x1);
			valorSector(Partition + 0x8);
		}
		move(y,x);
		refresh();
		Caracter = getch();
		switch(Caracter){
			char temp[300];
			case KEY_DOWN:
				if(y<4){
					y++;
				}else{
					y=1;
				}
				break;
			case KEY_UP:
				if(y>1){
					y--;
				}else{
					y=4;
				}
				break;
			case 10:
				winstr(stdscr, temp);
				sscanf(temp, "\t\t%u \t%u \t%u\t%u \n", &c, &h, &s, &sector);
				verParticion(c, h, s, sector);

			break;
		}
		clear();
		refresh();
	}while(Caracter!='q');
	
	 //End window.
    endwin();
}

//Valores CHS
void valoresCHS(uint32_t Partition) {
	char s_bin[20];
	uint8_t h, s, so;
	uint16_t c;
	h = mbr[Partition];
	so = mbr[Partition + 0x1];
	/*sprintf(s_bin, "%b", s);
	move(5, 0);
	printw("%s\n", s_bin);*/
	s = so&0x3f;
	c = mbr[Partition + 0x2];
	// Opcion 1
	c = c + (so&0xc0)*4;
	// Opcion 2
	
	//c = c + so - s;
	printw("\t\t%u \t%u \t%u", c, h, s);
	//printw("    \t%u", so);
}


//Sectores
void valorSector(uint32_t Partition) {
	uint8_t *inicio = &mbr[Partition];
	uint32_t sector = *(uint32_t *)inicio;
	if(sector == 0)
	{
		printw("\tNo hay particion\n");
	}
	else
	{
		printw("\t%u \n", sector);
	}
}

void cursesInit(){
	 //We initialize the curses screen
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    refresh();
}

void verParticion(unsigned int c,unsigned int h,unsigned int s,unsigned int sector){
	clear();







	getch();
	refresh();
}