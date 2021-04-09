//
// ProyectoFinal.c
//
// Creado por: Mauricio, Bernardo y Samantha.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MBR_SIZE 512 //particiones 512 bytes
#define numPartitions 4 //cuatro particiones

// Donde empieza y acaba el MBR
#define MBRstart 0x1BE
#define MBRend 0x1FE

//Tamaño de particion/sector es 512 bytes
static uint8_t mbr[512]; //El sector debe ser de ese tamaño

void valoresCHS(uint32_t);
void valorSector(uint32_t);

int main(int argc, char **argv){

	system("clear");
	if (argc < 2) {
		printf("usage: %s (filename)\n", argv[0]);
		exit(0);
	}

	// open & read primer particion en mbr 
	FILE *file = fopen(argv[1], "r");
	if (!file) {
		printf("No se pudo abrir %s.\n", argv[1]);
		exit(0);
	}

	fread(&mbr, MBR_SIZE, 1, file);

	
	//Checar que tenga MBR ROOT signature
	if (mbr[MBRend + 1] == 0xaa && mbr[MBRend] == 0x55)
		puts("MBR");
	else {
		puts("No hay MBR ROOT signature");
		exit(0);
	}

	//Ya imprimimos
	puts("\t\tC\tH\tS\tSector");

	//Leyendo las particiones una por una
	for (uint8_t i = 0; i < numPartitions; i++) 
	{
		uint32_t Partition = MBRstart + i * 0x10;
		valoresCHS(Partition + 0x1);
		valorSector(Partition + 0x8);
	}

	printf("\n\nPresiona [enter] para regresar al menu.\n");
    __fpurge(stdin);
    getchar();
}

//Valores CHS
void valoresCHS(uint32_t Partition) {
	uint8_t c, h, s;
	c = mbr[Partition];
	h = mbr[Partition + 0x1];
	s = mbr[Partition + 0x2];
	printf("\t\t%u \t%u \t%u", c, h, s);
}


//Sectores
void valorSector(uint32_t Partition) {
	uint8_t *inicio = &mbr[Partition];
	uint32_t sector = *(uint32_t *)inicio;
	if(sector == 0)
	{
		puts("\tNo hay particion");
	}
	else
	{
		printf("\t%u \n", sector);
	}
}

