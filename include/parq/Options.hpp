#ifndef __OPTIONS_HPP
#define __OPTIONS_HPP

#include <string.h>
#include <iostream>
#include <vector>

namespace parq {

  using namespace std;

  class Options
  {
  protected:
    string inputFileName = "";
    string inputFilePath = "";
    string outputFileName = "";
    string outputFilePath = "";

    int verbosity = 0;
    int printLog = 0;
    string version = "1.0";
    string toolName = "parq";

  public:
    inline string getInputFile() { return inputFilePath; }
    inline string getInputFileName() { return inputFileName; }
    inline string getOutputFile() { return outputFilePath; }
    inline string getOutputFileName() { return outputFileName; }
    inline int getVerbosity() { return verbosity; }
    inline int getPrintLog() { return printLog; }

    bool parse_cmdline(int argc, char** argv) {
      const char *OPT_HELP = "--h";
      const char *OPT_OUT = "--o";
      const char *OPT_VERB = "--v";
      const char *OPT_LOG = "--l";

      if (getBoolValue(OPT_HELP, false, argc, argv) || argc == 1){
        cout <<
          "Usage:                        Purpose:\n" <<
          " " << toolName << " [--h]                  show help\n" <<
          " " << toolName << " [options] <file.smt2>  parallelize a system of constrained Horn clauses\n\n" <<
          "Options:\n" <<
          // " " << OPT_OUT << "                          set output file\n" <<
          " " << OPT_VERB << "                          set verbosity\n" <<
          " " << OPT_LOG << "                          enable logging\n";
        return 0;
      }

      inputFilePath = string(argv[argc-1]);
      // char * defout = "/tmp/output.smt2";
      // outputFilePath = getStrValue(OPT_OUT, defout, argc, argv);
      verbosity = getIntValue(OPT_VERB, 0, argc, argv);
      printLog = getIntValue(OPT_LOG, 0, argc, argv);
    }

  private:
    bool getBoolValue(const char * opt, bool defValue, int argc, char ** argv)
    {
      for (int i = 1; i < argc; i++)
        {
          if (strcmp(argv[i], opt) == 0) return true;
        }
      return defValue;
    }

    char * getStrValue(const char * opt, char * defValue, int argc, char ** argv)
    {
      for (int i = 1; i < argc-1; i++)
        {
          if (strcmp(argv[i], opt) == 0)
            {
              return argv[i+1];
            }
        }
      return defValue;
    }

    int getIntValue(const char * opt, int defValue, int argc, char ** argv)
    {
      for (int i = 1; i < argc-1; i++)
        {
          if (strcmp(argv[i], opt) == 0)
            {
              char* p;
              int num = strtol(argv[i+1], &p, 10);
              if (*p) return 1;      // if used w/o arg, return boolean
              else return num;
            }
        }
      return defValue;
    }

    void getStrValues(const char * opt, vector<string> & values, int argc, char ** argv)
    {
      for (int i = 1; i < argc-1; i++)
        {
          if (strcmp(argv[i], opt) == 0)
            {
              values.push_back(string(argv[i+1]));
            }
        }
    }

  };

}

#endif
