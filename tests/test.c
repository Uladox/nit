#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "munit/munit.h"

#define NIT_SHORT_NAMES
#include "../macros.h"
#include "../list.h"
#include "../hset.h"
#include "../hmap.h"
#include "../gap-buf.h"
#include "../gc.h"
#include "../radix.h"

static void
hmap_free_contents(void *key, void *storage)
{
	free(storage);
}

static MunitResult
test_hmap(const MunitParameter params[], void* data)
{
	Nit_hmap_iter iter;
	Nit_hmap *map = hmap_new(2);
	int i = 0;

	(void) params;
	(void) data;

	for (; i < 500; ++i) {
		int *storage = malloc(sizeof(i));

		*storage = i;
	        hmap_add(map, &i, sizeof(i), storage);
	}

	for (i = 0; i < 500; ++i)
		munit_assert_int(i, ==,
				 *(int *) hmap_get(map, &i, sizeof(i)));

	hmap_iter_init(&iter, map);
	i = 0;

        do {
		int *key = hmap_iter_key(&iter);
		int *val = hmap_iter_val(&iter);

		++i;
		munit_assert_int(*key, ==, *val);
	} while (hmap_iter_next(&iter));

	munit_assert_int(i, ==, 500);

	i = 42;
        free(hmap_remove(map, &i, sizeof(i)));
	munit_assert_null(hmap_get(map, &i, sizeof(i)));

        hmap_free(map, hmap_free_contents);
	return MUNIT_OK;
}

static MunitResult
test_gap_buf(const MunitParameter params[], void* data)
{
	Nit_gap clone;
	static const char str1[] = "hello, world";
	char str2[ARRAY_UNITS(str1)];
	static const char str3[] = " plus lots and lots and lots of words";
	char str4[ARRAY_UNITS(str3)];

	static const char str5[] =
		"hello, world"
		" plus lots and lots and lots of words";
	char str6[ARRAY_UNITS(str5)];
	char *str7;

	Nit_gap gap;
	char clone_str[ARRAY_UNITS(str5)];

	munit_assert_false(gap_init(&gap, sizeof(str1)));

	munit_assert_false(gap_write(&gap, str1, sizeof(str1)));
	gap_read(&gap, str2);
	munit_assert_string_equal(str2, str1);

	/* gap_print(&gap); */

	/* Reset buffer */
	gap_empty(&gap);

	/* Check same with a buffer increase after empty */
	munit_assert_false(gap_write(&gap, str3, sizeof(str3)));
	gap_read(&gap, str4);
	munit_assert_string_equal(str4, str3);

	/* gap_print(&gap); */

	/* Reset buffer in another way */
	munit_assert_false(nit_gap_erase_b(&gap, sizeof(str4)));

	/* Try adding strings together */
	munit_assert_false(gap_write(&gap, str1, sizeof(str1) - 1));
	munit_assert_false(gap_write(&gap, str3, sizeof(str3) - 1));
        gap_read_str(&gap, str6);
	munit_assert_string_equal(str6, str5);

	/* Check using another way to get string. */
	munit_assert_not_null(str7 = gap_str(&gap));
	munit_assert_string_equal(str7, str5);

	/* Tries cloning */
        munit_assert_false(gap_clone(&clone, &gap));
	gap_read_str(&clone, clone_str);
	munit_assert_string_equal(clone_str, str5);

	/* gap_print(&gap); */
	free(str7);
	gap_dispose(&gap);

	return MUNIT_OK;
}

/* typedef struct { */
/* } iter; */

void *
next(void *scan, void *iter)
{
	return NULL;
}

static MunitResult
test_gc(const MunitParameter params[], void* data)
{
	Nit_gc *gc = gc_new(NULL, next);
	int *val = gc_malloc(gc, sizeof(int));
	int *val2 = gc_calloc(gc, sizeof(int));

	/* munit_assert_not_null(val); */
	munit_assert_int(*val2, ==, 0);

	munit_assert_int(gc_free(gc), ==, 1);

	munit_assert_int(gc_scan_1(gc), ==, 0);
	munit_assert_int(gc_scan_1(gc), ==, 1);
	munit_assert_int(gc_restart(gc, NULL), ==, 0);
	munit_assert_not_null((val = gc_collect_1(gc)));
	gc_reclaim(gc, val);
	munit_assert_not_null((val = gc_collect_1(gc)));
	gc_reclaim(gc, val);

	munit_assert_int(gc_free(gc), ==, 0);

	return MUNIT_OK;
}

static MunitResult
test_radix(const MunitParameter params[], void* data)
{
	Nit_radix *radix = radix_new(NULL);

	radix_insert(radix, "firs",     &(int){ 3 });
	radix_insert(radix, "first",    &(int){ 1 });
	radix_insert(radix, "second",   &(int){ 2 });
	radix_insert(radix, "secs",     &(int){ 4 });
	radix_insert(radix, "secoms",   &(int){ 5 });
	radix_insert(radix, "a",        &(int){ 6 });
	radix_insert(radix, "absolute", &(int){ 7 });
	radix_insert(radix, "bottle",   &(int){ 8 });
	radix_insert(radix, "b",        &(int){ 9 });

	munit_assert_int(1, ==, *(int *) radix_lookup(radix, "first"));
	munit_assert_int(2, ==, *(int *) radix_lookup(radix, "second"));
	munit_assert_int(3, ==, *(int *) radix_lookup(radix, "secs"));
	munit_assert_int(4, ==, *(int *) radix_lookup(radix, "secoms"));
	munit_assert_int(5, ==, *(int *) radix_lookup(radix, "firs"));
	munit_assert_int(6, ==, *(int *) radix_lookup(radix, "a"));
	munit_assert_int(7, ==, *(int *) radix_lookup(radix, "absolute"));
	munit_assert_int(8, ==, *(int *) radix_lookup(radix, "bottle"));
	munit_assert_int(9, ==, *(int *) radix_lookup(radix, "b"));
}

static MunitTest test_suite_tests[] = {
	{ (char *) "/hmap", test_hmap,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/gap-buf", test_gap_buf,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/gc", test_gc,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }

};

static const MunitSuite test_suite = {
	(char*) "nit",
	test_suite_tests,
	NULL,
	1,
	MUNIT_SUITE_OPTION_NONE
};

int
main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
	/* test_bimap(NULL, NULL); */
	/* test_hmap(NULL, NULL); */
	/* test_gap_buf(NULL, NULL); */
	/* test_gc(NULL, NULL); */

	return munit_suite_main(&test_suite, NULL, argc, argv);
}
