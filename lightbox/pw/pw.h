#pragma once

#include <string>

namespace pw {
	std::string random_pw(int allowed_chars_opt, const int pw_length);

	namespace options {
		const int ALL_CHARS = 0;
		const int ONLY_LETTERS_ALL = 1;
		const int ONLY_LETTERS_LC = 2;
		const int ONLY_LETTERS_UC = 3;
		const int ONLY_NUMBERS_LETTERS = 4;
	}
}