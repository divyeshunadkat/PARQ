#ifndef __KSYNTHESIZER_HPP
#define __KSYNTHESIZER_HPP

#include "apara/Options.hpp"
#include "deep/Horn.hpp"
#include "ae/SMTUtils.hpp"
#include "apara/RuleInfoManager.hpp"
#include "apara/Learner.hpp"

using namespace std;
using namespace ufo;

namespace apara
{
  class KSynthesizer
  {
  protected:

    ExprFactory &m_efac;
    CHCs& ruleManager;
    Learner& ds;
    ExprVector& decls;
    RuleInfoManager& rim;
    Options& o;
    SMTUtils u;

    map<int, vector<ArrAccessIter* >>& my_qvits;

    map<int, map<Expr, map<Expr, ExprVector>>> FM;
    map<int, map<Expr, map<Expr, vector<bool>>>> redundantStores;
    map<int, ExprVector> KS;
    map<int, ExprVector> KSResult;

    void extractVars (const Expr & e, ExprSet& es)
    {
      if (isOpX<FORALL>(e) || isOpX<EXISTS>(e))
      {
        for (int i = 0; i < e->arity() - 1; i++)
        {
          Expr var = bind::fapp(e->arg(i));
          es.insert(var);
        }
        extractVars(e->last(), es);
      }
      else if (isOpX<AND>(e))
      {
        ExprSet cnjs;
        getConj(e, cnjs);
        for (auto & c : cnjs)
          extractVars(c, es);
      }
      else if (isOpX<OR>(e))
      {
        ExprSet dsjs;
        getDisj(e, dsjs);
        for (auto & d : dsjs)
          extractVars(d, es);
      }
      else if (isOpX<IMPL>(e) || isOp<ComparissonOp>(e))
      {
        extractVars(e->left(), es);
        extractVars(e->right(), es);
      }
      else if (isOpX<ITE>(e))
      {
        extractVars(e->left(), es);
        extractVars(e->right(), es);
        extractVars(e->last(), es);
      }
      else if (isOpX<STORE>(e) || isOpX<SELECT>(e))
      {
        for (int i = 0; i < e->arity() - 1; i++)
          extractVars(e->arg(i), es);
      }
      else  if (isOpX<PLUS>(e) || isOpX<MINUS>(e) ||
                isOpX<MULT>(e) || isOpX<DIV>(e) ||
                isOpX<MOD>(e) || isOpX<IDIV>(e) ||
                isOpX<REM>(e) ||  isOpX<UN_MINUS>(e))
      {
        for (int i = 0; i < e->arity(); i++)
          extractVars(e->arg(i), es);
      }
      else if (isOpX<FDECL>(e))
      {}
      else es.insert(e);
    }

    Expr getCorrespondingVar(Expr v, int invNum) {
      bool found = false;
      int i = 0;
      for(auto& sv : rim.getSrcs().at(invNum)) {
        if(v == sv) {
          found = true;
          break;
        }
        i++;
      }
      if(found) {
        Expr cv = rim.getDsts().at(invNum)[i];
        if(o.getVerbosity() > 10) outs () << "\nCorresponding Variable: " << *cv;
        return cv;
      } else {
        i = 0;
      }
      for(auto& dv : rim.getDsts().at(invNum)) {
        if(v == dv) {
          found = true;
          break;
        }
        i++;
      }
      if(found) {
        Expr cv = rim.getSrcs().at(invNum)[i];
        if(o.getVerbosity() > 10) outs () << "\nCorresponding Variable: " << *cv;
        return cv;
      } else {
        outs () << "\nUnable to find the variable " << *v << " in the horn rule\n";
        return mk<FALSE>(m_efac);
      }
    }

