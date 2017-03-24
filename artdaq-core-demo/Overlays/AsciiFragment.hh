#ifndef artdaq_demo_Overlays_AsciiFragment_hh
#define artdaq_demo_Overlays_AsciiFragment_hh

#include "artdaq-core/Data/Fragment.hh"
#include "cetlib/exception.h"

#include <ostream>
#include <vector>

// Implementation of "AsciiFragment", an artdaq::Fragment overlay class
// used for pedagogical purposes

namespace demo
{
	class AsciiFragment;

	// Let the "<<" operator dump the AsciiFragment's data to stdout
	std::ostream& operator <<(std::ostream&, AsciiFragment const&);
}

class demo::AsciiFragment
{
public:

	// The "Metadata" struct is used to store info primarily related to
	// the upstream hardware environment from where the fragment came

	// "data_t" is a typedef of the fundamental unit of data the
	// metadata structure thinks of itself as consisting of; it can give
	// its size via the static "size_words" variable (
	// AsciiFragment::Metadata::size_words )

	struct Metadata
	{
		typedef char data_t;
		typedef uint32_t chars_in_line_t;

		chars_in_line_t charsInLine; // 4 Characters

		static size_t const size_words = 4ul; // Units of Metadata::data_t
	};

	static_assert (sizeof (Metadata) == Metadata::size_words * sizeof (Metadata::data_t), "AsciiFragment::Metadata size changed");


	// The "Header" struct contains "metadata" specific to the fragment
	// which is not hardware-related

	// Header::data_t -- not to be confused with Metadata::data_t ! --
	// describes the standard size of a data type not just for the
	// header data, but ALSO the physics data beyond it; the size of the
	// header in units of Header::data_t is given by "size_words", and
	// the size of the fragment beyond the header in units of
	// Header::data_t is given by "event_size"

	// Notice only the first 28 bits of the first 32-bit unsigned
	// integer in the Header is used to hold the event_size ; this means
	// that you can't represent a fragment larger than 2**28 units of
	// data_t, or 1,073,741,824 bytes

	struct Header
	{
		typedef char data_t;

		typedef uint64_t event_size_t;
		typedef uint64_t line_number_t; // Line number, 8 characters

		event_size_t event_size : 28;
		event_size_t unused_1 : 36;

		line_number_t line_number;

		static size_t const size_words = 16ul; // Units of Header::data_t
	};

	static_assert (sizeof (Header) == Header::size_words * sizeof (Header::data_t), "AsciiFragment::Header size changed");

	// The constructor simply sets its const private member "artdaq_Fragment_"
	// to refer to the artdaq::Fragment object

	AsciiFragment(artdaq::Fragment const& f) : artdaq_Fragment_(f) {}

	// const getter functions for the data in the header

	Header::event_size_t hdr_event_size() const { return header_()->event_size; }
	Header::line_number_t hdr_line_number() const { return header_()->line_number; }
	static constexpr size_t hdr_size_words() { return Header::size_words; }

	// The number of characters in the line
	size_t total_line_characters() const
	{
		return (hdr_event_size() - hdr_size_words()) * chars_per_word_();
	}

	// Start of the line, returned as a pointer to the char type
	char const* dataBegin() const
	{
		return reinterpret_cast<char const *>(header_() + 1);
	}

	// End of the line, returned as a pointer to the char type
	char const* dataEnd() const
	{
		return dataBegin() + total_line_characters();
	}

protected:

	// Functions to translate between size (in bytes) of an ADC, size of
	// this fragment overlay's concept of a unit of data (i.e.,
	// Header::data_t) and size of an artdaq::Fragment's concept of a
	// unit of data (the artdaq::Fragment::value_type).

	static constexpr size_t chars_per_word_()
	{
		return sizeof(Header::data_t) / sizeof(char);
	}

	// header_() simply takes the address of the start of this overlay's
	// data (i.e., where the AsciiFragment::Header object begins) and
	// casts it as a pointer to AsciiFragment::Header

	Header const* header_() const
	{
		return reinterpret_cast<AsciiFragment::Header const *>(artdaq_Fragment_.dataBeginBytes());
	}

private:

	artdaq::Fragment const& artdaq_Fragment_;
};

#endif /* artdaq_demo_Overlays_AsciiFragment_hh */
