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
#include <sys/mman.h>
#include <string.h>

#define MBR_SIZE 512 //particiones 512 bytes
#define numPartitions 4 //cuatro particiones

// Donde empieza y acaba el MBR
#define MBRstart 0x1BE
#define MBRend 0x1FE

//Tamaño de particion/sector es 512 bytes
static uint8_t mbr[512]; //El sector debe ser de ese tamaño

// Typedefs Tipo de Variables NTFS
typedef unsigned char BYTE;
typedef unsigned short int WORD;
typedef unsigned int DWORD;
typedef long long LONGLONG;

struct DatosParticion{
	WORD *BytesPerSector;
	BYTE *SectorsPerCluster;
	BYTE *MediaDescriptor;
	WORD *SectosPerTrack;
	WORD *NumberOfHeads;
	DWORD *HiddenSectors;
	LONGLONG *TotalSectors;
	LONGLONG *DMFT;
	LONGLONG *DMMFT;
	DWORD *ClustersPerFRS;
	BYTE *ClustersPerIB;
};

struct NTFS_MFT_FILE
 {
   	char		szSignature[4];		// Signature "FILE"
   	WORD		wFixupOffset;		// offset to fixup pattern
   	WORD		wFixupSize;			// Size of fixup-list +1
   	LONGLONG	n64LogSeqNumber;	// log file seq number
   	WORD		wSequence;			// sequence nr in MFT
   	WORD		wHardLinks;			// Hard-link count
   	WORD		wAttribOffset;		// Offset to seq of Attributes
   	WORD		wFlags;				// 0x01 = NonRes; 0x02 = Dir
   	DWORD		dwRecLength;		// Real size of the record
   	DWORD		dwAllLength;		// Allocated size of the record
  	LONGLONG	n64BaseMftRec;		// ptr to base MFT rec or 0
   	WORD		wNextAttrID;		// Minimum Identificator +1
   	WORD		wFixupPattern;		// Current fixup pattern
   	DWORD		dwMFTRecNumber;		// Number of this MFT Record
   								// followed by resident and∫  								// part of non-res attributes
 };

 typedef struct	// if resident then + RESIDENT
   {					//  else + NONRESIDENT
   	DWORD	dwType;
   	DWORD	dwFullLength;
   	BYTE	uchNonResFlag;
   	BYTE	uchNameLength;
   	WORD	wNameOffset;
   	WORD	wFlags;
   	WORD	wID;
   
   	union ATTR
   	{
   		struct RESIDENT
   		{
   			DWORD	dwLength;
   			WORD	wAttrOffset;
   			BYTE	uchIndexedTag;
   			BYTE	uchPadding;
   		} Resident;
   
   		struct NONRESIDENT
   		{
   			LONGLONG	n64StartVCN;
   			LONGLONG	n64EndVCN;
   			WORD		wDatarunOffset;
   			WORD		wCompressionSize; // compression unit size
   			BYTE		uchPadding[4];
   			LONGLONG	n64AllocSize;
   			LONGLONG	n64RealSize;
   			LONGLONG	n64StreamSize;
   			// data runs...
   		}NonResident;
   	}Attr;
 } NTFS_ATTRIBUTE;

void valoresCHS(uint32_t);
void valorSector(uint32_t);
void verParticion(unsigned int c,unsigned int h,unsigned int s,unsigned int sector, uint32_t Partition);
void verArchivos(int initParticion);

void cursesInit();
unsigned long long int leerMap(uint8_t Bytes, int Direccion);
char *mapFile(char *filePath);

/* Variable global para mejor legibilidad */
int fd; // Archivo a leer
char *map;
struct DatosParticion DP;
struct Attr *ntfsAttr;
struct NTFS_MFT_FILE ntfsMFT;

