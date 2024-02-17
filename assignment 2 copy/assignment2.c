#include <stdio.h>
#include <math.h>  

#include "log.h"

// This function prints a 32-bit word out in hexadecimal, using only
// printf's "%c" or "%d" format specifiers.
// It works by selecting (using masking and shifting) each 4-bit group of bits
// in the word and then printing the value of that 4-bit group as a hex
// digit (between 0 and F, inclusive).

// complete this constant definition for the mask
#define FOUR_BIT_MASK 0xF
// It takes as a parameter a pointer p of type void *, so that p can be
// a pointer to anything (i.e. you can pass in the address of anything).

void print_hex(void *p)
{
  // copy the value that p points to into an unsigned integer variable.
  unsigned int x = *((unsigned int *) p);

  // In a loop, select four bits at a time using a mask.  Then, print
  // the value of that four-bit group using a single hex digit.

  for(int i = 28; i >= 0; i -= 4) {
        // Shift the 4 bits we are interested in to the least significant position, then mask off the rest
        unsigned int fourBits = (x >> i) & FOUR_BIT_MASK;

        // Determine if it is a digit or a letter in hexadecimal, and print accordingly
        if(fourBits <= 9) {
            printf("%d", fourBits);
        }
        else {
            printf("%c", 'a' + (fourBits - 10));
        }
    }
    printf("\n");
  
  // IMPORTANT: Do NOT use a bunch of "if" statements to map the
  // bits to a hex digit. Either use the value of the bits to
  // index into an array of characters or the following method:
  //   - if the value of the bits is between 0 and 9, just print the value

  // It's up to you whether to print leading zeros or not. Either is fine.

} // end of print_hex


// This function multiplies two 32-bit signed integers, only using
// shifting and addition.  The result is a 64-bit signed integer.

long int multiply(int x, int y)
{
  // Need variables to store a mask and a result. These should
  // be 64-bit unsigned numbers.

  unsigned long int mask = 1;
  unsigned long int prod = 0;

  // Need to sign-extend x and y to 64 bits

  long int lx = (long int)x;
  long int ly = (long int)y;

  // In a loop that uses a mask to iterate over the bits
  // of ly from right (bit 0) to left (bit 63):
  //   -- if the current bit of ly is 1, then add lx to the result.
  //   -- shift lx to the left by 1 (every time)

    for(int m = 0; m < 64; m++) {
      if((ly >> m) & mask) {
        prod = prod + lx;
      }
      lx = (lx << 1);
    }

    // Return the product, but as a signed long integer.
    return (long int)prod;
}


// Defining as macros the expressions for extracting the
// sign, exponent, and fraction fields of a 32-bit
// floating point number.

// select bit 31, shifted all the way to the right.

#define SIGN(x) ((x>>31) & 1)

// select bits 23 through 30, shifted right by 23 bits

#define EXP(x) ((x>>23) & 0xFF)

// select bits 0 through 22 (the rightmost 23 bits)

#define FRAC(x) (x & 0x7FFFFF)

// This mask puts a 1 in the sign position(bit 31)
// and a zero in all other bits. It can be used
// with ^ (XOR) to flip the sign of a number

#define BIT31_MASK (0x80000000)

// This mask puts a 1 in the bit 24 position,
// to be used to check overflow of the mantissa

#define BIT24_MASK (0x01000000)

// This function performs a floating point subtraction without
// using the built-in floating point subtraction -- instead only
// using integer addition and subtraction. It does so
// by extracting on the sign, exponent, and fraction of the
// operands and performing operations using those to
// compute the sign, exponent, and operand of the result.

// NOTE: Do NOT implement this by simply negating g and
// performing addition. Just follow the instructions below.

