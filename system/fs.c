#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef FS
#include <fs.h>

static struct fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
struct filetable oft[NUM_FD];
int next_open_fd = 0;


#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock);
/* YOUR CODE GOES HERE */

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEBLOCKS - 2) {
    printf("No indirect block support\n");
    return SYSERR;
  }

  diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

  return diskblock;
}

/* read in an inode and fill in the pointer */
int fs_get_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;
  int inode_off;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_get_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(struct inode);

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  printf("inn*sizeof(struct inode): %d\n", inode_off);
  */

  bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(in, &block_cache[inode_off], sizeof(struct inode));

  return OK;

}

int
fs_put_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_put_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  */

  bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
  memcpy(&block_cache[(inn*sizeof(struct inode))], in, sizeof(struct inode));
  bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}
     
int fs_mkfs(int dev, int num_inodes) {
  int i;
  
  if (dev == 0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  }
  else {
    printf("Unsupported device\n");
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  }
  else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) {i++;}
  fsd.freemaskbytes = i / 8; 
  
  if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *)SYSERR) {
    printf("fs_mkfs memget failed.\n");
    return SYSERR;
  }
  
  /* zero the free mask */
  for(i=0;i<fsd.freemaskbytes;i++) {
    fsd.freemask[i] = '\0';
  }
  
  fsd.inodes_used = 0;
  
  /* write the fsystem block to SB_BLK, mark block used */
  fs_setmaskbit(SB_BLK);
  bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));
  
  /* write the free block bitmask in BM_BLK, mark block used */
  fs_setmaskbit(BM_BLK);
  bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  return 1;
}

void
fs_print_fsd(void) {

  printf("fsd.ninodes: %d\n", fsd.ninodes);
  printf("sizeof(struct inode): %d\n", sizeof(struct inode));
  printf("INODES_PER_BLOCK: %d\n", INODES_PER_BLOCK);
  printf("NUM_INODE_BLOCKS: %d\n", NUM_INODE_BLOCKS);
}

/* specify the block number to be set in the mask */
int fs_setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

/* specify the block number to be read in the mask */
int fs_getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
  return OK;

}

/* specify the block number to be unset in the mask */
int fs_clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
   block is indicated in the high-order bit.  Shift the byte by j
   positions to make the match in bit7 (the 8th bit) and then shift
   that value 7 times to the low-order bit to print.  Yes, it could be
   the other way...  */
