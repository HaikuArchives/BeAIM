#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "DataContainer.h"

#define STDLEN 128

/*void* refdat::operator new (unsigned sz, short allocsz)
{
    return malloc (sz + allocsz);
}
*/

DataContainer::DataContainer(void)
{
    _datinit();
}

DataContainer::DataContainer(char const * s, short len)
{
    _datinit(s, len, -1);
}

DataContainer::DataContainer(unsigned char const * s, short len)
{
    _datinit((char const *)s, len, -1);
}

DataContainer::DataContainer( DataContainer const & s )
			 : data(s.data)
{
    ++data->_refs;
}

DataContainer::DataContainer(signed char const * s, short len)
{
    _datinit((char const *)s, len, -1);
}

DataContainer::DataContainer(char c)
{
    _datinit (&c, 1, -1);
}

DataContainer::DataContainer(int val, int radix)
{
    bool    positive;

    if (val >= 0)
        positive = true;
    else
    {
        positive = false;
        val = -val;
    }
    _datinit((unsigned long)val, positive, radix);
}

DataContainer::DataContainer(unsigned int val, int radix)
{
    _datinit((unsigned long)val, true, radix);
}

DataContainer::DataContainer(short val, int radix)
{
    bool    positive;

    if (val >= 0)
        positive = true;
    else
    {
        positive = false;
        val = short(-val);
    }
    _datinit((unsigned long)val, positive, radix);
}

DataContainer::DataContainer(unsigned short val, int radix)
{
    _datinit((unsigned long)val, true, radix);
}

DataContainer::DataContainer(long val, int radix)
{
    bool positive;

    if (val >= 0L)
        positive = true;
    else
    {
        positive = false;
        val = -val;
    }
    _datinit((unsigned long)val, positive, radix);
}

DataContainer::DataContainer(unsigned long val, int radix)
{
    _datinit(val, true, radix);
}

DataContainer::DataContainer(unsigned char c)
{
    _datinit ((char const *)&c, 1, -1);
}

DataContainer::DataContainer(signed char c)
{
    _datinit ((char const *)&c, 1, -1);
}

DataContainer::~DataContainer(void)
{
    if (!--data->_refs)
        delete data;
}

int DataContainer::_chksize (short sz)
{
	refdat* old = 0;
	if (data->_refs > 1)		// Need to dup memory
		--data->_refs;			// Dec existing string reference
	else if (sz >= size())
		old = data;
	else
		return 0;
	_datinit(c_ptr(), length(), sz);
	delete old;
	return 1;
}

void DataContainer::_datinit( char const * s, short len, short siz )
{
	if (len < 0)
		len = (short) ((s) ? strlen (s) : 0);
	if (siz < 0)
		siz = STDLEN;
	if (siz < short(len + 1))
		siz = short(len + 1);
	data = new refdat(len, siz);
	data->dataptr = new char[len+siz];
	
	if (s && len)
		memcpy( c_ptr(), s, len );
}


void DataContainer::_datinit (unsigned long val, bool positive, int radix)
{
    char    buf[32], * p = buf;

    if (!positive)
        *p = '-';
    Ultoa(val, p, radix);
    _datinit(buf, -1, 0);
}

DataContainer& DataContainer::operator=( DataContainer const & s )
{
    if (&s != this)
    {
        if (!--data->_refs)
            delete data;
        data = s.data;
        ++data->_refs;
    }
    return *this;
}


DataContainer DataContainer::getrange(short start, short len) const
{
    if (start < 0)
        start = short(length() + start);
    if (start < 0 || start >= data->_length)
        return DataContainer();   // Empty
    if (len < 0 || (short(start + len) > data->_length))
        len = short(data->_length - start);
    return DataContainer(c_ptr() + start, len);
}


char const & DataContainer::operator[] (short pos) const
{
    if (pos < 0)            // Negative index addresses from eos
        pos = short(data->_length + pos);
    if (pos >= data->_length)
    {
        char * buf = c_ptr() + length();
        *buf = 0;
        return *buf;
    }
    return c_ptr()[pos];
}

    // ... but here it may be

char & DataContainer::operator[] (short pos)
{
    if (pos < 0)                       // Negative index addresses from eos
        pos = short(data->_length + pos);
    if (pos < 0)                     // Any cleaner way without exceptions?
        pos = data->_length;
    if (pos < data->_length)
        _chksize(0);
    else
    {
        _chksize(short(pos + 2));
        ::memset(c_ptr() + length(), ' ', pos - data->_length + 1);
        data->_length = short(pos+1);
    }
    return c_ptr()[pos];
}


