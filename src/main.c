#define IMPLEMENT_BUILD_H
#include "../build.h"

typedef struct {
	uint32_t pair[2];
} pair_key_t;

typedef struct {
	pair_key_t key;
	int value;
} pair_t;

pair_t *map = NULL;
uint32_t *tokens = NULL;

int qsort_compare(const void *a, const void *b)
{
	return ((pair_t*)a)->value < ((pair_t*)b)->value;
}

int main(void)
{
	tokens = init_darray(tokens, 8, sizeof(uint32_t));
	map = init_hm(map, 4, sizeof(pair_t), fnv_1a_hash, cmp_hash, 32);
	const char *data = read_file("input.txt");

	for (size_t i = 0; i < strlen(data); ++i)
	{
		darray_push(tokens, data[i]);
	}

	for (int i = 0; i < darray_len(tokens) - 1; ++i)
	{
		pair_t kv = { .key = { data[i], data[i + 1] } };
		hm_get(map, (&kv));
		kv.value++;
		hm_put(map, kv);
	}

	qsort(map, hm_len(map), sizeof(pair_t), qsort_compare);

	for (int i = 0; i < 10; ++i)
	{
		printf("(%d, %d) -> %d\n", map[i].key.pair[0], map[i].key.pair[1], map[i].value);
	}

	hm_free(map);

	return 0;
}
