#ifndef OCLUtil_h
#define OCLUtil_h

#include "OCLconfig.h"
#include "OCLdebug.h"
#include "opencl_compat.h"
#include <string>

namespace WebCore {

static cl_platform_id platform_;
static cl_context context_;
static cl_command_queue queue_;
static std::string version_;
static std::string name_;
static std::string vendor_;
static std::string profile_;
static std::string extensions_;
static cl_uint numberOfDevices_;
static cl_uint alignmentSize_;

class RivertrailExport OCLUtil {
 public:
  OCLUtil(); 
  ~OCLUtil();
  void Init();
  int getPlatformPropertyHelper(cl_platform_info param, char*& out);
  int numberOfDevices();
  cl_uint alignmentSize();
  std::string version();
  std::string name();
  std::string vendor();
  std::string profile();
  std::string extensions();
  cl_platform_id platform();
  cl_context context();
  cl_command_queue queue();
};

}  // namespace webcore

#endif  // OCLUtil_h
