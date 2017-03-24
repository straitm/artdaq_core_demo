#ifndef artdaq_demo_Overlays_FragmentType_hh
#define artdaq_demo_Overlays_FragmentType_hh
#include "artdaq-core/Data/Fragment.hh"

namespace demo
{
	std::vector<std::string> const names{"MISSED", "TOY1", "TOY2", "ASCII", "UDP", "UNKNOWN"};

	namespace detail
	{
		enum FragmentType : artdaq::Fragment::type_t
		{
			MISSED = artdaq::Fragment::FirstUserFragmentType,
			TOY1,
			TOY2,
			ASCII,
			UDP,
			INVALID // Should always be last.
		};

		// Safety check.
		static_assert(artdaq::Fragment::isUserFragmentType(FragmentType::INVALID - 1),
			"Too many user-defined fragments!");
	}

	using detail::FragmentType;

	FragmentType toFragmentType(std::string t_string);

	std::string fragmentTypeToString(FragmentType val);
}
#endif /* artdaq_demo_Overlays_FragmentType_hh */
