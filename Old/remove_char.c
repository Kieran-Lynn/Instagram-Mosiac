#include <stdio.h>

#define BUFFER_SIZE 2048

char *remove_all_char(char *str, char c) {
	char *p = str, *q = str;
	while (*q == c) q++;
	while (*q) {
		*p++ = *q++;
		while (*q == c) q++;
	}
	*p = *q;
	return str;
}

int main(int argc, char *argv[]) {
	FILE *f = fopen(argv[1],"r");
	char buffer[BUFFER_SIZE];
	while (fgets(buffer, BUFFER_SIZE, f)) {
		printf("%s",remove_all_char(buffer, '\\'));
	}
}
