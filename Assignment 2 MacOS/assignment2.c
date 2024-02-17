
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "log.h"



#define FOUR_BIT_MASK 0xF

void print_hex(void *p) {
    unsigned int x = *((unsigned int *)p);
    char hex_str[9];
    int i = 7;
    for (; i >= 0; i--) {
        unsigned int four_bits = x & FOUR_BIT_MASK;
        if (four_bits <= 9) {
            hex_str[i] = '0' + four_bits;
        } else {
            hex_str[i] = 'a' + (four_bits - 10);
        }
        x >>= 4;
    }
    hex_str[8] = '\0';
    printf("%s", hex_str);
}

long int multiply(int x, int y) {
    unsigned long int mask = 1;
    unsigned long int prod = 0;
    long int lx = (unsigned long int) x;
    long int ly = (unsigned long int) y;

    for (int i = 0; i < 32; i++) {
        if (ly & mask) {
            prod += lx;
        }
        lx <<= 1;
        mask <<= 1;
    }
    return (long int) prod;
}



#define SIGN(x) ((x >> 31) & 1)
#define EXP(x) ((x >> 23) & 0xFF)
#define FRAC(x) (x & 0x7FFFFF)
#define BIT31_MASK (1 << 31)
#define BIT24_MASK (1 << 24)

float float_subtract(float f, float g) {
  unsigned int uf = *((unsigned int *)&f);
  unsigned int ug = *((unsigned int *)&g);
  
  unsigned int sign_f = SIGN(uf);
  unsigned int sign_g = SIGN(ug);
  
  unsigned int exp_f = EXP(uf);
  unsigned int exp_g = EXP(ug);
  
  unsigned int frac_f = FRAC(uf);
  unsigned int frac_g = FRAC(ug);
  
  if (exp_f == 0 && frac_f == 0) {
    unsigned int result_neg_g = ug ^ BIT31_MASK;
    return *(float *)&result_neg_g;
  }
  
  if (exp_g == 0 && frac_g == 0)
    return f;
  
  unsigned int mantissa_f = (1 << 23) | frac_f;
  unsigned int mantissa_g = (1 << 23) | frac_g;
  
  if (exp_f < exp_g) {
    mantissa_f >>= (exp_g - exp_f);
    exp_f = exp_g;
  } else if (exp_f > exp_g) {
    mantissa_g >>= (exp_f - exp_g);
    exp_g = exp_f;
  }
  
  unsigned int exp_res = exp_f;
  unsigned int mantissa_res;
  unsigned int sign_res;
  
  if (sign_f != sign_g) {
    mantissa_res = mantissa_f + mantissa_g;
    sign_res = sign_f;
    if (mantissa_res & BIT24_MASK) {
      mantissa_res >>= 1;
      exp_res++;
    }
  } else {
    if (mantissa_f >= mantissa_g) {
      mantissa_res = mantissa_f - mantissa_g;
      sign_res = sign_f;
    } else {
      mantissa_res = mantissa_g - mantissa_f;
      sign_res = !sign_g;
    }
  
    if (mantissa_res == 0)
      return 0.0;
    
    while (!(mantissa_res & (1 << 23))) {
      mantissa_res <<= 1;
      exp_res--;
    }
  }
  
  unsigned int result = (sign_res << 31) | (exp_res << 23) | (mantissa_res & FRAC(0xFFFFFFFF));
  return *(float *)&result;
}



int main() {
    int x;
    int y;
    printf("Enter a number to print in hex > ");
    scanf("%d", &x);
    print_hex(&x);
    printf("\n");
    printf("Checking, answer should be: %x\n", x);

    printf("Enter two integers (to multiply) > ");
    scanf("%d %d", &x, &y);
    printf("%d * %d = %ld\n", x, y, multiply(x, y));
    printf("Checking, answer should be %ld\n", ((long)x) * ((long)y));

    float f, g;
    printf("Enter two floating point numbers for the subtraction > ");
    scanf("%f", &f);
    scanf("%f", &g);
    printf("Computed %f - %f = %f\n", f, g, float_subtract(f, g));
    printf("Checking, answer should be close to %f\n", f - g);

    unsigned long n; 
    printf("Enter a non-negative integer n to compute the log of >");
    scanf("%lu", &n);
    unsigned long res = log_2(n);
    if ((long) res == -1)
      printf("Error: log(0) is undefined\n");
    else {
      printf("Log(%lu) is %lu\n", n, res);
      printf("Checking, answer should be %lu\n", (unsigned long) log2((double) n));
    }
    
}

