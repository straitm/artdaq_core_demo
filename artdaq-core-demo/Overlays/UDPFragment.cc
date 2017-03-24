#include "artdaq-core-demo/Overlays/UDPFragment.hh"

#include "cetlib/exception.h"

std::ostream& demo::operator <<(std::ostream& os, UDPFragment const& f)
{
	os << "UDPFragment_event_size: "
		<< f.hdr_event_size()
		<< ", data_type: "
		<< f.hdr_data_type()
		<< "\n";

	return os;
}
