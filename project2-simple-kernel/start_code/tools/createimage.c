#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SECTOR_SIZE 512       /* floppy sector size in bytes */

void write_bootblock(FILE *image, FILE *bbfile, Elf32_Phdr *Phdr);
Elf32_Phdr *read_exec_file(FILE *opfile);
uint8_t count_kernel_sectors(Elf32_Phdr *Phdr);
void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz);
void write_to_image(FILE *imagefile, FILE *readfile, Elf32_Ehdr *elf_hdr, Elf32_Phdr *prog_hdr);

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

  	kernelfile = fopen("main","r");
  	fread(&kernel_ehdr, 1,sizeof(Elf32_Ehdr) , kernelfile);

  	for(i = 0; i < kernel_ehdr.e_phnum; i++) {
      kernel_size += Phdr[i].p_memsz;
    }

  	sector_cnt = (int) kernel_size / 512 + ((kernel_size % 512 == 0)? 0 : 1);

  	return sector_cnt;

}
void write_bootblock(FILE *image, FILE *file, Elf32_Phdr *phdr)
{ 
    fseek(file, 0, SEEK_SET);
    Elf32_Ehdr ehdr;
    fread(&ehdr, 1,sizeof(Elf32_Ehdr) , file);
 
    fseek(image, 0, SEEK_SET);
    write_to_image(image, file, &ehdr, phdr); 

    fseek(image, 0x1fe, SEEK_SET);
    fputc(0x55, image);
    fputc(0xAA, image);
}

void write_kernel(FILE *image, FILE *knfile, Elf32_Phdr *phdr)
{
    fseek(knfile, 0, SEEK_SET);
    Elf32_Ehdr kernel_ehdr;
    fread(&kernel_ehdr, 1,sizeof(Elf32_Ehdr) , knfile);
    fseek(image, 512, SEEK_SET);
    write_to_image(image, knfile, &kernel_ehdr, phdr); 
}
void write_to_image(FILE *imagefile, FILE *readfile, Elf32_Ehdr *elf_hdr, Elf32_Phdr *prog_hdr) {
  size_t required_padding; // padding (in bytes) required to bring write to the end of a sector in image
  size_t totalread = 0; // total bytes read thusfar
  size_t numremaining, num2read, numread; //for current pidx: number of bytes remaining to r/w, to r/w this turn, and to r/w during this loop
  int pidx; //which program header we're on
  char buffer[SECTOR_SIZE]; // buffer for copying a sector's worth of bytes

  // for each program header
  for(pidx = 0; pidx < (*elf_hdr).e_phnum; pidx++) {
    /* populate variables specific to this program header */
    numremaining = prog_hdr[pidx].p_filesz;
    required_padding = prog_hdr[pidx].p_memsz - numremaining;
    fseek(readfile, prog_hdr[pidx].p_offset, SEEK_SET); // this is where first byte of program resides

    /* copy code segment */
    while(numremaining > 0) {
      // read bytes   ( num2read = min(numremaining, SECTOR_SIZE) )
      num2read = (numremaining < SECTOR_SIZE) ? numremaining : SECTOR_SIZE;
      numread = fread(buffer, 1, num2read, readfile);
      //assert(numread == num2read);
      // write bytes
      numread = fwrite(buffer, 1, num2read, imagefile);
      //assert(numread == num2read);
      // update counters
      totalread += numread;
      numremaining -= numread;
    }
    /* pad up to memsz */     
    while(required_padding > 0) {
      fputc(0, imagefile);
      required_padding--;
      totalread++;
    }
  }
  /* pad to next sector */
  required_padding = (SECTOR_SIZE - totalread) % SECTOR_SIZE;
  while(required_padding > 0) {
    fputc(0, imagefile);
    required_padding--;
  }
}

void record_kernel_sectors(FILE *image, uint8_t kernelsz)
{
    fseek(image, 0x1e0, SEEK_SET);
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
    kernelfile = fopen("main","r");
    bootfile = fopen("bootblock","r");

    boot_phdr = read_exec_file(bootfile);

    /* write bootblock */  
    write_bootblock(imagefile, bootfile, boot_phdr);

    /* read executable kernel file*/
    kernel_phdr = read_exec_file(kernelfile);
    num_sectors = count_kernel_sectors(kernel_phdr);
    /* write kernel segments to image */
    write_kernel(imagefile, kernelfile, kernel_phdr);

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
