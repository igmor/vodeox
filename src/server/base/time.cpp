#include "base/time.h"

namespace vodeox
{

std::ostream &
    operator<<(std::ostream & ostr, time const & t)
 {
     ostr << t.usec();
     return ostr;
 }

}//namespace
