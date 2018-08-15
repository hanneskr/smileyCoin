// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "richlistdb.h"

#include "main.h"
#include "base58.h"

// #include <vector>

// #include <boost/version.hpp>
// #include <boost/filesystem.hpp>

const string EIASaddresses[10] = {"BEaZDZ8gCbbP1y3t2gPNKwqZa76rUDfR73",
                                  "BDwfNiAvpb4ipNfPkdAXcPGExWHeeMdRcK",
                                  "BPVuYwyeJiXExEmEXHfCwPtRXDRqxBxTNW",
                                  "B4gB18iZWZ8nTuAvi9kq9cWbavCj6xSmny",
                                  "BGFEYswtWfo5nrKRT53ToGwZWyuRvwC8xs",
                                  "B7WWgP1fnPDHTL2z9vSHTpGqptP53t1UCB",
                                  "BEtL36SgjYuxHuU5dg8omJUAyTXDQL3Z8V",
                                  "BQaNeMcSyrzGkeKknjw6fnCSSLUYAsXCVd",
                                  "BDLAaqqtBNoG9EjbJCeuLSmT5wkdnSB8bc",
                                  "BQTar7kTE2hu4f4LrRmomjkbsqSW9rbMvy"};

CAddressPriority::CAddressPriority(const CScript &scriptPubKeyIn, int nHeightIn) : scriptPubKey(scriptPubKeyIn), nHeight(nHeightIn) { }


bool CRichView::Init(const CCoinsView &coins)
{
    assert(richaddresses.empty());
    addressPrioritySet priorities;
    if(!coins.GetRichPriorities(priorities))
        return false;
    BOOST_FOREACH(const CAddressPriority &priority, priorities) {
        if(!UpdatePriority(priority, false))
            return false;
    }
    return true;
}

bool CRichView::UpdatePriority(const CAddressPriority &priority, bool fEraseOnly)
{
    bool ret = true; 
    LogPrintf("richaddresses size before erase: %d ----------------------------------------------------\n", richaddresses.size());
    bool erase = richaddresses.erase(priority);
    LogPrintf("richaddresses size after erase: %d ----------------------------------------------------\n", richaddresses.size());
    if(!fEraseOnly) {
        std::pair<addressPrioritySet::iterator, bool> item = richaddresses.insert(priority);
        ret = item.second;
        if(!ret) {
            CTxDestination dest1, dest2;
            ExtractDestination(item.first->scriptPubKey, dest1);
            ExtractDestination(priority.scriptPubKey, dest2);
            CBitcoinAddress addr1, addr2;
            addr1.Set(dest1);
            addr2.Set(dest2);
            LogPrintf("Erase successful : %s -- %s prevents insertion of %s. Old Height: %d - New Height: %d\n",erase, addr1.ToString(), addr2.ToString(), item.first->nHeight, priority.nHeight);
        }  
    }
    return ret;
}


bool CRichView::GetNextRichScriptPubKey(CScript &scriptPubKey)
{
    if(richaddresses.empty())
        return false;
    scriptPubKey = (*richaddresses.begin()).scriptPubKey;
    return true;
}

bool CRichView::GetNewerAddresses(const int &nHeightIn, std::set<CScript> &addresses)
{
    for(addressPrioritySet::const_reverse_iterator it = richaddresses.rbegin(); it!= richaddresses.rend() && it->nHeight > nHeightIn; it++)
        addresses.insert((*it).scriptPubKey);
    return true;
}





CRichViewCache::CRichViewCache(CRichView &baseIn) : base(&baseIn) { }

bool CRichViewCache::GetNextRichScriptPubKey(CScript &scriptPubKey) { return base->GetNextRichScriptPubKey(scriptPubKey); }

bool CRichViewCache::GetNewerAddresses(const int &nHeightIn, std::set<CScript> &addresses) { return base->GetNewerAddresses(nHeightIn, addresses); }

bool CRichViewCache::UpdatePriority(const CAddressPriority &priority, bool fEraseOnly)
{
    priorities[priority] = fEraseOnly;
    return true;
}

// The RichView is never written to disk directly so there is no need to roll back 
// in case of an (exceptional) insertion failure as long as we don't continue
bool CRichViewCache::Flush()
{
    bool fOK = true; 
    BOOST_FOREACH(const PAIRTYPE(CAddressPriority, bool)& item, priorities) {
        if(!base->UpdatePriority(item.first, item.second)) {
            fOK = false;
            break;
        } 
    }
    if(fOK)
        priorities.clear();
    return fOK;
}






CScript NextEIASScriptPubKey(const int &nHeight){
    CScript ret;
    ret.SetDestination(CBitcoinAddress(EIASaddresses[nHeight % 10]).Get());
    return ret;
}



