#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define FILENAME 30 //dimensiunea numelui fisierului
#define LINELENGHT 50 //dimensiunea unei linii din fisier
#define DIMENS 10 //numarul maxim de cifre al unei dimensiuni a imaginii
#define FACTNO 10 //numarul de cifre din numarul factor
#define TYPENO 3 //numarul de elemente al sirului type (e.g. 'P6\n')

//Structura specifica vectorului asociat arborelui de compresie
typedef struct QuadtreeNode
{
	unsigned char blue, green, red;
	uint32_t area;
	int32_t top_left, top_right;
	int32_t bottom_left, bottom_right;
} __attribute__((packed)) QuadtreeNode ;

//Structura matricei imagine (RGB)
typedef struct matrix
{
	unsigned char red; 
	unsigned char green; 
	unsigned char blue;
} __attribute__((packed)) Matrix;

//Structura arborelui de compresie
typedef struct tree 
{
	unsigned char blue, green, red;
	uint32_t area;
	struct tree* top_left, *top_right;
	struct tree* bottom_left, *bottom_right;
	int32_t no; //indicele nodului curent - pozitia la care se va afla in vector
	int32_t s1, s2, s3, s4; //indicele la care se vor afla in vector fiii
}*Tree;

//functia de initializare a arborilor
Tree initTree(unsigned char blue, unsigned char green, unsigned char red, uint32_t area, int32_t no) 
{
	Tree tree = (Tree) malloc(sizeof(struct tree));
	tree->blue = blue;
	tree->green = green;
	tree->red = red;
	tree->area = area;
	tree->no = no;
	tree->top_left = NULL;
	tree->top_right = NULL;
	tree->bottom_right = NULL;
	tree->bottom_left = NULL;
	tree->s1 = -1;
	tree->s2 = -1;
	tree->s3 = -1;
	tree->s4 = -1;
	return tree;
}

//functie care creeaza 4 fii pentru nodul curent din arbore, utilizand culorile calculate ca medie
void createSons(Tree tree, unsigned char blue1, unsigned char green1, unsigned char red1, unsigned char blue2, unsigned char green2, unsigned char red2, 
	unsigned char blue3, unsigned char green3, unsigned char red3, unsigned char blue4, unsigned char green4, unsigned char red4, uint32_t area, int32_t *no)
{
	Tree son1 = initTree(blue1, green1, red1, area, *no);
	tree->s1 = *no;
	(*no)++;
	Tree son2 = initTree(blue2, green2, red2, area, *no);
	tree->s2 = *no;
	(*no)++;
	Tree son3 = initTree(blue3, green3, red3, area, *no);
	tree->s3 = *no;
	(*no)++;
	Tree son4 = initTree(blue4, green4, red4, area, *no);
	tree->s4 = *no;
	tree->top_left = son1;
	tree->top_right = son2;
	tree->bottom_right = son3;
	tree->bottom_left = son4;
}

//functie care insereaza informatia din arbore in vectorul structura QuadTreeNode
void insert(Tree tree, QuadtreeNode *vect, uint32_t *leaves) 
{
	if(tree != NULL)
	{
		vect[tree->no].blue = tree->blue;
		vect[tree->no].red = tree->red;
		vect[tree->no].green = tree->green;
		vect[tree->no].area = tree->area;
		vect[tree->no].top_left = tree->s1;
		vect[tree->no].top_right = tree->s2;
		vect[tree->no].bottom_right = tree->s3;
		vect[tree->no].bottom_left = tree->s4;
		if(tree->s1 == -1)
			(*leaves)++; //momentul potrivit pentru a numara si frunzele
		insert(tree->top_left, vect, leaves);
		insert(tree->top_right, vect, leaves);
		insert(tree->bottom_right, vect, leaves);
		insert(tree->bottom_left, vect, leaves);
	}
}

//functie de eliberare a memoriei ocupate de matrice
void freeMatrix(Matrix **p, uint32_t size)
{
	int i;
    for (i = 0; i < size; i++)
        free(p[i]);
    free(p);
}

//functie de eliberare a memoriei ocupate de arbori
Tree freeTree(Tree tree)
{
	if (tree == NULL)
		return tree;
	tree->top_left = freeTree(tree->top_left);
	tree->top_right = freeTree(tree->top_right);
	tree->bottom_right = freeTree(tree->bottom_right);
	tree->bottom_left = freeTree(tree->bottom_left);
	free(tree);
	return NULL;
}

