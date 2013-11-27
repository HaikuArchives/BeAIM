#ifndef _DATA_CONTAINER_
#define _DATA_CONTAINER_

// Much of this code was cannibalized from David Nugent's great
// str class... which sadly, I couldn't get to work on BeOS (and
// was probably overkill anyway).

#define  _Div32(n1, n2) (n1 / n2)
#define  _Mul32(n1, n2) (n1 * n2)

typedef struct _Uldiv_t {
	unsigned long quot;
    unsigned long rem;
} Uldiv_t;


struct refdat
{
    short _length;
    short _size;
    short _refs;
    char* dataptr;

    refdat (short length, short size, unsigned short flgs =0)
        : _length(length), _size(size), _refs(1)
        { dataptr = NULL; }
    ~refdat (void) { delete dataptr; }
	//void * operator new(unsigned sz, short allocsz);
    char * ptr (void)
    {
		//return ((char *)this) + sizeof(refdat);
		return dataptr;
    }
};


class DataContainer {

	public:
	
		// constructors
		DataContainer( void );
		DataContainer( char const * s, short len =-1 );
		DataContainer( unsigned char const * s, short len =-1 );
		DataContainer( DataContainer const & s );
		DataContainer( signed char const * s, short len =-1);
		DataContainer( int val, int radix =10);
		DataContainer( unsigned int val, int radix =10);
		DataContainer( short val, int radix =10);
		DataContainer( unsigned short val, int radix =10);
		DataContainer( long val, int radix =10);
		DataContainer( unsigned long val, int radix =10);
		DataContainer( char c);
		DataContainer( unsigned char c);
		DataContainer( signed char c);
		
		// destructor
		~DataContainer();
	
		char * c_ptr (void) const			// not necessarily NUL terminated!
		{									// Use with caution...
			return data->ptr();
		}

        // primitive members
		short length (void) const
		{
			return data->_length;
		}
		
		short size (void) const
		{
			return data->_size;
		}
		
		// assignment
		DataContainer & operator=( DataContainer const & s );
		
		// gets a range of the data
		DataContainer getrange(short start, short len =-1) const;
		
		// indices
		char const & operator[] (short pos) const;
		char & operator[] (short pos);
		
		// concatenation
		DataContainer & operator<< (char const * s);
		DataContainer & operator<< (unsigned char const * s);
		DataContainer & operator<< (signed char const * s);
		DataContainer & operator<< (DataContainer const & s);
		DataContainer & operator<< (int val);
		DataContainer & operator<< (unsigned int val);
		DataContainer & operator<< (short val);
		DataContainer & operator<< (unsigned short val);
		DataContainer & operator<< (long val);
		DataContainer & operator<< (unsigned long val);
		DataContainer & operator<< (char c);
		DataContainer & operator<< (unsigned char c);
		DataContainer & operator<< (signed char c);
		
		// comparison
		bool operator== (DataContainer const & s) const
		{
			return (_compare(s) == 0) ? true : false;
		}
		bool operator!= (DataContainer const & s) const
		{
			return (_compare(s) != 0) ? true : false;
		}
		bool operator< (DataContainer const & s) const
		{
			return (_compare(s) < 0) ? true : false;
		}
		bool operator<= (DataContainer const & s) const
		{
			return (_compare(s) <= 0) ? true : false;
		}
		bool operator> (DataContainer const & s) const
		{
			return (_compare(s) > 0) ? true : false;
		}
		bool operator>= (DataContainer const & s) const
		{
			return (_compare(s) >= 0) ? true : false;
		}

	protected:

		// Comparison
		int _compare (DataContainer const s) const;

		// Check to see if big enough for size
	    int _chksize (short sz =0);
	    void _datinit (char const * s =0, short slen =0, short siz =-1);
	    void _datinit (unsigned long val, bool positive, int radix);
		int _concat (char const * s, short len =-1);
		int _concat (DataContainer const & s)
		{
			return _concat (s.c_ptr(), s.length());
		}
		int _concat (char ch)
		{
			return _concat (&ch, 1);
		}
		int _concat (unsigned char const * s, short len =-1)
		{
			return _concat ((char const *)s, len);
		}
		int _concat (signed char const * s, short len =-1)
		{
			return _concat ((char const *)s, len);
		}
		
		// supposedly 'standard' C functions... included just in case
		char *Ultoa(unsigned long value, char *string, int radix);
		char *Strrev(char *string);
		Uldiv_t Uldiv(unsigned long numer, unsigned long denom);

		refdat* data;
};

#endif
