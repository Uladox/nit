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
/* #include "../gc.h" */
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

/* static MunitResult */
/* test_gc(const MunitParameter params[], void* data) */
/* { */
/* 	Nit_gc *gc = gc_new(NULL, next); */
/* 	int *val = gc_malloc(gc, sizeof(int)); */
/* 	int *val2 = gc_calloc(gc, sizeof(int)); */

/* 	/\* munit_assert_not_null(val); *\/ */
/* 	munit_assert_int(*val2, ==, 0); */

/* 	munit_assert_int(gc_free(gc), ==, 1); */

/* 	munit_assert_int(gc_scan_1(gc), ==, 0); */
/* 	munit_assert_int(gc_scan_1(gc), ==, 1); */
/* 	munit_assert_int(gc_restart(gc, NULL), ==, 0); */
/* 	munit_assert_not_null((val = gc_collect_1(gc))); */
/* 	gc_reclaim(gc, val); */
/* 	munit_assert_not_null((val = gc_collect_1(gc))); */
/* 	gc_reclaim(gc, val); */

/* 	munit_assert_int(gc_free(gc), ==, 0); */

/* 	return MUNIT_OK; */
/* } */

#define r_insert_int(radix, str, val)		\
	radix_insert(radix, str, sizeof(str), &(int){ val })

#define r_assert_int_lookup(radix, str, val)				\
	munit_assert_int(val, ==,					\
			 *(int *) radix_lookup(radix, str, sizeof(str)))

static MunitResult
test_radix(const MunitParameter params[], void* data)
{
	Nit_radix *radix = radix_new(NULL);
	Nit_radix_iter iter;

	int a[8] = { 233, 2341, 0, 1234, 754, 0, 34, 87 };
	int b[4] = { 233, 2341, 0, 1234 };
	radix_iter_init(&iter, radix);

	r_insert_int(radix, "firs",     3);
	r_insert_int(radix, "first",    1);
	r_insert_int(radix, "second",   2);
	r_insert_int(radix, "secs",     4);
	r_insert_int(radix, "secoms",   5);
	r_insert_int(radix, "a",        6);
	r_insert_int(radix, "absolute", 7);
	r_insert_int(radix, "bottle",   8);
	r_insert_int(radix, "b",        9);

	radix_insert(radix, a, sizeof(a), &(int){ 10 });
	radix_insert(radix, b, sizeof(b), &(int){ 11 });

	r_assert_int_lookup(radix, "first",    1);
	r_assert_int_lookup(radix, "second",   2);
	r_assert_int_lookup(radix, "secs",     4);
	r_assert_int_lookup(radix, "secoms",   5);
	r_assert_int_lookup(radix, "firs",     3);
	r_assert_int_lookup(radix, "a",        6);
	r_assert_int_lookup(radix, "absolute", 7);
	r_assert_int_lookup(radix, "bottle",   8);
	r_assert_int_lookup(radix, "b",        9);

	munit_assert_int(10, ==, *(int *) radix_lookup(radix, a, sizeof(a)));
	munit_assert_int(11, ==, *(int *) radix_lookup(radix, b, sizeof(b)));

	radix_iter_move(&iter, "f", 1);
	radix_iter_move(&iter, "ir", 2);
	radix_iter_move(&iter, "st", 3);

	munit_assert_int(1, ==, *(int *) radix_iter_get(&iter));
	munit_assert_int(1, ==, radix_iter_move(&iter, "&", 1));

	return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
	{ (char *) "/hmap", test_hmap,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/gap-buf", test_gap_buf,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/radix", test_radix,
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
	/* test_radix(NULL, NULL); */

	return munit_suite_main(&test_suite, NULL, argc, argv);
}
