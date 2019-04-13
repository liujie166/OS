#include "type.h"

#define MAGIC 0x66666666
#define FS_OFFSET 0X20000000 //512M
#define FS_SIZE 0x20000000 //512MB
#define BLOCK_SIZE 512
#define IMAP_NUM 1
#define IMAP_OFFSET 1 //1BLOCK
#define SMAP_NUM 256
#define SMAP_OFFSET 2 //2BLOCK
#define INO_NUM 512
#define INO_OFFSET 258
#define DATA_NUM 1047807
#define DATA_OFFSET 770
typedef struct superblock{
	uint32_t magic;  //用于判断文件系统是否存在
    uint32_t fs_size;  //文件系统大小
	uint32_t imap_num; //imap块数目
	uint32_t imap_offset;  //imap的起始位置
	uint32_t ino_num;  //inodes的数目
	uint32_t ino_offset;  //inodes的起始位置
	uint32_t smap_num; //smap块的数目
	uint32_t smap_offest;  //smap的起始位置
	uint32_t data_num;  //数据块数目
	uint32_t data_offset; //数据块的起始位置
	uint32_t inode_entry_size; //inode大小
	uint32_t dir_entry_size; //目录项大小
}superblock_t;
typedef struct inode{
	uint32_t mode; //权限
	uint32_t size;  //文件大小
	uint32_t timestamp; //时间戳
	uint32_t direct_point[12]; //直接寻址块的偏移
	uint32_t indirect_point; //一级间址块移
}inode_t;
typedef struct dir_entry{
	char filename[28];
	uint32_t inode_num;
}dir_entry_t;
typedef struct fd_entry
{
	uint32_t inode_num;   //文件inode
	uint32_t availabity;     //权限（可读，可写，可读写）
	uint32_t current_seek;   //读写指针

}fd_entry_t;