//functie principala, care calculeaza culorile medii pe matrice si apeleaza functia
//de creare a fiilor
void checkndivide(Matrix **image, Tree tree, int x, int y, int size, int prag, int32_t *no)
{
	long long int score = 0; //variabila in care calculam pragul culorii
	long long int mred = 0, mgreen = 0, mblue = 0; //culorile medii
	int i = 0, j = 0;
	for(i = x; i < x + size; i++)
		for(j = y; j < y + size; j++)
			{
				mred = mred + image[i][j].red;
				mblue = mblue + image[i][j].blue;
				mgreen = mgreen + image[i][j].green;
			}
	mred = mred / (size * size);
	mblue = mblue / (size * size);
	mgreen = mgreen / (size * size);
	//calculul propriu-zis al pragului
	for(i = x; i < x + size; i++)
		for(j = y; j < y + size; j++)
			score = score + (mred - image[i][j].red) * (mred - image[i][j].red) + 
				    (mblue - image[i][j].blue) * (mblue - image[i][j].blue) + (mgreen - image[i][j].green) * (mgreen - image[i][j].green);
	score = score / (3 * size * size);
	if(score <= prag) 
		{
			//daca exista un nod unic, permitem codificarea intregii imagini
			if((*no) == 0)
			{
				tree->red = mred;
				tree->green = mgreen;
				tree->blue = mblue;
			}
			return ;
		}
	else
	{
		//calculam culorile medii din fiecare din cele 4 colturi ale zonei curente
		long long int mred1 = 0, mgreen1 = 0, mblue1 = 0;
		long long int mred2 = 0, mgreen2 = 0, mblue2 = 0;
		long long int mred3 = 0, mgreen3 = 0, mblue3 = 0;
		long long int mred4 = 0, mgreen4 = 0, mblue4 = 0;
		for(i = x; i < x + size/2; i++)
			for(j = y; j < y + size/2; j++)
				{
					mred1 = mred1 + image[i][j].red;
					mblue1 = mblue1 + image[i][j].blue;
					mgreen1 = mgreen1 + image[i][j].green;
				}
		mred1 = mred1 / (size/2 * size/2);
		mblue1 = mblue1 / (size/2 * size/2);
		mgreen1 = mgreen1 / (size/2 * size/2);
		for(i = x; i < x + size/2; i++)
			for(j = y + size/2; j < y + size; j++)
				{
					mred2 = mred2 + image[i][j].red;
					mblue2 = mblue2 + image[i][j].blue;
					mgreen2 = mgreen2 + image[i][j].green;
				}
		mred2 = mred2 / (size/2 * size/2);
		mblue2 = mblue2 / (size/2 * size/2);
		mgreen2 = mgreen2 / (size/2 * size/2);
		for(i = x + size/2; i < x + size; i++)
			for(j = y + size/2; j < y + size; j++)
				{
					mred3 = mred3 + image[i][j].red;
					mblue3 = mblue3 + image[i][j].blue;
					mgreen3 = mgreen3 + image[i][j].green;
				}
		mred3 = mred3 / (size/2 * size/2);
		mblue3 = mblue3 / (size/2 * size/2);
		mgreen3 = mgreen3 / (size/2 * size/2);
		for(i = x + size/2; i < x + size; i++)
			for(j = y; j < y + size/2; j++)
				{
					mred4 = mred4 + image[i][j].red;
					mblue4 = mblue4 + image[i][j].blue;
					mgreen4 = mgreen4 + image[i][j].green;
				}
		mred4 = mred4 / (size/2 * size/2);
		mblue4 = mblue4 / (size/2 * size/2);
		mgreen4 = mgreen4 / (size/2 * size/2);
		(*no)++; //incrementam identificatorul de pozitie (ex.: 0 radacina -> 1 primul fiu)
		//cream cei 4 fii ai nodului curent
		createSons(tree, mblue1, mgreen1, mred1, mblue2, mgreen2, mred2, mblue3, 
			mgreen3, mred3, mblue4, mgreen4, mred4, size/2 * size/2, no);
		checkndivide(image, tree->top_left, x, y, size/2, prag, no);
		checkndivide(image, tree->top_right, x, y + size/2, size/2, prag, no);
		checkndivide(image, tree->bottom_right, x + size/2, y + size/2, size/2, prag, no);
		checkndivide(image, tree->bottom_left, x + size/2, y, size/2, prag, no);
	}
}

