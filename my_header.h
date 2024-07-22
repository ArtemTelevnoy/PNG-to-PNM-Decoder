<<<<<<< HEAD
#ifndef HEAD
#define HEAD

#include "return_codes.h"

#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined ZLIB
#include <zlib.h>
#elif defined LIBDEFLATE
#include <libdeflate.h>
#elif defined ISAL
#include <include/igzip_lib.h>
#else
#error Invalid lib
#endif

typedef unsigned char uChar;
typedef unsigned int uInt;
#define SuChar sizeof(uChar)
#define SuInt sizeof(uInt)

=======
#ifndef HEAD
#define HEAD

#include "return_codes.h"

#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined ZLIB
#include <zlib.h>
#elif defined LIBDEFLATE
#include <libdeflate.h>
#elif defined ISAL
#include <include/igzip_lib.h>
#else
#error Invalid lib
#endif

typedef unsigned char uChar;
typedef unsigned int uInt;
#define SuChar sizeof(uChar)
#define SuInt sizeof(uInt)

>>>>>>> 516be8d0c321e568e5e838715239cd47b8be55f3
#endif