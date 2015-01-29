#include "Setup.h"
#include "File/File.h"
#include "Calibrator/Calibrator.h"
#include "Downscaler/Downscaler.h"

Setup::Setup(std::vector<std::string> argv) {
   inputFile  = File::getScheme(argv[0], true);
   outputFile = File::getScheme(argv[1], false);

   // Implement a finite state machine
   enum State {START = 0, VAR = 1, NEWVAR = 2, DOWN = 10, DOWNOPT = 15, CAL = 20, NEWCAL = 22, CALOPT = 25, END = 30, ERROR = 40};
   State state = START;
   State prevState = START;

   Variable::Type variable;
   Options dOptions;
   Options cOptions;
   int index = 2;
   std::string errorMessage = "";

   if(argv.size() == 2) {
      return;
   }

   std::string downscaler = defaultDownscaler();
   std::string calibrator = "";
   std::vector<Calibrator*> calibrators;
   while(true) {
      // std::cout << state << std::endl;
      if(state != ERROR)
         prevState = state;
      if(state == START) {
         if(argv[index] == "-v") {
            state = VAR;
            index++;
         }
         else {
            errorMessage = "No variables defined";
            state = ERROR;
         }
      }
      else if(state == VAR) {
         if(argv.size() <= index) {
            // -v but nothing after it
            errorMessage = "No variable after '-v'";
            state = ERROR;
         }
         else {
            variable = Variable::getType(argv[index]);
            index++;
            if(argv.size() <= index) {
               state = NEWVAR;
            }
            else if(argv[index] == "-v") {
               state = NEWVAR;
            }
            else if(argv[index] == "-d") {
               state = DOWN;
               index++;
            }
            else if(argv[index] == "-c") {
               state = CAL;
               index++;
            }
            else {
               errorMessage = "No recognized option after '-v var'";
               state = ERROR;
            }
         }
      }
      else if(state == NEWVAR) {
         // Check that we haven't added the variable before
         bool alreadyExists = false;
         for(int i = 0; i < variableConfigurations.size(); i++) {
            if(variableConfigurations[i].variable == variable)
               alreadyExists = true;
         }

         if(!alreadyExists) {
            dOptions.addOption("variable", Variable::getTypeName(variable));
            Downscaler* d = Downscaler::getScheme(downscaler, variable, dOptions);
            VariableConfiguration varconf;
            varconf.variable = variable;
            varconf.downscaler = d;
            varconf.calibrators = calibrators;
            variableConfigurations.push_back(varconf);
         }
         else {
            Util::warning("Variable '" + Variable::getTypeName(variable) + "' already read. Using first instance.");
         }

         // Reset to defaults
         downscaler = defaultDownscaler();
         dOptions.clear();
         calibrators.clear();

         if(argv.size() <= index) {
            state = END;
         }
         else {
            state = VAR;
            index++;
         }
      }
      else if(state == DOWN) {
         if(argv.size() <= index) {
            // -d but nothing after it
            errorMessage = "No downscaler after '-d'";
            state = ERROR;
         }
         else {
            downscaler = argv[index];
            index++;
            if(argv.size() <= index) {
               state = NEWVAR;
            }
            else if(argv[index] == "-c") {
               state = CAL;
               index++;
            }
            else if(argv[index] == "-v") {
               state = NEWVAR;
            }
            else if(argv[index] == "-d") {
               // Two downscalers defined for one variable
               state = DOWN;
               index++;
            }
            else {
               state = DOWNOPT;
            }
         }
      }
      else if(state == DOWNOPT) {
         if(argv.size() <= index) {
            state = NEWVAR;
         }
         else if(argv[index] == "-c") {
            state = CAL;
            index++;
         }
         else if(argv[index] == "-v") {
            state = NEWVAR;
         }
         else {
            // Process downscaler options
            dOptions.addOptions(argv[index]);
            index++;
         }
      }
      else if(state == CAL) {
         if(argv.size() <= index) {
            // -c but nothing after it
            state = ERROR;
            errorMessage = "No calibrator after '-c'";
         }
         else {
            calibrator = argv[index];
            index++;
            if(argv.size() <= index) {
               state = NEWCAL;
            }
            else if(argv[index] == "-v") {
               state = NEWCAL;
            }
            else if(argv[index] == "-c") {
               state = NEWCAL;
            }
            else if(argv[index] == "-d") {
               state = NEWCAL;
            }
            else {
               state = CALOPT;
            }
         }
      }
      else if(state == CALOPT) {
         if(argv.size() <= index) {
            state = NEWCAL;
         }
         else if(argv[index] ==  "-c") {
            state = NEWCAL;
         }
         else if(argv[index] == "-v") {
            state = NEWCAL;
         }
         else if(argv[index] == "-d") {
            state = NEWCAL;
         }
         else {
            // Process calibrator options
            cOptions.addOptions(argv[index]);
            index++;
         }
      }
      else if(state == NEWCAL) {
         // We do not need to check that the same calibrator has been added for this variable
         // since this is perfectly fine (e.g. smoothing twice).
         cOptions.addOption("variable", Variable::getTypeName(variable));
         Calibrator* c = Calibrator::getScheme(calibrator, cOptions);
         calibrators.push_back(c);

         // Reset
         calibrator = "";
         cOptions.clear();
         if(argv.size() <= index) {
            state = NEWVAR;
         }
         else if(argv[index] == "-c") {
            state = CAL;
            index++;
         }
         else if(argv[index] == "-v") {
            state = NEWVAR;
         }
         else if(argv[index] == "-d") {
            state = DOWN;
            index++;
         }
         else {
            state = ERROR;
            errorMessage = "No recognized option after '-c calibrator'";
         }
      }
      else if(state == END) {
         break;
      }
      else if(state == ERROR) {
         std::stringstream ss;
         ss << "Could not understand command line arguments: " << errorMessage << ".";
         Util::error(ss.str());
      }
   }
}
Setup::~Setup() {
   delete inputFile;
   delete outputFile;
   for(int i = 0; i < variableConfigurations.size(); i++) {
      delete variableConfigurations[i].downscaler;
      for(int c = 0; c < variableConfigurations[i].calibrators.size(); c++) {
         delete variableConfigurations[i].calibrators[c];
      }
   }
}
std::string Setup::defaultDownscaler() {
   return "nearestNeighbour";
}
