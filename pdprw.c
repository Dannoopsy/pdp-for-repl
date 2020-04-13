#include <stdio.h>

#define memsize (64 * 1024) 

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

byte mem[memsize];

void load_file( );
void mem_dump(adr start, word n);
byte b_read  (adr a);            // читает из "старой памяти" mem байт с "адресом" a.
void b_write (adr a, byte val);  // пишет значение val в "старую память" mem в байт с "адресом" a.
word w_read  (adr a);            // читает из "старой памяти" mem слово с "адресом" a.
void w_write (adr a, word val);  // пишет значение val в "старую память" mem в слово с "адресом" a.


int main () {
	byte x = 0xa1;
	byte y = 0xb2;
	int i;
	b_write(2, y);   
	b_write(3, x);
	printf("%04hx\n", w_read(2));  //a1b2
	
	w_write (4, 0x0b0a);
	printf("%04hx\n", w_read(4));  //0b0a
	
	printf ("%02hhx%02hhx\n", b_read(2), b_read(3));  // b2a1
	for (i = 0; i < 16; i ++) {
		b_write(i, i);
		printf("%02hhx\n", b_read(i));
	}									// from 00 to 0f
	load_file ();
	mem_dump(0x100, 2);  // 000400  000400
	printf("%04hx\n", w_read(0x100)); // 0100
	printf("\n");
	load_file();
	mem_dump (0x40, 16);  // 8 chet
	printf("\n");
	load_file();
	mem_dump (0x20, 16); 
	return 0;
}

void mem_dump(adr start, word n) {
	int i;
	for (i = 0; i < n; i = i + 2) {
		printf("%06o : %06o\n", (start + i), w_read(start + i));
	}
}
void load_file( ) {
	int a;
	int n, i;
	int b;
	scanf ("%x%x", &a, &n);
	for (i = 0; i < n; i ++) {
		scanf("%x", &b);
		b_write((adr)(a + i), (byte)b);
	}
}
byte b_read  (adr a) {
	return mem[a];
}

void b_write (adr a, byte val) {
	mem[a] = val;
}

word w_read  (adr a) {
	if (a % 2 == 0) {
		word w = (word)mem[a + 1] << 8;
		w = w | mem[a];
		return w;
	}
}
void w_write (adr a, word val) {
	if (a % 2 == 0) {
		mem [a] = (byte)(val );
		mem [a + 1] = (byte) ( (val >> 8) );
	}
}



	
	
	

	
