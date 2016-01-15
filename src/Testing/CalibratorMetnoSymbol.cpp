#include "../File/Fake.h"
#include "../Util.h"
#include "../ParameterFile/ParameterFile.h"
#include "../Calibrator/MetnoSymbol.h"
#include <gtest/gtest.h>

namespace {
   class TestCalibratorMetnoSymbol : public ::testing::Test {
      protected:
         TestCalibratorMetnoSymbol() {
         }
         virtual ~TestCalibratorMetnoSymbol() {
         }
         virtual void SetUp() {
         }
         virtual void TearDown() {
         }
   };
   TEST_F(TestCalibratorMetnoSymbol, 10x10) {
      FileArome from("testing/files/10x10.nc");
      CalibratorMetnoSymbol cal = CalibratorMetnoSymbol(Options("precipThresholds=0.5,1,2"));

      cal.calibrate(from);

      FieldPtr after = from.getField(Variable::Symbol, 0);
      EXPECT_FLOAT_EQ(9, (*after)(5,2,0)); // 1.384878 mm, 100%
      EXPECT_FLOAT_EQ(46, (*after)(2,9,0)); // 0.9287777 mm, 100%
   }
   // Invalid thresholds
   TEST_F(TestCalibratorMetnoSymbol, locationIndependent) {
      ::testing::FLAGS_gtest_death_test_style = "threadsafe";
      Util::setShowError(false);
      EXPECT_DEATH(CalibratorMetnoSymbol(Options("precipThresholds=0,1")), ".*");
      EXPECT_DEATH(CalibratorMetnoSymbol(Options("precipThresholds=0,1 cloudThresholds=10,20,30")), ".*");
      EXPECT_DEATH(CalibratorMetnoSymbol(Options("precipThresholds=0,1,2 cloudThresholds=10,20")), ".*");
      EXPECT_DEATH(CalibratorMetnoSymbol(Options("cloudThresholds=10,20")), ".*");
   }
   TEST_F(TestCalibratorMetnoSymbol, description) {
      CalibratorMetnoSymbol::description();
   }
}
int main(int argc, char **argv) {
     ::testing::InitGoogleTest(&argc, argv);
       return RUN_ALL_TESTS();
}
