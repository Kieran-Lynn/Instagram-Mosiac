#include <stdio.h>
int main(int argc, char* argv[]) {
	FILE *in = (argc > 1) ? fopen(argv[1],"r") : stdin, *out = (argc > 2) ? fopen(argv[2],"a") : stdout;
	char c, indent = 0, inquotes = 0, *spaces = "                                                                   ";
	while ((c = fgetc(in)) != EOF) {
		(c == '{' || c == '[') ? indent += 2 : (c == '}' || c == ']') ? indent -= 2 : (c == ':' && !inquotes) ? (c = fprintf(out, "%c", c) + ' ' - 1) : (c == '\"') ? inquotes = !inquotes : 1 ;;;
		(c == '{' || c == '[' || c == ',') ? fprintf(out, "%c\n%*.*s", c, indent, indent, spaces) : (c == '}' || c == ']') ? fprintf(out, "\n%*.*s%c", indent, indent, spaces, c) : fprintf(out, "%c", c);
	}
}