    // Assumes all array indices are from the same statement
    // TODO: Seperate design for multiple statements in a single loop
    // TODO: Handles only constant difference in indices. Need to handle variables shifts
    // TODO: Algorithm optimization??
    bool checkOverlappingIndices(const int invNum)
    {
      if(o.getVerbosity() > 1) outs () << "\nCheck for overlap in array indices\n";
      bool result = false;
      Expr CVar = bind::intConst(mkTerm<string> ("_APARA_C_", m_efac));
      map<Expr, ExprVector>::iterator itst  = rim.getAllArrStoreAccess()[invNum].begin();
      map<Expr, ExprVector>::iterator itsel = rim.getAllArrSelectAccess()[invNum].begin();
      while (itst != rim.getAllArrStoreAccess()[invNum].end()) {
        itsel = rim.getAllArrSelectAccess()[invNum].begin();
        while (itsel != rim.getAllArrSelectAccess()[invNum].end()) {
          if(o.getVerbosity() > 5)
            outs () << "\nStore Array: " << *(itst->first)
                    << "\nSelect Array: " << *(itsel->first) << "\n";
          if(itst->first != itsel->first) { itsel++; continue; }
          for(auto & e1 : itst->second) {
            for(auto & e2 : itsel->second) {
              Expr e = mk<AND>(mk<GT>(CVar, mkTerm(mpz_class (0), m_efac)),
                               mk<EQ>(e1, mk<PLUS>(e2, CVar)));
              result = bool(u.isSat(e));
              if(o.getVerbosity() > 5)
                outs () << "\nOverlap Query: " << *e << "\nResult: " << result << "\n";
              if(result) break;
            }
            if(result) break;
          }
          if(result) break;
          itsel++;
        }
        if(result) break;
        itst++;
      }
      return result;
    }

    bool checkModified(const int invNum)
    {
      if(o.getVerbosity() > 1) outs () << "\nCheck if any array index has variables that are modified\n";
      bool result = true;
      map<Expr, ExprVector>::iterator itst  = rim.getAllArrStoreAccess()[invNum].begin();
      while (itst != rim.getAllArrStoreAccess()[invNum].end()) {
        for(auto & e1 : itst->second) {
          if(o.getVerbosity() > 5) outs () << "\nChecking index: " << *e1 << "\n";
          ExprSet vars;
          extractVars(e1, vars);
          for(auto & v : vars) {
            if(o.getVerbosity() > 5) outs () << "\nVars in the index: " << *v << "\n";
            if(v != my_qvits[invNum].front()->iter && !isOpX<MPZ>(v)) {
              Expr vp = getCorrespondingVar(v, invNum);
              if (isOpX<FALSE>(vp)) continue;
              if(o.getVerbosity() > 10)
                outs () << "\nBody of the inv\n" << *(rim.getBodys().at(invNum)) << "\n\n";
              if(o.getVerbosity() > 10)
                outs () << "\nEquality expression\n" << *mk<EQ>(v,vp) << "\n\n";
              result = bool(u.implies(rim.getBodys().at(invNum), mk<EQ>(v,vp)));
              if(o.getVerbosity() > 1 && result) outs () << "\n\nModified Var in the index\n\n";
              if(!result) break;
            }
          }
          if(!result) break;
        }
        if(!result) break;
        itst++;
      }
      if(!result) return result;

      map<Expr, ExprVector>::iterator itsel = rim.getAllArrSelectAccess()[invNum].begin();
      while (itsel != rim.getAllArrSelectAccess()[invNum].end()) {
        for(auto & e2 : itsel->second) {
          if(o.getVerbosity() > 5) outs () << "\nChecking index: " << *e2 << "\n";
          ExprSet vars;
          extractVars(e2, vars);
          for(auto & v : vars) {
            if(o.getVerbosity() > 5) outs () << "\nVars in the index: " << *v << "\n";
            if(v != my_qvits[invNum].front()->iter && !isOpX<MPZ>(v)) {
              Expr vp = getCorrespondingVar(v, invNum);
              if (isOpX<FALSE>(vp)) continue;
              if(o.getVerbosity() > 10)
                outs () << "\nBody of the inv\n" << *(rim.getBodys().at(invNum)) << "\n\n";
              if(o.getVerbosity() > 10)
                outs () << "\nEquality expression\n" << *mk<EQ>(v,vp) << "\n\n";
              result = bool(u.implies(rim.getBodys().at(invNum), mk<EQ>(v,vp)));
              if(o.getVerbosity() > 1 && result) outs () << "\n\nModified Var in the index\n\n";
              if(!result) break;
            }
          }
          if(!result) break;
        }
        if(!result) break;
        itsel++;
      }
      return result;
    }

