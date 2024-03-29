#include "pw.h"

#include <random>

namespace pw {
	std::string random_pw(int allowed_chars_opt, const int pw_length)
	{
		std::string uppercase_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()/\"{}-_+?~^`
		std::string lowercase_chars = "abcdefghijklmnopqrstuvwxyz";
		std::string number_chars = "1234567890";
		std::string special_chars = "!@#$%^&*()/\"{}-_+?~^`";

		std::string allowed_chars = "";

		switch (allowed_chars_opt)
		{
			case options::ALL_CHARS: { 
				allowed_chars = uppercase_chars + lowercase_chars + number_chars + special_chars;
			}break;
			case options::ONLY_LETTERS_ALL: {
				allowed_chars = uppercase_chars + lowercase_chars;
			}break;
			case options::ONLY_LETTERS_LC: {
				allowed_chars = lowercase_chars;
			}break;
			case options::ONLY_LETTERS_UC: {
				allowed_chars = uppercase_chars;
			}break;
			case options::ONLY_NUMBERS_LETTERS: {
				allowed_chars = uppercase_chars + lowercase_chars + number_chars;
			}break;
		}

		size_t num_chars = allowed_chars.size() - 1;

		std::random_device device;
		std::seed_seq seq{ device(), device(), device(), device() }; 
		std::mt19937_64 eng(seq);

		std::uniform_int_distribution<> distr(0, (int)num_chars - 1);

		std::string pw;
		for (int i = 0; i < pw_length; i++)
		{
			pw += allowed_chars[distr(eng)];
		}

		return pw;
	}
}