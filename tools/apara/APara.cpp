#include "apara/Options.hpp"
#include "apara/Parallelize.hpp"

using namespace std;
using namespace ufo;
using namespace apara;

int main (int argc, char ** argv) {
  Options o;
  o.parse_cmdline(argc, argv);
  parallelizeCHCs(o);
  return 0;
}
