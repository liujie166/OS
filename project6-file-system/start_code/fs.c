#include "fs.h"
#include "screen.h"
#include "stdio.h"
#include "string.h"
int init_fs(){
	superblock_t super_sd;
	superblock_t super;
    char rbuffer[BLOCK_SIZE];
    char wbuffer[BLOCK_SIZE];
    super.magic = MAGIC;
	super.fs_size = FS_SIZE;
	super.imap_num = IMAP_NUM;
	super.imap_offset = IMAP_OFFSET*BLOCK_SIZE;
	super.ino_num = INO_NUM;  //inodes的数目
	super.ino_offset = INO_OFFSET*BLOCK_SIZE;  //inodes的起始位置
	super.smap_num = SMAP_NUM; //smap块的数目
	super.smap_offest =SMAP_OFFSET*BLOCK_SIZE;  //smap的起始位置
	super.data_num = DATA_NUM;  //数据块数目
	super.data_offset = DATA_OFFSET*BLOCK_SIZE; //数据块的起始位置
	super.inode_entry_size = sizeof(inode_t); //inode大小
	super.dir_entry_size = sizeof(dir_entry_t); //目录项大小

    sdread(rbuffer, FS_OFFSET, BLOCK_SIZE);
    memcpy(&super_sd, rbuffer, sizeof(uint32_t));
    if(super_sd.magic == MAGIC){
    	vt100_move_cursor(0,0);
    	printk("[FS] Filesystem exists!");

    }
    else{
    	vt100_move_cursor(0,0);
    	printk("[FS] Start to initialize filesystem!\n");
    	printk("[FS] Setting superblock...\n");
    	memcpy(wbuffer, &super, sizeof(superblock_t));
    	sdwrite(wbuffer, FS_OFFSET, BLOCK_SIZE);
    	printk(" magic : 0x66666666\n");
    	printk(" num sector : %d, start sector: %d\n",FS_SIZE/BLOCK_SIZE,FS_OFFSET/BLOCK_SIZE);
    	printk(" inode map offset : %d(%d)\n",IMAP_OFFSET,IMAP_NUM);
    	printk(" sector map offset : %d(%d)\n",SMAP_OFFSET,SMAP_NUM);
    	printk(" inode offset : %d(%d)\n",INO_OFFSET,INO_NUM);
    	printk(" data offset : %d(%d)\n",DATA_OFFSET,DATA_NUM);
    	printk(" inode entry size : %dB,dir entry size: %dB\n",sizeof(inode_t),sizeof(dir_entry_t));
    	printk("[FS] Setting inode-map...\n");
    	printk("[FS] Setting sector-map...\n");
    	printk("[FS] Setting inode...\n");
    	printk("[FS] Initialize filesystem finish!");
    }
}