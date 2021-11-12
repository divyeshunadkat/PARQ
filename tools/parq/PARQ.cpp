#include "parq/Options.hpp"
#include "parq/Parallelize.hpp"

using namespace std;
using namespace ufo;
using namespace parq;

int main (int argc, char ** argv) {
  Options o;
  o.parse_cmdline(argc, argv);
  parallelizeCHCs(o);
  return 0;
}
