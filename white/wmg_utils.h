#ifndef _WMG_UTILS_H
#define _WMG_UTILS_H

#define TC 84
const double TCt=0.5*(1+0.1*TC/3);
const double TCp=(1+0.03*TC);
#define PB 7
const double PB1=(1+0.04*PB);
const double PB7=(1+0.05*PB/3);  // in non premium I couldn't activate this

const int ACC=1000;			// accuracy for comparisons

/* Info: to go from low to high accuracy: change gem_less_equal in an exact version, and
 * if ((int)(ACC*temp_array[l].pbound)<=(int)(ACC*lim_pbound)) {
 * in
 * if (temp_array[l].pbound<=lim_pbound) {
 */

struct Gem_OW {
	short grade;
	float leech;
	float pbound;
	struct Gem_OW* father;
	struct Gem_OW* mother;
};

inline int int_max(int a, int b)
{
	if (a > b) return a;
	else return b;
}

inline double pb_power(double pbound)
{
	if (pbound==0) return 0;
	double power = 0.12 * (-1 + log(3.7 + pbound * TCt) / log(3.7) * PB1);
	//if (power > 8) power = 7 + pow(power - 7, 0.7);
	return power;
}

inline int gem_more_powerful(gem gem1, gem gem2)
{
	return (gem1.leech*pb_power(gem1.pbound) > gem2.leech*pb_power(gem2.pbound));		// optimization at MP lv infinity
}

void gem_print(gem *p_gem) {
	printf("Grade:\t%d\nLeech:\t%f\nPbound:\t%f\nPb pow:\t%f\nPower:\t%f\n\n",
		p_gem->grade, p_gem->leech, p_gem->pbound, pb_power(p_gem->pbound), p_gem->leech*pb_power(p_gem->pbound));
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.88*p_gem1->leech + 0.5*p_gem2->leech;
	else p_gem_combined->leech = 0.88*p_gem2->leech + 0.5*p_gem1->leech;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.89*p_gem1->leech + 0.44*p_gem2->leech;
	else p_gem_combined->leech = 0.89*p_gem2->leech + 0.44*p_gem1->leech;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.9*p_gem1->leech + 0.38*p_gem2->leech;
	else p_gem_combined->leech = 0.9*p_gem2->leech + 0.38*p_gem1->leech;
}

void gem_combine (gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->father=p_gem1;
	p_gem_combined->mother=p_gem2;
	int delta = p_gem1->grade - p_gem2->grade;
	switch (delta){
		case 0:
			gem_comb_eq(p_gem1, p_gem2, p_gem_combined);
			break;
		case 1:
			gem_comb_d1(p_gem1, p_gem2, p_gem_combined);
			break;
		case -1:
			gem_comb_d1(p_gem2, p_gem1, p_gem_combined);
			break;
		default:
			gem_comb_gn(p_gem1, p_gem2, p_gem_combined);
			break;
	}																								// pbound is universal
	if (p_gem1->pbound > p_gem2->pbound) p_gem_combined->pbound = 0.87*p_gem1->pbound + 0.38*p_gem2->pbound;
	else p_gem_combined->pbound = 0.87*p_gem2->pbound + 0.38*p_gem1->pbound;
}

