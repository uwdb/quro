#ifndef _UTIL_H_
#define _UTIL_H_
#include <iostream>
#include <string>
#include <DateTime.h>
#include <CommonStructs.h>
using namespace std;

#define TimeStamp TIMESTAMP_STRUCT
#define numAirports 9264
#define numCountries 249
#define numAirlines 1251
#define numFlights   100000
#define numCustomers 1000000
#define MIN_DIS 100
#define MAX_DIS 4000

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

inline long unsigned int getApId(long unsigned int cust_id){
		long unsigned int ap_id_1 = ((cust_id & 0xFE000)>>6);
		long unsigned int ap_id_2 = ((cust_id & 0x3FF)>>3);
		return (ap_id_1+ap_id_2)%(numAirports+1);
}
inline long unsigned int getCusId(long unsigned int ap_id, fast_random& r){
		long unsigned int cust_id_1 = ap_id & 0x7F;
		long unsigned int cust_id_2 = ap_id & 0x3F80;
		long unsigned int rnd = r.next()%8;
		long unsigned int x = r.next()%4;
		long unsigned int cust_id = (cust_id_2<<6) + (x<<10) + (cust_id_1<<3) + (rnd);
		return cust_id;
}
inline int get_random(fast_random& r, int a){
	return r.next()%a;
}
//1023, 1, 3000
inline int get_nurand(int a, int x, int y, fast_random& r)
{
	return ((get_random(r, a + 1) | (x + get_random(r, y + 1))) % (y - x + 1)) + x;
}

inline long unsigned int getRandomAirportId(fast_random& r){
		return get_random(r, numAirports+1);
}

inline long unsigned int getCustomerId(fast_random& r){
		return get_nurand(1023, 1, 3000, r);
}

inline double getRandomDist(fast_random& r){
		return r.next_uniform()*(MAX_DIS-MIN_DIS) + MIN_DIS;
}

extern int calendar[12];
inline TIMESTAMP_STRUCT GenerateRandomTimestamp(fast_random& r){
		TIMESTAMP_STRUCT ts;
		ts.year = 2014;
		ts.month = get_random(r, 12)+1;
		ts.day = get_random(r, calendar[ts.month-1])+1;
		ts.hour = get_random(r, 24);
		ts.minute = get_random(r, 60);
		ts.second = 0;
		return ts;
}
TIMESTAMP_STRUCT addHour(const TimeStamp& t, size_t hour, size_t min);

string toStr(const TimeStamp& ts);

TIMESTAMP_STRUCT addDay(const TimeStamp& t, size_t day);

inline int min(int a, int b){
	return a>b?b:a;
}
#endif //_UTIL_H_