int main(int argc, char **argv){
	int x=0, y=1, j=0, z=0, Sam=20;
	unsigned int c, h, s, sector;
	int Caracter=0;
	uint32_t Partition;

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
	map = mapFile(argv[1]);
    if (map == NULL) {
      exit(EXIT_FAILURE);
    }
	cursesInit();
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
			Partition = MBRstart + i * 0x10;
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
				verParticion(c, h, s, sector, Partition);
				verArchivos(sector*512);

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

void verParticion(unsigned int c,unsigned int h,unsigned int s,unsigned int sector, uint32_t Partition){
	clear();
	int initParticion = sector*512;
	unsigned int sectoresTotales = map[initParticion+0x28];
	uint8_t MedioF = map[initParticion+0x15];

	uint32_t bytesSector = map[initParticion+0x0B];
	printw("\nInformación de Disco\n");
	printw("ID : %c%c%c%c\n\n", map[initParticion+0x03], map[initParticion+0x04], map[initParticion+0x05], map[initParticion+0x06]);

	DP.BytesPerSector=(WORD*)&map[initParticion+0x0B];
	printw("Tamaño sector (bytes): %ld\n\n", *DP.BytesPerSector);

	DP.SectorsPerCluster=(BYTE*)&map[initParticion+0x0D];
	printw("Número de sectores por cluster: %ld\n\n", *DP.SectorsPerCluster);

	DP.MediaDescriptor=(BYTE*)&map[initParticion+0x15];
	if(*DP.MediaDescriptor==248){
		printw("Medio fijo  (HD)\n\n");
	}else{
		printw("Medio fijo  (High Density Flop)\n\n");
	}

	DP.SectosPerTrack=(WORD*)&map[initParticion+0x18];
	printw("Sectores por pista: %ld\n\n", *DP.SectosPerTrack);


	DP.NumberOfHeads = (WORD*) &map[initParticion+0x1a];
	printw("Numero de cabezas %d\n\n", *DP.NumberOfHeads);

	DP.HiddenSectors = (DWORD *) &map[initParticion+0x1c];
	printw("Sectores ocultos: %d\n\n", *DP.HiddenSectors);

	DP.TotalSectors = (LONGLONG*) &map[initParticion+0x28];
	printw("Total de sectores: %ld\n\n", *DP.TotalSectors);

	DP.DMFT = (LONGLONG*) &map[initParticion+0x30];
	printw("Direccion MFT: %ld\n\n", *DP.DMFT);

	DP.DMMFT = (LONGLONG*) &map[initParticion+0x38];
	printw("Direccion espejo MFT: %ld\n\n", *DP.DMMFT);

	DP.ClustersPerFRS = (DWORD*) &map[initParticion+0x40];
	printw("Clusters per file record segment: %d\n\n", *DP.ClustersPerFRS);

	DP.ClustersPerIB = (BYTE*) &map[initParticion+0x44];
	printw("Clusters per index buffer: %d\n\n", *DP.ClustersPerIB);
	/*
	printw("Número de cabezas: %u\n\n", (uint32_t)leerMap(2, initParticion+0x1A));
	printw("Sectores ocultos: %u\n\n", (uint64_t)leerMap(4, initParticion+0x1C));
	printw("Total sectores: %llu\n\n", (unsigned long long int)leerMap(8, initParticion+0x28));
	printw("Dirección MFT: %llu\t\t\t", (unsigned long long int)leerMap(8, initParticion+0x30));
	printw("Dirección espejo MFT: %llu\n\n", (unsigned long long int)leerMap(8, initParticion+0x38));
	printw("Clusters per File Record Segment: %u\t\t", (uint8_t)leerMap(1, initParticion+0x40));
	printw("Clusters per Index Buffer: %u\n\n", (uint8_t)leerMap(1, initParticion+0x44));
	*/

	getch();
	refresh();
}

char *mapFile(char *filePath) {
    /* Abre archivo */
    fd = open(filePath, O_RDWR);
    if (fd == -1) {
    	perror("Error abriendo el archivo");
	    return(NULL);
    }

    /* Mapea archivo */
    struct stat st;
    fstat(fd,&st);
    int fs = st.st_size;

    char *map = mmap(0, fs, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
    	close(fd);
	    perror("Error mapeando el archivo");
	    return(NULL);
    }

  return map;
}

unsigned long long int leerMap(uint8_t Bytes, int Direccion){
	char arreglo[50], final[50], temp[50];
	unsigned long long int cosa=0;
	final[0]=0;
	/*for(int i=0; i<Bytes; i++){
		cosa=cosa+map[Direccion+i]*(Bytes-i-1)*16;
		printw("\n%x\n", map[Direccion+i]);
	}
	printw("\n%x\n", cosa);*/
	for (int i=0; i<50; i++){
		temp[i]=0;
		arreglo[i]=0;
		final[i]=0;
	}
	for(int i=0; i<Bytes; i++){
		arreglo[i]=map[Direccion+i];
	}
	//printw("\n%x\n",(unsigned)map[Direccion]);
	arreglo[Bytes]=0;
		//printw("\n%u\n", arreglo[0]);
	for(int i=0; i<Bytes; i++){
		
		sprintf(temp, "%llu", arreglo[i]);
		strcat(final, temp);
		//printw("%lu\n", arreglo[i]);

	}
	sscanf(final, "%llu", &cosa);
	/*if(Bytes==8){
		printw("\n%llx\n", cosa);
	}*/
	return cosa;
}

void verArchivos(int initParticion){
	int Caracter = 0, i=0;
	clear();
	printw("\n\nNombre\tTipo\tResidente\tTamaño\n\n");

	do
	{
		memcpy((void*)&ntfsAttr,&m_pMFTRecord[0x100000+0x4000],sizeof(NTFS_ATTRIBUTE));
		switch(ntfsAttr.dwType){

		}

		i++;
	} while (ntfsAttr.dwFullLength);
	
	Caracter = getch();
	/*do{
		clear();
		printw("\n\nNombre\tTipo\tResidente\tTamaño\n\n");
		printw("%c\n", map[initParticion, *DP.DMFT+15]);

		Caracter = getch();
	}while(Caracter!='q');*/∫
}