//functie de citire din fisierul PPM
//returneaza matricea imagine RGB
Matrix **readMatrix(char *file, char *type, int *width, int *height, int *vmax)
{
	char line[LINELENGHT], widthchar[DIMENS], heightchar[DIMENS], vmaxchar[DIMENS];
	int i, j, k;
	FILE *ppm = fopen(file, "rb");
	if(!ppm)
	{
		printf("Nu am putut deschide fisierul");
		return 0;
	}
	fread(type, sizeof(char), 3, ppm); //citim 'P6\n'
	i = 0;
	do //citim width si height
		{
			fread(&line[i], sizeof(char), 1, ppm);
			i++;
		}
	while (line[i - 1] != '\n');
	i--;
	for(j = 0; isspace(line[j]) == 0; j++)
		widthchar[j] = line[j];
	widthchar[j] = 0;
	//prelucram pentru a obtine dimensiunile in format int
	*width = atoi(widthchar);
	while(!isdigit(line[j]))
		j++;
	k = 0;
	while(j < i)
	{
		heightchar[k] = line[j];
		k++; j++;
	}
	heightchar[k] = 0;
	*height = atoi(heightchar);
	i = 0;
	do //citim valoarea maxima
		{
			fread(&line[i], sizeof(char), 1, ppm);
			i++;
		}
	while (line[i - 1] != '\n');
	i--;
	j = 0;
	while(j < i)
	{
		vmaxchar[j] = line[j];
		j++;
	}
	vmaxchar[j] = 0;
	*vmax = atoi(vmaxchar); //convertim in int

	//alocarea matricei RGB
	Matrix **p = (Matrix **) malloc (*height * sizeof(Matrix *));
	if(p != NULL)
		for(i = 0; i < *height; i++)
			p[i] = (Matrix *) calloc (*width, sizeof(Matrix));
	//citirea datelor in matrice
	for(i = 0; i < *height; i++)
		fread(p[i], sizeof(Matrix), *width, ppm);
	fclose(ppm);
	return p;
}

//functie care gestioneaza crearea vectorului QuadTreeNode
QuadtreeNode *vector(Tree tree, uint32_t size, uint32_t *leaves)
{
	if(tree == NULL) return NULL;
	QuadtreeNode *vect = (QuadtreeNode *) calloc (size, sizeof(QuadtreeNode));
	insert(tree, vect, leaves);
	return vect;
}

//functia de salvare a fisierului comprimat
int save(char *file, uint32_t size, QuadtreeNode *vect, uint32_t leaves)
{	
	FILE *dest = fopen(file, "wb");
	if(!dest)
	{
		printf("Nu am putut deschide fisierul");
		return 0;
	}
	fwrite(&leaves, sizeof(uint32_t), 1, dest);
	fwrite(&size, sizeof(uint32_t), 1, dest);
	fwrite(vect, sizeof(QuadtreeNode), size, dest);
	fclose(dest);
	return 1; //la succes, returnam 1
}

//functie de citire din fisierul comprimat
QuadtreeNode *readVector(char *file, uint32_t *leaves, uint32_t *size)
{
	FILE *comp = fopen(file, "rb");
	if(!comp)
	{
		printf("Nu am putut deschide fisierul");
		return 0;
	}
	fread(leaves, sizeof(uint32_t), 1, comp);
	fread(size, sizeof(uint32_t), 1, comp);
	QuadtreeNode *vect = (QuadtreeNode *) calloc (*size, sizeof(QuadtreeNode));
	fread(vect, sizeof(QuadtreeNode), *size, comp);
	fclose(comp);
	return vect;
}

