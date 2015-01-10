#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_HC gem;
int ACC=80;							// ACC is for z-axis sorting and for the length of the interval tree
const int ACC_TR=750;		// ACC_TR is for firerate comparisons inside tree
#include "hexfcrit_utils.h"

void worker(int len, int output_options, int size)
{
	printf("\n");
	int i;
	gem gems[len];
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(sizeof(gem));
	pool_length[0]=1;
	gem_init(pool[0],1,1,1,1);		// start gem does not matter
	gem_init(gems   ,1,1,1,1);		// grade damage crit firerate
	if (size==0) size=1000;				// reasonable comb sizing
	if (!(output_options & mask_quiet)) gem_print(gems);

	for (i=1; i<len; ++i) {
		int j,k,h,l;
		int eoc=(i+1)/2;				//end of combining
		int comb_tot=0;
		
		int grade_max=(int)(log2(i+1)+1);						// gems with max grade cannot be destroyed, so this is a max, not a sup
		gem* temp_pools[grade_max-1];								// get the temp pools for every grade
		int  temp_index[grade_max-1];								// index of work point in temp pools
		gem* subpools[grade_max-1];									// get subpools for every grade
		int  subpools_length[grade_max-1];
		for (j=0; j<grade_max-1; ++j) {							// init everything
			temp_pools[j]=malloc(size*sizeof(gem));
			temp_index[j]=0;
			subpools[j]=malloc(sizeof(gem));
			subpools_length[j]=1;
			subpools[j][0]=(gem){0};		// 0-NULL init
		}
		for (j=0;j<eoc;++j)										// combine gems and put them in temp pools
		if ((i-j)/(j+1) < 10) {								// value ratio < 10
			for (k=0; k< pool_length[j]; ++k)
			if ((pool[j]+k)->grade!=0) {				// extensive false gems check ahead
				for (h=0; h< pool_length[i-1-j]; ++h)
				if ((pool[i-1-j]+h)->grade!=0) {
					int delta=(pool[j]+k)->grade - (pool[i-1-j]+h)->grade;
					if (abs(delta)<=2) {						// grade difference <= 2
						comb_tot++;
						gem temp;
						gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
						int grd=temp.grade-2;
						temp_pools[grd][temp_index[grd]]=temp;
						temp_index[grd]++;
						if (temp_index[grd]==size) {									// let's skim a pool
							int length=size+subpools_length[grd];
							gem* temp_array=malloc(length*sizeof(gem));
							int index=0;
							float maxcrit=0;				// this will help me create the minimum tree
							for (l=0; l<temp_index[grd]; ++l) {					// copy new gems
								temp_array[index]=temp_pools[grd][l];
								maxcrit=max(maxcrit, (temp_array+index)->crit);
								index++;
							}
							temp_index[grd]=0;			// temp index reset
							for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
								temp_array[index]=subpools[grd][l];
								maxcrit=max(maxcrit, (temp_array+index)->crit);
								index++;
							}
							free(subpools[grd]);		// free
							
							gem_sort(temp_array,length);								// work starts
							int broken=0;
							int crit_cells=(int)(maxcrit*ACC)+1;		// this pool will be big from the beginning, but we avoid binary search
							int tree_length= 1 << (int)ceil(log2(crit_cells)) ;				// this is pow(2, ceil()) bitwise for speed improvement
							int* tree=malloc((tree_length+crit_cells+1)*sizeof(int));									// memory improvement, 2* is not needed
							for (l=0; l<tree_length+crit_cells+1; ++l) tree[l]=-1;										// init also tree[0], it's faster
							for (l=length-1;l>=0;--l) {																								// start from large z
								gem* p_gem=temp_array+l;
								index=(int)(p_gem->crit*ACC);																						// find its place in x
								if (tree_check_after(tree, tree_length, index, (int)(p_gem->firerate*ACC_TR))) {		// look at y
									tree_add_element(tree, tree_length, index, (int)(p_gem->firerate*ACC_TR));
								}
								else {
									p_gem->grade=0;
									broken++;
								}
							}														// all unnecessary gems destroyed
							free(tree);									// free
							
							subpools_length[grd]=length-broken;
							subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
							
							index=0;
							for (l=0; l<length; ++l) {			// copying to subpool
								if (temp_array[l].grade!=0) {
									subpools[grd][index]=temp_array[l];
									index++;
								}   
							}
							free(temp_array);			// free
						}												// rebuilt subpool[grd], work restarts
					}
				}
			}
		}
		int grd;
		for (grd=0; grd<grade_max-1; ++grd) {						// let's put remaining gems on
			if (temp_index[grd] != 0) {
				int length=temp_index[grd]+subpools_length[grd];
				gem* temp_array=malloc(length*sizeof(gem));
				int index=0;
				float maxcrit=0;				// this will help me create the minimum tree
				for (l=0; l<temp_index[grd]; ++l) {					// copy new gems
					temp_array[index]=temp_pools[grd][l];
					maxcrit=max(maxcrit, (temp_array+index)->crit);
					index++;
				}
				for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
					temp_array[index]=subpools[grd][l];
					maxcrit=max(maxcrit, (temp_array+index)->crit);
					index++;
				}
				free(subpools[grd]);		// free
				
				gem_sort(temp_array,length);								// work starts
				int broken=0;
				int crit_cells=(int)(maxcrit*ACC)+1;		// this pool will be big from the beginning, but we avoid binary search
				int tree_length= 1 << (int)ceil(log2(crit_cells)) ;				// this is pow(2, ceil()) bitwise for speed improvement
				int* tree=malloc((tree_length+crit_cells+1)*sizeof(int));										// memory improvement, 2* is not needed
					for (l=0; l<tree_length+crit_cells+1; ++l) tree[l]=-1;										// init also tree[0], it's faster
					for (l=length-1;l>=0;--l) {																								// start from large z
						gem* p_gem=temp_array+l;
						index=(int)(p_gem->crit*ACC);																						// find its place in x
						if (tree_check_after(tree, tree_length, index, (int)(p_gem->firerate*ACC_TR))) {		// look at y
							tree_add_element(tree, tree_length, index, (int)(p_gem->firerate*ACC_TR));
						}
						else {
						p_gem->grade=0;
						broken++;
					}
				}														// all unnecessary gems destroyed
				free(tree);									// free
			
				subpools_length[grd]=length-broken;
				subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
				index=0;
				for (l=0; l<length; ++l) {			// copying to subpool
					if (temp_array[l].grade!=0) {
						subpools[grd][index]=temp_array[l];
						index++;
					}   
				}
				free(temp_array);			// free
			}												// subpool[grd] is now full
		}
		pool_length[i]=0;
		for (grd=0; grd<grade_max-1; ++grd) pool_length[i]+=subpools_length[grd];
		pool[i]=malloc(pool_length[i]*sizeof(gem));
		
		int place=0;
		for (grd=0;grd<grade_max-1;++grd) {			// copying to pool
			for (j=0; j<subpools_length[grd]; ++j) {
				pool[i][place]=subpools[grd][j];
				place++;
			}   
		}
		for (grd=0;grd<grade_max-1;++grd) {			// free
			free(temp_pools[grd]);
			free(subpools[grd]);
		}
		gems[i]=pool[i][0];						// choosing gem (criteria moved to more_power def)
		for (j=1;j<pool_length[i];++j) if (gem_more_powerful(pool[i][j],gems[i])) {
			gems[i]=pool[i][j];
		}
		
		if (!(output_options & mask_quiet)) {
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_info) {
				printf("Growth:\t%f\n", log(gem_power(gems[i]))/log(i+1));
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n",pool_length[i]);
			}
			gem_print(gems+i);
			fflush(stdout);								// forces buffer write, so redirection works well
		}
	}
	
	if (output_options & mask_quiet) {		// outputs last if we never seen any
		printf("Value:\t%d\n",len);
		printf("Growth:\t%f\n", log(gem_power(gems[len-1]))/log(len));
		gem_print(gems+len-1);
	}

	if (output_options & mask_upto) {
		double best_growth=0;
		int best_index=0;
		for (i=0; i<len; ++i) {
			if (log(gem_power(gems[i]))/log(i+1) > best_growth) {
				best_index=i;
				best_growth=log(gem_power(gems[i]))/log(i+1);
			}
		}
		printf("Best gem up to %d:\n\n", len);
		printf("Value:\t%d\n",best_index+1);
		printf("Growth:\t%f\n", best_growth);
		gem_print(gems+best_index);
		gems[len-1]=gems[best_index];
	}

	gem* gem_array;
	gem red;
	if (output_options & mask_red) {
		if (len < 3) printf("I could not add red!\n\n");
		else {
			int value=gem_getvalue(gems+len-1);
			gems[len-1]=gem_putred(pool[value-1], pool_length[value-1], value, &red, &gem_array, 0, 0, 0, 0);
			printf("Gem with red added:\n\n");
			printf("Value:\t%d\n", value);		// made to work well with -u
			printf("Growth:\t%f\n", log(gem_power(gems[len-1]))/log(value));
			gem_print(gems+len-1);
		}
	}

	if (output_options & mask_parens) {
		printf("Compressed combining scheme:\n");
		print_parens_compressed(gems+len-1);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Gem tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
	}
	if (output_options & mask_table) print_table(gems, len);
	
	if (output_options & mask_equations) {		// it ruins gems, must be last
		printf("Equations:\n");
		print_equations(gems+len-1);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);		// free
	if (output_options & mask_red && len > 2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int output_options=0;
	int size=0;						// worker or user must initialize it
	
	while ((opt=getopt(argc,argv,"iptcequrs:"))!=-1) {
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
			case 's':
				size = atoi(optarg);
				break;
			case '?':
				return 1;
			default:
				break;
		}
	}
	if (optind+1==argc) {
		len = atoi(argv[optind]);
	}
	else {
		printf("Unknown arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		return 1;
	}
	if (len<1) printf("Improper gem number\n");
	else worker(len, output_options, size);
	return 0;
}

