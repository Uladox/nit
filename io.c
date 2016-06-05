#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <uniconv.h>
#include <unistr.h>

#include "io.h"

void
get_u8(const char *charset, const char *src, size_t src_size,
       uint8_t **result, size_t *result_size)
{
	uint8_t *u8;
	size_t size = *result_size;

        u8 = u8_conv_from_encoding(charset, iconveh_question_mark, src,
				   src_size, NULL, *result, &size);
	if (u8 != *result && *result)
		free(*result);
	if (size > *result_size)
		*result_size = size;
	*result = u8;
}

void
get_encoded(const char *charset, const uint8_t *src, size_t src_size,
	    char **result, size_t *result_size)
{
	char *encoded;
	size_t size = *result_size;

	encoded = u8_conv_to_encoding(charset, iconveh_question_mark, src,
				      src_size, NULL, *result, &size);
	if (encoded != *result && *result)
		free(*result);
	if (size > *result_size)
		*result_size = size;
	*result = encoded;
}

