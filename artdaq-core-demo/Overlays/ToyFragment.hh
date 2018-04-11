#ifndef artdaq_demo_Overlays_ToyFragment_hh
#define artdaq_demo_Overlays_ToyFragment_hh

#include "artdaq-core/Data/Fragment.hh"

#include <ostream>

namespace demo
{
	class ToyFragment;
}

class demo::ToyFragment
{
  public:

    explicit ToyFragment(artdaq::Fragment const& f) : artdaq_Fragment_(f) {}

  private:
    artdaq::Fragment const& artdaq_Fragment_;
};

#endif /* artdaq_demo_Overlays_ToyFragment_hh */
