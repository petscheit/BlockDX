//******************************************************************************
//******************************************************************************

#include "xbridgewalletconnectorbcc.h"

#include "xkey.h"
#include "xbitcoinsecret.h"
#include "xbitcoinaddress.h"
#include "xbitcointransaction.h"

#include "util/logger.h"

//******************************************************************************
//******************************************************************************
enum
{
    SIGHASH_FORKID = 0x40
};

////******************************************************************************
////******************************************************************************
//xbridge::CTransactionPtr createTransaction();
//xbridge::CTransactionPtr createTransaction(const std::vector<std::pair<std::string, int> > & inputs,
//                                           const std::vector<std::pair<std::string, double> >  & outputs,
//                                           const uint64_t COIN,
//                                           const uint32_t txversion,
//                                           const uint32_t lockTime);

//******************************************************************************
//******************************************************************************
XBridgeBccWalletConnector::XBridgeBccWalletConnector()
    : XBridgeBtcWalletConnector()
{

}

//******************************************************************************
//******************************************************************************
bool XBridgeBccWalletConnector::createRefundTransaction(const std::vector<std::pair<std::string, int> > & inputs,
                                                        const std::vector<std::pair<std::string, double> > & outputs,
                                                        const std::vector<unsigned char> & mpubKey,
                                                        const std::vector<unsigned char> & mprivKey,
                                                        const std::vector<unsigned char> & innerScript,
                                                        const uint32_t lockTime,
                                                        std::string & txId,
                                                        std::string & rawTx)
{
    std::string refundTx;
    if(!createRawTransaction(inputs, outputs, lockTime, refundTx))
    {
        // cancel transaction
        LOG() << "create transaction error, transaction canceled " << __FUNCTION__;
        return false;
    }

    CScript redeem;
    redeem << mpubKey << OP_TRUE << innerScript;

    std::string redeemStr = HexStr(redeem.begin(), redeem.end());
//    redeemStr.append(std::string(innerScript.begin(), innerScript.end()));

    std::vector<std::pair<std::string, int> > vins;
    std::string scriptPubKey;
    if(!decodeRawTransaction(refundTx, vins, scriptPubKey))
    {
        // cancel transaction
        LOG() << "decode unsigned transaction error, transaction canceled " << __FUNCTION__;
        return false;
    }

    std::vector<std::tuple<std::string, int, std::string, std::string> > prevtxs;
    for(std::pair<std::string, int> vin : vins)
    {
        prevtxs.push_back(std::make_tuple(vin.first, vin.second, scriptPubKey, redeemStr));
    }

    std::vector<std::string> privKeys;
    privKeys.push_back(HexStr(mprivKey.begin(), mprivKey.end()));

    bool complete = false;
    if(!signRawTransaction(refundTx, prevtxs, privKeys, complete))
    {
        // cancel transaction
        LOG() << "sign transaction error, transaction canceled " << __FUNCTION__;
        return false;
    }

    if(!complete)
    {
        LOG() << "transaction not fully signed" << __FUNCTION__;
        return false;
    }

    std::string json;
    std::string reftxid;
    if (!decodeRawTransaction(refundTx, reftxid, json))
    {
        LOG() << "decode signed transaction error, transaction canceled " << __FUNCTION__;
        return false;
    }

    rawTx = refundTx;
    txId  = reftxid;

    return true;



//    xbridge::CTransactionPtr txUnsigned = createTransaction(inputs, outputs, COIN, txVersion, lockTime);
//    txUnsigned->vin[0].nSequence = std::numeric_limits<uint32_t>::max()-1;

//    CScript inner(innerScript.begin(), innerScript.end());

//    xbridge::CKey m;
//    m.Set(mprivKey.begin(), mprivKey.end(), true);
//    if (!m.IsValid())
//    {
//        // cancel transaction
//        LOG() << "sign transaction error, restore private key failed, transaction canceled " << __FUNCTION__;
//        return false;
//    }

//    CScript redeem;
//    {
//        CScript tmp;
//        std::vector<unsigned char> raw(mpubKey.begin(), mpubKey.end());
//        tmp << raw << OP_TRUE << inner;

//        std::vector<unsigned char> signature;
//        uint256 hash = xbridge::SignatureHash2(inner, txUnsigned, 0, SIGHASH_ALL);
//        if (!m.Sign(hash, signature))
//        {
//            // cancel transaction
//            LOG() << "sign transaction error, transaction canceled " << __FUNCTION__;
//            return false;
//        }

//        signature.push_back((unsigned char)SIGHASH_ALL);

//        redeem << signature;
//        redeem += tmp;
//    }

//    xbridge::CTransactionPtr tx(createTransaction());
//    if (!tx)
//    {
//        ERR() << "transaction not created " << __FUNCTION__;
//        return false;
//    }
//    tx->nVersion  = txUnsigned->nVersion;
//    tx->vin.push_back(CTxIn(txUnsigned->vin[0].prevout, redeem, std::numeric_limits<uint32_t>::max()-1));
//    tx->vout      = txUnsigned->vout;
//    tx->nLockTime = txUnsigned->nLockTime;

//    std::string json;
//    std::string reftxid;
//    if (!decodeRawTransaction(tx->toString(), reftxid, json))
//    {
//        LOG() << "decode signed transaction error, transaction canceled " << __FUNCTION__;
//            return true;
//    }

//    rawTx = tx->toString();
//    txId  = reftxid;

//    return true;
}

