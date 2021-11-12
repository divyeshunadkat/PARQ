#ifndef __RULEINFOMANAGER_HPP
#define __RULEINFOMANAGER_HPP

#include "apara/Options.hpp"
#include "deep/Horn.hpp"

using namespace std;
using namespace ufo;

namespace apara
{
  class RuleInfoManager
  {
  protected:

    CHCs& ruleManager;
    ExprVector& decls;
    Options& o;

    map<int, ExprVector> srcVarsInRule;
    map<int, ExprVector> dstVarsInRule;
    map<int, Expr> bodyInRule;
    map<int, ExprVector> itesPerLoop;
    map<int, map<Expr, ExprVector>> allArrStore;
    map<int, map<Expr, ExprVector>> allArrSelect;
    map<int, map<Expr, ExprVector>> allArrStoreAccess;
    map<int, map<Expr, ExprVector>> allArrSelectAccess;
    map<Expr, map<int, map<Expr, ExprVector>>> arrStoreAccessInITEBr;
    map<Expr, map<int, map<Expr, ExprVector>>> arrSelectAccessInITEBr;
    map<Expr, map<Expr, ExprVector>> arrStoreAccessInITE;
    map<Expr, map<Expr, ExprVector>> arrSelectAccessInITE;
    map<int, map<Expr, ExprVector>> arrStoreAccessOutsideAllITE;
    map<int, map<Expr, ExprVector>> arrSelectAccessOutsideAllITE;

    void populateAccesses()
    {
      if(o.getVerbosity() > 10) outs () << "\nPopulate All Accesses\n";
      populateSrcDstVars();
      populateAllArrAccess();
      populateAllArrSelectStore();
      // Following functions are needed for an experimental algorithm in KSynth
      // Currently that algorithm is not in use and hence following lines are commented
      // populateITESNAccess();
      // populateOutsideAccessLists();
    }

    void populateSrcDstVars()
    {
      if(o.getVerbosity() > 10) outs () << "\nPopulating All Src and Dst Vars\n";
      for (auto & hr : ruleManager.chcs) {
        //        if (hr.isFact || hr.isQuery || hr.srcRelation != hr.dstRelation) continue;
        if (!hr.isInductive) continue;
        int invNum = getVarIndex(hr.srcRelation, decls);
        if(invNum < 0) continue;
        srcVarsInRule[invNum].insert(srcVarsInRule[invNum].end(), hr.srcVars.begin(), hr.srcVars.end());
        dstVarsInRule[invNum].insert(dstVarsInRule[invNum].end(), hr.dstVars.begin(), hr.dstVars.end());
        //        pprint(hr.srcVars);
        //        cout << "\nSrc Vars In Rule from my info manager" << "\n\n";
        //        pprint(srcVarsInRule[invNum]);
        bodyInRule[invNum] = hr.body;
      }
    }

    void getITES(const Expr term, ExprVector& iteVec)
    {
      if(o.getVerbosity() > 10) outs () << "\nFetching Top Level ITEs from " << *term << "\n";
      if (isOpX<ITE>(term))
        iteVec.push_back(term);
      else
        for (auto it = term->args_begin(), end = term->args_end(); it != end; ++it)
          getITES(*it, iteVec);
    }

    void populateAllArrAccess()
    {
      if(o.getVerbosity() > 10) outs () << "\nPopulating All Array Accesses\n";
      for (auto & hr : ruleManager.chcs) {
        if (hr.isFact || hr.isQuery || hr.srcRelation != hr.dstRelation) continue;
        int invNum = getVarIndex(hr.srcRelation, decls);
        if(invNum < 0) continue;
        getArrAccess(hr.body, allArrStoreAccess[invNum], allArrSelectAccess[invNum]);
      }
    }

