#ifndef artdaq_demo_Overlays_ToyFragment_hh
#define artdaq_demo_Overlays_ToyFragment_hh

#include "artdaq-core/Data/Fragment.hh"

#include <ostream>

// Implementation of "ToyFragment", an artdaq::Fragment overlay class
// used for pedagogical purposes.
//
// The intention of this class is to provide an example of how an Overlay
// can be constructed for a hypothetical 16-bit ADC module.  In this 
// example, the ADC data can have either 12 or 14 bits of meaningful
// information per 16-bit word.  (This is just a characteristic of this
// example.  Separate classes could certainly be used to overlay the two
// types of ADC data.)

namespace demo {
  class ToyFragment;

  // Let the "<<" operator dump the ToyFragment's data to stdout
  std::ostream & operator << (std::ostream &, ToyFragment const &);
}

class demo::ToyFragment {
  public:

  // The "Metadata" struct declared here is used to define the set of
  // information that is stored in the "metadata" section of
  // artdaq::Fragments that contain ToyFragments in their payload.
  // This struct is not used by the ToyFragment Overlay class itself;
  // it is defined here simply as a convenience.  The types of
  // information that are typically stored in artdaq::Fragment
  // metadata blocks are things like hardware serial numbers or values
  // of an interesting configuration parameter. These things are
  // generally *not* part of the data read out from the electronics
  // hardware in each event, hence the need to store them outside the
  // block of data that the hardware provides.

  // "data_t" is a typedef of the fundamental unit of data the
  // metadata structure thinks of itself as consisting of; it can give
  // its size via the static "size_words" variable (
  // ToyFragment::Metadata::size_words )

  struct Metadata {

    typedef uint32_t data_t;

    data_t board_serial_number : 16;
    data_t num_adc_bits : 8;
    data_t unused : 8; 
    
    static size_t const size_words = 1ul; // Units of Metadata::data_t
  };

  static_assert (sizeof (Metadata) == Metadata::size_words * sizeof (Metadata::data_t), "ToyFragment::Metadata size changed");


  // ToyFragment is intended to represent/interpret data that has an
  // inherent size of 16 bits (unsigned). This is represented by the
  // adc_t type that is declared here.

  typedef uint16_t adc_t;

  // The "Header" struct is used to interpret the header information that is
  // created by the hardware and *is* part of the data blob that is read from
  // the hardware for each event.

  // Header::data_t -- not to be confused with Metadata::data_t ! --
  // describes the standard size of a data type in the header.
  // In this example, it is ALSO used to describe the size of the physics data
  // beyond the header. This is not a general requirement for Overlay classes;
  // it is simply the choice that was made for this example.
  // The size of the header in units of Header::data_t is given by "size_words",
  // and the size of the fragment beyond the header in units of
  // Header::data_t is given by "event_size". Again, this is simply an
  // artifact of this example. A real-life hardware module may pack its data
  // differently, and any "size" fields in that real-life data could include
  // the size of any header information provided by the hardware.

  struct Header {
    typedef uint32_t data_t;

    typedef uint32_t event_size_t;  
    typedef uint32_t trigger_number_t;

    event_size_t event_size : 28;
    event_size_t unused_1   :  4;

    trigger_number_t trigger_number : 32;

    static size_t const size_words = 2ul; // Units of Header::data_t
  };

  static_assert (sizeof (Header) == Header::size_words * sizeof (Header::data_t), "ToyFragment::Header size changed");

  // The constructor simply sets its const private member "artdaq_Fragment_"
  // to refer to the artdaq::Fragment object

  ToyFragment(artdaq::Fragment const & f ) : artdaq_Fragment_(f) {}

  // const getter functions for the data in the header

  Header::event_size_t hdr_event_size() const { return header_()->event_size; } 
  Header::trigger_number_t hdr_trigger_number() const { return header_()->trigger_number; }
  static constexpr size_t hdr_size_words() { return Header::size_words; }

  // The number of ADC values describing data beyond the header
  size_t total_adc_values() const {
    return (hdr_event_size() - hdr_size_words()) * adcs_per_word_();
  }

  // The specified ADC value
  adc_t adc_value(uint32_t index) {
    // Simple way to handle index out of bounds - better ways are surely possible
    if (index >= total_adc_values()) {return 0xffff;}
    return dataBeginADCs()[index];
  }

  // Start of the ADC values, returned as a pointer to the ADC type
  adc_t const * dataBeginADCs() const {
    return reinterpret_cast<adc_t const *>(header_() + 1);
  }

  // End of the ADC values, returned as a pointer to the ADC type
  adc_t const * dataEndADCs() const {
    return dataBeginADCs() + total_adc_values();
  }

  // Functions to check if any ADC values are corrupt

  // findBadADC() checks to make sure that the ADC type (adc_t) variable
  // holding the ADC value doesn't contain bits beyond the expected
  // range, i.e., can't be evaluated to a larger value than the max
  // permitted ADC value

  adc_t const * findBadADC(int daq_adc_bits) const {
    return std::find_if(dataBeginADCs(), dataEndADCs(), 
			[&](adc_t const adc) -> bool { 
			  return (adc >> daq_adc_bits); });
  }

  bool fastVerify(int daq_adc_bits) const {
    return (findBadADC(daq_adc_bits) == dataEndADCs());
  };

  // Defined in ToyFragment.cc, this throws if any ADC appears corrupt
  void checkADCData(int daq_adc_bits) const; 


  // Largest ADC value possible
  size_t adc_range(int daq_adc_bits) {
    return (1ul << daq_adc_bits );
  }

  protected:

  // Functions to translate between size (in bytes) of an ADC, size of
  // this fragment overlay's concept of a unit of data (i.e.,
  // Header::data_t) and size of an artdaq::Fragment's concept of a
  // unit of data (the artdaq::Fragment::value_type).

  static constexpr size_t adcs_per_word_() {
    return sizeof(Header::data_t) / sizeof(adc_t);
  }

  // header_() simply takes the address of the start of this overlay's
  // data (i.e., where the ToyFragment::Header object begins) and
  // casts it as a pointer to ToyFragment::Header

  Header const * header_() const {
    return reinterpret_cast<ToyFragment::Header const *>(artdaq_Fragment_.dataBeginBytes());
  }

private:

  artdaq::Fragment const & artdaq_Fragment_;
};

#endif /* artdaq_demo_Overlays_ToyFragment_hh */
