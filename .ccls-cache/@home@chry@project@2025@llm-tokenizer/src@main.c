#define IMPLEMENT_BUILD_H
#include "../build.h"

typedef typeof((int*)NULL - (int*)NULL) ptrdiff_t;

typedef struct {
	uint32_t l, r;
} pair_t;

typedef struct {
	pair_t key;
	int value;
} freq_t;

int qsort_compare(const void *a, const void *b)
{
	return ((freq_t*)a)->value < ((freq_t*)b)->value;
}

void render_tokens(pair_t *pairs, uint32_t *tokens)
{
	for (size_t i = 0; i < darray_len(tokens); ++i)
	{
		uint32_t token = tokens[i];
		if (token > darray_len(pairs)) return;

		if (pairs[token].l == token)
			printf("%c", token);
		else
			printf("[%u]", token);
	}
	printf("\n");
}

int main(void)
{
	const char *text = read_file("input.txt");
	const size_t text_size = strlen(text);

	freq_t *freqs = NULL;
	pair_t *pairs = NULL;
	uint32_t *tokens_in = NULL;
	uint32_t *tokens_out = NULL;

	freqs = init_hm(freqs, 4, sizeof(freq_t), fnv_1a_hash, cmp_hash, 32);
	pairs = init_darray(pairs, 8, sizeof(pair_t));
	tokens_in = init_darray(tokens_in, 8, sizeof(uint32_t));
	tokens_out = init_darray(tokens_out, 8, sizeof(uint32_t));

	for (uint32_t i = 0; i < 256; ++i)
	{
		darray_push(pairs, ((pair_t) { .l = i }));
	}

	for (size_t i = 0; i < text_size; ++i)
	{
		darray_push(tokens_in, text[i]);
	}

	for (;;)
	{
		printf("%zu\n", darray_len(tokens_out));

		hm_reset(freqs);
		for (size_t i = 0; i < darray_len(tokens_in) - 1; ++i)
		{
			pair_t pair = {
				.l = tokens_in[i],
				.r = tokens_in[i + 1]
			};
			long long int i = hm_geti(freqs, ((freq_t) { .key = pair }));
			if (i < 0) {
				hm_put(freqs, ((freq_t) { .key = pair, .value = 1 }));
			}
			else {
				freqs[i].value++;
			}
		}

		long long int max_index = 0;
		for (long long int i = 1; i < hm_len(freqs); ++i) {
			if (freqs[i].value > freqs[max_index].value)
				max_index = i;
		}

		if (freqs[max_index].value <= 1) break;

		darray_push(pairs, freqs[max_index].key);

		darray_reset(tokens_out);
		for (size_t i = 0; i < darray_len(tokens_in); ) {
			if (i + 1 >= darray_len(tokens_in)) {
				darray_push(tokens_out, tokens_in[i]);
				i += 1;
			} else {
				pair_t pair = {.l = tokens_in[i], .r = tokens_in[i + 1]};
				if (memcmp(&pair, &freqs[max_index].key, sizeof(pair)) == 0) {
					darray_push(tokens_out, darray_len(pairs) - 1);
					i += 2;
				} else {
					darray_push(tokens_out, tokens_in[i]);
					i += 1;
				}
			}
		}

		SWAP(uint32_t *, tokens_in, tokens_out);
	}

	// free
	hm_free(freqs);
	darray_free(pairs);
	darray_free(tokens_in);
	darray_free(tokens_out);
	free((void*)text);

	return 0;
}
