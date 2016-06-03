#include <stdint.h>
#include <stdlib.h>

#include "munit/munit.h"

#define NIT_SHORT_NAMES
#include "../list.h"
#include "../hashmap.h"
#include "../bimap.h"

static int
hashmap_compare(const void *entry_key, uint32_t entry_key_size,
	const void *key, uint32_t key_size)
{
	return (*(int *) entry_key) == (*(int *) key);
}

static void
hashmap_free_contents(void *key, void *storage)
{
	free(key);
	free(storage);
}

static MunitResult
test_hashmap(const MunitParameter params[], void* data)
{
	struct nit_hashmap *map = hashmap_new(2, hashmap_compare,
					      hashmap_free_contents);
	int i = 0;

	(void) params;
	(void) data;

	for (; i <= 500; ++i) {
		int *key = malloc(sizeof(i));
		int *storage = malloc(sizeof(i));

		*storage = (*key = i);
	        hashmap_add(map, key, sizeof(*key), storage);
	}

	for (i = 0; i <= 500; ++i)
		munit_assert_int(i, ==,
				 *(int *) hashmap_get(map, &i, sizeof(i)));

	i = 42;
        hashmap_remove(map, &i, sizeof(i));
	munit_assert_null(hashmap_get(map, &i, sizeof(i)));

        hashmap_free(map);
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
	struct nit_entry_list *list = storage;
	struct nit_entry_list *prev = NULL;

	nit_foreach (list) {
		free(prev);
		prev = list;
	}

	free(prev);
}

static MunitResult
test_bimap(const MunitParameter params[], void* data)
{
	struct nit_bimap *map =
		bimap_new(2, bimap_lcompare, bimap_free_contents,
			  0, bimap_rcompare, bimap_free_contents);
	char str1[] = "cats";
	int int1 = 5;
	char str2[] = "dogs";
	int int2 = 32;
	int int3 = 42;
	char str3[] = "not used";
	struct nit_entry_list *entry;

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

static MunitTest test_suite_tests[] = {
	{ (char *) "/hashmap", test_hashmap,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ (char *) "/bimap", test_bimap,
	  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }

};

static const MunitSuite test_suite = {
	(char*) "nitlib",
	test_suite_tests,
	NULL,
	1,
	MUNIT_SUITE_OPTION_NONE
};

int
main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
	/* test_bimap(NULL, NULL); */
	/* test_hashmap(NULL, NULL); */
	return munit_suite_main(&test_suite, NULL, argc, argv);
}
