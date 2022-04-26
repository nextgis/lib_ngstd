%Module(name=ngstd._core,
        keyword_arguments="Optional")

%Include core/request.sip

%ModuleHeaderCode
#include <core/core.h>
%End

const char* getVersion();