DataContainer &
DataContainer::operator<< (char const * s)    // concatenate
{
    _concat (s);
    return *this;
}

DataContainer &
DataContainer::operator<< (unsigned char const * s)
{
    _concat ((char const *)s);
    return *this;
}

DataContainer &
DataContainer::operator<< (signed char const * s)
{
    _concat ((char const *)s);
    return *this;
}

DataContainer &
DataContainer::operator<< (DataContainer const & s)
{
    _concat (s);
    return *this;
}

DataContainer &
DataContainer::operator<< (int val)
{
    _concat (DataContainer(val));
    return *this;
}

DataContainer &
DataContainer::operator<< (unsigned int val)
{
    _concat (DataContainer(val));
    return *this;
}

DataContainer &
DataContainer::operator<< (short val)
{
    _concat (DataContainer(val));
    return *this;
}

DataContainer &
DataContainer::operator<< (unsigned short val)
{
    _concat (DataContainer(val));
    return *this;
}

DataContainer &
DataContainer::operator<< (long val)
{
    _concat (DataContainer(val));
    return *this;
}

DataContainer &
DataContainer::operator<< (unsigned long val)
{
    _concat (DataContainer(val));
    return *this;
}

DataContainer &
DataContainer::operator<< (char c)
{
    _concat (c);
    return *this;
}

DataContainer &
DataContainer::operator<< (unsigned char c)
{
    _concat (c);
    return *this;
}


DataContainer &
DataContainer::operator<< (signed char c)
{
    _concat (c);
    return *this;
}


int DataContainer::_concat (char const * s, short len)
{
    if (len < 0)
        len = (short) strlen (s);
    if (len > 0)
    {
        // Special case - are we concatenating ourselves??
        if (data->_refs == 1 &&  // No danger if we'll be reallocated anyway
            s >= c_ptr() &&         // Refers to us, or substring of us
            s <= (c_ptr() + length()))
        {   // This is handled separately, since we do not wish
            // to pass this heinous overhead onto all cases,
            // especially when this particular case is so rare...
			DataContainer tmpdat(s, len);      // Copy this string first
			_chksize(short(len + length() + 1));
			memcpy(c_ptr() + length(), tmpdat.c_ptr(), len);
        }
        else
        {
            _chksize (short(len + length() + 1));
            memcpy (c_ptr() + length(), s, len);
        }
        data->_length += len;
    }
    return length();
}


char* DataContainer::Ultoa( unsigned long value, char *string, int radix )
{
	Uldiv_t result;
	char *p = string;
	if(radix >= 2 && radix <= 36)
	{
		do {
			result = Uldiv(value, (unsigned long)radix);
			value = result.quot;
			if(result.rem < 10)
			{
				*p = (char)((char)(result.rem) + (char)'0');
			} else {
				*p = (char)((char)(result.rem - 10) + (char)'a');
			}
			p++;
		} while (value > 0);
	} else {
		*p = '0';
		p++;
	}
	*p = '\0';
	return(Strrev(string));
}


Uldiv_t DataContainer::Uldiv(unsigned long numer, unsigned long denom)
{
	Uldiv_t result;
	result.quot = _Div32(numer, denom);
	result.rem  = numer - _Mul32(result.quot, denom);
	return(result);
}


char* DataContainer::Strrev(char *string)
{
	char temp;
	size_t slen;
	char *p1, *p2;
	slen = strlen(string);
	if(slen > 1)
	{
		p1 = string;
		p2 = string + (slen - 1);
		while (p1 < p2)
		{
			temp = *p1;
			*p1 = *p2;
			*p2 = temp;
			p1++;
			p2--;
		}
	}
	return(string);
}

int DataContainer::_compare( DataContainer const d ) const {

	short min, which;
	bool equalSize = false;
	char *iter1, *iter2;
	
	// get the smaller of the two
	if( length() < d.length() ) {
		min = length();
		which = -1;
	} else {
		min = d.length();
		which = 1;
		if( d.length() == length() )
			equalSize = true;
	}
	
	// assign the pointers
	iter1 = c_ptr();
	iter2 = d.c_ptr();
	
	// go to the end of the smaller container, doing comparisons
	for( short i = 0; i < min; ++i ) {
	
		if( *iter1 < *iter2 )
			return -1;
		if( *iter1 > *iter2 )
			return 1;
	
		++iter1;
		++iter2;
	}

	// At this point, they are equal up to the length of the smaller one
	if( equalSize )
		return 0;
	return which;
}
