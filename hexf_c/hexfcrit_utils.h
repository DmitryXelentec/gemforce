#ifndef _HEXFCRIT_UTILS_H
#define _HEXFCRIT_UTILS_H

struct Gem_HC {
	int grade;              // short does NOT help
	float damage;           // this is MAX damage, with the rand() part neglected
	float crit;             // assumptions: crit chance capped
	float firerate;         // for hexf fr < 60
	struct Gem_HC* father;  // maximize damage*firerate*crit*firerate
	struct Gem_HC* mother;
};

int int_max(int a, int b) 
{
	if (a > b) return a;
	else return b;
}

double gem_power(gem gem1)
{
	return gem1.damage*gem1.crit*gem1.firerate;			// amp-less
}

int gem_more_powerful(gem gem1, gem gem2)
{
	return (gem_power(gem1) > gem_power(gem2));
}

void gem_print(gem* p_gem) {
	printf("Grade:\t%d\nDamage:\t%f\nCrit:\t%f\nF.rate:\t%f\nPower:\t%f\n\n", 
		p_gem->grade, p_gem->damage, p_gem->crit, p_gem->firerate, gem_power(*p_gem));
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.87*p_gem1->damage + 0.71*p_gem2->damage;
	else p_gem_combined->damage = 0.87*p_gem2->damage + 0.71*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.5*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.5*p_gem1->crit;
	if (p_gem1->firerate > p_gem2->firerate) p_gem_combined->firerate = 0.74*p_gem1->firerate + 0.44*p_gem2->firerate;
	else p_gem_combined->firerate = 0.74*p_gem2->firerate + 0.44*p_gem1->firerate;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.86*p_gem1->damage + 0.7*p_gem2->damage;
	else p_gem_combined->damage = 0.86*p_gem2->damage + 0.7*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
	if (p_gem1->firerate > p_gem2->firerate) p_gem_combined->firerate = 0.8*p_gem1->firerate + 0.25*p_gem2->firerate;
	else p_gem_combined->firerate = 0.8*p_gem2->firerate + 0.25*p_gem1->firerate;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.85*p_gem1->damage + 0.69*p_gem2->damage;
	else p_gem_combined->damage = 0.85*p_gem2->damage + 0.69*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
	if (p_gem1->firerate > p_gem2->firerate) p_gem_combined->firerate = 0.92*p_gem1->firerate + 0.09*p_gem2->firerate;
	else p_gem_combined->firerate = 0.92*p_gem2->firerate + 0.09*p_gem1->firerate; 
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
	}
	if (p_gem_combined->damage < p_gem1->damage) p_gem_combined->damage = p_gem1->damage;
	if (p_gem_combined->damage < p_gem2->damage) p_gem_combined->damage = p_gem2->damage;
	//if (p_gem_combined->firerate > 60) firerate=60;
}

void gem_init(gem *p_gem, int grd, double damage, double crit, double firerate)
{
	p_gem->grade =grd;
	p_gem->damage=damage;
	p_gem->crit  =crit;
	p_gem->firerate=firerate;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_less_equal(gem gem1, gem gem2)
{
	if ((int)(gem1.damage*ACC) != (int)(gem2.damage*ACC))
		return gem1.damage<gem2.damage;
	if ((int)(gem1.firerate*ACC) != (int)(gem2.firerate*ACC))
		return gem1.firerate<gem2.firerate;
	return gem1.crit<gem2.crit;
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
	if (len > 10)  {
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

char gem_color(gem* p_gem)
{
	if (p_gem->crit==0) return 'r';
	else return 'y';
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

double gem_cfr_power(gem gem1, double amp_damage, double amp_crit, double damage_ratio, double crit_ratio)
{
	if (gem1.crit==0) return 0;
	return gem1.damage*gem1.crit*gem1.firerate; // no amps
}

gem gem_putred(gem* pool, int pool_length, int value, gem* red, gem** gem_array, double amp_damage, double amp_crit, double dr, double cr)
{
	int isRed;
	int last;
	int curr;
	double best_pow=0;
	gem_init(red,1,0.909091,0,1.1);
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
			double new_pow=gem_cfr_power(*gp, amp_damage, amp_crit, dr, cr);
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

#endif // _HEXFCRIT_UTILS_H
