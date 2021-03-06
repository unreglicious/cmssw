/*
 *  callback_t.cc
 *  EDMProto
 *  Created by Chris Jones on 4/17/05.
 *  Changed by Viji Sundararajan on 03-Jul-05.
 *
 */

#include <memory>
#include "cppunit/extensions/HelperMacros.h"
#include "FWCore/Utilities/interface/do_nothing_deleter.h"
#include "FWCore/Framework/interface/Callback.h"
#include "FWCore/Framework/interface/ESProducts.h"
#include <cassert>

namespace callbacktest {
   struct Data {
      Data() : value_(0) {}
      Data(int iValue) : value_(iValue) {}
      virtual ~Data() {}
      int value_;
   };
   
   struct Double {
      Double() : value_(0) {}
      Double(double iValue) : value_(iValue) {}
      virtual ~Double() {}
      double value_;
   };

   struct Record { };
   
   struct ConstPtrProd {
      ConstPtrProd() : data_() {}
      const Data* method(const Record&) {
         ++data_.value_;
         return &data_;
      }      
      Data data_;
   };

   struct UniquePtrProd {
      UniquePtrProd() : value_(0) {}
      std::unique_ptr<Data> method(const Record&) {
         return std::make_unique<Data>(++value_);
      }
      
      int value_;
   };

   struct SharedPtrProd {
      SharedPtrProd() : ptr_(new Data()) {}
      std::shared_ptr<Data> method(const Record&) {
         ++ptr_->value_;
         return ptr_;
      }      
      std::shared_ptr<Data> ptr_;
   };
   
   struct PtrProductsProd {
      PtrProductsProd() : data_(), double_() {}
      edm::ESProducts<std::shared_ptr<Data>, std::shared_ptr<Double>> method(const Record&) {
         using namespace edm::es;
         auto dataT = std::shared_ptr<Data>(&data_, edm::do_nothing_deleter());;
         auto doubleT = std::shared_ptr<Double>(&double_, edm::do_nothing_deleter());
         ++data_.value_;
         ++double_.value_;
         return products(dataT, doubleT);
      }
      Data data_;
      Double double_;
   };
}

using namespace callbacktest;
using namespace edm::eventsetup;

class testCallback: public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(testCallback);

CPPUNIT_TEST(uniquePtrTest);
CPPUNIT_TEST(sharedPtrTest);
CPPUNIT_TEST(ptrProductsTest);

CPPUNIT_TEST_SUITE_END();
public:
  void setUp(){}
  void tearDown(){}

  void uniquePtrTest();
  void sharedPtrTest();
  void ptrProductsTest();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testCallback);

typedef Callback<UniquePtrProd, std::unique_ptr<Data>, Record> UniquePtrCallback;

void testCallback::uniquePtrTest()
{
   UniquePtrProd prod;
   
   UniquePtrCallback callback(&prod, &UniquePtrProd::method);
   std::unique_ptr<Data> handle;
   
   
   callback.holdOntoPointer(&handle);
   
   Record record;
   callback.newRecordComing();
   callback(record);
   CPPUNIT_ASSERT(0 != handle.get());
   CPPUNIT_ASSERT(prod.value_ == 1);
   assert(0 != handle.get());
   CPPUNIT_ASSERT(prod.value_ == handle->value_);
   
   //since haven't cleared, should not have changed
   callback(record);
   CPPUNIT_ASSERT(prod.value_ == 1);
   CPPUNIT_ASSERT(prod.value_ == handle->value_);
   
   handle.release();

   callback.newRecordComing();
   
   callback(record);
   CPPUNIT_ASSERT(0 != handle.get());
   CPPUNIT_ASSERT(prod.value_ == 2);
   assert(0 != handle.get());
   CPPUNIT_ASSERT(prod.value_ == handle->value_);
   
}

typedef Callback<SharedPtrProd, std::shared_ptr<Data>, Record> SharedPtrCallback;

void testCallback::sharedPtrTest()
{
   SharedPtrProd prod;
   
   SharedPtrCallback callback(&prod, &SharedPtrProd::method);
   std::shared_ptr<Data> handle;
   
   
   callback.holdOntoPointer(&handle);
   
   Record record;
   callback.newRecordComing();
   callback(record);
   CPPUNIT_ASSERT(handle.get() == prod.ptr_.get());
   CPPUNIT_ASSERT(prod.ptr_->value_ == 1);
   
   //since haven't cleared, should not have changed
   callback(record);
   CPPUNIT_ASSERT(handle.get() == prod.ptr_.get());
   CPPUNIT_ASSERT(prod.ptr_->value_ == 1);
   
   handle.reset() ;
   callback.newRecordComing();
   
   callback(record);
   CPPUNIT_ASSERT(handle.get() == prod.ptr_.get());
   CPPUNIT_ASSERT(prod.ptr_->value_ == 2);
   
}

typedef Callback<PtrProductsProd, edm::ESProducts<std::shared_ptr<Data>, std::shared_ptr<Double>>, Record> PtrProductsCallback;

void testCallback::ptrProductsTest()
{
   PtrProductsProd prod;
   
   PtrProductsCallback callback(&prod, &PtrProductsProd::method);
   std::shared_ptr<Data> handle;
   std::shared_ptr<Double> doubleHandle;
   
   callback.holdOntoPointer(&handle);
   callback.holdOntoPointer(&doubleHandle);
   
   Record record;
   callback.newRecordComing();
   callback(record);
   CPPUNIT_ASSERT(handle.get() == &(prod.data_));
   CPPUNIT_ASSERT(prod.data_.value_ == 1);
   
   //since haven't cleared, should not have changed
   callback(record);
   CPPUNIT_ASSERT(handle.get() == &(prod.data_));
   CPPUNIT_ASSERT(prod.data_.value_ == 1);
   
   callback.newRecordComing();
   
   callback(record);
   CPPUNIT_ASSERT(handle.get() == &(prod.data_));
   CPPUNIT_ASSERT(prod.data_.value_ == 2);
   
}
