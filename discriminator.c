#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#define memsize (64 * 1024) 

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

char bait; // для байтовой или словесной операции мы сейчас работаем

byte mem[memsize];
word reg[8];
#define pc reg[7]




typedef struct {
	word val;
	word adress;
	char isreg;		// работает в регистры 1 или в память 0
	
} Arg;

Arg ss, dd;

typedef struct { // отдельная структура под нн
	word n;
	byte r;
} NN;
NN nn;




void do_halt ();
void do_mov ();
void do_add ();
void do_sob ();
void do_clr ();
void do_movb (); // байтовая функция
void do_nothing ();
void load_file( );
void mem_dump(adr start, word n);
byte b_read  (adr a);            // читает из "старой памяти" mem байт с "адресом" a.
void b_write (adr a, byte val);  // пишет значение val в "старую память" mem в байт с "адресом" a.
word w_read  (adr a);            // читает из "старой памяти" mem слово с "адресом" a.
void w_write (adr a, word val);  // пишет значение val в "старую память" mem в слово с "адресом" a.
void trace (const char * format, ...);
Arg get_mr (word w);
NN get_nn (word w);
void bm_write(adr a, word w, char isr);
void wm_write (adr a, word w, char isr);	// место записи зависит от моды 
word wm_read (adr a, char isr);	
void printreg ();		// печать всех регистров


typedef struct {
	word mask;
	word opcode;
	char * name;
	void (* func) (void);
	char isbait; // байтовая ли операция
	char isss;
	char isdd;
	char isnn;
	
	
}command;

command cmd[] = {
	{0777777, 0000000, "halt", do_halt, 0, 0, 0, 0},
	{0170000, 0010000, "mov", do_mov, 0, 1, 1, 0},
	{0170000, 0060000, "add", do_add, 0, 1, 1, 0},
	{0777000, 0077000, "sob", do_sob, 0, 0, 0, 1},
	{0777700, 0005000, "clr", do_clr, 0, 0, 1, 0},
	{0170000, 0110000, "movb", do_movb, 1, 1, 1, 0},
	{0000000, 0000000, "unknown", do_nothing, 0, 0, 0, 0},
	
};

void do_halt () {
	printreg();
	trace("\nThe end\n");
	exit(0);
}

void do_clr () {
	wm_write(dd.adress, 0, dd.isreg);
}

void do_mov () {
	wm_write (dd.adress, ss.val, dd.isreg); // функция знает, писать в регистры или в память
	
}

void do_movb () {
	bm_write(dd.adress, ss.val, dd.isreg);
}

void do_add () {
	wm_write(dd.adress, 
	wm_read(dd.adress, dd.isreg) + wm_read(ss.adress, ss.isreg),
	dd.isreg); // функция знает, читать из регистров или из памяти
}
void do_sob () {
	if(--reg[nn.r] != 0)
		pc = pc - 2 * (nn.n);
}
void do_nothing () {
}

int main () {
	pc = 01000;
	int i = 0;
	load_file();
	load_file();
	word w;
	printreg();
	while (1) {							// работает до do_halt
		if (pc % 2) { 
			trace ("pc = %o HOW?\n", pc);
		}
		w = w_read(pc);
		trace("\n%06o %06o: ", pc, w);
		pc = pc + 2;
		i = 0;
		while(1) {						// точно остановится на unknown
			if((w & cmd[i].mask) == cmd[i].opcode) {
				trace("%s ", cmd[i].name);
				bait = cmd[i].isbait;
				if( cmd[i].isss) { 
					ss = get_mr(w >> 6);
				}
				if(cmd[i].isdd) {
					dd = get_mr(w);
				}
				if(cmd[i].isnn) {
					nn = get_nn(w);
				}
				cmd[i].func();
				printreg();
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
		trace("ERROR w_read, adr = %o\n", a);
	}
}
void w_write (adr a, word val) {
	
	if (a % 2 == 0) {
		mem [a] = (byte)(val );
		mem [a + 1] = (byte) ( (val >> 8) );
	}
	else {
		trace("ERROR w_write, adr = %o\n", a);
	}
}                               

void wm_write (adr a, word w, char isr) { // выбирает, работать в регистры или в память
	if(isr) 
		reg[a] = w;
	else 
		w_write(a, w);
}
void bm_write(adr a, word w, char isr) {
	if(isr) 
		reg[a] = w;
	else 
		b_write(a, w);
	
}

word wm_read (adr a, char isr) { // выбирает, работать в регистры или в память
	if (isr) 
		return reg[a];
	else
		return w_read(a);
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
			res.isreg = 1;
			break;
		case 1: 
			res.isreg = 0;
			res.adress = reg[r];
			if (bait == 0) 
				res.val = w_read(res.adress);
			else {
				res.val = b_read(res.adress);
				if((res.val >> 7) & 1) // если значение в байте было отрицательным
					res.val += 0377 << 8;
			}
			trace("(R%o) ", r);
			break;
		case 2: 
			 res.isreg = 0;
			 res.adress = reg[r];
			 if (bait == 0) {			
				res.val = w_read(res.adress);
				reg [r] += 2;
			}	
			else {		
				res.val = b_read(res.adress);
					if((res.val >> 7) & 1) // если значение в байте было отрицательным
						res.val += 0377 << 8;
					if (r < 6) {
						reg [r] += 1;
					}
					else {
						reg [r] += 2;
					}
			}
			if( r == 7) 
						trace("#%o ", res.val);
					else
						trace("(R%o) ", r);
			break;
		case 3: 
			res.isreg = 0;
			if(bait == 0) {
				res.adress = w_read(reg[r]);
				res.val = w_read(res.adress);
			}
			else {
				res.adress = b_read(reg[r]) ;
				res.val = b_read(res.adress);
				if((res.val >> 7) & 1) // если значение в байте было отрицательным
						res.val += 0377 << 8;
			}
			reg[r] += 2;
			if (r != 7) 
				trace("@(R%o)+ ", r);
			else 
				trace("@#%o ", res.adress);
			break;
		case 4:
			res.isreg = 0;
			reg[r] -= 2;
			res.adress = reg[r];
			res.val = w_read( res.adress);
			trace("-(R%o) ", r);
			break;
		default:
			fprintf(stderr, "Mode %o NOT IMPLEMENTED YET!", mode);
			exit(1);
	}
	return res;
}
NN get_nn (word w) {
	NN res;
	res.n = w & 077;
	res.r = (w >> 6) & 7;
	return res;
}

void printreg () {
	int i = 0;
	trace("\n");
	for ( i = 0; i < 8; i ++) {
		trace("R%o:%o ", i, reg[i]);
	}
	printf("\n");
}


