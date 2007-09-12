/*  
**  (C) 2007 by Remo Dentato (rdentato@users.sourceforge.net)
**
** Permission to use, copy, modify and distribute this code and
** its documentation for any purpose is hereby granted without
** fee, provided that the above copyright notice, or equivalent
** attribution acknowledgement, appears in all copies and
** supporting documentation.
** 
** Copyright holder makes no representations about the suitability
** of this software for any purpose. It is provided "as is" without
** express or implied warranty.
*/

#ifndef VLS_H
#define VLS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <strings.h>
#include "hul.h"

typedef struct vls_ptr {
  struct vls_ptr *nxt;
  uint32_t        off;
  void            ptr[1];  
} vls_ptr;

typedef struct vls {
  vls_ptr   *ptr;
  void      *aux;
  uint32_t   cur;
  uint16_t   esz;
  uint16_t   ksz;
} vls;


#endif VLS_H
