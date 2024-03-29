#include "enc.h"

#include <iostream>
#include <array>
#include <algorithm>
#include <vector>

#include "../../util/helpers.h"
#include "../../cfg/cfg.h"
#include "argon2/argon2.h"
#include "AES/AES.h"

namespace enc {
    std::pair<std::string, std::array<uint8_t, 16>> salt_from_password(const std::string& password) {
        uint8_t hash[32];
        uint8_t salt[16] = { 0 };

        uint32_t t_cost = 2;
        uint32_t m_cost = (1 << 16);
        uint32_t parallelism = 1;

        int result = argon2id_hash_raw(t_cost, m_cost, parallelism, (uint8_t*)password.c_str(), password.size(), salt, 16, hash, 32);

        if (result != ARGON2_OK) {
            std::cerr << "error hashing\n";
            return { "", std::array<uint8_t, 16>{0} };
        }

        std::array<uint8_t, 16> new_salt;
        std::copy(hash, hash + 16, new_salt.begin());

        std::string hashed;
        char buffer[3];
        for (size_t i = 0; i < 32; ++i) {
            sprintf_s(buffer, sizeof(buffer), "%02x", hash[i]);
            hashed += buffer;
        }

        return { hashed, new_salt };
    }

	std::string hash_password(const std::string& password)
	{
        std::pair<std::string, std::array<uint8_t, 16>> result = salt_from_password(password);
        std::array<uint8_t, 16> salt = result.second;

        uint8_t hash[32];
        uint32_t t_cost = 2;
        uint32_t m_cost = (1 << 16);
        uint32_t parallelism = 1;

        argon2id_hash_raw(t_cost, m_cost, parallelism, (uint8_t*)password.c_str(), password.size(), salt.data(), salt.size(), hash, 32);

        std::string hashed;
        char buffer[3];
        for (size_t i = 0; i < 32; ++i) {
            sprintf_s(buffer, sizeof(buffer), "%02x", hash[i]);
            hashed += buffer;
        }

        return hashed;
	}

    void write_enc_file(const std::string& hash, std::string content)
    {
        std::vector<unsigned char> key(hash.begin(), hash.end());

        std::vector<unsigned char> pt_v(content.begin(), content.end());

        std::vector<unsigned char> pt_padded = helpers::add_padding(pt_v);
        int pt_padded_len = pt_padded.size();

        std::string iv = "1234567890123456";
        std::vector<unsigned char> iv_v(iv.begin(), iv.end());

        AES aes;
        std::vector<unsigned char> cipher = aes.EncryptCBC(pt_padded, key, iv_v);

        std::string file_content(cipher.begin(), cipher.end());

        helpers::bytes_to_file(cfg::_path.c_str(), cipher);
    }

    std::string decrypt_enc_file(std::string path, std::string hash)
    {
        std::string path_r = path;
        std::vector<unsigned char> ct_v = helpers::file_to_vec(path);

        std::vector<unsigned char> key(hash.begin(), hash.end());

        std::string iv = "1234567890123456";
        std::vector<unsigned char> iv_v(iv.begin(), iv.end());

        AES aes;
        std::vector<unsigned char> pt = aes.DecryptCBC(ct_v, key, iv_v);
        std::vector<unsigned char> pt_unpadded = helpers::remove_padding(pt);

        std::string pt_s(pt_unpadded.begin(), pt_unpadded.end());
        return pt_s;
    }

    //DEBUG
    /* ENCRYPTION
        * grab uint8_t array from hash_pw to derive crypto keys for AES
        * AES encrypt data and write data to a .lbdb file
        */

    /*
    * DECRYPTION
    * read header data from .lbdb file
    * get AES key from hash_pw
    * get checksum
    * voila
    */
}