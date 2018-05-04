#ifndef artdaq_demo_Overlays_CRTFragment_hh
#define artdaq_demo_Overlays_CRTFragment_hh

#include "artdaq-core/Data/Fragment.hh"

#include <ostream>

// XXX probably don't want this in "demo" anymore
namespace demo
{
	class CRTFragment;
}

class demo::CRTFragment
{
public:

  struct header_t{
    uint8_t magic; // must be "M"
    uint8_t nhit;
    uint16_t module_num;
    int32_t unixtime;
    uint32_t fifty_mhz_time;
  };

  struct hit_t{
    uint8_t magic; // must be "H"
    uint8_t channel;
    uint16_t adc;
  };

  uint16_t module_num() const
  {
    return header()->module_num;
  }

  size_t num_hits() const
  {
    return header()->nhit;
  }

  int32_t unixtime()
  {
    return header()->unixtime;
  }

  uint32_t fifty_mhz_time()
  {
    return header()->fifty_mhz_time;
  }

  uint8_t channel(const int i) const
  {
    return hit(i)->adc;
  }

  uint16_t adc(const int i) const
  {
    return hit(i)->adc;
  }

  // Return a pointer to hit 'i'.  Not range checked.
  const hit_t * hit(const int i) const
  {
    return reinterpret_cast<const hit_t *>
      (thefrag.dataBeginBytes() + sizeof(header_t) + i*sizeof(hit_t));
  }

  // Return a pointer to the header
  const header_t * header() const
  {
    return reinterpret_cast<const header_t *>(thefrag.dataBeginBytes());
  }

  explicit CRTFragment(artdaq::Fragment const& f) : thefrag(f) {}

private:
  artdaq::Fragment const& thefrag;
};

#endif /* artdaq_demo_Overlays_CRTFragment_hh */