void gem_init(gem *p_gem, int grd, double leech, double pbound)
{
	p_gem->grade=grd;
	p_gem->leech=leech;
	p_gem->pbound=pbound;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

inline int gem_less_equal(gem gem1, gem gem2)
{
	if ((int)(gem1.leech*ACC) != (int)(gem2.leech*ACC))
		return gem1.leech<gem2.leech;
	return gem1.pbound<gem2.pbound;
}

void ins_sort (gem* gems, int len)
{
	int i,j;
	gem element;
	for (i=1; i<len; i++) {
		element=gems[i];
		for (j=i; j>0 && gem_less_equal(element, gems[j-1]); j--) {
			gems[j]=gems[j-1];
		}
		gems[j]=element;
	}
}

void quick_sort (gem* gems, int len)
{
	if (len > 20)  {
		gem pivot = gems[len/2];
		gem* beg = gems;
		gem* end = gems+len-1;
		while (beg <= end) {
			while (gem_less_equal(*beg, pivot)) {
				beg++;
			}
			while (gem_less_equal(pivot,*end)) {
				end--;
			}
			if (beg <= end) {
				gem temp = *beg;
				*beg = *end;
				*end = temp;
				beg++;
				end--;
			}
		}
		if (end-gems+1 < gems-beg+len) {		// sort smaller first
			quick_sort(gems, end-gems+1);
			quick_sort(beg, gems-beg+len);
		}
		else {
			quick_sort(beg, gems-beg+len);
			quick_sort(gems, end-gems+1);
		}
	}
}

void gem_sort (gem* gems, int len)
{
	quick_sort (gems, len);		// partially sort
	ins_sort (gems, len);			// finish the nearly sorted array
}

inline double gem_power(gem gem1)
{
	return gem1.leech*pb_power(gem1.pbound);     // amp-less
}

char gem_color(gem* p_gem)
{
	if (p_gem->leech==0 && p_gem->pbound==0) return 'r';
	if (p_gem->leech==0) return 'w';
	if (p_gem->pbound==0) return 'o';
	else return 'm';
}

gem* gem_explore(gem* gemf, int* isRed, gem* pred, int last, int* curr, gem* new_array, int* new_index)
{
	if (gemf->father==NULL || *isRed) return gemf;
	if (gemf->father->father==NULL) {		// father is g1
		if (*curr < last) (*curr)++;
		else {
			(*new_index)++;
			gem* gemt=new_array+(*new_index);
			gem_combine(pred, gemf->mother, gemt);
			*isRed=1;
			return gemt;
		}
	}
	if (gemf->mother->father==NULL) {		// mother is g1
		if (*curr < last) (*curr)++;
		else {
			(*new_index)++;
			gem* gemt=new_array+(*new_index);
			gem_combine(gemf->father, pred, gemt);
			*isRed=1;
			return gemt;
		}
	}
	gem* g1= gem_explore(gemf->father, isRed, pred, last, curr, new_array, new_index);
	gem* g2= gem_explore(gemf->mother, isRed, pred, last, curr, new_array, new_index);
	if (g1==gemf->father && g2==gemf->mother) return gemf;
	
	(*new_index)++;
	gem* gemt=new_array+(*new_index);
	gem_combine(g1, g2, gemt);
	return gemt;
}

inline double gem_cfr_power(gem gem1, double amp_leech, double leech_ratio)
{
	if (gem1.leech==0) return 0;
	return (gem1.leech+leech_ratio*amp_leech)*pb_power(gem1.pbound);
}

gem gem_putred(gem* pool, int pool_length, int value, gem* red, gem** gem_array, double amp_leech, double leech_ratio)
{
	int isRed;
	int last;
	int curr;
	double best_pow=0;
	gem_init(red,1,0,0);
	gem* best_gem=NULL;
	gem* new_array;
	gem* best_array=NULL;
	int new_index;
	int i;
	for (i=0; i<pool_length; ++i) {
		gem* gemf=pool+i;
		for (last=0; last<value; last++) {
			isRed=0;
			curr=0;
			new_array=malloc(value*sizeof(gem));
			new_index=0;
			gem* gp=gem_explore(gemf, &isRed, red, last, &curr, new_array, &new_index);
			double new_pow=gem_cfr_power(*gp, amp_leech, leech_ratio);
			if (new_pow > best_pow) {
				best_pow=new_pow;
				if (best_gem!=NULL) free(best_array);
				best_gem=gp;
				best_array=new_array;
			}
			else free(new_array);
		}
	}
	(*gem_array)=best_array;
	if (best_gem==NULL) return (gem){0};
	return *best_gem;
}

#include "print_utils.h"

#endif // _	WMG_UTILS_H
