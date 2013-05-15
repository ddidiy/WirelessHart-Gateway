/*
 * $Id: bits.h,v 1.2 2008/11/20 08:40:38 mariusn Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#ifndef _bits_h_
#define _bits_h_

#ifndef _min_
#define _min_(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef _max_
#define _max_(a,b) ((a) > (b) ? (a) : (b))
#endif

#define hexdigit(x) (((x) <= '9') ? (x) - '0' : ((x) & 7) + 9)
#define error_ptr(error) ((void*)error)
#define is_error(ptr) ((unsigned long)ptr > (unsigned long)-4000L)

#endif
