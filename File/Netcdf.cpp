#include "Netcdf.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "../Util.h"

FileNetcdf::FileNetcdf(std::string iFilename) :
   File(iFilename),
   mFile(getFilename().c_str(), NcFile::Write) {
   if(!mFile.is_valid()) {
      Util::error("Error: Netcdf file " + iFilename + " not valid");
   }
}

FileNetcdf::~FileNetcdf() {
   mFile.close();
}

bool FileNetcdf::hasVariable(Variable::Type iVariable) const {
   NcError q(NcError::silent_nonfatal); 
   std::string variable = getVariableName(iVariable);
   NcVar* var = mFile.get_var(variable.c_str());
   return var != NULL;
}

float FileNetcdf::getScale(NcVar* iVar) const {
   NcError q(NcError::silent_nonfatal); 
   NcAtt* scaleAtt = iVar->get_att("scale_factor");
   float scale  = 1;
   if(scaleAtt != NULL) {
      scale = scaleAtt->as_float(0);
   }
   return scale;
}
float FileNetcdf::getOffset(NcVar* iVar) const {
   NcError q(NcError::silent_nonfatal); 
   NcAtt* offsetAtt = iVar->get_att("add_offset");
   float offset = 0;
   if(offsetAtt != NULL) {
      offset = offsetAtt->as_float(0);
   }
   return offset;
}

NcDim* FileNetcdf::getDim(std::string iDim) const {
   NcError q(NcError::silent_nonfatal); 
   NcDim* dim = mFile.get_dim(iDim.c_str());
   if(dim == NULL) {
      std::stringstream ss;
      ss << "File '" << getFilename() << "' does not have dimension '" << iDim << "'";
      Util::error(ss.str());
   }
   return dim;
}
NcVar* FileNetcdf::getVar(std::string iVar) const {
   NcError q(NcError::silent_nonfatal); 
   NcVar* var = mFile.get_var(iVar.c_str());
   if(var == NULL) {
      std::stringstream ss;
      ss << "File '" << getFilename() << "' does not have variable '" << iVar << "'";
      Util::error(ss.str());
   }
   return var;
}

float FileNetcdf::getMissingValue(const NcVar* iVar) {
   NcError q(NcError::silent_nonfatal); 
   NcAtt* fillValueAtt = iVar->get_att("_FillValue");
   if(fillValueAtt != NULL)
      return fillValueAtt->as_float(0);
   else
      return ncBad_float;//Util::MV;
}