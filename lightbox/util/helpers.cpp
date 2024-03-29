#include "helpers.h"

#include <climits>
#include <fstream>
#include <iterator>
#include <thread>
#include <chrono>

#include "../ui/ui.h"

namespace helpers {
	bool directory_exists(std::string path)
	{
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
            return true;
        } else {
            return false;
        }
	}

    bool file_exists(std::string path)
    {
        if (std::filesystem::is_directory(path)) return false;
        return std::filesystem::exists(path);
    }

    std::vector<unsigned char> str_to_bytes(const std::string& str) 
    {
        return std::vector<unsigned char>(str.begin(), str.end());
    }

    std::string bytes_to_str(const std::vector<unsigned char>& bytes)
    {
        return std::string(bytes.begin(), bytes.end());
    }

    bool is_digit(const std::string& s)
    {
        for (char c : s) {
            if (!isdigit(c)) {
                return false;
            }
        }

        return true;
    }

    WORD int_to_word(int val)
    {
        if (val >= 0 && val <= USHRT_MAX)
        {
            WORD word_val = static_cast<WORD>(val);
            return word_val;
        }
        else {
            return (WORD)65000;
        }
    }

    std::string file_to_str(const std::string& filepath) 
    {
        std::ifstream ifs(filepath);
        return std::string((std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>());
    }

    void str_to_file(const std::string& filename, const std::string& content) 
    {
        std::fstream file(filename, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            return;
        }

        file << content;
        file.close();
    }

    std::string b64_encode(const std::string& input) {
        static const char* b64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string output;
        int val = 0, valb = -6;
        for (unsigned char c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                output.push_back(b64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) output.push_back(b64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (output.size() % 4) output.push_back('=');
        return output;
    }

    std::vector<unsigned char> add_padding(const std::vector<unsigned char>& data) {
        size_t padding_size = 16 - (data.size() % 16);
        std::vector<unsigned char> padded_data(data);
        padded_data.resize(data.size() + padding_size);
        std::fill(padded_data.end() - padding_size, padded_data.end(), static_cast<unsigned char>(padding_size));
        return padded_data;
    }

    std::vector<unsigned char> remove_padding(const std::vector<unsigned char>& data) {
        if (data.empty()) {
            throw std::runtime_error("Data is empty.");
        }
        unsigned char padding_size = data.back();
        if (padding_size > data.size()) {
            return std::vector<unsigned char>{{' '}};
        }

        std::vector<unsigned char> unpadded_data(data.begin(), data.end() - padding_size);
        
        return unpadded_data;
    }

    std::string tchar_to_utf8(TCHAR* tchar_arr) {
        std::vector<char> buffer;
        int size = WideCharToMultiByte(CP_UTF8, 0, tchar_arr, -1, NULL, 0, NULL, NULL);
        if (size > 0) {
            buffer.resize(size);
            WideCharToMultiByte(CP_UTF8, 0, tchar_arr, -1, &buffer[0], buffer.size(), NULL, NULL);
        }
        
        return std::string(&buffer[0]);
    }

    void to_clipboard(const std::string& s) {
        if (!OpenClipboard(NULL)) {
            return;
        }
        if (!EmptyClipboard()) {
            CloseClipboard();
            return;
        }
        HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
        if (!hg) {
            CloseClipboard();
            return;
        }
        memcpy(GlobalLock(hg), s.c_str(), s.size() + 1);
        GlobalUnlock(hg);
        SetClipboardData(CF_TEXT, hg);
        CloseClipboard();
        GlobalFree(hg);
    }

    std::vector<unsigned char> file_to_vec(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            //std::cerr << "Error opening file: " << filename << std::endl;
            return std::vector<unsigned char>();
        }

        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<unsigned char> data(size);
        file.read(reinterpret_cast<char*>(data.data()), size);

        return data;
    }

    void bytes_to_file(const char* file_path, std::vector<unsigned char>& bytes) {
        std::ofstream file(file_path, std::ios::out | std::ios::binary);
        file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }

    void clear_clipboard()
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        if (OpenClipboard(NULL)) {
            EmptyClipboard();
            CloseClipboard();
        }
    }
}