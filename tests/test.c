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
#include "../bimap.h"
#include "../gap-buf.h"
#include "../gc.h"
#include "../ftree.h"

static void
hmap_free_contents(void *key, void *storage)
{
	free(storage);
}

static MunitResult
test_hmap(const MunitParameter params[], void* data)
{
	Nit_hmap *map = hmap_new(2);
	int i = 0;

	(void) params;
	(void) data;

	for (; i <= 500; ++i) {
		int *storage = malloc(sizeof(i));

		*storage = i;
	        hmap_add(map, &i, sizeof(i), storage);
	}

	for (i = 0; i <= 500; ++i)
		munit_assert_int(i, ==,
				 *(int *) hmap_get(map, &i, sizeof(i)));

	i = 42;
        free(hmap_remove(map, &i, sizeof(i)));
	munit_assert_null(hmap_get(map, &i, sizeof(i)));

        hmap_free(map, hmap_free_contents);
	return MUNIT_OK;
}

static void
bimap_free_contents(void *key, void *storage)
{
	Nit_entry_list *list = storage;
	Nit_entry_list *prev = NULL;

	nit_foreach (list) {
		free(prev);
		prev = list;
	}

	free(prev);
}

static MunitResult
test_bimap(const MunitParameter params[], void* data)
{
	Nit_bimap *map = bimap_new(2, 0);
	char str1[] = "cats";
	int int1 = 5;
	char str2[] = "dogs";
	int int2 = 32;
	int int3 = 42;
	char str3[] = "not used";
	Nit_entry_list *entry;

	(void) params;
	(void) data;

	bimap_add(map, str1, sizeof(str1), &int1, sizeof(int1));

	bimap_add(map, str2, sizeof(str2), &int2, sizeof(int2));
	bimap_add(map, str2, sizeof(str2), &int3, sizeof(int3));

	entry = bimap_lget(map, str1, sizeof(str1));
	munit_assert_int(int1, ==, *(int *) entry->entry->dat);

	entry = bimap_rget(map, &int2, sizeof(int2));
	munit_assert_string_equal(entry->entry->dat, str2);

	entry = bimap_lget(map, str2, sizeof(str2));
	munit_assert_int(int3, ==, *(int *) entry->entry->dat);
	entry = NIT_LIST_NEXT(entry);
	munit_assert_int(int2, ==, *(int *) entry->entry->dat);

	munit_assert_null(bimap_lget(map, str3, sizeof(str3)));

        bimap_free(map, bimap_free_contents, bimap_free_contents);
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

/* void */
/* print_ftree(Nit_ftree *tree) */
/* { */
/*         int i; */

/* 	foreach (tree) { */
/* 		printf("\n%i %i\n", tree->precnt, tree->sufcnt); */
/* 		for (i = 0; i < FTREE_BS; ++i) */
/* 			if (tree->pre[i]) */
/* 				printf("p"); */
/* 			else */
/* 				printf("x"); */

/* 		printf("\n"); */

/* 		for (i = 0; i < FTREE_BS; ++i) */
/* 			if (tree->suf[i]) { */
/* 				printf("s"); */
/* 				/\* printf("\n%i %i %i\n", tree->precnt, tree->sufcnt, *\/ */
/* 				/\*        *(int *) tree->suf[i]); *\/ */
/* 			} */
/* 			else */
/* 				printf("x"); */
/* 		printf("\n"); */
/* 		printf("\n"); */
/* 	} */

/* } */

/* void */
/* print_ftree_refs(Nit_ftree *tree) */
/* { */
/*         int i; */

/* 	foreach (tree) { */
/* 		printf("tree: %" PRIu32 "\n", tree->refs); */
/* 		if (tree->depth) { */
/* 			for (i = 0; i < FTREE_BS; ++i) */
/* 				if (tree->pre[i]) */
/* 					printf("\tp: %" PRIu32 "\n", */
/* 					       ((Nit_fbnch *) tree->pre[i])->refs); */

/* 			for (i = 0; i < FTREE_BS; ++i) */
/* 				if (tree->suf[i]) */
/* 					printf("\ts: %" PRIu32 "\n", */
/* 					       ((Nit_fbnch *) tree->suf[i])->refs); */
/* 		} */
/* 	} */

/* 	printf("\n"); */
/* } */

static MunitResult
test_ftree(const MunitParameter params[], void* data)
{
	Nit_ftree *tree = ftree_new();
	Nit_ftree *tree2;
	Nit_ftree *tree3;
	Nit_ftree *tree4;
        int *val;
	int i = 0;
	int num = 5;
	int num2 = 42;
	int num3 = 100;

	munit_assert_true(ftree_prepend(tree, &num, 0));
	munit_assert_int(*(int *) ftree_first(tree), ==, 5);

	for (; i < 299; ++i)
		munit_assert_true(ftree_prepend(tree, &num, 0));

	munit_assert_true(ftree_prepend(tree, &num2, 0));
	munit_assert_int(*(int *) ftree_first(tree), ==, 42);
	munit_assert_int(*(int *) ftree_pop(tree, 0), ==, 42);

	tree2 = ftree_copy(tree, 0);

	for (i = 0; val = ftree_pop(tree, 0); ++i)
		munit_assert_int(*val, ==, 5);

	munit_assert_int(i, ==, 300);

	munit_assert_true(ftree_append(tree, &num2, 0));
	munit_assert_true(ftree_append(tree, &num, 0));
	munit_assert_int(*(int *) ftree_rpop(tree, 0), ==, 5);

	for (i = 0; val = ftree_pop(tree2, 0); ++i)
		munit_assert_int(*val, ==, 5);

	munit_assert_int(i, ==, 300);

	munit_assert_true(ftree_prepend(tree, &num2, 0));
	munit_assert_true(ftree_prepend(tree2, &num, 0));
	munit_assert_not_null(tree3 = ftree_concat(tree, tree2));

	/* print_ftree(tree); */
	/* print_ftree(tree2); */
	/* print_ftree(tree3); */
	/* print_ftree_refs(tree3); */
	munit_assert_int(*(int *) ftree_first(tree3), ==, 42);
	munit_assert_int(*(int *) ftree_last(tree3), ==, 5);

	for (i = 0; i < 299; ++i)
		munit_assert_true(ftree_prepend(tree, &num, 0));

	for (i = 0; i < 299; ++i)
		munit_assert_true(ftree_append(tree3, &num3, 0));

	munit_assert_not_null(tree4 = ftree_concat(tree, tree3));

	/* print_ftree(tree4); */
	/* print_ftree_refs(tree4); */

	munit_assert_int(*(int *) ftree_first(tree4), ==, 5);
	munit_assert_int(*(int *) ftree_last(tree4), ==, 100);

        ftree_reduce(tree, 0);
	ftree_reduce(tree2, 0);
	ftree_reduce(tree3, 0);
	ftree_reduce(tree4, 0);

	return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
	{ (char *) "/hmap", test_hmap,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/bimap", test_bimap,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/gap-buf", test_gap_buf,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/gc", test_gc,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/ftree", test_ftree,
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
	/* Nit_ftree f; */
	/* Nit_fbnch b; */
	/* printf("%zu, %zu, %zu\n", sizeof(Nit_ftree), sizeof(f.pre), */
	/*        sizeof(f) - (sizeof(f.pre) + sizeof(f.suf))); */
	/* printf("%zu, %zu, %zu\n", sizeof(b), sizeof(b.elems), */
	/*        sizeof(b.refs)); */
	/* test_ftree(NULL, NULL); */
	return munit_suite_main(&test_suite, NULL, argc, argv);
}
