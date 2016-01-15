#include "MetnoSymbol.h"
#include <cmath>
#include "../Util.h"
#include "../File/File.h"
#include "../ParameterFile/ParameterFile.h"
CalibratorMetnoSymbol::CalibratorMetnoSymbol(const Options& iOptions) :
      Calibrator(iOptions),
      mFactory(weather_symbol::Factory(1)) {
   if(!iOptions.getValues("precipThresholds", mPrecipThresholds)) {
      mPrecipThresholds.push_back(0.5);
      mPrecipThresholds.push_back(1);
      mPrecipThresholds.push_back(2);
   }
   if(mPrecipThresholds.size() != 3) {
      Util::error("Number of precipitation thresholds must be 3");
   }

   if(!iOptions.getValues("cloudThresholds", mCloudThresholds)) {
      mCloudThresholds.push_back(13);
      mCloudThresholds.push_back(38);
      mCloudThresholds.push_back(86);
   }
   if(mCloudThresholds.size() != 3) {
      Util::error("Number of cloud thresholds must be 3");
   }
}
bool CalibratorMetnoSymbol::calibrateCore(File& iFile, const ParameterFile* iParameterFile) const {
   int nLat = iFile.getNumLat();
   int nLon = iFile.getNumLon();
   int nTime = iFile.getNumTime();
   int nEns = iFile.getNumEns();
   vec2 lats = iFile.getLats();
   vec2 lons = iFile.getLons();
   vec2 elevs = iFile.getElevs();

   for(int t = 0; t < nTime; t++) {
      Field& t2mField = *iFile.getField(Variable::T, t);
      Field& precipField = *iFile.getField(Variable::Precip, t);
      Field& cloudField = *iFile.getField(Variable::Cloud, t);
      Field& symbolField = *iFile.getField(Variable::Symbol, t);

      #pragma omp parallel for
      for(int i = 0; i < nLat; i++) {
         for(int j = 0; j < nLon; j++) {
            for(int e = 0; e < nEns; e++) {
               float t2m = t2mField(i,j,e);
               float precip = precipField(i,j,e);
               float cloud = cloudField(i,j,e);
               float symbol = Util::MV;
               if(Util::isValid(t2m) && Util::isValid(precip) && Util::isValid(cloud)) {
                  // Calculate number of drops
                  int numberOfDrops = getNumberOfDrops(precip);

                  // Calculate cloud cover
                  float cloudCover = getCloudyness(cloud);
                  symbol = getSymbol(numberOfDrops, cloudCover);
               }
               symbolField(i,j,e) = symbol;
            }
         }
      }
   }
   return true;
}

int CalibratorMetnoSymbol::getSymbol(int iNumberOfDrops, int iCloudyness) const {
   return mFactory.getCode_(iCloudyness, iNumberOfDrops);
   return iNumberOfDrops + iCloudyness * 10;
}
int CalibratorMetnoSymbol::getNumberOfDrops(float iPrecip) const {
   int numberOfDrops = Util::MV;
   if(Util::isValid(iPrecip)) {
      if(iPrecip < mPrecipThresholds[0])
         numberOfDrops = 0;
      else if(iPrecip < mPrecipThresholds[1])
         numberOfDrops = 1;
      else if(iPrecip < mPrecipThresholds[2])
         numberOfDrops = 2;
      else
         numberOfDrops = 3;
   }
   return numberOfDrops;
}
int CalibratorMetnoSymbol::getCloudyness(float iCloudCover) const {
   int cloudyness = Util::MV;
   if(Util::isValid(iCloudCover)) {
      if(iCloudCover <= mCloudThresholds[0])
         cloudyness = 0;
      else if(iCloudCover <= mCloudThresholds[1])
         cloudyness = 1;
      else if(iCloudCover <= mCloudThresholds[2])
         cloudyness = 2;
      else
         cloudyness = 3;
   }
   return cloudyness;
}

std::string CalibratorMetnoSymbol::description() {
   std::stringstream ss;
   ss << Util::formatDescription("-c metnoSymbol", "Adds the MET Norway symbol to the file.") << std::endl;
   ss << Util::formatDescription("   precipThresholds=0.5,1,2", "Precipitation thresholds (in mm) for 1, 2, and 3 drops respectively.") << std::endl;
   ss << Util::formatDescription("   cloudThresholds=13,38,86", "Cloud cover thresholds (in %) for light cloud, partly cloudy, and cloudy respectively.") << std::endl;
   return ss.str();
}