float float_subtract(float f, float g)
{

  // We need to treat the values stored in f and g
  // as 32-bit unsigned numbers. See the hint sheet
  // for ways to do that.  We then need to extract the
  // sign, exponent, and fraction fields from f
  // and g, using the SIGN, EXP, and FRAC macros
  // above.

  unsigned int sign_f = SIGN(*((unsigned int *) &f)); 
  unsigned int sign_g = SIGN(*((unsigned int *) &g));

  
  unsigned int exp_f = EXP(*((unsigned int *) &f));
  unsigned int exp_g = EXP(*((unsigned int *) &g));

  unsigned int frac_f = FRAC(*((unsigned int *) &f));
  unsigned int frac_g = FRAC(*((unsigned int *) &g));

  // Handle the special case where f is zero (i.e.
  // both the exp_f and frac_f are zero), 
  // in which case the result is the negative of 
  // g, which is computed by just flipping the sign bit
  // using ^ (XOR).
  
  if((exp_f == 0) && (frac_f == 0)) {
    return ((*((unsigned int *) &g)) ^ 0x80000000);
  }

  // Do the same for g (i.e. check if g is zero), in
  // which case return f.
  
  if ((exp_g == 0) && (frac_g == 0))
    return f;

  // In order to perform the subtraction, the implicit
  // leading 1 in the mantissa for f and g must be made
  // explicit. That is, the mantissa for f should contain
  // a 1 in the bit 23 position, followed by the bits of frac_f.
  // The same is true for the mantissa of g.

  unsigned int mantissa_f = frac_f | 0x00800000;
  unsigned int mantissa_g = frac_g | 0x00800000;

  // Before performing the subtraction, the two numbers must have the
  // same exponent. Take the mantissa of the number with the smaller
  // exponent, and shift that mantissa right by the difference in the
  // exponents, and set the smaller exponent to the larger exponent.
  // For example, if f has a smaller exponent than g, shift mantissa_f
  // the right by (exp_g - exp_f) bits and set exp_f to exp_g.

   if(exp_f < exp_g) {
    mantissa_f = (mantissa_f>>(exp_g - exp_f));
    exp_f = exp_g;
   }
   else {
    mantissa_g = (mantissa_g>>(exp_f - exp_g));
    exp_g = exp_f;
   }

  // Now it's time to compute the exponent, sign, and
  // mantissa of the result. 

  // The exponent of the result (before normalization) is
  // now the same as exponent_f (which is the same as
  // exponent_g).

  unsigned int exp_res = exp_f;
  
  unsigned int mantissa_res;

  unsigned int sign_res;
  
  // If  sign_f and sign_g are different, i.e. f is positive
  // and g is negative or f is negative and g is positive,
  // then:
  //  --  the sign of the result will be the same as the sign of f.
  //  --  the mantissa of the result will be the sum of mantissa_f
  //      and mantissa_g
  //  --  since the sum of the two mantissas may cause a carry into
  //      bit 24 of the result, the result may need to be renormalized.
  //      That is, if bit 24 of the result mantissa is 1, then the
  //      result mantissa should be shifted to the right by 1 and the
  //      exponent of the result should be incremented by 1.

  if (sign_f != sign_g) {

    sign_res = sign_f;
    mantissa_res = mantissa_f + mantissa_g;
    if(mantissa_res & BIT24_MASK) {
      mantissa_res = mantissa_res>>1;
      exp_res++;
    }
  }
  else {

    // Otherwise, namely if sign_f and sign_g are same (i.e. either both
    // numbers were positive or both were negative), then:
    //    -- if f has the larger mantissa, the sign of the result will
    //       be the sign of f.
      if(mantissa_f > mantissa_g) {
        sign_res = sign_f;
        mantissa_res = mantissa_f - mantissa_g;
      }
      else {
        sign_res = sign_g ^ 1;
        mantissa_res = mantissa_g - mantissa_f;
      }
      if(mantissa_res == 0) {
        return 0.0;
      }

    //    -- otherwise (i.e. if g has the larger mantissa), the sign of the result
    //       will be the opposite of the sign of g.
    //    -- the mantissa of the result should be the result of subtracting
    //       the smaller mantissa from the larger mantissa.
    //
    //       For example, if mantissa_f > mantissa_g, then the sign of the result is
    //       set to sign_f and the mantissa of the result is set to
    //       (mantissa_f - mantissa_g).
    //    -- If the resulting mantissa is 0, then the entire result is 0 and 
    //       the function should just return 0.0.
    //    -- Otherwise, the resulting mantissa may be small enough that it has to be
    //       renormalized to have a 1 in the bit 23 position. Do this in
    //       a loop, shifting the result mantissa to the left by 1 bit and subtracting
    //       1 from the result exponent, until the mantissa has a 1 in the
    //       bit 23 position.
    
      while(!(mantissa_res & 0x00800000)) {
        mantissa_res = mantissa_res<<1;
        exp_res--;
      }

  }
    
  // Now construct the result from OR'ing (using bitwise-or, | ) together the
  // following components of the result:
  //  -- the sign bit of the result, shifting into the sign position
  //  -- the lowest 8 bits of the exponent, shifted into exponent position
  //  -- the lowest 22 bits of the mantissa (i.e. removing the 1 in bit 23 position,
  //     since it is implicit)

  unsigned int result = (sign_res << 31) | (exp_res << 23) | (mantissa_res & FRAC(0xFFFFFFFF));
  return *(float *)&result;
}


// No code in this function should be changed. Just uncomment the appropriate
// code as you complete each of the functions above.

int main()
{
  int x;
  int y;
  printf("Enter a number to print in hex > ");
  scanf("%d", &x);
  print_hex(&x);
  printf("\n");
  printf("Checking, answer should be: %x\n", x);

  // UNCOMMENT THESE AS YOU IMPLEMENT THE REQUIRED FUNCTIONS
  
  printf("Enter two integers (to multiply) > ");
  scanf("%d %d", &x, &y);

  printf("%d * %d = %ld\n", x, y, multiply(x,y));
  printf("Checking, answer should be %ld\n", ((long) x) * ((long) y));

  float f, g;

  printf("Enter two floating point numbers for the subtraction > ");
  scanf("%f", &f);
  scanf("%f", &g);

  printf("Computed %f - %f = %f\n", f, g, float_subtract(f,g));
  printf("Checking, answer should be close to %f\n", f-g);

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