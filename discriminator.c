#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#define memsize (64 * 1024) 

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

char bait; // для байтовой или словесной операции мы сейчас работаем
char isload; // грузим/не грузим программу
byte mem[memsize];
#define ostat 0177564
#define odata 0177566

word reg[8];
#define pc reg[7]
char psw;

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

word xx;



void do_halt ();
void do_mov ();
void do_add ();
void do_sob ();
void do_clr ();
void do_movb (); // байтовая функция
void do_br ();
void do_bpl ();
void do_beq ();
void do_tst ();
void do_tstb(); // байтовая функция
void do_nothing ();
void load_file( );
void set_N (word w);
void set_Z (word w);
void set_V (word w);
void set_C (unsigned long int x); 
void mem_dump(adr start, word n);
byte b_read  (adr a);            // читает из "старой памяти" mem байт с "адресом" a.
void b_write (adr a, byte val);  // пишет значение val в "старую память" mem в байт с "адресом" a.
word w_read  (adr a);            // читает из "старой памяти" mem слово с "адресом" a.
void w_write (adr a, word val);  // пишет значение val в "старую память" mem в слово с "адресом" a.
void trace (const char * format, ...);
Arg get_mr (word w);
NN get_nn (word w);
word get_xx (word w);
void bm_write(adr a, word w, char isr);
void wm_write (adr a, word w, char isr);	// место записи зависит от моды 
word wm_read (adr a, char isr);	
byte bm_read (adr a, char isr); // чтение из регистра и возвращение байта рисковая операция
								//но она и не планируется, а функция для единообразия
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
	char isxxx;
	
}command;

command cmd[] = {
	{0177777, 0000000, "halt", do_halt, 0, 0, 0, 0, 0},
	{0170000, 0010000, "mov", do_mov, 0, 1, 1, 0, 0},
	{0170000, 0060000, "add", do_add, 0, 1, 1, 0, 0},
	{0177000, 0077000, "sob", do_sob, 0, 0, 0, 1, 0},
	{0177700, 0005000, "clr", do_clr, 0, 0, 1, 0, 0},
	{0170000, 0110000, "movb", do_movb, 1, 1, 1, 0, 0},
	{0177400, 0000400, "br", do_br, 0, 0, 0, 0, 1},
	{0177400, 0001400, "beq", do_beq, 0, 0, 0, 0, 1},
	{0177400, 0100000, "bpl", do_bpl, 0, 0, 0, 0, 1},
	{0177700, 0005700, "tst", do_tst, 0, 0, 1, 0, 0},
	{0177700, 0105700, "tstb", do_tstb, 1, 0, 1, 0, 0},
	{0000000, 0000000, "unknown", do_nothing, 0, 0, 0, 0, 0},
	
};

void do_halt () {
	printreg();
	//trace ("\n%o\n\n", odata);
	trace("\nThe end\n");
	exit(0);
}

void do_clr () {
	wm_write(dd.adress, 0, dd.isreg);
}
void do_br () {
	pc = pc + xx*2;
}
void do_beq () {
	if ( (psw & 04)) 
		do_br();
}
void do_bpl () {
	//trace ("\n\n%o\n", (psw & 010));
	if ( (psw & 010) == 0) 
		do_br();
}
void do_tst () {
	word q = wm_read(dd.adress, dd.isreg);
	set_N (q);
	set_Z (q);
}
void do_tstb () {
	//word q = (word)bm_read(dd.adress, dd.isreg);
	//trace("\n%o\n", dd.val);
	set_N (dd.val);
	set_Z (dd.val);
}
void do_mov () {
	wm_write (dd.adress, ss.val, dd.isreg); // функция знает, писать в регистры или в память
	set_N(ss.val);
	set_Z(ss.val);
}

void do_movb () {
	bm_write(dd.adress, ss.val, dd.isreg);
	set_N (ss.val);
	set_Z(ss.val);
}

void do_add () {
	word w1 = wm_read(dd.adress, dd.isreg); // функция знает, читать из регистров или из памяти
	word w2 = wm_read(ss.adress, ss.isreg);
	wm_write(dd.adress, w1 + w2, dd.isreg); 
	unsigned long int x1 = 0;
	unsigned long int x2 = 0;
	x1 = x1 & w1;
	x2 = x2 & w2;
	set_N (w1 + w2);
	set_C (x1 + x2);
	set_Z (w1 + w2);
}
void do_sob () {
	if(--reg[nn.r] != 0)
		pc = pc - 2 * (nn.n);
}
void do_nothing () {
}

int main () {
	b_write (ostat, -1);  //всегда готов
	pc = 01000;
	int i = 0;
	while (!isload) 
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
				if(cmd[i].isxxx) {
					xx = get_xx(w);
				}
				cmd[i].func();
				printreg();
				break;	
			}
			i++;
		}
	}
	// сюда программа не должна заходить
	trace("ERROR ERROR ERROR КРИК УМИРАЮЩЕГО ДЕЛЬФИНА ERROR");
	
	
	
	
	
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
	if (scanf ("%x%x", &a, &n) != 2) {
		isload = 1;
	}
	else {
		for (i = 0; i < n; i ++) {
			scanf("%x", &b);
			b_write((adr)(a + i), (byte)b);
		}
}
}
byte b_read  (adr a) {
	return mem[a];
}

void b_write (adr a, byte val) {
	mem[a] = val;
	if(a == odata) 
		printf("%c", val);
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
	if(a == odata) 
		printf("%c", val);
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

byte bm_read (adr a, char isr) {
	if (isr) 
		return reg[a];
	else
		return b_read(a);
}
		
	   

void trace (const char * format, ...) {
	if (0) {
		va_list ap;
		va_start(ap, format);
		vprintf(format, ap); 
		va_end(ap);
	}
}

Arg get_mr (word w) {
	Arg res;
	int r = w & 7;
	int mode = (w >> 3) & 7;
	//trace ("mode = %o, r = %o\n", mode, r); // отладка!!!
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
				res.adress = w_read(reg[r]) ;
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
			if (bait == 0) {
				reg[r] -= 2;
				res.adress = reg[r];
				res.val = w_read(res.adress);
			}
			else {
				if(r < 6) 
					reg[r] --;
				else 
					reg[r] -= 2;
				res.adress = reg[r];
				res.val = b_read(res.adress);
			}	
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
void set_N (word w) { // устанавливает эн по левому биту слова/байта
	if (bait == 0) {
		if((w >> 15))
			psw = psw | 010;
		else 
			psw = psw & 07;
	}
	else { 
		if ((w >> 7)) 
			psw = psw | 010;
		else 
			psw = psw & 07;
	}
}
void set_Z (word w) { // устанавливает зет по слову/байту
	if (bait == 0) {
		if(w)
			psw = psw & 013;
		else 
			psw = psw | 04;
		}
	else {
		if((w & 0377))
			psw = psw & 013;
		else 
			psw = psw | 04;
		}
}
void set_C (unsigned long int x) {
	if (bait == 0) {
		if (((x >> 16) & 1)) 
			psw = psw | 1;
		else 
			psw = psw & 0;
	}
	else {
		if (((x >> 8) & 1)) 
			psw = psw | 1;
		else 
			psw = psw & 0;
	}
	
}
word get_xx (word w) {
	if ((w & 0200)) {
		//trace ("xx = %o\n", ( * ((~(w - 1)) & 0377));
		return  (w & 0377) - 0400; 
	}
	else
		return  (w & 0177);
}
		
void printreg () {
	int i = 0;
	trace("\n");
	for ( i = 0; i < 8; i ++) {
		trace("R%o:%o ", i, reg[i]);
	}
	trace("\n");
}


