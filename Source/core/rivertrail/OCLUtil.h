#ifndef OCLUtil_h
#define OCLUtil_h

//#include "base/memory/singleton.h"
#include "OCLconfig.h"
#include "OCLdebug.h"
#include "opencl_compat.h"
/*#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>*/
#include <wtf/text/WTFString.h>

namespace WebCore {

static cl_platform_id platform_;
static cl_context context_;
static cl_command_queue queue_;
static String version_;
static String name_;
static String vendor_;
static String profile_;
static String extensions_;
static cl_uint numberOfDevices_;
static cl_uint alignmentSize_;

class OCLUtil {
 public:
  OCLUtil(); 
  ~OCLUtil();
  void Init();
//  void post();
  int getPlatformPropertyHelper(cl_platform_info param, char*& out);
  int numberOfDevices();
  cl_uint alignmentSize();
  String version();
  String name();
  String vendor();
  String profile();
  String extensions();
  cl_platform_id platform();
  cl_context context();
  cl_command_queue queue();
};

}  // namespace webcore

#endif  // OCLUtil_h