    bool runKSynth()
    {
      if(o.getVerbosity() > 1) outs () << "\nPerforming K Synthesis\n";
      for (auto & hr : ruleManager.chcs) {
        if (hr.isFact || hr.isQuery || hr.srcRelation != hr.dstRelation) continue;
        int invNum = getVarIndex(hr.srcRelation, decls);
        if(invNum < 0) continue;

        ExprVector encCons;
        if (!encode(invNum, encCons)) return false;
        // Before uncommenting following lines, need to uncomment the functions
        // for ppopulating the data-structure in the RuleInfoManager class
        // generateFM(invNum);
        // generateKSynthFormulas(invNum);
        // synthesizeK(invNum);
      }
      return true;
    }

    bool encode(const int invNum, ExprVector& res)
    {
      if(o.getVerbosity() > 1) outs () << "\nEncoding the constraints for K Synthesis\n";
      Expr k1Var = bind::intConst(mkTerm<string> ("_APARA_K1_", m_efac));
      Expr k2Var = bind::intConst(mkTerm<string> ("_APARA_K2_", m_efac));
      Expr k3Var = bind::intConst(mkTerm<string> ("_APARA_K3_", m_efac));
      Expr k4Var = bind::intConst(mkTerm<string> ("_APARA_K4_", m_efac));
      Expr igeqk3 = mk<GEQ>(my_qvits[invNum].front()->iter, k3Var);
      Expr iltk4 = mk<LT>(my_qvits[invNum].front()->iter, k4Var);
      if(o.getVerbosity() > 1) outs () << "\nEncoding of i >= k3\n" << *igeqk3 << "\n";
      if(o.getVerbosity() > 1) outs () << "\nEncoding of i < k4\n" << *iltk4 << "\n";
      ExprVector conj_min_constraints;
      ExprVector conj_max_constraints;
      map<Expr, ExprVector>::iterator itst = rim.getAllArrStoreAccess()[invNum].begin();
      while (itst != rim.getAllArrStoreAccess()[invNum].end()) {
        for(auto & e2 : itst->second) {
          Expr minc = mk<GEQ>(e2, k1Var);
          Expr maxc = mk<LT>(e2, k2Var);
          conj_min_constraints.push_back(minc);
          conj_max_constraints.push_back(maxc);
          if(o.getVerbosity() > 1) outs () << "\nAdding min constraint\n" << *minc << "\n";
          if(o.getVerbosity() > 1) outs () << "\nAdding max constraint\n" << *maxc << "\n";
        }
        itst++;
      }
      map<Expr, ExprVector>::iterator itsel = rim.getAllArrSelectAccess()[invNum].begin();
      while (itsel != rim.getAllArrSelectAccess()[invNum].end()) {
        for(auto & e2 : itsel->second) {
          Expr minc = mk<GEQ>(e2, k1Var);
          Expr maxc = mk<LT>(e2, k2Var);
          conj_min_constraints.push_back(minc);
          conj_max_constraints.push_back(maxc);
          if(o.getVerbosity() > 1) outs () << "\nAdding min constraint\n" << *minc << "\n";
          if(o.getVerbosity() > 1) outs () << "\nAdding max constraint\n" << *maxc << "\n";
        }
        itsel++;
      }

      Expr resInvs = mk<TRUE>(m_efac);
      /*
      ExprSet nonQFInvs;
      for (int i = 0; i < ds.getDecls().size(); i++)
      {
        SamplFactory& sf = ds.getSFS()[i].back();
        for( auto & e : sf.learnedExprs ) {
          if (isOpX<FORALL>(e) || isOpX<EXISTS>(e) ) continue;
          nonQFInvs.insert(e);
        }
      }
      resInvs = conjoin(nonQFInvs, m_efac);
      if(o.getVerbosity() > 1) outs () << "\nInvariants\n" << *resInvs << "\n";
      */

      ExprSet qVars;
      qVars.insert(my_qvits[invNum].front()->iter);
      Expr minCons  = mkNeg(eliminateQuantifiers(mkNeg(mk<IMPL>(igeqk3, conjoin(conj_min_constraints, m_efac))), qVars));
      Expr maxCons  = mkNeg(eliminateQuantifiers(mkNeg(mk<IMPL>(iltk4, conjoin(conj_max_constraints, m_efac))), qVars));

      if(o.getVerbosity() > 1) {
        outs () << "Printing min constraint:\n" << *minCons << "\n";
        outs () << "Printing max constraint:\n" << *maxCons << "\n";
      }

      if(o.getVerbosity() > 1) outs () << "\nSolving for min constraints for the bound k3\n";
      qVars.clear();
      qVars.insert(k3Var);
      AeValSolver ae1(mk<TRUE>(m_efac), mk<AND>(mk<GEQ>(k3Var, k1Var), minCons, resInvs), qVars);

      if (ae1.solve()) return false;
        else minCons = simplifyArithm(ae1.getSimpleSkolemFunction());

      if(o.getVerbosity() > 1) outs () << "\nSolving for max constraints for the bound k4\n";
      qVars.clear();
      qVars.insert(k4Var);
      AeValSolver ae2(mk<TRUE>(m_efac), mk<AND>(mk<LEQ>(k4Var, k2Var), maxCons, resInvs), qVars);

      if (ae2.solve()) return false;
        else maxCons = simplifyArithm(ae2.getSimpleSkolemFunction());

      if(o.getVerbosity() > 1) {
        outs () << "\nSolved min constraint:\n" << *minCons << "\n";
        outs () << "\nSolved max constraint:\n" << *maxCons << "\n";
      }

      res.push_back(minCons);
      res.push_back(maxCons);
      res.push_back(my_qvits[invNum].front()->iter);
      res.push_back(k1Var);
      res.push_back(k2Var);
      res.push_back(k3Var);
      res.push_back(k4Var);
      return true;
    }

