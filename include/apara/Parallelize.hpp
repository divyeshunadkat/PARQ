#ifndef __PARALLELIZE_HPP
#define __PARALLELIZE_HPP

#include <fstream>
#include "apara/Options.hpp"
#include "apara/Learner.hpp"
#include "apara/RuleInfoManager.hpp"
#include "apara/KSynthesizer.hpp"

using namespace std;
using namespace ufo;

namespace apara
{
  class Parallelize
  {
  protected:
    ExprFactory m_efac;
    EZ3 z3;
    CHCs ruleManager;
    ZSolver<EZ3> m_smt_solver;
    SMTUtils u;
    Options& o;
    Learner ds;

    map<int, Expr> invExpr;
    map<int, ExprSet> eqInvExprSet;
    int maxAttempts = 2000000;
    bool parallelized = false;

    void initializeLearner()
    {
      if(o.getVerbosity() > 1) cout << "\nInitializing Learner\n";
      BndExpl bnd(ruleManager, 500, 20);
      if (!ruleManager.hasCycles())
        return (void)bnd.exploreTraces(1, ruleManager.chcs.size(), true);

      map<Expr, ExprSet> cands;
      for (int i = 0; i < ruleManager.cycles.size(); i++)
      {
        Expr dcl = ruleManager.chcs[ruleManager.cycles[i][0]].srcRelation;
        if (ds.initializedDecl(dcl)) continue;
        ds.initializeDecl(dcl);
        if (!true/*dSee*/) continue;

        Expr pref = bnd.compactPrefix(i);
        ExprSet tmp;
        getConj(pref, tmp);
        for (auto & t : tmp)
          if (hasOnlyVars(t, ruleManager.invVars[dcl]))
            cands[dcl].insert(t);

        if (/*mut*/1 > 0) ds.mutateHeuristicEq(cands[dcl], cands[dcl], dcl, true);
        ds.initializeAux(cands[dcl], bnd, i, pref);
      }

      if (true/*enableDataLearning*/) ds.getDataCandidates(cands);

      for (auto & dcl: ruleManager.wtoDecls)
      {
        for (int i = 0; i < 0/*doProp*/; i++)  // TODO: Check with GF
          for (auto & a : cands[dcl]) ds.propagate(dcl, a, true);
        ds.addCandidates(dcl, cands[dcl]);
        ds.prepareSeeds(dcl, cands[dcl]);
      }
    }

    bool bootstrapInvs()
    {
      if(o.getVerbosity() > 1) cout << "\nBootstrapping Invariants\n";
      bool res = ds.bootstrap();
      if(o.getVerbosity() > 3) ds.printSolution();
      return res;
    }

    bool learnInvs()
    {
      if(o.getVerbosity() > 1) cout << "\nLearning Invariants\n";
      ds.calculateStatistics();
      ds.deferredPriorities();
      std::srand(std::time(0));
      ds.synthesize(maxAttempts);
      if(o.getVerbosity() > 3) ds.printSolution();
      return true;
    }

