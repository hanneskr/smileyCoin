// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef BITCOIN_RICHLISTDB_H
#define BITCOIN_RICHLISTDB_H

#include "init.h" 
#include "core.h"

class CCoinsView;

struct CAddressPriority
{
    CScript scriptPubKey;
    int nHeight; 

    CAddressPriority(const CScript &scriptPubKeyIn, int nHeightIn);
};

struct AddressPriorityCompare
{
    bool operator()(const CAddressPriority& lhs, const CAddressPriority& rhs)
    {
        if (!(lhs.scriptPubKey < rhs.scriptPubKey) && !(rhs.scriptPubKey < lhs.scriptPubKey))
            return false;
        if (!(lhs.nHeight < rhs.nHeight) && !(rhs.nHeight < lhs.nHeight))
            return lhs.scriptPubKey < rhs.scriptPubKey;
        return lhs.nHeight < rhs.nHeight;
    }
};

typedef std::set<CAddressPriority, AddressPriorityCompare> addressPrioritySet;

class CRichView
{
protected:
	addressPrioritySet richaddresses;
public:
    virtual bool UpdatePriority(const CAddressPriority &priority, bool fEraseOnly);
	virtual bool GetNextRichScriptPubKey(CScript &scriptPubKey);
	virtual bool GetNewerAddresses(const int &nHeightIn, std::set<CScript> &addresses);
    addressPrioritySet GetRichPriorities() {return richaddresses;}

    bool Init(const CCoinsView &view);

    virtual ~CRichView() {}



};

class CRichViewCache: public CRichView
{
protected:
    CRichView *base;
    std::map<CAddressPriority, bool, AddressPriorityCompare> priorities;
public:
    CRichViewCache(CRichView &baseIn);

    bool UpdatePriority(const CAddressPriority &priority, bool fEraseOnly);
    bool GetNextRichScriptPubKey(CScript &scriptPubKey);
    bool GetNewerAddresses(const int &nHeightIn, std::set<CScript> &addresses);

    bool Flush();
};

CScript NextEIASScriptPubKey(const int &nHeight);

#endif
