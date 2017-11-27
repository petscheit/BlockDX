#ifndef XBITCOINSECRET_H
#define XBITCOINSECRET_H

#include "base58.h"
#include "xkey.h"

namespace xbridge
{

//******************************************************************************
//******************************************************************************
class CBitcoinSecret : public CBase58Data
{
public:
    void SetKey(const CKey& vchSecret, const std::vector<unsigned char>& version);
    CKey GetKey();
    bool IsValid() const;
    bool SetString(const char* pszSecret);
    bool SetString(const std::string& strSecret);

    CBitcoinSecret(const CKey& vchSecret) { SetKey(vchSecret, std::vector<unsigned char>(1, 0)); }
    CBitcoinSecret(const CKey& vchSecret, const std::vector<unsigned char>& version)  { SetKey(vchSecret, version); }
    CBitcoinSecret() {}
};

} // namespace xbridge

#endif // XBITCOINSECRET_H
