#ifndef __LEARNER_HPP
#define __LEARNER_HPP

#include <fstream>
#include "apara/Options.hpp"
#include "deep/RndLearnerV2.hpp"
#include "deep/RndLearnerV3.hpp"

using namespace std;
using namespace ufo;

namespace apara
{
  class Learner : public RndLearnerV3
  {
  protected:
    Options& o;

  public:
    Learner (ExprFactory &efac, EZ3 &z3, CHCs& r, unsigned to, bool freqs, bool aggp, int _mu,
             bool _d, int _m, bool _dAllMbp, bool _dAddProp, bool _dAddDat, bool _dStrenMbp,
             int _dFwd, int _to, int debug, Options& opt) :
      RndLearnerV3 (efac, z3, r, to, freqs, aggp, _mu, _d, _m, _dAllMbp, _dAddProp,
                    _dAddDat, _dStrenMbp, _dFwd, _to, debug), o(opt) { }

    inline ExprVector& getDecls() { return decls; }
    inline vector<vector<SamplFactory>>& getSFS() { return sfs; }
    inline map<int, vector<ArrAccessIter* >>& getQvits() { return qvits; }
    inline void setPrintLog(int pLog) { printLog = pLog; }

  };

}

#endif
