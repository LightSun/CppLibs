#ifndef RSAUtil_H
#define RSAUtil_H

#include <string>
#include <vector>

class RSAUtil
{
public:
    static std::string enc_by_public_key(const std::vector<char>&  publicKey,
                                         const std::vector<char>& data);
    static std::string dec_by_public_key(const std::vector<char>&  publicKey,
                                         const std::vector<char>& data);
    static std::string enc_by_private_key(const std::vector<char>&  privateKey,
                                          const std::vector<char>& data);
    static std::string dec_by_private_key(const std::vector<char>&  privateKey,
                                          const std::vector<char>& data);

    static std::string enc_by_public_key(const std::string&  publicKey,
                                         const std::vector<char>& data);
    static std::string dec_by_public_key(const std::string&  publicKey,
                                         const std::vector<char>& data);
    static std::string enc_by_private_key(const std::string&  privateKey,
                                          const std::vector<char>& data);
    static std::string dec_by_private_key(const std::string&  privateKey,
                                          const std::vector<char>& data);
};

#endif // RSA_H