void fs_printfreemask(void) {
  int i,j;

  for (i=0; i < fsd.freemaskbytes; i++) {
    for (j=0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}

int fs_open(char *filename, int flags) {
	struct inode node;
	int i;
	for(i = 0; i < fsd.root_dir.numentries; i++)
		if(strcmp(filename, fsd.root_dir.entry[i].name) == 0){
			fs_get_inode_by_num(dev0, fsd.root_dir.entry[i].inode_num, &node);
			oft[i].state = FSTATE_OPEN;
			oft[i].fileptr = 0;
			oft[i].de = &fsd.root_dir.entry[i];
			oft[i].in = node;
			return i;
		}
	printf("File not found!\n");
	return SYSERR;
}

int fs_close(int fd) {
	if(fd<NUM_FD){
		oft[fd].state = FSTATE_CLOSED;
		oft[fd].fileptr = 0;
		return OK;
	}
	printf("Invalid File Descripter!\n");
	return SYSERR;
}

int fs_create(char *filename, int mode) {
	if(mode == O_CREAT){
		int i;
		for(i = 0; i < fsd.root_dir.numentries; i++)
			if(strcmp(filename, fsd.root_dir.entry[i].name) == 0){
				printf("Filename exists.\n");
				return SYSERR;
			}
		struct inode newnode;
		fs_get_inode_by_num(dev0, fsd.inodes_used, &newnode);
		newnode.id = fsd.inodes_used;
		newnode.type = INODE_TYPE_FILE;
		newnode.nlink=1;
		newnode.device = dev0;
		newnode.size = 0;
		fs_put_inode_by_num(dev0, fsd.inodes_used, &newnode);
		fsd.inodes_used++;	
		fsd.root_dir.entry[fsd.root_dir.numentries].inode_num = fsd.inodes_used;
		strcpy(fsd.root_dir.entry[fsd.root_dir.numentries].name, filename);
		fsd.root_dir.numentries++;
		return fs_open(filename, O_RDWR);
	}
	printf("Unsupported mode!\n");
	return SYSERR;
}

int fs_seek(int fd, int offset){ 
	if(oft[fd].state==FSTATE_OPEN){
		oft[fd].fileptr += offset;
		return OK;
	}
	printf("File not open\n");
	return SYSERR;
}

int fs_read(int fd, void *buf, int nbytes) {
  if (oft[fd].state != FSTATE_OPEN) {
    printf("File not open\n");
    return SYSERR;
  }
  struct inode *inodePtr;
  inodePtr = (struct inode *)getmem(sizeof(struct inode));
  memcpy(inodePtr, &(oft[fd].in), sizeof(struct inode));
  int fileSize = inodePtr->size;
  int end = oft[fd].fileptr + nbytes;
  if (end > fileSize) {
    nbytes = fileSize - oft[fd].fileptr;
    end = fileSize;
  }

  int startblk = oft[fd].fileptr / fsd.blocksz;
  int endblk = end / fsd.blocksz;
  if (oft[fd].fileptr % fsd.blocksz == 0)
    startblk--;
  if (end % fsd.blocksz == 0)
    endblk--;

  int block;
  int offset;
  int size;
  char *bufPtr = (char *)buf;
  int i;
  for (i = startblk + 1; i <= endblk; i++) {
    block = inodePtr->blocks[i];
    if (i == startblk + 1) {
      offset = oft[fd].fileptr % fsd.blocksz;
      size = fsd.blocksz - offset;
    } else if (i == endblk) {
      offset = 0;
      size = end % fsd.blocksz;
    } else {
      offset = 0;
      size = fsd.blocksz;
    }
    bs_bread(dev0, block, offset, bufPtr, size);
    bufPtr += size;
  }
  oft[fd].fileptr = end;

  return nbytes;
}

int fs_write(int fd, void *buf, int nbytes) {
  if (oft[fd].state != FSTATE_OPEN) {
    printf("File not open\n");
    return SYSERR;
  }
  struct inode *inodePtr;
  inodePtr = (struct inode *)getmem(sizeof(struct inode));
  memcpy(inodePtr, &(oft[fd].in), sizeof(struct inode));

  int newSize = oft[fd].fileptr + nbytes;
  if (newSize >= fsd.blocksz * INODEBLOCKS) {
    printf("Not enough space\n");
    return SYSERR;
  }

  int startblk = oft[fd].fileptr / fsd.blocksz;
  if (oft[fd].fileptr % fsd.blocksz == 0) {
    startblk--;
  }
  int endblk = newSize / fsd.blocksz;
  if (newSize % fsd.blocksz == 0) {
    endblk--;
  }
  int curr = inodePtr->size / fsd.blocksz;
  if (inodePtr->size % fsd.blocksz == 0) {
    curr--;
  }

  int i,j;
  if (endblk > curr) {
    for (i = curr + 1; i <= endblk; i++) {
      for (j = FIRST_INODE_BLOCK + NUM_INODE_BLOCKS; j < fsd.nblocks; j++) 
        if (fs_getmaskbit(j) == 0)
          break;
      if (j == fsd.nblocks) {
        printf("No empty block\n");
        return SYSERR;
      }
      fs_setmaskbit(j);
      inodePtr->blocks[i] = j;
    }
  }

  int block;
  int offset;
  int size;
  char *bufPtr = (char *)buf;
  for (i = startblk + 1; i <= endblk; i++) {
    block = inodePtr->blocks[i];
    if (i == startblk + 1) {
      offset = oft[fd].fileptr % fsd.blocksz;
      size = fsd.blocksz - offset;
    } else if (i == endblk) {
      offset = 0;
      size = newSize % fsd.blocksz;
    } else {
      offset = 0;
      size = fsd.blocksz;
    }
    bs_bwrite(dev0, block, offset, bufPtr, size);
    bufPtr += size;
  }
  
  oft[fd].fileptr = newSize;
  if (newSize > inodePtr->size) {
    inodePtr->size = newSize;
  }
  memcpy(&(oft[fd].in), inodePtr, sizeof(struct inode));

  return nbytes;
}

#endif /* FS */
