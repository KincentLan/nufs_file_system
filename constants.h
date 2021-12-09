#include "inode.h"
#ifndef _CONSTANTS_H
#define _CONSTANTS_H

static const int DATABLOCK_COUNT = 255 * 4096 / (sizeof(inode_t) + 4096); 
static const int INODE_COUNT = 255 - DATABLOCK_COUNT;

#endif
