#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YW gem;
const int ACC=80;						// ACC is for z-axis sorting and for the length of the interval tree
const int ACC_TR=750;				// ACC_TR is for pbound comparisons inside tree
const double NT=0x1p+50; // 2^50
#include "wkg_utils.h"
typedef struct Gem_Y gemY;
#include "crit_utils.h"
#include "gfon.h"

double gem_amp_power(gem gem1, gemY amp1, double damage_ratio, double crit_ratio)
{
	return (gem1.damage+damage_ratio*amp1.damage)*gem1.pbound*(gem1.crit+crit_ratio*amp1.crit)*gem1.pbound;
}

void print_omnia_table(gem* gems, gemY* amps, double* powers, int len)
{
	printf("Killgem\tAmps\tPower (resc. 10q)\n");			// we'll rescale again for 10m, no need to have 10 digits
	int i;
	for (i=0;i<len;i++) printf("%d\t%d\t%.6f\n", i+1, gem_getvalue_Y(amps+i), powers[i]/1e16);
	printf("\n");
}

void worker(int len, int lenc, int output_options, char* filename, char* filenamec, char* filenameA, int Namps)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);							// if the file is not good we exit
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,1.000000,1,0);		// grade damage crit pbound
	gem_init(pool[0]+1,1,1.186168,0,1);		// BB has more dmg
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// killgem spec pool filling
	if (prevmax<len-1) {									// if the killgems are not enough
		fclose(table);
		for (i=0;i<=prevmax;++i) free(pool[i]);		// free
		printf("Gem table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}

	gem* poolf[len];
	int poolf_length[len];
	
	for (i=0;i<len;++i) {															// killgem spec compression
		int j;
		float maxcrit=0;
		gem* temp_pool=malloc(pool_length[i]*sizeof(gem));
		for (j=0; j<pool_length[i]; ++j) {			// copy gems and get maxcrit
			temp_pool[j]=pool[i][j];
			maxcrit=max(maxcrit, (pool[i]+j)->crit);
		}
		gem_sort(temp_pool,pool_length[i]);							// work starts
		int broken=0;
		int crit_cells=(int)(maxcrit*ACC)+1;		// this pool will be big from the beginning, but we avoid binary search
		int tree_length= 1 << (int)ceil(log2(crit_cells)) ;				// this is pow(2, ceil()) bitwise for speed improvement
		int* tree=malloc((tree_length+crit_cells+1)*sizeof(int));									// memory improvement, 2* is not needed
		for (j=0; j<tree_length+crit_cells+1; ++j) tree[j]=-1;										// init also tree[0], it's faster
		int index;
		for (j=pool_length[i]-1;j>=0;--j) {																				// start from large z
			gem* p_gem=temp_pool+j;
			index=(int)(p_gem->crit*ACC);																						// find its place in x
			if (tree_check_after(tree, tree_length, index, (int)(p_gem->pbound*ACC_TR))) {		// look at y
				tree_add_element(tree, tree_length, index, (int)(p_gem->pbound*ACC_TR));
			}
			else {
				p_gem->grade=0;
				broken++;
			}
		}														// all unnecessary gems broken
		free(tree);									// free
		
		poolf_length[i]=pool_length[i]-broken;
		poolf[i]=malloc(poolf_length[i]*sizeof(gem));			// pool init via broken
		index=0;
		for (j=0; j<pool_length[i]; ++j) {								// copying to subpool
			if (temp_pool[j].grade!=0) {
				poolf[i][index]=temp_pool[j];
				index++;
			}
		}
		free(temp_pool);
		if (output_options & mask_info) printf("Killgem value %d speccing compressed pool size:\t%d\n",i+1,poolf_length[i]);
	}
	printf("Gem speccing pool compression done!\n");

	FILE* tablec=file_check(filenamec);		// file is open to read
	if (tablec==NULL) exit(1);						// if the file is not good we exit
	gem** poolc=malloc(lenc*sizeof(gem*));;
	int* poolc_length=malloc(lenc*sizeof(int));
	poolc[0]=malloc(sizeof(gem));
	poolc_length[0]=1;
	gem_init(poolc[0],1,1,1,1);
	
	int prevmaxc=pool_from_table(poolc, poolc_length, lenc, tablec);		// killgem comb pool filling
	if (prevmaxc<lenc-1) {												// if the killgems are not enough
		fclose(tablec);
		for (i=0;i<=prevmaxc;++i) free(poolc[i]);		// free
		printf("Gem table stops at %d, not %d\n",prevmaxc+1,lenc);
		exit(1);
	}

	gem* poolcf;
	int poolcf_length;
	
	{															// killgem comb compression
		float maxcrit=0;
		for (i=0; i<poolc_length[lenc-1]; ++i) {		// get maxcrit;
			maxcrit=max(maxcrit, (poolc[lenc-1]+i)->crit);
		}
		gem_sort(poolc[lenc-1],poolc_length[lenc-1]);							// work starts
		int broken=0;
		int crit_cells=(int)(maxcrit*ACC)+1;			// this pool will be big from the beginning, but we avoid binary search
		int tree_length= 1 << (int)ceil(log2(crit_cells)) ;				// this is pow(2, ceil()) bitwise for speed improvement
		int* tree=malloc((tree_length+crit_cells+1)*sizeof(int));									// memory improvement, 2* is not needed
		for (i=0; i<tree_length+crit_cells+1; ++i) tree[i]=-1;										// init also tree[0], it's faster
		int index;
		for (i=poolc_length[lenc-1]-1;i>=0;--i) {																	// start from large z
			gem* p_gem=poolc[lenc-1]+i;
			index=(int)(p_gem->crit*ACC);																						// find its place in x
			if (tree_check_after(tree, tree_length, index, (int)(p_gem->pbound*ACC_TR))) {		// look at y
				tree_add_element(tree, tree_length, index, (int)(p_gem->pbound*ACC_TR));
			}
			else {
				p_gem->grade=0;
				broken++;
			}
		}														// all unnecessary gems destroyed
		free(tree);									// free
		
		poolcf_length=poolc_length[lenc-1]-broken;
		poolcf=malloc(poolcf_length*sizeof(gem));			// pool init via broken
		index=0;
		for (i=0; i<poolc_length[lenc-1]; ++i) {			// copying to subpool
			if (poolc[lenc-1][i].grade!=0) {
				poolcf[index]=poolc[lenc-1][i];
				index++;
			}
		}
	}
	printf("Killgem comb compressed pool size:\t%d\n",poolcf_length);

	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);						// if the file is not good we exit
	int lena;
	if (lenc > len) lena=lenc;						// see which is bigger between spec len and comb len
	else lena=len;												// and we'll get the amp pool till there
	gemY** poolY=malloc(lena*sizeof(gemY*));
	int* poolY_length=malloc(lena*sizeof(int));;
	poolY[0]=malloc(sizeof(gemY));
	poolY_length[0]=1;
	gem_init_Y(poolY[0],1,1,1);
	
	int prevmaxA=pool_from_table_Y(poolY, poolY_length, lena, tableA);		// amps pool filling
	if (prevmaxA<lena-1) {
		fclose(tableA);
		for (i=0;i<=prevmaxA;++i) free(poolY[i]);		// free
		printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}

	gemY** poolYf=malloc(lena*sizeof(gemY*));		// if not malloc-ed 140k is the limit
	int poolYf_length[lena];
	
	for (i=0; i<lena; ++i) {			// amps pool compression
		int j;
		gemY* temp_pool=malloc(poolY_length[i]*sizeof(gem));
		for (j=0; j<poolY_length[i]; ++j) {			// copy gems
			temp_pool[j]=poolY[i][j];
		}
		gem_sort_Y(temp_pool,poolY_length[i]);		// work starts
		int broken=0;
		float lim_crit=-1;
		for (j=poolY_length[i]-1;j>=0;--j) {
			if (temp_pool[j].crit<=lim_crit) {
				temp_pool[j].grade=0;
				broken++;
			}
			else lim_crit=temp_pool[j].crit;
		}													// all unnecessary gems marked
		poolYf_length[i]=poolY_length[i]-broken;
		poolYf[i]=malloc(poolYf_length[i]*sizeof(gemY));		// pool init via broken
		int index=0;
		for (j=0; j<poolY_length[i]; ++j) {			// copying to pool
			if (temp_pool[j].grade!=0) {
				poolYf[i][index]=temp_pool[j];
				index++;
			}
		}
		free(temp_pool);
		if (output_options & mask_info) printf("Amp value %d compressed pool size:\t%d\n", i+1, poolYf_length[i]);
	}

	gemY poolYc[poolYf_length[lenc-1]];
	int poolYc_length=poolYf_length[lenc-1];
	
	for (i=0; i<poolYf_length[lenc-1]; ++i) {		// amps fast access combining pool
		poolYc[i]=poolYf[lenc-1][i];
	}
	printf("Amp combining pool compression done!\n\n");

	int j,k,h,l,m;								// let's choose the right gem-amp combo
	gem gems[len];								// for every speccing value
	gemY amps[len];								// we'll choose the best amps
	gem gemsc[len];								// and the best NC combine
	gemY ampsc[len];							// for both
	double powers[len];
	gem_init(gems,1,1,1,0);
	gem_init_Y(amps,0,0,0);
	gem_init(gemsc,1,1,0,0);
	gem_init_Y(ampsc,0,0,0);
	powers[0]=0;
	double iloglenc=1/log(lenc);
	double crit_ratio  =Namps*0.23*TCp/TCts;
	double damage_ratio=Namps*0.28*TCp/TCtd;
	if (!(output_options & mask_quiet)) {
		printf("Killgem:\n");
		gem_print(gems);
		printf("Amplifier:\n");
		gem_print_Y(amps);
	}

	for (i=1;i<len;++i) {																		// for every gem value
		gems[i]=(gem){0};																			// we init the gems
		amps[i]=(gemY){0};																		// to extremely weak ones
		gemsc[i]=(gem){0};
		ampsc[i]=(gemY){0};
		powers[i]=0;
		int NS=i+1;
		double c0 = log(NT/(i+1))*iloglenc;										// we compute the combination number
																													// first we compare the gem alone
		for (l=0; l<poolcf_length; ++l) {											// first search in the NC gem comb pool
			double Cp = pow(poolcf[l].pbound, c0);
			double Cd = pow(poolcf[l].damage, c0);
			double Cc = pow(poolcf[l].crit, c0);
			for (k=0;k<poolf_length[i];++k) {										// and then in the the gem pool
				double Pp = pb_power(Cp * poolf[i][k].pbound);
				double Pd = Cd * poolf[i][k].damage;							// build the final gem
				double Pc = Cc * poolf[i][k].crit;
				if (Pp > 8) Pp = 7 + pow(Pp - 7, 0.7);
				double power = Pp * Pp * Pd * Pc;
				if (power > powers[i]) {													// and compare
					powers[i]=power;
					gems[i]=poolf[i][k];
					gemsc[i]=poolcf[l];
				}
			}
		}
																													// now we compare the whole setup
		for (j=0;j<i+1;++j) {																	// for every amp value from 1 to to gem_value
			NS+=Namps;																					// we get the num of gems used in speccing
			double c = log(NT/NS)*iloglenc;											// we compute the combination number
			for (l=0; l<poolcf_length; ++l) {										// then we search in the NC gem comb pool
				double Cpg = pow(poolcf[l].pbound,c);
				double Cdg = pow(poolcf[l].damage,c);
				double Ccg = pow(poolcf[l].crit  ,c);
				for (m=0;m<poolYc_length;++m) {										// and in the amp NC pool
					double Cda = damage_ratio* pow(poolYc[m].damage,c);
					double Cca = crit_ratio  * pow(poolYc[m].crit  ,c);
					for (k=0;k<poolf_length[i];++k) {								// then in the gem pool
						if (poolf[i][k].crit!=0) {										// if the gem has crit we go on
							double Pp = pb_power(Cpg * poolf[i][k].pbound);
							if (Pp > 8) Pp = 7 + pow(Pp - 7, 0.7);
							double Pp2 = Pp * Pp;
							double Pdg = Cdg * poolf[i][k].damage;
							double Pcg = Ccg * poolf[i][k].crit  ;
							for (h=0;h<poolYf_length[j];++h) {					// and in the reduced amp pool
								double Pdamage = Pdg + Cda * poolYf[j][h].damage ;
								double Pcrit   = Pcg + Cca * poolYf[j][h].crit   ;
								double power   = Pp2 * Pdamage * Pcrit ;
								if (power>powers[i]) {
									powers[i]=power;
									gems[i]=poolf[i][k];
									amps[i]=poolYf[j][h];
									gemsc[i]=poolcf[l];
									ampsc[i]=poolYc[m];
								}
							}
						}
					}
				}
			}
		}
		if (!(output_options & mask_quiet)) {
			printf("Killgem spec\n");
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_info) printf("Pool:\t%d\n",poolf_length[i]);
			gem_print(gems+i);
			printf("Amplifier spec (%dx)\n", Namps);
			printf("Value:\t%d\n",gem_getvalue_Y(amps+i));
			if (output_options & mask_info) printf("Pool:\t%d\n",poolYf_length[gem_getvalue_Y(amps+i)-1]);
			gem_print_Y(amps+i);
			printf("Killgem combine\n");
			printf("Comb:\t%d\n",lenc);
			if (output_options & mask_info) printf("Pool:\t%d\n",poolcf_length);
			gem_print(gemsc+i);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			if (output_options & mask_info) printf("Pool:\t%d\n",poolYc_length);
			gem_print_Y(ampsc+i);
			printf("Spec base power (resc.):\t%f\n", gem_amp_power(gems[i], amps[i], damage_ratio, crit_ratio));
			printf("Global power (resc. 10q):\t%f\n\n\n", powers[i]/1e16);
			fflush(stdout);								// forces buffer write, so redirection works well
		}
	}
	
	if (output_options & mask_quiet) {		// outputs last if we never seen any
		printf("Killgem spec\n");
		printf("Value:\t%d\n",len);
		gem_print(gems+len-1);
		printf("Amplifier spec (%dx)\n", Namps);
		printf("Value:\t%d\n",gem_getvalue_Y(amps+len-1));
		gem_print_Y(amps+len-1);
		printf("Killgem combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemsc+len-1);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print_Y(ampsc+len-1);
		printf("Spec base power (resc.):\t%f\n", gem_amp_power(gems[len-1], amps[len-1], damage_ratio, crit_ratio));
		printf("Global power (resc. 10q):\t%f\n\n\n", powers[len-1]/1e16);
	}

	if (output_options & mask_upto) {
		double best_pow=0;
		int best_index=0;
		for (i=0; i<len; ++i) {
			if (powers[i] > best_pow) {
				best_index=i;
				best_pow=powers[i];
			}
		}
		printf("Best setup up to %d:\n\n", len);
		printf("Killgem spec\n");
		printf("Value:\t%d\n", gem_getvalue(gems+best_index));
		gem_print(gems+best_index);
		printf("Amplifier spec (%dx)\n", Namps);
		printf("Value:\t%d\n", gem_getvalue_Y(amps+best_index));
		gem_print_Y(amps+best_index);
		printf("Killgem combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemsc+best_index);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print_Y(ampsc+best_index);
		printf("Spec base power (resc.):\t%f\n", gem_amp_power(gems[best_index], amps[best_index], damage_ratio, crit_ratio));
		printf("Global power (resc. 10q):\t%f\n\n\n", powers[best_index]/1e16);
		gems[len-1]=gems[best_index];
		amps[len-1]=amps[best_index];
		gemsc[len-1]=gemsc[best_index];
		ampsc[len-1]=ampsc[best_index];
	}

	gem* gem_array;
	gem red;
	if (output_options & mask_red) {
		if (len < 3) printf("I could not add red!\n\n");
		else {
			int value=gem_getvalue(gems+len-1);
			gems[len-1]=gem_putred(poolf[value-1], poolf_length[value-1], value, &red, &gem_array, (amps+len-1)->damage, (amps+len-1)->crit, damage_ratio, crit_ratio);
			printf("Setup with red added:\n\n");
			printf("Killgem spec\n");
			printf("Value:\t%d\n", value);		// made to work well with -u
			gem_print(gems+len-1);
			printf("Amplifier spec (%dx)\n", Namps);
			printf("Value:\t%d\n",gem_getvalue_Y(amps+len-1));
			gem_print_Y(amps+len-1);
			printf("Managem combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(gemsc+len-1);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print_Y(ampsc+len-1);
			printf("Spec base power with red:\t%f\n\n\n", gem_amp_power(gems[len-1], amps[len-1], damage_ratio, crit_ratio));
		}
	}


	if (output_options & mask_parens) {
		printf("Killgem speccing scheme:\n");
		print_parens_compressed(gems+len-1);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed_Y(amps+len-1);
		printf("\n\n");
		printf("Killgem combining scheme:\n");
		print_parens_compressed(gemsc+len-1);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_compressed_Y(ampsc+len-1);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Killgem speccing tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
		printf("Amplifier speccing tree:\n");
		print_tree_Y(amps+len-1, "");
		printf("\n");
		printf("Killgem combining tree:\n");
		print_tree(gemsc+len-1, "");
		printf("\n");
		printf("Amplifier combining tree:\n");
		print_tree_Y(ampsc+len-1, "");
		printf("\n");
	}
	if (output_options & mask_table) print_omnia_table(gems, amps, powers, len);
	
	if (output_options & mask_equations) {		// it ruins gems, must be last
		printf("Killgem speccing equations:\n");
		print_equations(gems+len-1);
		printf("\n");
		printf("Amplifier speccing equations:\n");
		print_equations_Y(amps+len-1);
		printf("\n");
		printf("Killgem combining equations:\n");
		print_equations(gemsc+len-1);
		printf("\n");
		printf("Amplifier combining equations:\n");
		print_equations_Y(ampsc+len-1);
		printf("\n");
	}
	
	fclose(table);
	fclose(tablec);
	fclose(tableA);
	for (i=0;i<len;++i) free(pool[i]);			// free gems
	for (i=0;i<len;++i) free(poolf[i]);			// free gems compressed
	for (i=0;i<lenc;++i) free(poolc[i]);		// free gems
	free(poolc);
	free(poolc_length);
	free(poolcf);
	for (i=0;i<lena;++i) free(poolY[i]);		// free amps
	for (i=0;i<lena;++i) free(poolYf[i]);		// free amps compressed
	free(poolY);
	free(poolY_length);
	free(poolYf);
	if (output_options & mask_red && len > 2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	int lenc;
	char opt;
	int Namps=6;
	int output_options=0;
	char filename[256]="";		// it should be enough
	char filenamec[256]="";		// it should be enough
	char filenameA[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"iptcequrf:N:"))!=-1) {
		switch(opt) {
			case 'i':
				output_options |= mask_info;
				break;
			case 'p':
				output_options |= mask_parens;
				break;
			case 't':
				output_options |= mask_tree;
				break;
			case 'c':
				output_options |= mask_table;
				break;
			case 'e':
				output_options |= mask_equations;
				break;
			case 'q':
				output_options |= mask_quiet;
				break;
			case 'u':
				output_options |= mask_upto;
				break;
			case 'r':
				output_options |= mask_red;
				break;
			case 'f':			// can be "filename,filenamec,filenameA", if missing default is used
				;
				char* p=optarg;
				while (*p != ',' && *p != '\0') p++;
				if (*p==',') *p='\0';			// ok, it's "f,..."
				else p--;									// not ok, it's "f" -> empty string
				char* q=p+1;
				while (*q != ',' && *q != '\0') q++;
				if (*q==',') *q='\0';			// ok, it's "...,fc,fA"
				else q--;									// not ok, it's "...,fc" -> empty string
				strcpy(filename,optarg);
				strcpy(filenamec,p+1);
				strcpy(filenameA,q+1);
				break;
			case 'N':
				Namps=atoi(optarg);
				break;
			case '?':
				return 1;
			default:
				break;
		}
	}
	if (optind+1==argc) {
		len = atoi(argv[optind]);
		lenc= 16;		// 16c as default combine
	}
	else if (optind+2==argc) {
		len = atoi(argv[optind]);
		lenc= atoi(argv[optind+1]);
	}
	else {
		printf("Unknown arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		return 1;
	}
	if (len<1 || lenc<1) {
		printf("Improper gem number\n");
		return 1;
	}
	if (filename[0]=='\0') strcpy(filename, "table_kgspec");
	if (filenamec[0]=='\0') strcpy(filenamec, "table_kgcomb");
	if (filenameA[0]=='\0') strcpy(filenameA, "table_crit");
	worker(len, lenc, output_options, filename, filenamec, filenameA, Namps);
	return 0;
}