    bool getEqualityInvs()
    {
      if(o.getVerbosity() > 1) cout << "\nFetching Equality Invariants by Merging Ineqs\n";
      bool result = false;
      for (int i = 0; i < ds.getDecls().size(); i++)
      {
        Expr rel = ds.getDecls()[i];
        SamplFactory& sf = ds.getSFS()[i].back();
        ExprSet eqInvs;
        std::set<Expr>::iterator it1, it2;
        for (it1 = sf.learnedExprs.begin(); it1 != sf.learnedExprs.end(); ++it1) {
          cout << "\nIT1\n"; pprint(*it1);
          Expr e1 = *it1;
          if (!containsOp<FORALL>(e1)) continue;
          if (!(isOpX<IMPL>(e1->last()))) continue;
            // Following is available by construction for single loop CHC programs
            // if (!u.checkSameExpr(e1->last()->left(), e2->last()->left())) continue;
            // TODO: Support the above for multiple loops
          Expr impl1 = e1->last();
          Expr consequent1 = impl1->last();
          if (!(isOpX<EQ>(consequent1))) continue;
          if(o.getVerbosity() > 3) {
            outs () << "\nIdentified equality invariant\n";
            u.print(e1); outs () << "\n";
          }
          eqInvs.insert( e1 );
          result = true;
        }
        for (it1 = sf.learnedExprs.begin(); it1 != sf.learnedExprs.end(); ++it1) {
          cout << "\nIT1\n"; pprint(*it1);
          for (it2 = it1; it2 != sf.learnedExprs.end(); ++it2) {
            cout << "\nIT2\n"; pprint(*it2);
            if(it1 == it2)
              continue;
            Expr e1 = *it1, e2 = *it2;
            if (!(containsOp<FORALL>(e1) && containsOp<FORALL>(e2))) continue;
            if (e1->arity() != e2->arity()) continue;
            if (!(isOpX<IMPL>(e1->last()) && isOpX<IMPL>(e2->last()))) continue;
            // Following is available by construction for single loop CHC programs
            // if (!u.checkSameExpr(e1->last()->left(), e2->last()->left())) continue;
            // TODO: Support the above for multiple loops
            Expr impl1 = e1->last(), impl2 = e2->last();
            Expr consequent1 = impl1->last(); Expr consequent2 = impl2->last();
            if (!(isOp<ComparissonOp>(consequent1) && isOp<ComparissonOp>(consequent2))) continue;
            if(!u.isSat(mk<EQ>(mkMPZ(0, m_efac),
                               mk<PLUS>(consequent1->left(), consequent2->left())),
                        mk<EQ>(mkMPZ(0, m_efac),
                               mk<PLUS>(consequent1->right(), consequent2->right())))) continue;
            if(u.isSat(mk<NEQ>(mkMPZ(0, m_efac),
                               mk<PLUS>(consequent1->left(), consequent2->left())))) continue;
            if(u.isSat(mk<NEQ>(mkMPZ(0, m_efac),
                               mk<PLUS>(consequent1->right(), consequent2->right())))) continue;
            if(o.getVerbosity() > 3) {
              outs () << "\nIdentified expressions that form a equality\n";
              u.print(consequent1); outs () << "\n"; u.print(consequent2); outs () << "\n";
            }
            Expr eqConsequent = mk<EQ>(consequent1->left(), consequent1->right());
            Expr eqInv = replaceAll(e1, consequent1, eqConsequent);
            if(o.getVerbosity() > 3) {
              outs () << "\nIdentified equality invariant\n";
              u.print(eqInv); outs () << "\n";
            }
            eqInvs.insert( eqInv );
            result = true;
          }
        }
        eqInvExprSet[i] = eqInvs;
      }
      return result;
    }


  public:
    Parallelize (Options& opt) :
      z3(m_efac), ruleManager(m_efac, z3, opt.getVerbosity()-5), m_smt_solver(z3), u(m_efac), o(opt),
      // ExprFactory &efac, EZ3 &z3, CHCs& r, unsigned to, bool freqs, bool aggp, int _mu, bool _d, int _m,
      // bool _dAllMbp, bool _dAddProp, bool _dAddDat, bool _dStrenMbp, int _dFwd, int _to, int debug
      ds(m_efac, z3, ruleManager, 500, false, false, 2, false, 0,
         false, false, false, false, 1, 1000, opt.getVerbosity(), o)
    {
      ds.setPrintLog(o.getPrintLog());
      ruleManager.parse(o.getInputFile(), true, false);
      initializeLearner();
    }

    bool makeParallel()
    {
      if(o.getVerbosity() > 1) outs () << "\nInvoked the Parallelization Engine\n";
      RuleInfoManager rim(ruleManager, ds.getDecls(), o);
      KSynthesizer ksynth(m_efac, ruleManager, ds, rim, o);
      bool bs = bootstrapInvs();
      bool mv = ksynth.checkModifiedVarsInIndices();
      bool co = ksynth.checkOverlap();
      if(mv && !co) {
        bool rks = ksynth.runKSynthesizer();
        if(rks) outs () << "\nPARALLELIZATION_BY_SKOLEM_SUCCESSFUL\n";
        else outs () << "\nPARALLELIZATION_UNKNOWN(1)\n";
        return rks;
      } else {
        if(bs) learnInvs();
        bool ei = getEqualityInvs();
        if(ei) {
          outs () << "\nPARALLELIZATION_BY_EQ_INVS_SUCCESSFUL\n";
          /*
          bool aei = applyEqInvToCHCs(rim);
          if(aei) outs () << "\nPARALLELIZATION_SUCCESSFUL\n";
          else outs () << "\nPARALLELIZATION_UNKNOWN\n";
          return aei;
          */
        } else outs () << "\nPARALLELIZATION_UNKNOWN(2)\n";
        return ei;
      }
      /*
      getSimplifiedInvExpr();
      transformCHCs();
      printTransformedCHCs();
      outputParallelVersion();
      return parallelized;
      */
    }

  };

