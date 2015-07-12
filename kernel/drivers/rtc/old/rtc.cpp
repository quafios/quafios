// this was last written on 01 July 2010

asm("clock:");			// Header of ISA CMOS Real Time Clock.
asm("call  clock_main");	// Go to start of code.
asm("iret");			// Return.
asm("clock_package_data: .space 128");
asm("drive_size:  .long 0x00002000");	
asm("drive_class: .long 0x010103FF"); // ISA -> Base Systems Peripheral -> RTC -> General
unsigned int *clock_pack;

#include <kernel.h>
#include <asm.h>
#include <cga.h>

void clock_update();

struct rtc_date_time {
	unsigned short year;
	unsigned char  month;
	unsigned char  day;
	unsigned char  dayw;
	unsigned char  hour;
	unsigned char  minute;
	unsigned char  second;
} static system_time;

void clock_initialize() {
	printk(" Initializing CMOS RTC (Real Time Clock) Driver ...\n");
	asm("movl $clock_package_data, clock_pack");
	unsigned int static pack[32];
	pack[1] = 1;
	pack[2] = 0x0405;
	pack[3] = 10;
	asm("int $0x81"::"a"(0x0401), "b"(3), "D"(pack));
	clock_update();
}

void clock_gettime() {
	// Outputs:
	// ----------
	// 0: Year
	// 1: Month
	// 2: Day
	// 3: Day of Week.
	// 4: Hour
	// 5: Minutes
	// 6: Seconds

	// Output Information:
	// ----------------------
	clock_pack[6] = system_time.second;
	clock_pack[5] = system_time.minute;
	clock_pack[4] = system_time.hour;
	clock_pack[3] = system_time.dayw;
	clock_pack[2] = system_time.day;
	clock_pack[1] = system_time.month;
	clock_pack[0] = system_time.year;
}


void clock_update() {
	outb(0x0, 0x70); system_time.second = (inb(0x71) & 0x0F) + (((inb(0x71)&0xF0)>>4)*10); // Get Seconds from CMOS.
	outb(0x2, 0x70); system_time.minute = (inb(0x71) & 0x0F) + (((inb(0x71)&0xF0)>>4)*10); // Get Minutes from CMOS.
	outb(0x4, 0x70); system_time.hour   = (inb(0x71) & 0x0F) + (((inb(0x71)&0xF0)>>4)*10); // Get Hours from CMOS.
	outb(0x7, 0x70); system_time.day    = (inb(0x71) & 0x0F) + (((inb(0x71)&0xF0)>>4)*10); // Get Day from CMOS.
	outb(0x8, 0x70); system_time.month  = (inb(0x71) & 0x0F) + (((inb(0x71)&0xF0)>>4)*10); // Get Month from CMOS.
	outb(0x9, 0x70); system_time.year   = (inb(0x71) & 0x0F) + (((inb(0x71)&0xF0)>>4)*10) + 2000; // Get Year.

	// Algorithm of getting the day of week: 
	// ---------------------------------------
	// [Source: http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week ]
	unsigned char months[] = {0, 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
	unsigned char months_leap[] = {0, 6, 2, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
	unsigned int step1 = 6; // We are working on the 2000s
	unsigned int step2 = system_time.year - 2000; // Last two digits.
	unsigned int step3 = step2/4;	// Divide the two digits by 4. notice that this is integer not float.
	unsigned int step4; if (step2%4) step4=months[system_time.month]; else step4=months_leap[system_time.month];
	unsigned int step5 = step1 + step2 + step3 + step4 + system_time.day;
	system_time.dayw = step5 % 7;
}

extern "C" {
void clock_main() {
	unsigned char func;
	asm("":"=b"(func));
	switch (func) {
		case  1: {clock_initialize();	break;}
		case  2: {clock_gettime();	break;}
		case 10: {clock_update();	break;}
	}
}
}