    void getArrAccess(const Expr term, map<Expr, ExprVector>& arrStoreAccess,
                      map<Expr, ExprVector>& arrSelectAccess)
    {
      if(o.getVerbosity() > 10) outs () << "\nFetching Array Accesses\n";
      if (isOpX<SELECT>(term)) {
        arrSelectAccess[term->left()].push_back(term->right());
        if(o.getVerbosity() > 5) {
          outs () << "\nSelect Array Expr :" << *term->left();
          outs () << "\tIndex Access Expr : " << *term->right() << "\n";
        }
      } else if (isOpX<STORE>(term)) {
        arrStoreAccess[term->left()].push_back(term->right());
        if(o.getVerbosity() > 5) {
          outs () << "\nStore Array Expr : " << *term->left();
          outs () << "\tIndex Access Expr : " << *term->right() << "\n";
        }
      } else {}
      for (auto it = term->args_begin(), end = term->args_end(); it != end; ++it)
        getArrAccess(*it, arrStoreAccess, arrSelectAccess);
    }

    void populateITESNAccess()
    {
      if(o.getVerbosity() > 10) outs () << "\nPopulating ITEs\n";
      for (auto & hr : ruleManager.chcs) {
        if (hr.isFact || hr.isQuery || hr.srcRelation != hr.dstRelation) continue;
        int invNum = getVarIndex(hr.srcRelation, decls);
        if(invNum < 0) continue;
        getITES(hr.body, itesPerLoop[invNum]);
        for(int i=0; i<itesPerLoop[invNum].size(); i++) {
          if(o.getVerbosity() > 2) outs () << "\nITE:" << *itesPerLoop[invNum][i] << "\n";
          getArrAccess(itesPerLoop[invNum][i]->right(),
                       arrStoreAccessInITEBr[itesPerLoop[invNum][i]][0],
                       arrSelectAccessInITEBr[itesPerLoop[invNum][i]][0]);
          getArrAccess(itesPerLoop[invNum][i]->last(),
                       arrStoreAccessInITEBr[itesPerLoop[invNum][i]][1],
                       arrSelectAccessInITEBr[itesPerLoop[invNum][i]][1]);
          map<Expr, ExprVector>::iterator it = arrStoreAccessInITEBr[itesPerLoop[invNum][i]][0].begin();
          while (it != arrStoreAccessInITEBr[itesPerLoop[invNum][i]][0].end()) {
            for (auto & e : it->second)
              arrStoreAccessInITE[itesPerLoop[invNum][i]][it->first].push_back(e);
            it++;
          }
          it = arrStoreAccessInITEBr[itesPerLoop[invNum][i]][1].begin();
          while (it != arrStoreAccessInITEBr[itesPerLoop[invNum][i]][1].end()) {
            for (auto & e : it->second)
              arrStoreAccessInITE[itesPerLoop[invNum][i]][it->first].push_back(e);
            it++;
          }
          it = arrSelectAccessInITEBr[itesPerLoop[invNum][i]][0].begin();
          while (it != arrSelectAccessInITEBr[itesPerLoop[invNum][i]][0].end()) {
            for (auto & e : it->second)
              arrSelectAccessInITE[itesPerLoop[invNum][i]][it->first].push_back(e);
            it++;
          }
          it = arrSelectAccessInITEBr[itesPerLoop[invNum][i]][1].begin();
          while(it != arrSelectAccessInITEBr[itesPerLoop[invNum][i]][1].end()) {
            for (auto & e : it->second)
              arrSelectAccessInITE[itesPerLoop[invNum][i]][it->first].push_back(e);
            it++;
          }
        }
      }
    }

