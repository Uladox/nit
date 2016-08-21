#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "munit/munit.h"

#define NIT_SHORT_NAMES
#include "../macros.h"
#include "../list.h"
#include "../hmap.h"
#include "../bimap.h"
#include "../gap-buf.h"

static int
hmap_compare(const void *entry_key, uint32_t entry_key_size,
	     const void *key, uint32_t key_size)
{
	return (*(int *) entry_key) == (*(int *) key);
}

static void
hmap_free_contents(void *key, void *storage)
{
	free(key);
	free(storage);
}

static MunitResult
test_hmap(const MunitParameter params[], void* data)
{
	Nit_hmap *map = hmap_new(2, hmap_compare,
					      hmap_free_contents);
	int i = 0;

	(void) params;
	(void) data;

	for (; i <= 500; ++i) {
		int *key = malloc(sizeof(i));
		int *storage = malloc(sizeof(i));

		*storage = (*key = i);
	        hmap_add(map, key, sizeof(*key), storage);
	}

	for (i = 0; i <= 500; ++i)
		munit_assert_int(i, ==,
				 *(int *) hmap_get(map, &i, sizeof(i)));

	i = 42;
        hmap_remove(map, &i, sizeof(i));
	munit_assert_null(hmap_get(map, &i, sizeof(i)));

        hmap_free(map);
	return MUNIT_OK;
}

static int
bimap_lcompare(const void *entry_key, uint32_t entry_key_size,
	       const void *key, uint32_t key_size)
{
	return !strcmp(entry_key, key);
}

static int
bimap_rcompare(const void *entry_key, uint32_t entry_key_size,
	       const void *key, uint32_t key_size)
{
	return (*(int *) entry_key) == (*(int *) key);
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
	Nit_bimap *map =
		bimap_new(2, bimap_lcompare, bimap_free_contents,
			  0, bimap_rcompare, bimap_free_contents);
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
	munit_assert_int(int1, ==, *(int *) entry->entry->key);

	entry = bimap_rget(map, &int2, sizeof(int2));
	munit_assert_string_equal(entry->entry->key, str2);

	entry = bimap_lget(map, str2, sizeof(str2));
	munit_assert_int(int3, ==, *(int *) entry->entry->key);
	entry = NIT_LIST_NEXT(entry);
	munit_assert_int(int2, ==, *(int *) entry->entry->key);

	munit_assert_null(bimap_lget(map, str3, sizeof(str3)));

        bimap_free(map);
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

static MunitTest test_suite_tests[] = {
	{ (char *) "/hmap", test_hmap,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/bimap", test_bimap,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/gap-buf", test_gap_buf,
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
	return munit_suite_main(&test_suite, NULL, argc, argv);
}