  inline static bool parallelizeCHCs(Options& o)
  {
    Parallelize par(o);
    return par.makeParallel();
  }

}

#endif

/*
    void getSimplifiedInvExpr()
    {
      if(o.getVerbosity() > 1) cout << "\nSimplifying Invariant Expressions\n";
      for (int i = 0; i < ds.getDecls().size(); i++)
      {
        Expr rel = ds.getDecls()[i];
        SamplFactory& sf = ds.getSFS()[i].back();
        ExprSet lms;
        for( auto & e : sf.learnedExprs )
          if (containsOp<FORALL>(e)) lms.insert( e->last() );
          else lms.insert(e);

        // TODO: Replace the qv with the iterator
        // for (auto &hr: ruleManager.chcs)
        // {
        //   int iNum = getVarIndex(hr.srcRelation, decls);
        //   if(iNum != i) continue;
        //   if(hr.isFact || hr.isQuery) continue;
        //   for (int h = 0; h < hr.srcVars.size(); h++)
        //   {
        //       if(qvar == hr.srcVars[h])
        //         outs () << "Original iterator variable:" << *(hr.origSrcArgs[h]) << "\n";
        //   }
        // }

        Expr tmp = conjoin(lms, m_efac);
        if (!containsOp<FORALL>(tmp)) u.removeRedundantConjuncts(lms);
        Expr res = simplifyArithm(tmp);
        invExpr[i] = res;
      }
    }

    void transformCHCs()
    {
      if(o.getVerbosity() > 1) cout << "\nTransforming CHCs for output\n";
      for (auto & hr : ruleManager.chcs)
      {
        if (hr.isFact || hr.isQuery || hr.srcRelation != hr.dstRelation) continue;
        int invNum = getVarIndex(hr.srcRelation, ds.getDecls());
        hr.body = invExpr[invNum];
      }
      for (auto & hr : ruleManager.chcs)  // Cannot be merged with loop above due to continue stmts
        hr.rewriteToOrigVars();
    }

    void printTransformedCHCs()
    {
      for (int i = 0; i < ds.getDecls().size(); i++)
      {
        u.print(invExpr[i]);
        outs () << "\n";
      }
    }

    bool outputParallelVersion()
    {
      if (!parallelized) {
        if(o.getVerbosity() > 20)
          cout << "\nUnable to parallelize the given input file " << o.getInputFile() << "\n";
        return false;
      }
      if(o.getVerbosity() > 20)
        cout << "\nWriting the parallelized version to " << o.getOutputFile() << "\n";
      std::ofstream outputFileStream;
      outputFileStream.open(o.getOutputFile());
      outputFileStream << "#Parallelized version of " << o.getInputFile() << "\n";
      outputFileStream << ruleManager;
      outputFileStream.flush();
      if(o.getVerbosity() > 20)
        cout << "\nParallelized version written to " << o.getOutputFile() << "\n";
      return true;
    }

    bool applyEqInvToCHCs(RuleInfoManager& rim)
    {
      bool result = false;
      if(o.getVerbosity() > 1) cout << "\nAppling Inferred Equality Invariants to CHCs\n";
      for (auto & hr : ruleManager.chcs)
      {
        if (hr.isFact || hr.isQuery || hr.srcRelation != hr.dstRelation) continue;
        int invNum = getVarIndex(hr.srcRelation, ds.getDecls());
        // Report if more than one equality is present
        if(eqInvExprSet[invNum].size() > 1)
          if(o.getVerbosity() > 3)
            outs () << "\nWarning: Multiple equality invariants detected for loop: "
                    << invNum << "\n";
        // Fetch inv for this loop/invNum
        Expr inferredInv = *eqInvExprSet[invNum].begin();
        map<Expr, ExprVector>::iterator itst = rim.getAllArrStore()[invNum].begin();
        while (itst != rim.getAllArrStore()[invNum].end())
        {
          Expr e1 = itst->first;
          for(auto & e2 : itst->second) {
            Expr arr= e2->left();
            // Check if inv is for the following array
            Expr ind = e2->right();
            // Check if the index accessed falls in the range of inv
            Expr write = e2->last();
            // Check that indices of each select access are within range the range of inv
            // and that there was a recurrence going on here
            // checkIndInRange();
            // Fetch the inferred value from inv for the rhs expression
            // Expr val = fetchValInv();
            // hr.body = replaceAll(hr.body, write, val);
            // result = true;
          }
          itst++;
        }
      }
      return result;
    }

*/

