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

int cmp(const void *a, const void *b, size_t size)
{
	uint64_t p = *(uint64_t*)a,
					 q = *(uint64_t*)b;

	return (p & q) != 0;
}

uint64_t hash(const void *key, size_t len, uint32_t seed)
{
	uint64_t _key = *(uint64_t*)key;
	return (_key >> seed) | (_key << (len - seed));
}

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

void log_bench_result(clock_t start, clock_t end, const char *name, size_t iteration)
{
	double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
	double ops_per_sec = iteration / cpu_time_used;
	double avg_time_per_op = cpu_time_used / iteration;
	INFO("%d %s in %f secs, %.2f ns/op, %.0f op/sec", iteration, name, cpu_time_used, avg_time_per_op * 1e9, ops_per_sec);
}

void report_progress(size_t iteration, uint32_t *tokens_in, pair_t *pairs, double *profile_samples, size_t profile_samples_count)
{
	double average_profile_samples = 0.0f;
	for (size_t i = 0; i < profile_samples_count; ++i) {
		average_profile_samples += profile_samples[i];
	}
	average_profile_samples /= profile_samples_count;

	printf("INFO: -- ITERATION %zu --\n", iteration);
	printf("INFO:   Token count: %zu\n", darray_len(tokens_in));
	printf("INFO:   Pair count:  %zu\n", darray_len(pairs));
	printf("INFO:   Time:        %lfsecs (avg. of %zu iter.)\n", average_profile_samples, profile_samples_count);
}

double get_time(void)
{
	struct timespec tp = { 0 };
	int ret = clock_gettime(CLOCK_MONOTONIC, &tp);
	if (!(ret == 0)) perror("failed to get time: "), exit(1);
	return (double)tp.tv_sec + (double)tp.tv_nsec * 1e-9;
}

int main(void)
{
	const char *text = read_file("skspear.txt");
	const size_t text_size = strlen(text);

	freq_t *freqs = NULL;
	pair_t *pairs = NULL;
	uint32_t *tokens_in = NULL;
	uint32_t *tokens_out = NULL;

	freqs = init_hm(freqs, 2, sizeof(freq_t), hash, cmp, 5186);
	pairs = init_darray(pairs, 4, sizeof(pair_t));
	tokens_in = init_darray(tokens_in, 4, sizeof(uint32_t));
	tokens_out = init_darray(tokens_out, 4, sizeof(uint32_t));

	for (uint32_t i = 0; i < 256; ++i)
	{
		darray_push(pairs, ((pair_t) { .l = i }));
	}

	for (size_t i = 0; i < text_size; ++i)
	{
		darray_push(tokens_in, text[i]);
	}

	double start, end;

	size_t iteration = 0;
	size_t total_iteration_dump = 10;
	size_t max_iteration = 1000;

	double *profile_samples = malloc(max_iteration * sizeof(double));
	size_t profile_samples_count = 0;

	for (; iteration < max_iteration; iteration++)
	{
		start = get_time();

		if (iteration % total_iteration_dump == 0) {
			report_progress(iteration, tokens_in, pairs, profile_samples, total_iteration_dump);
		}

		for (size_t i = 0; i < darray_len(tokens_in) - 1; ++i)
		{
			pair_t pair = {
				.l = tokens_in[i],
				.r = tokens_in[i + 1]
			};
			long int i = hm_geti(freqs, ((freq_t) { .key = pair }));
			if (i < 0) {
				hm_put(freqs, ((freq_t) { .key = pair, .value = 1 }));
			}
			else {
				freqs[i].value++;
			}
		}

		long int max_index = 0;
		for (long int i = 1; i < hm_len(freqs); ++i) {
			if (freqs[i].value > freqs[max_index].value || (freqs[i].value == freqs[max_index].value && memcmp(&freqs[i].key, &freqs[max_index].key, sizeof(freqs[i].key)) > 0))
				max_index = i;
		}

		if (freqs[max_index].value <= 1) break;

		pair_t max_pair = freqs[max_index].key;
		uint32_t max_token = darray_len(pairs);
		darray_push(pairs, max_pair);

		darray_reset(tokens_out);
		for (size_t i = 0; i < darray_len(tokens_in);) {

			if (i + 1 >= darray_len(tokens_in)) {
				darray_push(tokens_out, tokens_in[i]);
				i += 1;
				break;
			}

			pair_t pair = {
				.l = tokens_in[i],
				.r = tokens_in[i + 1]
			};

			if (memcmp(&pair, &max_pair, sizeof(pair)) == 0) {
				long int place;
				if (darray_len(tokens_out) > 0) {
					pair.l = tokens_out[darray_len(tokens_out) - 1];

					pair.r = tokens_in[i];
					place = hm_geti(freqs, ((freq_t) { .key = pair }));
					if (!(place >= 0)) {
						printf("%s:%d: i = %zu, pair = (%u, %u)\n", __FILE__, __LINE__, i, pair.l, pair.r);
						exit(1);
					}
					if (!(freqs[place].value > 0)) exit(1);
					freqs[place].value -= 1;

					pair.r = max_token;
					place = hm_geti(freqs, ((freq_t) { .key = pair }));
					if (place < 0) hm_put(freqs, ( (freq_t) { .key = pair, .value = 1 } ));
					else freqs[place].value += 1;
				}

				pair = max_pair;
				place = hm_geti(freqs, ( (freq_t) { .key = pair } ));
				if (!(place >= 0)) exit(1);
				if (!(freqs[place].value > 0)) exit(1);
				freqs[place].value -= 1;

				darray_push(tokens_out, max_token);
				i += 2;

				if (i < darray_len(tokens_in)) {
					pair.r = tokens_in[i];

					pair.l = tokens_in[i - 1];
					place = hm_geti(freqs, ( (freq_t) { .key = pair } ));
					if (!(place >= 0)) exit(1);
					if (!(freqs[place].value > 0)) exit(1);
					freqs[place].value -= 1;

					pair.l = tokens_out[darray_len(tokens_out) - 1];
					place = hm_geti(freqs, ((freq_t) { .key = pair }));
					if (place < 0) hm_put(freqs, ((freq_t) { .key = pair, .value = 1 }));
					else freqs[place].value += 1;
				}
			} else {
				darray_push(tokens_out, tokens_in[i]);
				i += 1;
			}
		}

		SWAP(uint32_t *, tokens_in, tokens_out);
		profile_samples[iteration%total_iteration_dump] = get_time() - start;
	}
	render_tokens(pairs, tokens_out);

	// free
	hm_free(freqs);
	darray_free(pairs);
	darray_free(tokens_in);
	darray_free(tokens_out);
	free((void*)text);

	return 0;
}