    void generateFM(const int invNum)
    {
      if(o.getVerbosity() > 1) outs () << "\nGenerating the formula map for INV: " << invNum << "\n";
      for(int i=0; i<rim.getITEs()[invNum].size(); i++) {
        map<Expr, ExprVector>& cfvec = FM[invNum][rim.getITEs()[invNum][i]];
        Expr C = rim.getITEs()[invNum][i]->left();
        if(o.getVerbosity() > 5) outs () << "\nGenerating for ITE: " << *C << "\n";
        map<Expr, ExprVector> allThenAccessConjunct;
        map<Expr, ExprVector> allThenAccessDisjunct;
        map<Expr, ExprVector>::iterator it = rim.getArrStoreAccessInITEBr()[rim.getITEs()[invNum][i]][0].begin();
        while (it != rim.getArrStoreAccessInITEBr()[rim.getITEs()[invNum][i]][0].end()) {
          for (auto & e1 : it->second) {
            map<Expr, ExprVector> singleAccessDisjunct;
            map<Expr, ExprVector> singleAccessConjunct;
            map<Expr, ExprVector>::iterator outit = rim.getArrStoreAccessOutsideITE()[invNum].begin();
            while (outit != rim.getArrStoreAccessOutsideITE()[invNum].end()) {
              if(it->first != outit->first) { outit++; continue; }
              for(auto & e2 : outit->second) {
                singleAccessDisjunct[it->first].push_back(mk<LT>(e1, e2));
                singleAccessConjunct[it->first].push_back(mk<GT>(e1, e2));
              }
              outit++;
            }
            if(singleAccessDisjunct[it->first].size() > 1)
              allThenAccessConjunct[it->first].push_back(disjoin(singleAccessDisjunct[it->first], m_efac));
            else if (singleAccessDisjunct[it->first].size() == 1)
              allThenAccessConjunct[it->first].push_back(singleAccessDisjunct[it->first][0]);
            else {}
            if(singleAccessConjunct[it->first].size() > 1)
              allThenAccessDisjunct[it->first].push_back(conjoin(singleAccessConjunct[it->first], m_efac));
            else if (singleAccessConjunct[it->first].size() == 1)
              allThenAccessDisjunct[it->first].push_back(singleAccessConjunct[it->first][0]);
            else {}
          }
          Expr fmlc1;
          if(allThenAccessConjunct[it->first].size() == 0)
            fmlc1 = mk<IMPL>(C, mk<FALSE>(m_efac));
          else
            fmlc1 = mk<IMPL>(C, conjoin(allThenAccessConjunct[it->first], m_efac));
          cfvec[it->first].push_back(fmlc1);
          if(o.getVerbosity() > 5) outs () << "\n FMLC1: " << *fmlc1 << "\n";
          Expr fmlc2 = mk<IMPL>(C, disjoin(allThenAccessDisjunct[it->first], m_efac));
          cfvec[it->first].push_back(fmlc2);
          if(o.getVerbosity() > 5) outs () << "\n FMLC2: " << *fmlc2 << "\n";
          it++;
        }

        map<Expr, ExprVector> allElseAccessConjunct;
        map<Expr, ExprVector> allElseAccessDisjunct;
        it = rim.getArrStoreAccessInITEBr()[rim.getITEs()[invNum][i]][1].begin();
        while (it != rim.getArrStoreAccessInITEBr()[rim.getITEs()[invNum][i]][1].end()) {
          for (auto & e1 : it->second) {
            map<Expr, ExprVector> singleAccessDisjunct;
            map<Expr, ExprVector> singleAccessConjunct;
            map<Expr, ExprVector>::iterator outit = rim.getArrStoreAccessOutsideITE()[invNum].begin();
            while (outit != rim.getArrStoreAccessOutsideITE()[invNum].end()) {
              if(it->first != outit->first) { outit++; continue; }
              for(auto & e2 : outit->second) {
                singleAccessDisjunct[it->first].push_back(mk<LT>(e1, e2));
                singleAccessConjunct[it->first].push_back(mk<GT>(e1, e2));
              }
              outit++;
            }
            if(singleAccessDisjunct[it->first].size() > 1)
              allElseAccessConjunct[it->first].push_back(disjoin(singleAccessDisjunct[it->first], m_efac));
            else if (singleAccessDisjunct[it->first].size() == 1)
              allElseAccessConjunct[it->first].push_back(singleAccessDisjunct[it->first][0]);
            else {}
            if(singleAccessConjunct[it->first].size() > 1)
              allElseAccessDisjunct[it->first].push_back(conjoin(singleAccessConjunct[it->first], m_efac));
            else if (singleAccessConjunct[it->first].size() == 1)
              allElseAccessDisjunct[it->first].push_back(singleAccessConjunct[it->first][0]);
            else {}
          }
          Expr fmlc3;
          if(allElseAccessConjunct[it->first].size() == 0)
            fmlc3 = mk<IMPL>(mk<NEG>(C), mk<FALSE>(m_efac));
          else
            fmlc3 = mk<IMPL>(mk<NEG>(C), conjoin(allElseAccessConjunct[it->first], m_efac));
          cfvec[it->first].push_back(fmlc3);
          if(o.getVerbosity() > 5) outs () << "\n FMLC3: " << *fmlc3 << "\n";
          Expr fmlc4 = mk<IMPL>(mk<NEG>(C), disjoin(allElseAccessDisjunct[it->first], m_efac));
          cfvec[it->first].push_back(fmlc4);
          if(o.getVerbosity() > 5) outs () << "\n FMLC4: " << *fmlc4 << "\n";
          it++;
        }

        checkRedundant(invNum, i);
      }
    }

