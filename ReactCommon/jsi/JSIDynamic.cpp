//  Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
 // LICENSE file in the root directory of this source tree.

#include "JSIDynamic.h"

#include <folly/dynamic.h>
#include <jsi/jsi.h>
#include <android/log.h>

using namespace facebook::jsi;

namespace facebook {
namespace jsi {

int callCount = 0;
int rootCount = 0;

Value valueFromDynamic(Runtime& runtime, const folly::dynamic& dyn) {
  switch (dyn.type()) {
  case folly::dynamic::NULLT:
    return Value::null();
  case folly::dynamic::ARRAY: {
    Array ret = Array(runtime, dyn.size());
    for (size_t i = 0; i < dyn.size(); ++i) {
      ret.setValueAtIndex(runtime, i, valueFromDynamic(runtime, dyn[i]));
    }
    return std::move(ret);
  }
  case folly::dynamic::BOOL:
    return dyn.getBool();
  case folly::dynamic::DOUBLE:
    return dyn.getDouble();
  case folly::dynamic::INT64:
    // Can't use asDouble() here.  If the int64 value is too bit to be
    // represented precisely as a double, folly will throw an
    // exception.
    return (double)dyn.getInt();
  case folly::dynamic::OBJECT: {
    Object ret(runtime);
    for (const auto& element : dyn.items()) {
      Value value = valueFromDynamic(runtime, element.second);
      if (element.first.isNumber() || element.first.isString()) {
        ret.setProperty(runtime, element.first.asString().c_str(), value);
      }
    }
    return std::move(ret);
  }
  case folly::dynamic::STRING:
    return String::createFromUtf8(runtime, dyn.getString());
  }
  CHECK(false);
}

folly::dynamic dynamicFromValue(Runtime& runtime, const Value& value) {
  // std::string prefix = "dynamicFromValue: ";
  // std::string msg = value.toString(runtime).utf8(runtime);
  // throw JSError(runtime, String::createFromUtf8(runtime, prefix + msg));
  // std::string msg = "dynamicFromValue";
  callCount++;
  if (callCount == 1) {
    rootCount++;
    if (rootCount == 10) {
      //__android_log_print(ANDROID_LOG_DEBUG, "com.murderminute.murder", "\n Returning nullptr \n");
      //return nullptr;
      // throw JSError(runtime, "rootCount");
    }
  }
  
  if (value.isUndefined() || value.isNull()) {
    callCount--;
    return nullptr;
  } else if (value.isBool()) {
    callCount--;
    return value.getBool();
  } else if (value.isNumber()) {
    callCount--;
    return value.getNumber();
  } else if (value.isString()) {
    callCount--;
    return value.getString(runtime).utf8(runtime);
  } else {
    // Jakob start
      if (callCount >= 10) {
        
        /*
        std::string msg = "dynamicFromValue array size:";
        msg += std::to_string(array.size(runtime));
        msg += " : ";

        try {
          for (size_t i = 0; i < array.size(runtime); ++i) {
            Value prop = array.getValueAtIndex(runtime, i);
            if (prop.isUndefined()) {
              msg += "undefined, ";
            }
            else if (prop.isString()) {
              msg += prop.getString(runtime).utf8(runtime);
              msg += ", ";
            }
            else if (prop.isObject()) {
              msg += "object, ";
            }
            else {
              msg += "other, ";
            }
          }
        }
        catch (...) {
        }
        throw JSError(runtime, msg);
        */
        callCount--;
        return nullptr;
      }
      // Jakob end
    Object obj = value.getObject(runtime);
    if (obj.isArray(runtime)) {
      Array array = obj.getArray(runtime);
      folly::dynamic ret = folly::dynamic::array();
      for (size_t i = 0; i < array.size(runtime); ++i) {
        ret.push_back(dynamicFromValue(runtime, array.getValueAtIndex(runtime, i)));
      }


      callCount--;
      return ret;
    } else if (obj.isFunction(runtime)) {
      throw JSError(runtime, "JS Functions are not convertible to dynamic");
    } else {
      folly::dynamic ret = folly::dynamic::object();
      Array names = obj.getPropertyNames(runtime);


      // Jakob start
      if (callCount >= 10) {
        /*
        std::string msg = "dynamicFromValue: ";
        try {
          for (size_t i = 0; i < names.size(runtime); ++i) {
            msg += names.getValueAtIndex(runtime, i).getString(runtime).utf8(runtime);
            msg += ", ";
          }
        }
        catch (...) {
        }

        try {
          for (size_t i = 0; i < names.size(runtime); ++i) {
            String name = names.getValueAtIndex(runtime, i).getString(runtime);
            Value prop = obj.getProperty(runtime, name);
            if (prop.isUndefined()) {
              msg += "undefined, ";
            }
            else if (prop.isString()) {
              msg += prop.getString(runtime).utf8(runtime);
              msg += ", ";
            }
            else if (prop.isObject()) {
              msg += "object, ";
            }
            else {
              msg += "other, ";
            }
          }
        }
        catch (...) {
        }
        throw JSError(runtime, msg);
        */
        callCount--;
        return nullptr;
      }
      // Jakob end



      for (size_t i = 0; i < names.size(runtime); ++i) {
        String name = names.getValueAtIndex(runtime, i).getString(runtime);
        Value prop = obj.getProperty(runtime, name);
        if (prop.isUndefined()) {
          continue;
        }
        // The JSC conversion uses JSON.stringify, which substitutes
        // null for a function, so we do the same here.  Just dropping
        // the pair might also work, but would require more testing.
        if (prop.isObject() && prop.getObject(runtime).isFunction(runtime)) {
          prop = Value::null();
        }
        ret.insert(
            name.utf8(runtime), dynamicFromValue(runtime, std::move(prop)));
      }
      callCount--;
      return ret;
    }
  }
}

}
}
