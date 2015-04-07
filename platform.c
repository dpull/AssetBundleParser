#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "platform.h"

char *strndup(const char *s, size_t n)
{
	char *result;
	size_t len = strlen(s);

	if (n < len)
		len = n;

	result = (char *)malloc(len + 1);
	if (!result)
		return 0;

	result[len] = '\0';
	return (char *)memcpy(result, s, len);
}