    void checkRedundant(const int invNum, const int iteIndex)
    {
      if(o.getVerbosity() > 1) outs () << "\nChecking for Redundant Stores in Branches\n";
      Expr ite = rim.getITEs()[invNum][iteIndex];
      map<Expr, vector<bool>>& rvec = redundantStores[invNum][ite];
      map<Expr, ExprVector>& cfvec = FM[invNum][ite];
      map<Expr, ExprVector>::iterator it = cfvec.begin();
      while (it != cfvec.end()) {
        assert(it->second.size() == 4);
        Expr fmlc1 = it->second[0];
        Expr fmlc3 = it->second[2];
        Expr prerange, postrange;
        genIteratorRanges(invNum, prerange, postrange);
        Expr range = conjoinRanges(prerange, postrange);
        Expr erlc1 = mk<AND>(range, mk<NEG>(fmlc1));
        bool rlc1 = bool(!u.isSat(erlc1));
        rvec[it->first].push_back(rlc1);
        if(o.getVerbosity() > 5) outs () << "\nAre Stores Redundant in THEN: " << rlc1 << "\n";
        bool rlc2 = bool(!u.isSat(mk<AND>(range, mk<NEG>(fmlc3))));
        rvec[it->first].push_back(rlc2);
        if(o.getVerbosity() > 5) outs () << "\nAre Stores Redundant in ELSE: " << rlc2 << "\n";
        it++;
      }
    }

