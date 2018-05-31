/* Author: Matthew Strait <mstrait@fnal.gov> */

#ifndef artdaq_demo_Overlays_CRTFragment_hh
#define artdaq_demo_Overlays_CRTFragment_hh

#include "artdaq-core/Data/Fragment.hh"

#include <ostream>

namespace CRT
{
	class Fragment;
}

class CRT::Fragment
{
public:

  struct header_t{
    uint8_t magic; // must be 'M'
    uint8_t nhit;
    uint16_t module_num;
    int32_t unixtime;
    uint32_t fifty_mhz_time;
  };

  struct hit_t{
    uint8_t magic; // must be 'H'
    uint8_t channel;
    int16_t adc;
  };

  // Return the module number for this fragment.  A CRT fragment consists
  // of a set of hits sharing a time stamp from one module.
  uint16_t module_num() const
  {
    return header()->module_num;
  }

  // Return the number of hits claimed in the header of this fragment
  size_t num_hits() const
  {
    return header()->nhit;
  }

  // Return the Unix timestamp (seconds since 1 Jan 1970)
  int32_t unixtime()
  {
    return header()->unixtime;
  }

  // Return the value of the 50MHz counter
  uint32_t fifty_mhz_time()
  {
    return header()->fifty_mhz_time;
  }

  // Return the channel number of the ith hit.  That hit must exist.
  uint8_t channel(const int i) const
  {
    return hit(i)->adc;
  }

  // Return the ADC value of the ith hit.  That hit must exist.
  int16_t adc(const int i) const
  {
    return hit(i)->adc;
  }

  // Print the header to stdout, even if it is bad, but not if it
  // isn't all there.
  void print_header() const
  {
    if(size() < sizeof(header_t)){
      fprintf(stderr, "CRT fragment smaller (%uB) than header (%luB), "
              "can't print\n", size(), sizeof(header_t));
      return;
    }

    printf("CRT header: Magic = '%c'\n"
           "            n hit = %2u\n"
           "            module = %5u\n"
           "            Unix time  = %10d (0x%8x)\n"
           "            50Mhz time = %10u (0x%8x)\n",
           header()->magic, header()->nhit, header()->module_num,
           header()->unixtime, header()->unixtime,
           header()->fifty_mhz_time, header()->fifty_mhz_time);
  }

  // Print the given hit to stdout, even if it is bad, but not if it
  // isn't all there.
  void print_hit(const int i) const
  {
    // Is pointer arithmetic valid in the case that we're checking for?  Not
    // sure what the standard says, but I think that practically this should
    // work.
    if((uint8_t *)hit(i) + sizeof(hit_t) > thefrag.dataEndBytes()){
      fprintf(stderr, "Hit %d would be past end of fragment, can't print\n", i);
      return;
    }

    printf("CRT hit %2d: Magic = '%c'\n"
           "            channel = %2u\n"
           "            ADC     = %4hd\n",
           i, hit(i)->magic, hit(i)->channel, hit(i)->adc);
  }

  // Print all the hits
  void print_hits() const
  {
    for(int i = 0; i < header()->nhit; i++)
      print_hit(i);
    puts("");
  }

  // Returns true if the header contains sensible values.  Otherwise,
  // prints a complaint and returns false.  Assumes header is complete
  // (check good_size() first).
  bool good_header() const
  {
    const header_t * const h = header();
    if(h->magic != 'M'){
      fprintf(stderr, "CRT header has wrong magic: %c\n", h->magic);
      return false;
    }
    if(h->nhit == 0){
      fprintf(stderr, "CRT event has no hits\n");
      return false;
    }
    if(h->nhit > 64){
      fprintf(stderr, "CRT event has more hits (%d) than channels (64)\n",
              h->nhit);
      return false;
    }
    if(h->unixtime < 1525147200){
      // I know we didn't take data before 1 May 2018, so if it says it did,
      // the data must be corrupt.
      fprintf(stderr, "CRT Unix time (%d) is too early\n", h->unixtime);
      return false;
    }
    return true;
  }

  // Returns true if the hit contains sensible values.  Otherwise,
  // prints a complaint and returns false.  Assumes hit exists and
  // is complete (check good_size() first).
  bool good_hit(const int i) const
  {
    const hit_t * const h = hit(i);
    if(h->magic != 'H'){
      fprintf(stderr, "CRT hit has wrong magic: %c\n", h->magic);
      return false;
    }
    if(h->channel >= 64){
      fprintf(stderr, "CRT hit has bad channel %u >= 64\n", h->channel);
      return false;
    }
    if(h->adc >= 4096){
      // It is a 12-bit ADC.  This number probably represents the raw
      // ADC value before pedestal subtraction, but in any case, the
      // pedestal is positive, so the value still can't exceed 4095.
      fprintf(stderr, "CRT hit has bad ADC value %hd >= 4096\n", h->adc);
      return false;
    }
    return true;
  }

  // Return the size of the CRT fragment in bytes.
  unsigned int size() const
  {
    return thefrag.dataEndBytes() - thefrag.dataBeginBytes();
  }

  // Returns true if the fragment is as big as the header says it is,
  // and false if it isn't, or it doesn't even have a full header.
  // i.e. if this is false, you're going to seg fault (or wish you had)
  // if you read the fragment.
  bool good_size() const
  {
    if(size() < sizeof(header_t)){
      fprintf(stderr, "CRT fragment isn't as big (%dB) as header (%luB)\n",
              size(), sizeof(header_t));
      return false;
    }

    const unsigned int expect_size =
      (sizeof(header_t) + header()->nhit * sizeof(hit_t)
       + sizeof(artdaq::RawDataType) - 1)
       /sizeof(artdaq::RawDataType)
       *sizeof(artdaq::RawDataType);

    if(size() != expect_size){
      fprintf(stderr, "CRT fragment: N hit (%d -> %dB) mismatches size %uB\n",
              header()->nhit, expect_size, size());
      for(char * c = (char *) thefrag.dataBeginBytes();
                 c < (char *)thefrag.dataEndBytes();
                 c++){
        fprintf(stderr, "%02hhx/%c ", (unsigned char)*c,
                isprint((unsigned char)*c)?(unsigned char)*c:'.');
        if((c - (char*)thefrag.dataBeginBytes())%0x08 == 0x07)
          fprintf(stderr, " ");
        if((c - (char*)thefrag.dataBeginBytes())%0x10 == 0x0f)
          fprintf(stderr, "\n");
      }
      fprintf(stderr, "\n");
      return false;
    }
    return true;
  }

  // Return true if the fragment contains a complete and sensible event.
  bool good_event() const
  {
    if(!good_size()) return false;

    if(!good_header()) return false;

    for(unsigned int i = 0; i < header()->nhit; i++)
      if(!good_hit(i))
        return false;

    return true;
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

  explicit Fragment(artdaq::Fragment const& f) : thefrag(f) {}

private:
  artdaq::Fragment const& thefrag;
};

#endif /* artdaq_demo_Overlays_CRTFragment_hh */