//functie de creare a arborelui pe baza vectorului citit din fisierul comprimat
Tree compTree(QuadtreeNode *vect, int i)
{
	Tree tree = initTree(vect[i].blue, vect[i].green, vect[i].red, vect[i].area, i);
	if(vect[i].top_left != -1)
	{
		tree->s1 = vect[i].top_left;
		tree->s2 = vect[i].top_right;
		tree->s3 = vect[i].bottom_right;
		tree->s4 = vect[i].bottom_left;
		tree->top_left = compTree(vect, tree->s1);
		tree->top_right = compTree(vect, tree->s2);
		tree->bottom_right = compTree(vect, tree->s3);
		tree->bottom_left = compTree(vect, tree->s4);
	}
	return tree;
}

//functie de creare a matricii RGB pe baza arborelui
void compMatrix(Matrix **p, Tree tree, int x, int y, uint32_t size)
{
	if(tree->top_left != NULL)
	{
		compMatrix(p, tree->top_left, x, y, size/2);
		compMatrix(p, tree->top_right, x, y + size/2, size/2);
		compMatrix(p, tree->bottom_right, x + size/2, y + size/2, size/2);
		compMatrix(p, tree->bottom_left, x + size/2, y, size/2);
	}
	else
	{
		//cand ajungem la frunze, retinem culoarea pe zona respectiva
		int i, j;
		for(i = x; i < x + size; i++)
			for(j = y; j < y + size; j++)
			{
				p[i][j].red = tree->red;
				p[i][j].blue = tree->blue;
				p[i][j].green = tree->green;
			}
	}
}

//functie de salvare a fisierului PPM rezultat in urma cerintelor
//2, 3 si a bonusului
void saveComp(char *fileout, uint32_t size, Matrix **p, uint32_t maxv)
{
	FILE *dest = fopen(fileout, "wb");
	if(!dest)
		printf("Nu am putut deschide fisierul");
	else
	{
		int i;
		fprintf(dest, "P6\n%d %d\n%d\n", size, size, maxv);
		for(i = 0; i < size; i++)
			fwrite(p[i], sizeof(Matrix), size, dest);
		fclose(dest);
	}
}

//functie ce gestioneaza procesul de decompresie
void decomp(char *filein, char *fileout)
{
	int i;
	uint32_t leaves, size;
	QuadtreeNode *vect = readVector(filein, &leaves, &size);
	Tree tree = compTree(vect, 0);
	Matrix **p = (Matrix **) malloc ((uint32_t)sqrt(vect[0].area) * sizeof(Matrix *));
	if(p != NULL)
		for(i = 0; i < sqrt(vect[0].area); i++)
			p[i] = (Matrix *) calloc ((uint32_t)sqrt(vect[0].area), sizeof(Matrix));
	compMatrix(p, tree, 0, 0, (uint32_t)sqrt(vect[0].area));
	saveComp(fileout, (uint32_t)sqrt(vect[0].area), p, 255);
	//dupa scrierea in fisier, eliberam memoria pentru cele trei structuri de date
	freeTree(tree);
	freeMatrix(p, (uint32_t)sqrt(vect[0].area));
	free(&vect[0]);
}

//functia de oglindire orizontala
void hMirror(Tree tree)
{
	if(tree != NULL)
	{
		hMirror(tree->top_left);
		hMirror(tree->top_right);
		hMirror(tree->bottom_right);
		hMirror(tree->bottom_left);

		if(tree->s1 != -1)
		{
			//pana ajungem la frunze, schimbam ordinea fiilor
			Tree node;
			node = tree->top_left;
			tree->top_left = tree->top_right;
			tree->top_right = node;

			node = tree->bottom_right;
			tree->bottom_right = tree->bottom_left;
			tree->bottom_left = node;
		}
	}
}

//functia de oglindire verticala
void vMirror(Tree tree)
{
	if(tree != NULL)
	{
		vMirror(tree->top_left);
		vMirror(tree->top_right);
		vMirror(tree->bottom_right);
		vMirror(tree->bottom_left);

		if(tree->s1 != -1)
		{
			//pana ajungem la frunze, schimbam ordinea fiilo
			Tree node;
			node = tree->top_left;
			tree->top_left = tree->bottom_left;
			tree->bottom_left = node;

			node = tree->top_right;
			tree->top_right = tree->bottom_right;
			tree->bottom_right = node;
		}
	}
}

