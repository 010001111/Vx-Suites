/* R_RANDOM.C - random objects for RSAREF
 */

/* Copyright (C) RSA Laboratories, a division of RSA Data Security,
     Inc., created 1991. All rights reserved.
 */

#include "r_random.h"

#define RANDOM_UBYTES_NEEDED 512

int R_RandomInit (randomStruct)
R_RANDOM_STRUCT *randomStruct;                      /* new random structure */
{
  randomStruct->bytesNeeded = RANDOM_UBYTES_NEEDED;
  R_memset ((POINTER)randomStruct->state, 0, sizeof (randomStruct->state));
  randomStruct->outputAvailable = 0;
  
  return (0);
}

int R_RandomUpdate (randomStruct, block, blockLen)
R_RANDOM_STRUCT *randomStruct;                          /* random structure */
unsigned char *block;                          /* block of values to mix in */
unsigned int blockLen;                                   /* length of block */
{
  MD5_CTX context;
  unsigned char digest[16];
  unsigned int i, x;
  
  MD5Init (&context);
  MD5Update (&context, block, blockLen);
  MD5Final (digest, &context);

  /* add digest to state */
  x = 0;
  for (i = 0; i < 16; i++) {
    x += randomStruct->state[15-i] + digest[15-i];
    randomStruct->state[15-i] = (unsigned char)x;
    x >>= 8;
  }
  
  if (randomStruct->bytesNeeded < blockLen)
    randomStruct->bytesNeeded = 0;
  else
    randomStruct->bytesNeeded -= blockLen;
  
  /* Zeroize sensitive information.
   */
  R_memset ((POINTER)digest, 0, sizeof (digest));
  x = 0;
  
  return (0);
}

int R_GetRandomBytesNeeded (bytesNeeded, randomStruct)
unsigned int *bytesNeeded;                 /* number of mix-in UBYTEs needed */
R_RANDOM_STRUCT *randomStruct;                          /* random structure */
{
  *bytesNeeded = randomStruct->bytesNeeded;
  
  return (0);
}

int R_GenerateBytes (block, blockLen, randomStruct)
unsigned char *block;                                              /* block */
unsigned int blockLen;                                   /* length of block */
R_RANDOM_STRUCT *randomStruct;                          /* random structure */
{
  MD5_CTX context;
  unsigned int available, i;
  
  if (randomStruct->bytesNeeded)
    return (RE_NEED_RANDOM);
  
  available = randomStruct->outputAvailable;
  
  while (blockLen > available) {
    R_memcpy
      ((POINTER)block, (POINTER)&randomStruct->output[16-available],
       available);
    block += available;
    blockLen -= available;

    /* generate new output */
    MD5Init (&context);
    MD5Update (&context, randomStruct->state, sizeof(randomStruct->state));
    MD5Final (randomStruct->output, &context);
    available = 16;

    /* increment state */
    for (i = 0; i < 32; i++)
      if (randomStruct->state[31-i]++)
        break;
  }

  R_memcpy 
    ((POINTER)block, (POINTER)&randomStruct->output[16-available], blockLen);
  randomStruct->outputAvailable = available - blockLen;

  return (0);
}

void R_RandomFinal (randomStruct)
R_RANDOM_STRUCT *randomStruct;                          /* random structure */
{
  R_memset ((POINTER)randomStruct, 0, sizeof (*randomStruct));
}
