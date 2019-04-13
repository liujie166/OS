#define PORT 0xbfe48000


void __attribute__((section(".entry_function"))) _start(void)
{
	// Call PMON BIOS printstr to print message "Hello OS!"
	char *string = "Hello OS!";
	void (*printstr)(char *);
	printstr = 0x8007b980;
	(*printstr)(string);
	return;
}