//******************************************************************************
//******************************************************************************
bool XBridgeBccWalletConnector::createPaymentTransaction(const std::vector<std::pair<std::string, int> > & inputs,
                                                         const std::vector<std::pair<std::string, double> > & outputs,
                                                         const std::vector<unsigned char> & mpubKey,
                                                         const std::vector<unsigned char> & mprivKey,
                                                         const std::vector<unsigned char> & xpubKey,
                                                         const std::vector<unsigned char> & innerScript,
                                                         std::string & txId,
                                                         std::string & rawTx)
{
    std::string payTx;
    if(!createRawTransaction(inputs, outputs, 0, payTx))
    {
        // cancel transaction
        LOG() << "create transaction error, transaction canceled " << __FUNCTION__;
        return false;
    }

    CScript redeem;
    redeem << xpubKey
           << mpubKey
           << OP_FALSE;

    std::string redeemStr = HexStr(redeem.begin(), redeem.end());
    redeemStr.append(std::string(innerScript.begin(), innerScript.end()));

    std::vector<std::pair<std::string, int> > vins;
    std::string scriptPubKey;
    if(!decodeRawTransaction(payTx, vins, scriptPubKey))
    {
        // cancel transaction
        LOG() << "decode unsigned transaction error, transaction canceled " << __FUNCTION__;
        return false;
    }

    std::vector<std::tuple<std::string, int, std::string, std::string> > prevtxs;
    for(std::pair<std::string, int> vin : vins)
    {
        prevtxs.push_back(std::make_tuple(vin.first, vin.second, scriptPubKey, redeemStr));
    }

    std::vector<std::string> privKeys;
    privKeys.push_back(std::string(mprivKey.begin(), mprivKey.end()));

    bool complete = false;
    if(!signRawTransaction(payTx, prevtxs, privKeys, complete))
    {
        // cancel transaction
        LOG() << "sign transaction error, transaction canceled " << __FUNCTION__;
        return false;
    }

    if(!complete)
    {
        // cancel transaction
        LOG() << "transaction not fully signed" << __FUNCTION__;
        return false;
    }

    std::string json;
    std::string reftxid;
    if (!decodeRawTransaction(payTx, reftxid, json))
    {
        // cancel transaction
        LOG() << "decode signed transaction error, transaction canceled " << __FUNCTION__;
        return false;
    }

    rawTx = payTx;
    txId  = reftxid;

    return true;



//    xbridge::CTransactionPtr txUnsigned = createTransaction(inputs, outputs, COIN, txVersion, 0);

//    CScript inner(innerScript.begin(), innerScript.end());

//    xbridge::CKey m;
//    m.Set(mprivKey.begin(), mprivKey.end(), true);
//    if (!m.IsValid())
//    {
//        // cancel transaction
//        LOG() << "sign transaction error (SetSecret failed), transaction canceled " << __FUNCTION__;
//        return false;
//    }

//    std::vector<unsigned char> signature;
//    uint256 hash = xbridge::SignatureHash2(inner, txUnsigned, 0, SIGHASH_ALL);
//    if (!m.Sign(hash, signature))
//    {
//        // cancel transaction
//        LOG() << "sign transaction error, transaction canceled " << __FUNCTION__;
//        return false;
//    }

//    signature.push_back((unsigned char)SIGHASH_ALL);

//    CScript redeem;
//    redeem << xpubKey
//           << signature << mpubKey
//           << OP_FALSE << inner;

//    xbridge::CTransactionPtr tx(createTransaction());
//    if (!tx)
//    {
//        ERR() << "transaction not created " << __FUNCTION__;
//        return false;
//    }
//    tx->nVersion  = txUnsigned->nVersion;
//    tx->vin.push_back(CTxIn(txUnsigned->vin[0].prevout, redeem));
//    tx->vout      = txUnsigned->vout;

//    std::string json;
//    std::string paytxid;
//    if (!decodeRawTransaction(tx->toString(), paytxid, json))
//    {
//        LOG() << "decode signed transaction error, transaction canceled " << __FUNCTION__;
//        return true;
//    }

//    rawTx = tx->toString();
//    txId  = paytxid;

//    return true;
}
