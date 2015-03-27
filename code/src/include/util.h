#ifndef _UTIL_H_
#define _UTIL_H_
#include <iostream>
#include <string>

class fast_random {
public:
	fast_random(){
			set_seed0(rand()%65536);
	}
	fast_random(unsigned long seed)
    : seed(0)
  {
    set_seed0(seed);
  }

  inline unsigned long
  next()
  {
    return ((unsigned long) next(32) << 32) + next(32);
  }

  inline uint32_t
  next_u32()
  {
    return next(32);
  }

  inline uint16_t
  next_u16()
  {
    return next(16);
  }

  /** [0.0, 1.0) */
  inline double
  next_uniform()
  {
    return (((unsigned long) next(26) << 27) + next(27)) / (double) (1L << 53);
  }

  inline char
  next_char()
  {
    return next(8) % 256;
  }

  inline char
  next_readable_char()
  {
    static const char readables[] = "0123456789@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
    return readables[next(6)];
  }

  inline std::string
  next_string(size_t len)
  {
    std::string s(len, 0);
    for (size_t i = 0; i < len; i++)
      s[i] = next_char();
    return s;
  }

  inline std::string
  next_readable_string(size_t len)
  {
    std::string s(len, 0);
    for (size_t i = 0; i < len; i++)
      s[i] = next_readable_char();
    return s;
  }

  inline unsigned long
  get_seed()
  {
    return seed;
  }

  inline void
  set_seed(unsigned long seed)
  {
    this->seed = seed;
  }

private:
  inline void
  set_seed0(unsigned long seed)
  {
    this->seed = (seed ^ 0x5DEECE66DL) & ((1L << 48) - 1);
  }

  inline unsigned long
  next(unsigned int bits)
  {
    seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    return (unsigned long) (seed >> (48 - bits));
  }

  unsigned long seed;
};

long unsigned int getApId(long unsigned int cust_id){
		long unsigned int ap_id_1 = ((cust_id & 0xFE000)>>6);
		long unsigned int ap_id_2 = ((cust_id & 0x3FF)>>3);
		return (ap_id_1+ap_id_2)%(numAirports+1);
}
long unsigned int getCusId(long unsigned int ap_id){
		long unsigned int cust_id_1 = ap_id & 0x7F;
		long unsigned int cust_id_2 = ap_id & 0x3F80;
		long unsigned int rnd = rand()%8;
		long unsigned int x = rand()%4;
		long unsigned int cust_id = (cust_id_2<<6) + (x<<10) + (cust_id_1<<3) + (rnd);
		return cust_id;
}

#endif //_UTIL_H_