    void genIteratorRanges(const int invNum, Expr& prerange, Expr& postrange)
    {
      if(o.getVerbosity() > 1) outs () << "\nCreating Range Formula for Iterator:"
                                       << *my_qvits[invNum].front()->iter << "\n";
      Expr pre = ineqSimplifier(my_qvits[invNum].front()->iter, my_qvits[invNum].front()->precond);
      if (pre->right() == my_qvits[invNum].front()->iter)
        prerange = (my_qvits[invNum].front()->grows) ? mk<GEQ>(pre->right(), pre->left()) :
          mk<LEQ>(pre->right(), pre->left());
      else if (pre->left() == my_qvits[invNum].front()->iter)
        prerange = (my_qvits[invNum].front()->grows) ? mk<GEQ>(pre->left(), pre->right()) :
          mk<LEQ>(pre->left(), pre->right());
      if(o.getVerbosity() > 3) outs () << "\nPre Formula: " << *prerange << "\n";
      ExprSet postcnjs;
      getConj(my_qvits[invNum].front()->range, postcnjs);
      for(auto & e : postcnjs)
        if (contains(e, my_qvits[invNum].front()->iter) && !isOpX<EQ>(e))
          postrange = e;
      postrange = ineqSimplifier(my_qvits[invNum].front()->iter, postrange);
      if(o.getVerbosity() > 3) outs () << "\nPost Formula: " << *postrange << "\n";
    }

    inline Expr conjoinRanges(const Expr prerange, const Expr postrange)
    {
      ExprSet extr;
      extr.insert(postrange);
      extr.insert(prerange);
      Expr ret = conjoin(extr, m_efac);
      return ret;
    }

    void genForallSubranges(const int invNum, const Expr kVar, const Expr iter,
                            Expr& pre, Expr& post, Expr& sr1, Expr& sr2)
    {
      if(o.getVerbosity() > 1) outs () << "\nCreating Subranges for the ForAll Expr\n";
      ExprVector forallsr1, forallsr2;
      forallsr1.push_back(pre);
      if (pre->right() == my_qvits[invNum].front()->iter) {
        forallsr1.push_back((my_qvits[invNum].front()->grows) ? mk<LT>(pre->right(), kVar) :
                            mk<GT>(pre->right(), kVar));
        forallsr2.push_back((my_qvits[invNum].front()->grows) ? mk<GEQ>(pre->right(), kVar) :
                            mk<LEQ>(pre->right(), kVar));
      } else if (pre->left() == my_qvits[invNum].front()->iter) {
        forallsr1.push_back((my_qvits[invNum].front()->grows) ? mk<LT>(pre->left(), kVar) :
                            mk<GT>(pre->left(), kVar));
        forallsr2.push_back((my_qvits[invNum].front()->grows) ? mk<GEQ>(pre->left(), kVar) :
                            mk<LEQ>(pre->left(), kVar));
      } else {}
      forallsr2.push_back(post);
      sr1 = conjoin(forallsr1, m_efac);
      sr2 = conjoin(forallsr2, m_efac);
      if(o.getVerbosity() > 3) outs () << "\nForAll Subrange 1: " << *sr1 << "\n";
      if(o.getVerbosity() > 3) outs () << "\nForAll Subrange 2: " << *sr2 << "\n";
    }