//functie de suprapunere a arborilor a doua imagini
void merge(int32_t *no, Tree tree, int x, int y, uint32_t size, Tree tree1, Tree tree2, int x1, int y1, uint32_t size1, int x2, int y2, uint32_t size2)
{
	//analizam fiecare situatie, inaintand in arborii in care nu s-a ajuns inca la frunze
	//cat timp exista frunze (fii) intr-unul dintre cei doi arbori imagine, cream si in
	//arborele final alti 4 fii
	if(tree1->s1 != -1 && tree2->s2 == -1)
	{
		//utilizam culoarea medie din cei 2 arbori initiali
		tree->blue = (uint32_t)(tree1->blue + tree2->blue)/2;
		tree->green = (uint32_t)(tree1->green + tree2->green)/2;
		tree->red = (uint32_t)(tree1->red + tree2->red)/2;
		(*no)++;
		createSons(tree, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, size/2 * size/2, no);

		merge(no, tree->top_left, x, y, size/2, tree1->top_left, tree2, x1, y1, size1/2, x2, y2, size2);
		merge(no, tree->top_right, x, y + size/2, size/2, tree1->top_right, tree2, x1, y1 + size1/2, size1/2, x2, y2, size2);
		merge(no, tree->bottom_right, x + size/2, y + size/2, size/2, tree1->bottom_right, tree2, x1 + size1/2, y1 + size1/2, size1/2, x2, y2, size2);
		merge(no, tree->bottom_left, x + size/2, y, size/2, tree1->bottom_left, tree2, x1 + size1/2, y1, size1/2, x2, y2, size2);
		
	}
	else if(tree1->s1 == -1 && tree2->s2 != -1)
	{
		tree->blue = (uint32_t)(tree1->blue + tree2->blue)/2;
		tree->green = (uint32_t)(tree1->green + tree2->green)/2;
		tree->red = (uint32_t)(tree1->red + tree2->red)/2;
		(*no)++;
		createSons(tree, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, size/2 * size/2, no);

		merge(no, tree->top_left, x, y, size/2, tree1, tree2->top_left, x1, y1, size1, x2, y2, size2/2);
		merge(no, tree->top_right, x, y + size/2, size/2, tree1, tree2->top_right, x1, y1, size1, x2, y2 + size2/2, size2/2);
		merge(no, tree->bottom_right, x + size/2, y + size/2, size/2, tree1, tree2->bottom_right, x1, y1, size1, x2 + size2/2, y2 + size2/2, size2/2);
		merge(no, tree->bottom_left, x + size/2, y, size/2, tree1, tree2->bottom_left, x1, y1, size1, x2 + size2/2, y2, size2/2);
		
	}
	else if(tree1->s1 != -1 && tree2->s2 != -1)
	{
		tree->blue = (uint32_t)(tree1->blue + tree2->blue)/2;
		tree->green = (uint32_t)(tree1->green + tree2->green)/2;
		tree->red = (uint32_t)(tree1->red + tree2->red)/2;
		(*no)++;
		createSons(tree, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, size/2 * size/2, no);

		merge(no, tree->top_left, x, y, size/2, tree1->top_left, tree2->top_left, x1, y1, size1/2, x2, y2, size2/2);
		merge(no, tree->top_right, x, y + size/2, size/2, tree1->top_right, tree2->top_right, x1, y1 + size1/2, size1/2, x2, y2 + size2/2, size2/2);
		merge(no, tree->bottom_right, x + size/2, y + size/2, size/2, tree1->bottom_right, tree2->bottom_right, x1 + size1/2, y1 + size1/2, size1/2, x2 + size2/2, y2 + size2/2, size2/2);
		merge(no, tree->bottom_left, x + size/2, y, size/2, tree1->bottom_left, tree2->bottom_left, x1 + size1/2, y1, size1/2, x2 + size2/2, y2, size2/2);
	
	}
	else
	{
		//cand nu mai exista fii, doar retinem culoarea medie in nodul
		//curent din arborele final
		tree->blue = (uint32_t)(tree1->blue + tree2->blue)/2;
		tree->green = (uint32_t)(tree1->green + tree2->green)/2;
		tree->red = (uint32_t)(tree1->red + tree2->red)/2;
	}
}

