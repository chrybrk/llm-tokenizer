#define IMPLEMENT_BUILD_H
#define BUILD_ITSELF
#include "build.h"

const char *build_bin = "build";
const char *build_source = "build.c";

int main(int argc, char **argv)
{
	create_directory("bin/");

	const char **files = get_files("src/");
	if (is_binary_old("bin/main", files))
	{
		const char **source_files_array = get_files_with_specific_ext("src/", ".c");
		const char *source_files = string_list_to_const_string(source_files_array, darray_len(source_files_array), ' ');
		if (!execute(formate_string("cc -o %s %s -Isrc/ -Wall -Wextra", "bin/main", source_files)))
			ERROR("failed to compile."), exit(1);
	}

	execute("./bin/main");

	return 0;
}
