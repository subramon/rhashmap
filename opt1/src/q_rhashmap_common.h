/*
 * Copyright (c) 2019 Ramesh Subramonian <subramonian@gmail.com>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#ifndef _Q_RHASHMAP__H
#define _Q_RHASHMAP_H

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "q_macros.h"
#include "fastdiv.h"

#define	Q_RHM_SET  1
#define	Q_RHM_ADD 2

#define	HASH_INIT_SIZE		(65536)
#define	MAX_GROWTH_STEP		(1024U * 1024)

#define	LOW_WATER_MARK 0.4
#define	HIGH_WATER_MARK 0.85

#endif
