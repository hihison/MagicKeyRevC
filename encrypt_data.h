#pragma once
#include <string>
#include <fstream>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "json.hpp"

// Helper to read file
inline std::string read_file(const std::string& filename) {
    std::ifstream t(filename);
    return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

// Helper for base64 encoding (URL-safe)
inline std::string base64_encode(const std::string& data) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, mem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, data.data(), data.size());
    BIO_flush(b64);
    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);
    std::string encoded(bptr->data, bptr->length);
    BIO_free_all(b64);
    // Convert to URL-safe base64
    for (auto& c : encoded) {
        if (c == '+') c = '-';
        else if (c == '/') c = '_';
    }
    return encoded;
}

// Encrypt using RSA OAEP + SHA-256 and base64-encode output (from string)
inline std::string encrypt_data_from_key_string(const nlohmann::json& data, const std::string& public_key_content) {
    std::string data_str = data.dump();

    BIO* bio = BIO_new_mem_buf(public_key_content.data(), (int)public_key_content.size());
    if (!bio) return "";

    EVP_PKEY* pubkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pubkey) return "";

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pubkey, nullptr);
    if (!ctx) {
        EVP_PKEY_free(pubkey);
        return "";
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        EVP_PKEY_free(pubkey);
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    // OAEP + SHA-256
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0 ||
        EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0 ||
        EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0) {
        EVP_PKEY_free(pubkey);
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen,
                         reinterpret_cast<const unsigned char*>(data_str.data()), data_str.size()) <= 0) {
        EVP_PKEY_free(pubkey);
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    std::string outbuf(outlen, '\0');
    if (EVP_PKEY_encrypt(ctx, reinterpret_cast<unsigned char*>(&outbuf[0]), &outlen,
                         reinterpret_cast<const unsigned char*>(data_str.data()), data_str.size()) <= 0) {
        EVP_PKEY_free(pubkey);
        EVP_PKEY_CTX_free(ctx);
        return "";
    }
    EVP_PKEY_free(pubkey);
    EVP_PKEY_CTX_free(ctx);

    outbuf.resize(outlen);
    return base64_encode(outbuf); // base64, url-safe
}

// Encrypt using RSA OAEP + SHA-256 and base64-encode output (from file)
inline std::string encrypt_data(const nlohmann::json& data, const std::string& public_key_path) {
    std::string data_str = data.dump();
    std::string pubkey_str = read_file(public_key_path);
    
    return encrypt_data_from_key_string(data, pubkey_str);
}