//functia principala a programului
int main(int argc, char const *argv[])
{
	//initializarea variabilelor, precum cele utilizate in dimensiunile imaginilor
	//in cele ale arborilor, ca parametrii ai functiilor (fisiere, prag) etc.
	int height = 0, width = 0, vmax = 0, prag = 0;
	uint32_t leaves = 0, size = 0;
	int32_t no = 0; //indicele de pozitie al nodurilor din arbore
	char type[TYPENO], factor[FACTNO], file[FILENAME], dest[FILENAME];
	Tree tree = NULL;

	if(strcmp("-c", argv[1]) == 0) //daca se efectueaza compresia
	{
		QuadtreeNode *vect;
		//prelucram parametrii de program
		strcpy(factor, argv[2]);
		prag = atoi(factor);
		strcpy(file, argv[3]);
		strcpy(dest, argv[4]);
		//cream matricea RGB, arborele, vectorul
		Matrix **image = readMatrix(file, type, &width, &height, &vmax);
		tree = initTree(0, 0, 0, width * height, 0);
		checkndivide(image, tree, 0, 0, height, prag, &no);
		//dimensiunea vectorului va fi ultima pozitie utilizata + 1
		size = no + 1; 
		vect = vector(tree, size, &leaves);
		if(!save(dest, size, vect, leaves))
			return 0;
		//la final, eliberam memoria utilizata
		free(vect);
		freeMatrix(image, height);
		freeTree(tree);
	}
	else if(strcmp("-d", argv[1]) == 0) //daca se efectueaza decompresia
	{
		strcpy(file, argv[2]);
		strcpy(dest, argv[3]);
		decomp(file, dest);
	}
	else if(strcmp("-m", argv[1]) == 0) //daca se efectueaza oglindirea
	{
		int i;
		char flipType[TYPENO];
		strcpy(flipType, argv[2]);
		strcpy(factor, argv[3]);
		prag = atoi(factor);
		strcpy(file, argv[4]);
		strcpy(dest, argv[5]);

		Matrix **image = readMatrix(file, type, &width, &height, &vmax);
		tree = initTree(0, 0, 0, width * height, 0);
		checkndivide(image, tree, 0, 0, height, prag, &no);
		if(strcmp("h", flipType) == 0)
			hMirror(tree);
		else vMirror(tree);
		//cream matricea RGB a imaginii prelucrate
		Matrix **p = (Matrix **) malloc (width * sizeof(Matrix *));
		if(p != NULL)
			for(i = 0; i < width; i++)
				p[i] = (Matrix *) calloc (width, sizeof(Matrix));
		compMatrix(p, tree, 0, 0, width);
		saveComp(dest, width, p, vmax);

		freeMatrix(image, width);
		freeTree(tree);
		freeMatrix(p, width);
	}
	if(strcmp("-o", argv[1]) == 0) //daca se efectueaza suprapunerea
	{
		char file1[FILENAME], file2[FILENAME];
		int32_t no1 = 0, no2 = 0;
		int i = 0;
		Tree tree1 = NULL, tree2 = NULL;
		strcpy(factor, argv[2]);
		prag = atoi(factor);
		strcpy(file1, argv[3]);
		strcpy(file2, argv[4]);
		strcpy(dest, argv[5]);

		//numarul de structuri create se dubleaza, pentru fiecare imagine
		Matrix **image1 = readMatrix(file1, type, &width, &height, &vmax);
		tree1 = initTree(0, 0, 0, width * height, 0);
		checkndivide(image1, tree1, 0, 0, height, prag, &no1);
		Matrix **image2 = readMatrix(file2, type, &width, &height, &vmax);
		tree2 = initTree(0, 0, 0, width * height, 0);
		checkndivide(image2, tree2, 0, 0, height, prag, &no2);
		tree = initTree(0, 0, 0, width * height, 0);
		merge(&no, tree, 0, 0, width, tree1, tree2, 0, 0, width, 0, 0, width);
		Matrix **p = (Matrix **) malloc (width * sizeof(Matrix *));
		if(p != NULL)
			for(i = 0; i < width; i++)
				p[i] = (Matrix *) calloc (width, sizeof(Matrix));
		compMatrix(p, tree, 0, 0, width);
		saveComp(dest, width, p, vmax);

		freeMatrix(image1, width);
		freeTree(tree1);
		freeMatrix(image2, width);
		freeTree(tree2);
		freeTree(tree);
		freeMatrix(p, width);
	}
	return 0;
}