    void populateOutsideAccessLists()
    {
      if(o.getVerbosity() > 10) outs () << "\nPopulating Lists of Access Outside ITEs\n";
      for (int invNum=0; invNum<decls.size(); invNum++) {
        map<Expr, ExprVector>::iterator allstoit = allArrStoreAccess[invNum].begin();
        while(allstoit != allArrStoreAccess[invNum].end()) {
          for(auto & arrAcc : allstoit->second) {
            bool add = true;
            for(auto & ite : itesPerLoop[invNum]) {
              map<Expr, ExprVector>::iterator it = arrStoreAccessInITE[ite].begin();
              while(it != arrStoreAccessInITE[ite].end()) {
                if (it->first != allstoit->first) { it++; continue; }
                for(auto & iteArrAcc : it->second) {
                  if(o.getVerbosity() > 10)
                    outs () << "\narrAcc:" << *arrAcc << "\t iteArrAcc: " << *iteArrAcc << "\n";
                  if(arrAcc == iteArrAcc) add = false;
                  if(add == false) break;
                }
                it++;
              }
              if(add == false) break;
            }
            if(add) arrStoreAccessOutsideAllITE[invNum][allstoit->first].push_back(arrAcc);
          }
          allstoit++;
        }

        map<Expr, ExprVector>::iterator allselit = allArrSelectAccess[invNum].begin();
        while( allselit != allArrSelectAccess[invNum].end() ) {
          for(auto & arrAcc : allselit->second) {
            bool add = true;
            for(auto & ite : itesPerLoop[invNum]) {
              map<Expr, ExprVector>::iterator it = arrSelectAccessInITE[ite].begin();
              while(it != arrSelectAccessInITE[ite].end()) {
                if (it->first != allselit->first) { it++; continue; }
                for(auto & iteArrAcc : it->second) {
                  if(o.getVerbosity() > 10)
                    outs () << "\narrAcc:" << *arrAcc << "\t iteArrAcc: " << *iteArrAcc << "\n";
                  if(arrAcc == iteArrAcc) add = false;
                  if(add == false) break;
                }
                it++;
              }
              if(add == false) break;
            }
            if(add) arrSelectAccessOutsideAllITE[invNum][allselit->first].push_back(arrAcc);
          }
          allselit++;
        }
      }
    }

    void populateAllArrSelectStore()
    {
      if(o.getVerbosity() > 10) outs () << "\nPopulating All Array Select Store Exprs\n";
      for (auto & hr : ruleManager.chcs) {
        if (hr.isFact || hr.isQuery || hr.srcRelation != hr.dstRelation) continue;
        int invNum = getVarIndex(hr.srcRelation, decls);
        if(invNum < 0) continue;
        getAllArrSelectStore(hr.body, invNum);
      }
    }

    void getAllArrSelectStore(const Expr term, int invNum)
    {
      if(o.getVerbosity() > 10) outs () << "\nFetching Array Store and Select Exprs\n";
      if (isOpX<SELECT>(term)) {
        if(o.getVerbosity() > 5) outs () << "\nArray Select Expr :" << *term << "\n";
        allArrSelect[invNum][term->left()].push_back(term->right());
      } else if (isOpX<STORE>(term)) {
        if(o.getVerbosity() > 5) outs () << "\nArray Store Expr :" << *term << "\n";
        allArrStore[invNum][term->left()].push_back(term->right());
      } else {}
      for (auto it = term->args_begin(), end = term->args_end(); it != end; ++it)
        getAllArrSelectStore(*it, invNum);
    }

  public:
    RuleInfoManager (CHCs& r, vector<Expr>& d, Options& opt) :
      ruleManager(r), decls(d), o(opt) { populateAccesses(); }

    inline map<int, ExprVector>& getSrcs() { return srcVarsInRule; }
    inline map<int, ExprVector>& getDsts() { return dstVarsInRule; }
    inline map<int, Expr>& getBodys() { return bodyInRule; }
    inline map<int, ExprVector>& getITEs() { return itesPerLoop; }
    inline map<int, map<Expr, ExprVector>>& getAllArrStore() { return allArrStore; }
    inline map<int, map<Expr, ExprVector>>& getAllArrSelect() {return allArrSelect; }
    inline map<int, map<Expr, ExprVector>>& getAllArrStoreAccess() { return allArrStoreAccess; }
    inline map<int, map<Expr, ExprVector>>& getAllArrSelectAccess() {return allArrSelectAccess; }
    inline map<Expr, map<int, map<Expr, ExprVector>>>& getArrStoreAccessInITEBr() { return arrStoreAccessInITEBr; }
    inline map<Expr, map<int, map<Expr, ExprVector>>>& getArrSelectAccessInITEBr() { return arrSelectAccessInITEBr; }
    inline map<Expr, map<Expr, ExprVector>>& getArrStoreAccessInITE() { return arrStoreAccessInITE; }
    inline map<Expr, map<Expr, ExprVector>>& getArrSelectAccessInITE() { return arrSelectAccessInITE; }
    inline map<int, map<Expr, ExprVector>>& getArrStoreAccessOutsideITE() { return arrStoreAccessOutsideAllITE; }
    inline map<int, map<Expr, ExprVector>>& getArrSelectAccessOutsideITE() { return arrSelectAccessOutsideAllITE; }

  };

}

#endif