    void generateKSynthFormulas(const int invNum)
    {
      if(o.getVerbosity() > 1) outs () << "\nGenerating K Synthesis Formula for inv:" << invNum << "\n";
      map<Expr, map<int, ExprVector>> consMap;
      for(int i=0; i<rim.getITEs()[invNum].size(); i++) {
        map<Expr, ExprVector>& cfvec_m = FM[invNum][rim.getITEs()[invNum][i]];
        map<Expr, ExprVector>::iterator it = cfvec_m.begin();
        while (it != cfvec_m.end()) {
          ExprVector& cfvec = it->second;
          assert(cfvec.size() == 4);
          consMap[it->first][0].push_back(mk<AND>(mkNeg(cfvec[1]), mkNeg(cfvec[3])));
          consMap[it->first][1].push_back(mk<AND>(cfvec[1], mkNeg(cfvec[3])));
          consMap[it->first][2].push_back(mk<AND>(mkNeg(cfvec[1]), cfvec[3]));
          consMap[it->first][3].push_back(mk<AND>(cfvec[1], cfvec[3]));
          it++;
        }
      }
      Expr prerange, postrange;
      genIteratorRanges(invNum, prerange, postrange);
      Expr iterrange = conjoinRanges(prerange, postrange);
      Expr kVar = bind::intConst(mkTerm<string> ("_APARA_K_", m_efac));
      Expr ksrange = replaceAll(iterrange, my_qvits[invNum].front()->iter, kVar);
      Expr qVar = bind::intConst(mkTerm<string> ("_APARA_arr_it_", m_efac));
      // Expr e0 = replaceAll(conjoin(consMap[0], m_efac), my_qvits[invNum].front()->iter, qVar);
      // Expr e1 = replaceAll(conjoin(consMap[1], m_efac), my_qvits[invNum].front()->iter, qVar);
      // Expr e2 = replaceAll(conjoin(consMap[2], m_efac), my_qvits[invNum].front()->iter, qVar);
      // Expr e3 = replaceAll(conjoin(consMap[3], m_efac), my_qvits[invNum].front()->iter, qVar);
      ExprVector& ksvec = KS[invNum];
      Expr forallSubrange1, forallSubrange2;
      genForallSubranges(invNum, kVar, my_qvits[invNum].front()->iter, prerange, postrange,
                         forallSubrange1, forallSubrange2);
      ExprVector args;
      args.push_back(my_qvits[invNum].front()->iter->left());
      map<Expr, map<int, ExprVector>>::iterator it = consMap.begin();
      while(it != consMap.end()) {
        args.push_back(mk<AND>(mk<IMPL>(forallSubrange1, conjoin(consMap[it->first][2], m_efac)),
                               mk<IMPL>(forallSubrange2, conjoin(consMap[it->first][1], m_efac))));
        it++;
      }
      Expr forallSubformula1 = mknary<FORALL>(args);
      ksvec.push_back(mk<AND>(ksrange, forallSubformula1));
      if(o.getVerbosity() > 3) outs () << "\nForAll Subformula 1: " << *forallSubformula1 << "\n";
      args.clear();
      args.push_back(my_qvits[invNum].front()->iter->left());
      it = consMap.begin();
      while(it != consMap.end()) {
        args.push_back(mk<AND>(mk<IMPL>(forallSubrange1, conjoin(consMap[it->first][1], m_efac)),
                               mk<IMPL>(forallSubrange2, conjoin(consMap[it->first][2], m_efac))));
        it++;
      }
      Expr forallSubformula2 = mknary<FORALL>(args);
      ksvec.push_back(mk<AND>(ksrange, forallSubformula2));
      if(o.getVerbosity() > 3) outs () << "\nForall Subformula 2: " << *forallSubformula2 << "\n";
      args.clear();
      args.push_back(my_qvits[invNum].front()->iter->left());
      it = consMap.begin();
      while(it != consMap.end()) {
        args.push_back(mk<AND>(mk<IMPL>(forallSubrange1, conjoin(consMap[it->first][3], m_efac)),
                               mk<IMPL>(forallSubrange2, conjoin(consMap[it->first][0], m_efac))));
        it++;
      }
      Expr forallSubformula3 = mknary<FORALL>(args);
      ksvec.push_back(mk<AND>(ksrange, forallSubformula3));
      if(o.getVerbosity() > 3) outs () << "\nForAll Subformula 3: " << *forallSubformula3 << "\n";
      args.clear();
      args.push_back(my_qvits[invNum].front()->iter->left());
      it = consMap.begin();
      while(it != consMap.end()) {
        args.push_back(mk<AND>(mk<IMPL>(forallSubrange1, conjoin(consMap[it->first][0], m_efac)),
                               mk<IMPL>(forallSubrange2, conjoin(consMap[it->first][3], m_efac))));
        it++;
      }
      Expr forallSubformula4 = mknary<FORALL>(args);
      ksvec.push_back(mk<AND>(ksrange, forallSubformula4));
      if(o.getVerbosity() > 3) outs () << "\nForAll Subformula 4: " << *forallSubformula4 << "\n";
    }

