#ifndef CALIBRATOR_METNO_SYMBOL_H
#define CALIBRATOR_METNO_SYMBOL_H
#include "Calibrator.h"
#include "../weather_symbol/Factory.h"
class ParameterFile;
class Parameters;

//! Adds the weather symbol to the file
class CalibratorMetnoSymbol : public Calibrator {
   public:
      CalibratorMetnoSymbol(const Options& iOptions);
      static std::string description();
      std::string name() const {return "symbol";};
      bool requiresParameterFile() const {return false;};
      int getSymbol(int iNumberOfDrops, int iCloudyness) const;
   private:
      bool calibrateCore(File& iFile, const ParameterFile* iParameterFile) const;
      int getNumberOfDrops(float iPrecip) const;
      int getCloudyness(float iCloudCover) const;
      std::vector<float> mPrecipThresholds;
      std::vector<float> mCloudThresholds;
      weather_symbol::Factory mFactory;
};
#endif
