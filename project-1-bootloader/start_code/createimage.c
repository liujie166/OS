#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void write_bootblock(FILE *image, FILE *bbfile, Elf32_Phdr *Phdr);
Elf32_Phdr *read_exec_file(FILE *opfile);
uint8_t count_kernel_sectors(Elf32_Phdr *Phdr);
void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz);

Elf32_Phdr *read_exec_file(FILE *opfile)
{
  	uint8_t numread, num_phdr;

    Elf32_Ehdr ehdr;
    Elf32_Phdr *phdr = NULL;//return value

      //read in elf header
    fseek(opfile, 0, SEEK_SET);
    numread=fread(&ehdr, 1,sizeof(Elf32_Ehdr), opfile);
    assert(numread == sizeof(Elf32_Ehdr));
    num_phdr = ehdr.e_phnum;

      //read in program headers
    assert(ehdr.e_phentsize == sizeof(Elf32_Phdr));
    fseek(opfile, ehdr.e_phoff, SEEK_SET);//set read position to phdr
    phdr = calloc(sizeof(Elf32_Phdr), num_phdr); //allocate memmory to store phdr

    numread = fread(phdr,  sizeof(Elf32_Phdr),num_phdr, opfile);
    assert(numread == num_phdr);

    return phdr;
}


uint8_t count_kernel_sectors(Elf32_Phdr *Phdr)
{
  	int kernel_size = 0;
  	int sector_cnt = 0;
  	int i;
  	FILE *kernelfile;
  	Elf32_Ehdr kernel_ehdr;

  	kernelfile = fopen("kernel","r");
  	fread(&kernel_ehdr, 1,sizeof(Elf32_Ehdr) , kernelfile);

  	for(i = 0; i < kernel_ehdr.e_phnum; i++) {
      kernel_size += Phdr[i].p_filesz;
    }

  	sector_cnt = (int) kernel_size / 512 + ((kernel_size % 512 == 0)? 0 : 1);

  	return sector_cnt;

}
void write_bootblock(FILE *image, FILE *file, Elf32_Phdr *phdr)
{

    size_t numread;
    char buffer[512];
     
     
    /* set file offsets */  
    fseek(file, (*phdr).p_offset, SEEK_SET); 
    numread=fread(buffer, 1, (*phdr).p_filesz, file);
    assert(numread == (*phdr).p_filesz);
    // write bytes
    fseek(image, 0, SEEK_SET);
    numread = fwrite(buffer, 1, (*phdr).p_filesz, image);
    assert(numread == (*phdr).p_filesz);

    fseek(image, 0x1fe, SEEK_SET);
    fputc(0x55, image);
    fputc(0xAA, image);
}

void write_kernel(FILE *image, FILE *knfile, Elf32_Phdr *phdr, int kernelsz)
{
    char buffer[512*kernelsz];
    size_t numread; 

    fseek(knfile, (*phdr).p_offset, SEEK_SET); 
    numread = fread(buffer, 1,512*kernelsz, knfile);
    assert(numread == 512*kernelsz);

    fseek(image, 512, SEEK_SET);
    numread = fwrite(buffer, 1, 512*kernelsz, image);
    assert(numread == 512*kernelsz);
    
}

void record_kernel_sectors(FILE *image, uint8_t kernelsz)
{
    fseek(image, 0x1fd, SEEK_SET);
    fwrite(&kernelsz, 1,sizeof(uint8_t), image);
}

void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz)
{
	   
    printf("bootblock size is %d bytes!\n",Phdr_bb->p_filesz);
    printf("Bootblock message:\n" );
    printf("Bootblock image memmory size is 0x%x\n", Phdr_bb->p_memsz);
    printf("Bootblock image memmory offset is 0x%x\n", Phdr_bb->p_offset);
    printf("kernel messager\n" );
    printf("kernel image memmory size is 0x%x\n", Phdr_k->p_memsz);
    printf("kernel image memmory offset is 0x%x\n", Phdr_k->p_offset);
    printf("kernel sector size is 0x%d\n", kernelsz);
}

int main(){

    FILE *kernelfile, *bootfile,*imagefile;  //file pointers for bootblock,kernel and image
    Elf32_Ehdr *boot_header = malloc(sizeof(Elf32_Ehdr));//bootblock ELF header
    Elf32_Ehdr *kernel_header = malloc(sizeof(Elf32_Ehdr));//kernel ELF header

    Elf32_Phdr *boot_phdr; //bootblock ELF program header
    Elf32_Phdr *kernel_phdr; //kernel ELF program header
    int num_sectors;
   
    imagefile = fopen("image","w+");
    kernelfile = fopen("kernel","r");
    bootfile = fopen("bootblock","r");

    boot_phdr = read_exec_file(bootfile);

    /* write bootblock */  
    write_bootblock(imagefile, bootfile, boot_phdr);

    /* read executable kernel file*/
    kernel_phdr = read_exec_file(kernelfile);
    num_sectors = count_kernel_sectors(kernel_phdr);
    /* write kernel segments to image */
    write_kernel(imagefile, kernelfile, kernel_phdr, (int)num_sectors);

    /* tell the bootloader how many sectors to read to load the kernel */
    record_kernel_sectors(imagefile, num_sectors);


    /* check for  --extent option */
    extent_opt(boot_phdr, kernel_phdr, (int)num_sectors );

    
    // Free Memory
    free(boot_header);
    free(kernel_header);
    free(boot_phdr);
    free(kernel_phdr);

    // Close Files
    fclose(kernelfile);
    fclose(bootfile);
    fclose(imagefile);
    
    return 0;
} // ends main()