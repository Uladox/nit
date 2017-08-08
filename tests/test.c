#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "munit/munit.h"

#define NIT_SHORT_NAMES
#include "../macros.h"
#include "../palloc.h"
#include "../list.h"
#include "../hset.h"
#include "../hmap.h"
#include "../buf.h"
#include "../gap.h"

#define HMAP_MAX 100

static void
hmap_free_contents(void *key, void *storage, void *extra)
{
	free(storage);
}

static MunitResult
test_hmap(const MunitParameter params[], void *data)
{
	int i = 0;
	Nit_hmap *map = hmap_new(0);
	Nit_hentry *stack = NULL;
	Nit_hmap_iter iter;

	(void) params;
	(void) data;

	for (; i < HMAP_MAX; ++i) {
		int *storage = malloc(sizeof(i));

		*storage = i;
		munit_assert_false(hmap_add(map, &i, sizeof(i),
					   storage, &stack));
	}

	for (i = 0; i < HMAP_MAX; ++i)
		munit_assert_int(i, ==,
				 *(int *) hmap_get(map, &i, sizeof(i)));

	hmap_iter_init(&iter, map);
	i = 0;

	do {
		int *key = hmap_iter_key(&iter);
		int *val = hmap_iter_val(&iter);

		munit_assert_int(*key, ==, *val);
	} while (++i, hmap_iter_next(&iter));

	munit_assert_int(i, ==, HMAP_MAX);
	i = 42;
	free(hmap_remove(map, &i, sizeof(i), &stack));
	munit_assert_null(hmap_get(map, &i, sizeof(i)));
	hmap_free(map, hmap_free_contents, NULL);
	free(stack);
	return MUNIT_OK;
}

static MunitResult
test_gap(const MunitParameter params[], void *data)
{
	Nit_gap *gap = gap_new(1);
	char c;

	/* munit_assert_int(gap_expand(gap, 3), ==, 0); */
	munit_assert_int(gap_write(gap, 'a'), ==, 0);
	munit_assert_int(gap_write(gap, 'b'), ==, 0);
	munit_assert_int(gap_write(gap, 'c'), ==, 0);
	munit_assert_int(gap_write(gap, 'd'), ==, 0);
	munit_assert_int(gap_write(gap, 'e'), ==, 0);

	munit_assert_int(gap_read(gap, &c), ==, 0);
	munit_assert_char(c, ==, 'e');

	munit_assert_int(gap_erase(gap), ==, 0);
	munit_assert_int(gap_read(gap, &c), ==, 0);
	munit_assert_char(c, ==, 'd');

	munit_assert_int(gap_moveb(gap), ==, 0);
	munit_assert_int(gap_read(gap, &c), ==, 0);
	munit_assert_char(c, ==, 'c');

	munit_assert_int(gap_moveb(gap), ==, 0);
	munit_assert_int(gap_read(gap, &c), ==, 0);
	munit_assert_char(c, ==, 'b');

	munit_assert_int(gap_movef(gap), ==, 0);
	munit_assert_int(gap_read(gap, &c), ==, 0);
	munit_assert_char(c, ==, 'c');

	while (gap_movef(gap) == 0);

	munit_assert_int(gap_read(gap, &c), ==, 0);
	munit_assert_char(c, ==, 'd');

	while (gap_moveb(gap) == 0);

	munit_assert_int(gap_movef(gap), ==, 0);
	munit_assert_int(gap_read(gap, &c), ==, 0);
	munit_assert_char(c, ==, 'a');

	gap_free(gap);
	return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
	{ (char *) "/hmap", test_hmap,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/gap", test_gap,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }

};

static const MunitSuite test_suite = {
	(char *) "nit",
	test_suite_tests,
	NULL,
	1,
	MUNIT_SUITE_OPTION_NONE
};

int
main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
	return munit_suite_main(&test_suite, NULL, argc, argv);
}
