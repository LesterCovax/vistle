#include "hostname.h"

#include <boost/asio/ip/host_name.hpp>

namespace vistle {

std::string hostname() {

   static std::string hname;

   if (hname.empty()) {
#if 0
      const size_t HOSTNAMESIZE = 256;

      char hostname[HOSTNAMESIZE];
      gethostname(hostname, HOSTNAMESIZE-1);
      hostname[HOSTNAMESIZE-1] = '\0';
      hname = hostname;
#else
      hname = boost::asio::ip::host_name();
#endif
   }
   return hname;
}

}