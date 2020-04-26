#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#define memsize (64 * 1024) 

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

byte mem[memsize];
word reg[8];
#define pc reg[7]




typedef struct {
	word val;
	word adress;
} Arg;

Arg ss, dd;



void do_halt ();
void do_mov ();
void do_add ();
void do_nothing ();
void load_file( );
void mem_dump(adr start, word n);
byte b_read  (adr a);            // читает из "старой памяти" mem байт с "адресом" a.
void b_write (adr a, byte val);  // пишет значение val в "старую память" mem в байт с "адресом" a.
word w_read  (adr a);            // читает из "старой памяти" mem слово с "адресом" a.
void w_write (adr a, word val);  // пишет значение val в "старую память" mem в слово с "адресом" a.
void trace (const char * format, ...);
Arg get_mr (word w);

typedef struct {
	word mask;
	word opcode;
	char * name;
	void (* func) (void);
	char isss;
	char isdd;
	
}command;

command cmd[] = {
	{0777777, 0000000, "halt", do_halt, 0, 0},
	{0170000, 0010000, "mov", do_mov, 1, 1},
	{0170000, 0060000, "add", do_add, 1, 1},
	{0000000, 0000000, "unknown", do_nothing, 0, 0},
};

void do_halt () {
	printf("R0 = %o R1 = %o pc = %o\n", reg[0], reg[1], pc);
	trace("\nThe end\n");
	exit(0);
}

void do_mov () {
	reg[dd.adress] = ss.val;
}
void do_add () {
	reg[dd.adress] += reg[ss.adress];
}
void do_nothing () {
}

int main () {
	pc = 01000;
	int i = 0;
	load_file();
	word w;
	while (1) {							// работает до do_halt
		w = w_read(pc);
		trace("\n%06o %06o: ", pc, w);
		pc = pc + 2;
		i = 0;
		while(1) {						// точно остановится на unknown
			if((w & cmd[i].mask) == cmd[i].opcode) {
				trace("%s ", cmd[i].name);
				if( cmd[i].isss) 
					ss = get_mr(w >> 6);
				if(cmd[i].isdd)
					dd = get_mr(w);
				cmd[i].func();
				break;	
			}
			i++;
		}
	}
	
	
	
	
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
	else {
		trace("ERROR w_read, adr = %o", a);
	}
}
void w_write (adr a, word val) {
	if (a % 2 == 0) {
		mem [a] = (byte)(val );
		mem [a + 1] = (byte) ( (val >> 8) );
	}
	else {
		trace("ERROR w_write, adr = %o", a);
	}
}                                  

void trace (const char * format, ...) {
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap); 
	va_end(ap);
}

Arg get_mr (word w) {
	Arg res;
	int r = w & 7;
	int mode = (w >> 3) & 7;
	switch (mode) {
		case 0: 
			res.adress = r;
			res.val = reg[r];
			trace("R%o ", r);
			break;
		case 1: 
			res.adress = reg[r];
			res.val = w_read(res.adress);
			trace("(R%o) ", r);
			break;
		case 2: 
			res.adress = reg[r];
			res.val = w_read(res.adress);
			reg [r] += 2;
			if( r == 7) 
				trace("#%o ", res.val);
			else
				trace("(R%o) ", r);
			break;
		default :
			fprintf(stderr, "Mode %o NOT IMPLEMENTED YET!", mode);
			exit(1);
	}
	return res;
}