    void synthesizeK(const int invNum)
    {
      if(o.getVerbosity() > 1) outs () << "\nSynthesizing K for inv: " << invNum << "\n";
      ExprVector& kresvec = KSResult[invNum];
      ExprVector& ksvec = KS[invNum];
      for(int i=0; i<4; i++)
        kresvec.push_back(u.isSat(ksvec[i]) ? getSkolemExpr(ksvec[i]) : mkMPZ(-1, m_efac));
      Expr K = mkMPZ(-1, m_efac);
      for(int i=0; i<4; i++)
        if(kresvec[i] != mkMPZ(-1, m_efac)) K = kresvec[i];
      if(o.getVerbosity() > 1) outs () << "\nSynthesized value of K is: " << K << "\n";
    }

    Expr getSkolemExpr(const Expr e)
    {
      // TODO: Interface with skolem generation
      return mkMPZ(-1, m_efac);
    }

    bool checkOverlapFn()
    {
      if(o.getVerbosity() > 1) outs () << "\nChecking Overlap in Array Accesses\n";
      for (auto & hr : ruleManager.chcs) {
        if (hr.isFact || hr.isQuery || hr.srcRelation != hr.dstRelation) continue;
        int invNum = getVarIndex(hr.srcRelation, decls);
        if(invNum < 0) continue;
        bool overlap = checkOverlappingIndices(invNum);
        if(overlap) {
          if(o.getVerbosity() > 1)
            outs () << "\nOverlapping indices present in array access for loop:" << invNum << "\n";
          return true;
        }
      }
      return false;
    }

    bool checkModifiedFn()
    {
      if(o.getVerbosity() > 1) outs () << "\nChecking for modified variabes in the Array Accesses\n";
      for (auto & hr : ruleManager.chcs) {
        if (hr.isFact || hr.isQuery || hr.srcRelation != hr.dstRelation) continue;
        int invNum = getVarIndex(hr.srcRelation, decls);
        if(invNum < 0) continue;
        bool modified = checkModified(invNum);
        if(modified) {
          if(o.getVerbosity() > 1)
            outs () << "\nModified variables present in the array access for loop:" << invNum << "\n";
          return true;
        }
      }
      return false;
    }

  public:
    KSynthesizer (ExprFactory &efac, CHCs& r, Learner& ds_,
                  RuleInfoManager& rim_, Options& opt) :
      m_efac(efac), ruleManager(r), ds(ds_), decls(ds_.getDecls()),
      my_qvits(ds_.getQvits()),      /* TODO: Instead of iterators, iterGrows, preconds, postconds use new DATA STRUCT  */
      rim(rim_), o(opt), u(efac)
    {}

    inline bool checkOverlap() { return checkOverlapFn(); }
    inline bool checkModifiedVarsInIndices() { return checkModifiedFn(); }
    inline bool runKSynthesizer() { return runKSynth(); }

  };

}

#endif
