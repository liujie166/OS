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
	uint32_t num; //唯一
	uint32_t type; //类型
	
	uint32_t direct_point[11]; //直接寻址块的偏移
	uint32_t valid;
}inode_t;
typedef struct dir_entry{
	char file_name[20];
	
	uint32_t inode_num;
	uint32_t file_type;
	uint32_t valid;
}dir_entry_t;
typedef struct dir{
	char dir_name[24];
	uint32_t inode_num;
	dir_entry_t dty[10];	
}dir_t;
typedef struct fd_entry
{
	uint32_t inode_num;   //文件inode
	uint32_t availabity;     //权限（可读，可写，可读写）
	uint32_t current_seek;   //读写指针
	uint32_t valid;
}fd_entry_t;
extern fd_entry_t fd_table[8];
extern dir_t current_dir;
extern dir_t root;
extern dir_t cmp_dir;
extern int print_loc;
extern char current_path[24];
int map_search(uint8_t* map,int *first);
int map_set(uint8_t *map,int id);
int map_clear(uint8_t *map,int id);
void do_init_fs();
void show_fs_state();
void do_mkdir(char *name);
void do_rmdir(char *name);
void do_ls();
void do_cd(char *path);
int do_open_dir(char *name);
void do_touch(char *name);
void do_cat(char *name);

int do_openfile(char *name,int access);
int do_readfile(int fd,char *buffer,int size);
int do_writefile(int fd,char *buffer, int size);
void do_closefile(int fd);