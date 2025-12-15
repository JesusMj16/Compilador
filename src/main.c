#include <stdio.h>
#include <stdlib.h>

/* Se generan/definen por Flex */
extern FILE *yyin;
int yyparse(void);

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Uso: %s <archivo>\n", argv[0]);
		return 2;
	}

	yyin = fopen(argv[1], "rb");
	if (!yyin)
	{
		perror("No se pudo abrir el archivo");
		return 2;
	}

	int result = yyparse();
	fclose(yyin);

	if (result == 0)
	{
		puts("PARSE_OK");
		return 0;
	}

	puts("PARSE_FAIL");
	return 1;
}
