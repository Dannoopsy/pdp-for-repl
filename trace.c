#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>



void trace (const char * format, ...) {
	unsigned int q = 0, i = 0, w = strlen(format);
	char * s = (char*)malloc(w + 4);
	int * p = &format;
	p ++;
	while (q <= w) {
		i = 0;
		//printf ("%c\n", format[q] - '%');
		if (format[q] - '%' != 0) {
			while (q <= w && (format[q] - '%' != 0)) {
				s[i] = format[q];
				i ++;
				q ++;
			}
			s[i] = 0;
			
			printf(s);
		}
		else {
			while (q <= w && !(isalpha (format[q]))) {
				s[i] = format[q];
				i++;
				q++;
			}
			s[i] = format [q];
			q ++;
			s[i + 1] = 0;
			printf(s, *p);
			p ++;
		}
		//printf("%d\n", i);
	}
	free(s);	
}

int main () {
	trace("%06o %d %d\n", 01, 2, 3);
	trace("OOOOH    OOOH\n");
	trace("%x\n", 0xff);
	return 0;
}
