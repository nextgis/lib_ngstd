%Module(name=ngstd._core,
        keyword_arguments="Optional")

%Include request.sip

%ModuleHeaderCode
#include <core/core.h>
%End

const char* getVersion();
