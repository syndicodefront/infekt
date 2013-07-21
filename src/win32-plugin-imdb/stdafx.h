#include "imdb-plugin.h"

#include <Shlwapi.h>
#include <Shlobj.h>

#include <stdint.h>

#include <sstream>
#include <vector>
#include <map>
#include <limits>
#include <stack>
#include <memory>
using std::tr1::shared_ptr;

#include <pcre.h>
#pragma warning (push)
#pragma warning (disable: 4251)
// disable warning about DLL import of std:: class
#include <pcrecpp.h>
#pragma warning (pop)
