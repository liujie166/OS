#include "fs.h"
#include "screen.h"
#include "stdio.h"
#include "string.h"
uint8_t wbuffer[BLOCK_SIZE];
uint8_t rbuffer[BLOCK_SIZE];
uint8_t imap[BLOCK_SIZE];
uint8_t smap[BLOCK_SIZE];

char current_path[24];
int print_loc;
inode_t cmp_ino[8];
dir_t current_dir;
dir_t root;
dir_t create_dir;
dir_t cmp_dir;

fd_entry_t fd_table[8];

void do_init_fs(){
    int i;
    inode_t ino;
	superblock_t super_sd;
	superblock_t super;

    super.magic = MAGIC+1;
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
    for(i =0 ;i<8 ;i++){
        fd_table[i].valid = 0;
    }
    if(super_sd.magic == MAGIC+1){
    	vt100_move_cursor(0,0);
    	printk("[FS] Filesystem exists!\n");
        printk("[FS] loading root directory...\n");
        sdread(rbuffer, FS_OFFSET+DATA_OFFSET*BLOCK_SIZE, BLOCK_SIZE);
        memcpy(&root, rbuffer, sizeof(dir_t));
        printk("dir_name : %s\n",root.dir_name);
        if(!strcmp(root.dir_name,"root")){
            current_dir = root;
            strcpy(&current_path,"~");
            printk("[FS] root load succeed!!!\n");
        }
        else{
            printk("[FS] root not created!!!\n");
        }
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
        bzero(wbuffer, BLOCK_SIZE);
        sdwrite(wbuffer, FS_OFFSET+IMAP_OFFSET*BLOCK_SIZE, BLOCK_SIZE);
        //sdwrite(wbuffer, FS_OFFSET+DATA_OFFSET*BLOCK_SIZE, BLOCK_SIZE);
    	printk("[FS] Setting sector-map...\n");
        for(i = 0;i<SMAP_NUM;i++){
            sdwrite(wbuffer, FS_OFFSET+SMAP_OFFSET*BLOCK_SIZE+i*BLOCK_SIZE, BLOCK_SIZE);
        }
    	printk("[FS] Setting inode...\n");
        ino.valid = 0;
        memcpy(wbuffer, &ino, sizeof(inode_t));
        for (i = 0; i < 4096; i++)
        {
            sdwrite(wbuffer, FS_OFFSET+INO_OFFSET*BLOCK_SIZE+i*sizeof(inode_t), sizeof(inode_t));
        }
        
    	printk("[FS] Initialize filesystem finish!\n");
        printk("[FS] create and load root directory...\n");
        do_mkdir("root");
        sdread(&root, FS_OFFSET+DATA_OFFSET*BLOCK_SIZE, sizeof(dir_t));
        current_dir = root;
        strcpy(&current_path,"~");
    }
    return;
}
int map_search(uint8_t *map, int *first){
    int i,j;
    int num = 0;
    uint8_t flag = 0;
    uint8_t flag2 = 0;
    for(i = 0 ; i < BLOCK_SIZE ; i++){
        for (j = 8; j > 0; j--)
        {
            flag = map[i]&(1<<(j-1));
            if(flag!= 0){
                num ++;
                
            }
            if((0==flag)&&(0==flag2)){
                *first = i*8 + 8 - j;
                flag2 = 1;
                return num;
            }
        }
    }
    return num;
}
int map_set(uint8_t *map, int id){
    int i,j;
    int cnt = 0;
    for(i = 0 ; i < BLOCK_SIZE ; i++){
        for (j = 8; j > 0; j--)
        {
            if(cnt == id){
                map[i] = map[i]|(1<<(j-1));
                return 1;
            }
            cnt ++ ;
        }
    }
    return 0;
}
int map_clear(uint8_t *map,int id){
    int i,j;
    int cnt = 0;
    for(i = 0 ; i < BLOCK_SIZE ; i++){
        for (j = 8; j > 0; j--)
        {
            if(cnt == id){
                map[i] = map[i]&(~(1<<(j-1)));
                return 1;
            }
            cnt ++ ;
        }
    }
    return 0;
}
void show_fs_state(){
    superblock_t super;
    int first;
    int number;
    uint8_t buffer[BLOCK_SIZE];
    int used_cnt;
    int i;
    sdread(&buffer[0], FS_OFFSET, BLOCK_SIZE);
    memcpy(&super, &buffer[0], sizeof(superblock_t));

    vt100_move_cursor(0,0);
    printk("[FS] Filesystem information\n");
    printk(" magic : 0x%x (KFS)\n",super.magic);
    used_cnt = super.imap_num + super.smap_num + super.ino_num + 1;
    for(i = 0 ; i < SMAP_NUM ;i++){
        sdread(smap, FS_OFFSET + super.smap_offest + i * BLOCK_SIZE,BLOCK_SIZE);
        number = map_search(smap,&first);
        if(number == 0){
            break;
        }
        else{
            used_cnt = used_cnt + number;
        }   
        i++;
    }
    printk(" used sector : %d/%d, start sector: %d(0x%x)\n",used_cnt,super.fs_size/BLOCK_SIZE,FS_OFFSET/BLOCK_SIZE,FS_OFFSET);
    used_cnt = 0;
    for(i = 0 ; i < IMAP_NUM ;i++){
        sdread(imap, FS_OFFSET + super.imap_offset + i * BLOCK_SIZE,BLOCK_SIZE);
        number = map_search(imap,&first);
        if(number == 0){
            break;
        }
        else{
            used_cnt = used_cnt + number;
        }   
        i++;
    }
    printk(" inode map offset : %d, occupied sector : %d, used : %d/4096\n",super.imap_offset/BLOCK_SIZE,super.imap_num,used_cnt);
    printk(" sector map offset : %d, occupied sector : %d\n",super.smap_offest/BLOCK_SIZE,super.smap_num);
    printk(" inode offset : %d, occupied sector : %d\n",super.ino_offset/BLOCK_SIZE,super.ino_num);
    printk(" data offset : %d, occupied sector : %d\n",super.data_offset/BLOCK_SIZE,super.data_num);
    printk(" inode entry size : %dB,dir entry size: %dB\n",super.inode_entry_size,super.dir_entry_size);
    return;
}
void do_mkdir(char *name){
    int i = 0,j =0;
    int imap_first,smap_first;
    inode_t ino;
    inode_t ino_read;


    int imap_num,smap_num;

    vt100_move_cursor(0,0);
    sdread(&ino_read, FS_OFFSET+INO_OFFSET*BLOCK_SIZE+current_dir.inode_num*sizeof(inode_t),sizeof(inode_t));
    //printk("update data block num : %d\n",ino_read.direct_point[0]);
    sdread(imap, FS_OFFSET + IMAP_OFFSET*BLOCK_SIZE ,BLOCK_SIZE);
    imap_num = map_search(imap,&imap_first);
    
    ino.mode = 3;//1：read 2:write 3:read&write
    ino.size = sizeof(dir_t);
    ino.num = imap_first;
    ino.type = 1; //1:目录，2：文件
    ino.valid =1;
    //memcpy(&wbuffer[0],&ino,sizeof(inode_t));
    //sdwrite(&wbuffer[0], FS_OFFSET+INO_OFFSET*BLOCK_SIZE+i*sizeof(inode_t),sizeof(inode_t));
    for(i = 0 ; i < SMAP_NUM ;i++){
        sdread(smap, FS_OFFSET + SMAP_OFFSET*BLOCK_SIZE + i * BLOCK_SIZE,BLOCK_SIZE);
        smap_num = map_search(smap,&smap_first);
        if(smap_num < 4096){
            break;
        } 
        i++;
    }
    
    ino.direct_point[0] = smap_first;

    
    for(j =0;j<10;j++){
        create_dir.dty[j].valid = 0;
    }
    strcpy(&create_dir.dir_name, name);
    create_dir.inode_num = ino.num;
    create_dir.dty[0].file_name[0]='.';
    create_dir.dty[0].file_name[1]='\0';

    create_dir.dty[0].file_type = 1;
    create_dir.dty[0].inode_num = ino.num;
    create_dir.dty[0].valid =1;
    //strcpy(&create_dir.dty[1].file_name,"..");
    create_dir.dty[1].file_name[0]='.';
    create_dir.dty[1].file_name[1]='.';
    create_dir.dty[1].file_name[2]='\0';
    create_dir.dty[1].file_type = 1;
    create_dir.dty[1].valid =1;
    
    if(!strcmp(name,"root")){
        create_dir.dty[1].inode_num = ino.num;
    }else{
        //vt100_move_cursor(0,0);
        printk("used inodes : %d,first unused inode : %d \n",imap_num,imap_first);
        printk("used data : %d,first unused data : %d \n",smap_num + i*4096,smap_first);
        create_dir.dty[1].inode_num = current_dir.inode_num;
        for(j = 0 ; j < 10 ;j ++){
            if(current_dir.dty[j].valid == 0) {
                current_dir.dty[j].valid =1;
                strcpy(current_dir.dty[j].file_name, name);
                current_dir.dty[j].file_type = 1;
                current_dir.dty[j].inode_num = ino.num;
                sdread(&ino_read, FS_OFFSET+INO_OFFSET*BLOCK_SIZE+current_dir.inode_num*sizeof(inode_t),sizeof(inode_t));
                printk("update data block num : %d",ino_read.direct_point[0]);
                memcpy(wbuffer, &current_dir, sizeof(dir_t));
                sdwrite(wbuffer,FS_OFFSET+DATA_OFFSET*BLOCK_SIZE+ino_read.direct_point[0]*BLOCK_SIZE,sizeof(dir_t));
                break;
            } 
        }
    }
    sdread(&cmp_ino, FS_OFFSET+INO_OFFSET*BLOCK_SIZE+(int)(imap_first/8)*BLOCK_SIZE,BLOCK_SIZE);
    cmp_ino[imap_first%8] = ino;
    sdwrite(&cmp_ino, FS_OFFSET+INO_OFFSET*BLOCK_SIZE+(int)(imap_first/8)*BLOCK_SIZE,BLOCK_SIZE);
    map_set(imap,imap_first);
    sdwrite(imap,FS_OFFSET + IMAP_OFFSET*BLOCK_SIZE , BLOCK_SIZE);

    memcpy(wbuffer, &create_dir, sizeof(dir_t));
    sdwrite(wbuffer, FS_OFFSET+DATA_OFFSET*BLOCK_SIZE+smap_first*BLOCK_SIZE,sizeof(dir_t));
    map_set(smap, smap_first);
    sdwrite(smap,FS_OFFSET + SMAP_OFFSET*BLOCK_SIZE + i*BLOCK_SIZE, BLOCK_SIZE);
    sdread(&root,FS_OFFSET+DATA_OFFSET*BLOCK_SIZE,sizeof(dir_t));
    vt100_move_cursor(0,print_loc);
    printk("create finish...");
}
void do_ls(){
    int i;
    //inode_t ino;
    //sdread(&ino, FS_OFFSET+INO_OFFSET*BLOCK_SIZE+current_dir.inode_num*sizeof(inode_t),sizeof(inode_t));
    vt100_move_cursor(0,print_loc);
    //printk("[FS] current directory ino_num:%d, name: %s, data num:%d\n",current_dir.inode_num,current_dir.dir_name,ino.direct_point[0]);
    for (i = 0; i < 10; i++)
    {   
        if(current_dir.dty[i].valid == 1)
            printk(" %s ",current_dir.dty[i].file_name);
    }
    
}
void do_rmdir(char *name){
    int i,j,k;
    int flag =0;
    uint8_t clear = 0;
    inode_t ino[8];
    inode_t cmp;
    
    for ( i = 0; i < 10; i++)
    {
        if(!strcmp(&current_dir.dty[i].file_name,name)&&(current_dir.dty[i].valid ==1)){
            flag =1;
            vt100_move_cursor(0,print_loc);
            printk("[FS] found the dir!!!,dir ino num:%d\n",current_dir.dty[i].inode_num);
            break;
        }
    }
    if(!flag){
        vt100_move_cursor(0,print_loc);
        printk("[FS] not found the dir!!!");
    }
    else{
        for(j =0 ; j<INO_NUM ; j++){
            sdread(ino,FS_OFFSET+INO_OFFSET*BLOCK_SIZE+ j*BLOCK_SIZE,BLOCK_SIZE);
            for (k = 0; k < 8; k++)
            {
                /* code */

                if(current_dir.dty[i].inode_num == ino[k].num&&ino[k].valid == 1){
                    ino[k].valid = 0;
                    current_dir.dty[i].valid =0;
                    memcpy(wbuffer, &current_dir, sizeof(dir_t));
                    sdread(&cmp,FS_OFFSET+INO_OFFSET*BLOCK_SIZE+ current_dir.inode_num*sizeof(inode_t),sizeof(inode_t));
                    sdwrite(wbuffer,FS_OFFSET+DATA_OFFSET*BLOCK_SIZE+cmp.direct_point[0]*BLOCK_SIZE,sizeof(dir_t));
                    sdwrite(&ino,FS_OFFSET+INO_OFFSET*BLOCK_SIZE+ j*BLOCK_SIZE,BLOCK_SIZE);
                    sdread(imap, FS_OFFSET + IMAP_OFFSET*BLOCK_SIZE ,BLOCK_SIZE);
                    map_clear(imap, ino[k].num);
                    sdwrite(imap,FS_OFFSET + IMAP_OFFSET*BLOCK_SIZE , BLOCK_SIZE);
                    bzero(wbuffer, BLOCK_SIZE);
                    vt100_move_cursor(0,0);
                    printk("delete inode num : %d\n",ino[k].num);
                    printk("delete block num : %d\n",ino[k].direct_point[0]);
                    sdwrite(wbuffer,FS_OFFSET+DATA_OFFSET*BLOCK_SIZE+ino[k].direct_point[0]*BLOCK_SIZE,BLOCK_SIZE);
                    sdread(smap,FS_OFFSET + SMAP_OFFSET*BLOCK_SIZE + (int)(ino[k].direct_point[0]/4096)*BLOCK_SIZE,BLOCK_SIZE);
                    map_clear(smap, ino[k].direct_point[0]%4096);
                    sdwrite(smap,FS_OFFSET + SMAP_OFFSET*BLOCK_SIZE + (int)(ino[k].direct_point[0]/4096)*BLOCK_SIZE, BLOCK_SIZE);
                    printk("[FS] delete finish!!!");
                    return;
                }
                else{
                    vt100_move_cursor(0,0);
                    printk("%d.ino_num:%d \n",k+j*8,ino[k].num);
                }
            }
            break;
        }
            
    }
    
}
int do_open_dir(char* name){
    int i,j;
    inode_t cmp;
    for (i = 0; i < 10; i++)
    {
        if(!strcmp(cmp_dir.dty[i].file_name,name)){
            if(cmp_dir.dty[i].file_type != 1){
                vt100_move_cursor(0,print_loc);
                printk("error:%s is not directory\n",cmp_dir.dty[i].file_name);
                return 0;
            }else{
                for(j=0;j<4096;j++){
                    sdread(&cmp,FS_OFFSET+INO_OFFSET*BLOCK_SIZE+j*sizeof(inode_t), sizeof(inode_t));
                    if(cmp.num == cmp_dir.dty[i].inode_num) break;
                }
                sdread(&cmp_dir,FS_OFFSET+DATA_OFFSET*BLOCK_SIZE+cmp.direct_point[0]*BLOCK_SIZE,sizeof(dir_t));
                return 1;
            }
        }
    }
    vt100_move_cursor(0,print_loc);
    printk("error:%s not found\n",name);
    return 0;
}
void do_cd(char *path){
    int i,j;
    char dir_name[10];
    if(path[0] == '/'){
        cmp_dir = root;
        //printk("cmp_dir name :%s\n",cmp_dir.dir_name);
        //printk("cmp_dir name :%s\n",cmp_dir.dir_name);
        for(i = 1, j = 0; path[i]!='\0';i++){
            if(path[i]=='/'){
                dir_name[j] = '\0';
                do_open_dir(dir_name);
                j=0;
            }
            else{
                dir_name[j] = path[i];
                j++;
            }
        }
        dir_name[j] = '\0';
        if(do_open_dir(dir_name)){
        
            vt100_move_cursor(0,print_loc);
            printk("open succeed,current directory:%s\n",dir_name);
            current_dir = cmp_dir;
            strcpy(&current_path,path);
        }
    }
    else{
        cmp_dir = current_dir;
        for(i = 0, j = 0; path[i]!='\0';i++){
            if(path[i]=='/'){
                dir_name[j] = '\0';
                do_open_dir(dir_name);
                j=0;
            }
            else{
                dir_name[j] = path[i];
                j++;
            }
        }
        dir_name[j] = '\0';
        if(do_open_dir(dir_name)){
        
            vt100_move_cursor(0,print_loc);
            printk("open succeed,current directory:%s\n",dir_name);
            current_dir = cmp_dir;
            sdread(cmp_ino,FS_OFFSET+INO_OFFSET*BLOCK_SIZE+(int)(current_dir.dty[1].inode_num/8)*BLOCK_SIZE,BLOCK_SIZE);
            sdread(&cmp_dir,FS_OFFSET+DATA_OFFSET*BLOCK_SIZE+(cmp_ino[current_dir.dty[1].inode_num].direct_point[0])*BLOCK_SIZE,sizeof(dir_t));

            if(!strcmp(current_dir.dir_name,"root")){
                strcpy(&current_path,"~"); 
            }
            else if(!strcmp(cmp_dir.dir_name,"root")){
                current_path[0] = '/';
                strcpy(&current_path[1],current_dir.dir_name);
            }
            else if(strcmp(cmp_dir.dir_name,"root")){
                current_path[0] = '/';
                strcpy(&current_path[1],current_dir.dty[1].file_name);
                current_path[strlen(&current_path)] = '/';
                strcpy(&current_path[strlen(&current_path)+1],current_dir.dir_name);
            }
        }    
    }
}
void do_touch(char *name){
    int i = 0,j =0;
    int imap_first,smap_first;
    inode_t ino;
    inode_t ino_read;


    int imap_num,smap_num;

    vt100_move_cursor(0,0);
    sdread(&ino_read, FS_OFFSET+INO_OFFSET*BLOCK_SIZE+current_dir.inode_num*sizeof(inode_t),sizeof(inode_t));
    //printk("update data block num : %d\n",ino_read.direct_point[0]);
    sdread(imap, FS_OFFSET + IMAP_OFFSET*BLOCK_SIZE ,BLOCK_SIZE);
    imap_num = map_search(imap,&imap_first);
    
    ino.mode = 3;//1：read 2:write 3:read&write
    ino.size = sizeof(dir_t);
    ino.num = imap_first;
    ino.type = 2; //1:目录，2：文件
    ino.valid =1;
    //memcpy(&wbuffer[0],&ino,sizeof(inode_t));
    //sdwrite(&wbuffer[0], FS_OFFSET+INO_OFFSET*BLOCK_SIZE+i*sizeof(inode_t),sizeof(inode_t));
    for(i = 0 ; i < SMAP_NUM ;i++){
        sdread(smap, FS_OFFSET + SMAP_OFFSET*BLOCK_SIZE + i * BLOCK_SIZE,BLOCK_SIZE);
        smap_num = map_search(smap,&smap_first);
        if(smap_num < 4096){
            break;
        } 
        i++;
    }
    
    ino.direct_point[0] = smap_first;

    
    
    //vt100_move_cursor(0,0);
    printk("used inodes : %d,first unused inode : %d \n",imap_num,imap_first);
    printk("used data : %d,first unused data : %d \n",smap_num + i*4096,smap_first);
    for(j = 0 ; j < 10 ;j ++){
        if(current_dir.dty[j].valid == 0) {
            current_dir.dty[j].valid =1;
            strcpy(current_dir.dty[j].file_name, name);
            current_dir.dty[j].file_type = 2; //2：文件
            current_dir.dty[j].inode_num = ino.num;
            sdread(&ino_read, FS_OFFSET+INO_OFFSET*BLOCK_SIZE+current_dir.inode_num*sizeof(inode_t),sizeof(inode_t));
            printk("update data block num : %d",ino_read.direct_point[0]);
            memcpy(wbuffer, &current_dir, sizeof(dir_t));
            sdwrite(wbuffer,FS_OFFSET+DATA_OFFSET*BLOCK_SIZE+ino_read.direct_point[0]*BLOCK_SIZE,sizeof(dir_t));
            break;
        } 
    }
    
    sdread(&cmp_ino, FS_OFFSET+INO_OFFSET*BLOCK_SIZE+(int)(imap_first/8)*BLOCK_SIZE,BLOCK_SIZE);
    cmp_ino[imap_first%8] = ino;
    sdwrite(&cmp_ino, FS_OFFSET+INO_OFFSET*BLOCK_SIZE+(int)(imap_first/8)*BLOCK_SIZE,BLOCK_SIZE);
    map_set(imap,imap_first);
    sdwrite(imap,FS_OFFSET + IMAP_OFFSET*BLOCK_SIZE , BLOCK_SIZE);

    bzero(wbuffer, BLOCK_SIZE);
    memset(wbuffer , 23 , 1);
    sdwrite(wbuffer, FS_OFFSET+DATA_OFFSET*BLOCK_SIZE+smap_first*BLOCK_SIZE,BLOCK_SIZE);
    map_set(smap, smap_first);
    sdwrite(smap,FS_OFFSET + SMAP_OFFSET*BLOCK_SIZE + i*BLOCK_SIZE, BLOCK_SIZE);
    sdread(&root,FS_OFFSET+DATA_OFFSET*BLOCK_SIZE,sizeof(dir_t));
    vt100_move_cursor(0,print_loc);
    printk("create finish...");
}
void do_cat(char *name){
    int i;
    for (i = 0; i < 10; i++)
    {
        /* code */
        if(!strcmp(current_dir.dty[i].file_name,name)&&current_dir.dty[i].file_type == 2){
            vt100_move_cursor(0,print_loc);
            printk("cat succeed...");
            
            sdread(cmp_ino,FS_OFFSET+INO_OFFSET*BLOCK_SIZE+(int)(current_dir.dty[i].inode_num/8)*BLOCK_SIZE,BLOCK_SIZE);
            sdread(rbuffer,FS_OFFSET+DATA_OFFSET*BLOCK_SIZE+cmp_ino[(current_dir.dty[i].inode_num)%8].direct_point[0],BLOCK_SIZE);
            vt100_move_cursor(0,0);
            printk("%s",rbuffer);
            return;
        }
        if(!strcmp(current_dir.dty[i].file_name,name)&&current_dir.dty[i].file_type == 1){
            vt100_move_cursor(0,print_loc);
            printk("error : %s is directory...",name);
            return;
        }
    }
    vt100_move_cursor(0,print_loc);
    printk("error : not found %s ...",name);
}
int do_openfile(char *name,int access){
    int i,j;
    for (i = 0; i < 10; i++)
    {
        /* code */
        if(!strcmp(current_dir.dty[i].file_name,name)&&current_dir.dty[i].file_type == 2){
            vt100_move_cursor(0,0);
            printk("found file, openning ...\n");
            break;
        }
    }
    for(j =0; j<8 ;j++){
        if(fd_table[j].valid == 0){
            fd_table[j].inode_num = current_dir.dty[i].inode_num;
            fd_table[j].availabity = access;
            fd_table[j].valid =1;
            vt100_move_cursor(0,0);
            printk("open finish ...");
            break;
        }
    }
    syscall_return(j);
    return j;

}
int do_readfile(int fd,char *buffer,int size){
    sdread(cmp_ino,FS_OFFSET+INO_OFFSET*BLOCK_SIZE+(int)(fd_table[fd].inode_num/8)*BLOCK_SIZE,BLOCK_SIZE);
    sdread(buffer,FS_OFFSET+DATA_OFFSET*BLOCK_SIZE+cmp_ino[(fd_table[fd].inode_num)%8].direct_point[0],size);
    syscall_return(size);
    return size;
}
int do_writefile(int fd,char *buffer, int size){
    sdread(cmp_ino,FS_OFFSET+INO_OFFSET*BLOCK_SIZE+(int)(fd_table[fd].inode_num/8)*BLOCK_SIZE,BLOCK_SIZE);
    sdwrite(buffer,FS_OFFSET+DATA_OFFSET*BLOCK_SIZE+cmp_ino[(fd_table[fd].inode_num)%8].direct_point[0],BLOCK_SIZE);
    syscall_return(size);
    return size;
}
void do_closefile(int fd){
    fd_table[fd].valid = 0;
}
