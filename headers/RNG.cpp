/*----------------------------------------------------------------------------
*
* Author:   Liang Ma (liang-ma@polito.it)
*
* In the definition of Box_Muller(), sin/cos is implemented instead of sinf/cosf
* due to bugs in the synthesizer or the simulator in result of C/RTL co-simulation failing.
*
* The member function init_arrary() isn't defined here.
*
*----------------------------------------------------------------------------
*/

#include "RNG.h"
#define RNG_M 397

#define RNG_R 31

#define RNG_U 11
#define RNG_A 0x9908B0DF
#define RNG_D 0xFFFFFFFF
#define RNG_S 7
#define RNG_B 0x9D2C5680
#define RNG_T 15
#define RNG_C 0xEFC60000
#define RNG_L 18


#define lower_mask   0x7FFFFFFF//(1 << R) - 1
#define upper_mask   0x80000000//~lower_mask

#define PI (data_t)3.14159

#define MINI_RND (data_t)2.328306e-10

#define __LIANG__

void RNG::init(uint seed)
{
	this->index = 0;
	this->seed=seed;
	uint tmp=seed;

	loop_seed_init:for (int i = 0; i < RNG_H; i++)
	{
		mt_e[i]=tmp;
		tmp= RNG_F*(tmp^ (tmp >> (RNG_W - 2))) + i*2+1;
		mt_o[i]=tmp;
		tmp= RNG_F*(tmp^ (tmp >> (RNG_W - 2))) + i*2+2;
	}
}

void RNG::init_array(RNG* rng, uint* seed, const int size)
{
	uint tmp[size];
#pragma HLS ARRAY_PARTITION variable=tmp complete dim=1

	loop_set_seed:for(int i=0;i<size;i++)
	{
#pragma HLS UNROLL
		rng[i].index = 0;
		rng[i].seed=seed[i];
		tmp[i]=seed[i];
	}


	loop_seed_init:for (int i = 0; i < RNG_H; i++)
	{
		loop_group_init:for(int k=0;k<size;k++)
		{
#pragma HLS UNROLL
			rng[k].mt_e[i]=tmp[k];
			tmp[k]= RNG_F*(tmp[k]^ (tmp[k] >> (RNG_W - 2))) + i*2+1;
			rng[k].mt_o[i]=tmp[k];
			tmp[k]= RNG_F*(tmp[k]^ (tmp[k] >> (RNG_W - 2))) + i*2+2;
		}
	}
}


void RNG::extract_number(uint *num1, uint *num2)
{
#pragma HLS PIPELINE II=4
	int id1=increase(1), idm=increase(RNG_MH), idm1=increase(RNG_MHI);

 	uint x = this->seed,x1=this->mt_o[this->index],x2=this->mt_e[id1],
 			xm=this->mt_o[idm],xm1=this->mt_e[idm1];

	x = (x & upper_mask)+(x1 & lower_mask);
	uint xp = x >> 1;
	if ((x & 0x01) != 0)
		xp ^= RNG_A;
	x = xm ^ xp;

	uint y = x;
	y ^= ((y >> RNG_U)&RNG_D);
	y ^= ((y << RNG_S)&RNG_B);
	y ^= ((y << RNG_T)&RNG_C);
	y ^= (y >> RNG_L);
	*num1 = y;
	mt_e[this->index]=x;

	x1 =( x1 & upper_mask) + (x2 & lower_mask);
	uint xt = x1 >> 1;
	if ((x1 &0x01) != 0)
		xt ^= RNG_A;
	x1 = xm1 ^ xt;

	uint y1 = x1;
	y1 ^= ((y1 >> RNG_U)&RNG_D);
	y1 ^= ((y1 << RNG_S)&RNG_B);
	y1 ^= ((y1 << RNG_T)&RNG_C);
	y1 ^= (y1 >> RNG_L);
	*num2 = y1;
	mt_o[this->index]=x1;

	this->index=id1;
	this->seed=x2;
}


int RNG::increase(int k)
{
	int tmp= this->index+k;
	return (tmp>=RNG_H)? tmp-RNG_H:tmp;
}


void RNG::BOX_MULLER(data_t *data1, data_t *data2,data_t deviation)
{
#pragma HLS INLINE
	data_t tp,tmp1,tmp2;
	uint num1,num2;

	extract_number(&num1,&num2);
	tmp1=num1*MINI_RND;
	tmp2=num2*MINI_RND;
	tp=sqrtf(fmaxf(-2*logf(tmp1),0)*deviation);
#ifdef __LIANG__
	*data1=(data_t)cos(2*PI*(tmp2))*tp;
	*data2=(data_t)sin(2*PI*(tmp2))*tp;
#else
	*data1=cosf(2*PI*(tmp2))*tp;
	*data2=sinf(2*PI*(tmp2))*tp;
#endif
}

void RNG::BOX_MULLER(data_t *data1, data_t *data2,data_t ave, data_t deviation)
{
#pragma HLS INLINE
	data_t tp,tmp1,tmp2;
	uint num1,num2;

	extract_number(&num1,&num2);
	tmp1=num1*MINI_RND;
	tmp2=num2*MINI_RND;
	tp=sqrtf(fmaxf(-2*logf(tmp1),0)*deviation);
#ifdef __LIANG__
	*data1=(data_t)cos(2*PI*(tmp2))*tp+ave;
	*data2=(data_t)sin(2*PI*(tmp2))*tp+ave;
#else
	*data1=cosf(2*PI*(tmp2))*tp+ave;
	*data2=sinf(2*PI*(tmp2))*tp+ave;
#endif
}
