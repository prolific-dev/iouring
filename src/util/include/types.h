#ifndef __TYPES_H__
#define __TYPES_H__

#include <assert.h>

typedef unsigned char  (U8BIT);
typedef signed   char  (S8BIT);

typedef unsigned short (U16BIT);
typedef signed   short (S16BIT);

typedef unsigned int   (U32BIT);
typedef signed   int   (S32BIT);

typedef unsigned long  (U64BIT);
typedef signed   long  (S64BIT);

/* Compile-time checks to ensure types have expected size */
static_assert(sizeof(U8BIT) == 1, "U8BIT does not have a size of 1 byte");
static_assert(sizeof(S8BIT) == 1, "S8BIT does not have a size of 1 byte");

static_assert(sizeof(U16BIT) == 2, "U16BIT does not have a size of 2 byte");
static_assert(sizeof(S16BIT) == 2, "S16BIT does not have a size of 2 byte");

static_assert(sizeof(U32BIT) == 4, "U32BIT does not have a size of 4 byte");
static_assert(sizeof(S32BIT) == 4, "S32BIT does not have a size of 4 byte");

static_assert(sizeof(U64BIT) == 8, "U64BIT does not have a size of 8 byte");
static_assert(sizeof(S64BIT) == 8, "S64BIT does not have a size of 8 byte");

#endif