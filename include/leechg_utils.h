#ifndef _LEECHG_UTILS_H
#define _LEECHG_UTILS_H

struct Gem_O {
	int grade;			//using short does NOT improve time/memory usage
	double leech;		//using float does NOT improve time/memory usage
	struct Gem_O* father;
	struct Gem_O* mother;
};

inline int int_max(int a, int b)
{
	if (a > b) return a;
	else return b;
}

inline int gem_better(gem gem1, gem gem2)
{
	return gem1.leech>gem2.leech;
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.88*p_gem1->leech + 0.5*p_gem2->leech;
	else p_gem_combined->leech = 0.88*p_gem2->leech + 0.5*p_gem1->leech;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)		//bigger is always gem1
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
	}
}

void gem_init(gem *p_gem, int grd, double leech)
{
	p_gem->grade=grd;
	p_gem->leech=leech;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

inline double gem_power(gem gem1) {
	return gem1.leech;
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

gem gem_putred(gem* pool, int pool_length, int value, gem* red, gem** gem_array)
{
	int isRed;
	int last;
	int curr;
	double best_pow=0;
	gem_init(red,1,0);
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
			double new_pow=gem_power(*gp);
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

#endif // _LEECHG_UTILS_H
