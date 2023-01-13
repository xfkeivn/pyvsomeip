//python  module name
%module pyvsomeip
// copy all the %{}% to the wrapper
%{
#include "pyvsomeip.h"
%}

%include "pyvsomeip.h"