/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2010,2011 Thomas Davis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef __JSONHANDLE_H__
#define __JSONHANDLE_H__

#include <istream>
#include <ostream>
#include <string>
#include <stdint.h>

class _JS0; // private class to hold actual data

/**
 * class: JsonHandle
 *
 * This class manages all or part of a JSON structure. The
 * design allow for easy access to the JSON structure and
 * changing between data types.
 *
 * For example:
 *    JsonHandle H;
 *    H["here"][0] = "fred";
 *    cout << H;   // prints {"here":["fred"]}
 *
 * It is very copy constructor friendly ensuring that the data is
 * not copied when not desired.
 */
class JsonHandle {
public:

	/**
	 * Constructor  - creates an empty node
	 */
	JsonHandle();

	// destructor
	~JsonHandle();

	/**
	 * Copy Constructor - makes a shallow copy such that actual node
	 *    data is not copies (but reference counted).
	 */
	JsonHandle(const JsonHandle&);

	/**
	 * Assignment operator - makes a deep copy such that actual node
	 *    data is completely cloned.
	 */
	JsonHandle &operator=(const JsonHandle&);

	/**
	 * isValid - check to see if handle is usable.  In some cases, it is
	 *     possible to get an invalid node via.  Usually it will be
	 *     JsonHandle::JSON_ERROR or a reference off it.
	 */
	bool isValid() const {
		return state == 1 || parent || state == 0;
	}

	/**
	 * isDefined - Tests if the handle contains actual data.  Note that Null
	 *    is a defined type.
	 */
	bool isDefined() const {
		return state == 1;
	}

	/**
	 * isNull - tests if the current data is of type Null.
	 */
	bool isNull() const;

	/**
	 * isArray - tests if the current data is of type Array.
	 */
	bool isArray() const;

	/**
	 * isObject - tests if the current data is of type Object.
	 */
	bool isObject() const;

	/**
	 * isString - tests if the current data is of type String.
	 */
	bool isString() const;

	/**
	 * isNumber - tests if the current data is of type Number.
	 */
	bool isNumber() const;

	/**
	 * isBoolean - tests if the current data is of type Boolean.
	 */
	bool isBoolean() const;

	/**
	 * operator[](int)
	 *   if handle is Array, index into an array.  It is possible to index
	 *        1 past the size of the array for assignment.
	 *   if handle is Object, index into an object.
	 *   otherwise, handle is converted into an Array.
	 */
	JsonHandle operator[](int index); // [-1] same as [size()] for easy access to the place to append

	/**
	 * operator[](char *)
	 *   if handle is Object, property into an array.  It is possible to use
	 *        a nonexistent key for assignment.
	 *   otherwise, handle is converted into an Object.
	 */
	JsonHandle operator[](const char *key);

	/**
	 * operator[](std::string &)
	 *   if handle is Object, property into an array.  It is possible to use
	 *        a nonexistent key for assignment.
	 *   otherwise, handle is converted into an Object.
	 */
	JsonHandle operator[](std::string const &key);

	/**
	 * key - returns the object's key at the specified index.  Only
	 * valid for object types.
	 */
	const std::string &key(int index) const;

  /**
   * exists - checks whether a specific key is present.
   */
  bool exists(std::string const &key) const;

	/**
	 * size()
	 *    if handle is an Array - size of array.
	 *    if handle is an Object - number of object properties.
	 *    otherwise 0
	 */
	int size();

	/**
	 * erase -  removes item at specified index.  Valid for objects and
	 *    arrays.
	 */
	JsonHandle &erase(int index);

	/**
	 * erase -  removes item at specified key.  Valid for objects.
	 */
	JsonHandle &erase(const char *key);

	/**
	 * erase -  removes item at specified key.  Valid for objects.
	 */
	JsonHandle &erase(const std::string &key);

	/**
	 * clear -  removes all items.  Valid for objects and arrays.
	 *    passive otherwise.
	 */
	JsonHandle &clear();

	/**
	 * at(int) - Same as operator[int]
	 */
	JsonHandle at(int index);

	/**
	 * at(char *) - Same as operator[char *]
	 */
	JsonHandle at(const char *key);

	/**
	 * at(string &) - Same as operator[string &]
	 */
	JsonHandle at(std::string const &key);

	// convenient cast operators
	 operator int64_t () const {
		return longValue();
	}
	operator uint64_t() const {
		return (uint64_t) longValue();
	}
	operator int() const {
		return (int) longValue();
	}
	operator bool() const {
		return booleanValue();
	}
	operator unsigned() const {
		return (unsigned) longValue();
	}
	operator long double() const {
		return doubleValue();
	}
	operator double() const {
		return (double) doubleValue();
	}
	operator float() const {
		return (float) doubleValue();
	}
	operator const char *() const {
		return stringValuePtr();
	}
	operator const std::string &() const {
		return stringValue();
	}

	/**
	 * operatpr=(bool) same as setBoolean()
	 */
	JsonHandle &operator=(bool value) {
		return setBoolean(value);
	}

	/**
	 * operatpr=(int64_t) same as setLong()
	 */
	JsonHandle &operator=(int64_t value) {
		return setLong(value);
	}

	/**
	 * operatpr=(int) same as setLong()
	 */
	JsonHandle &operator=(int value) {
		return setLong(value);
	}

	/**
	 * operatpr=(unsigned) same as setLong()
	 */
	JsonHandle &operator=(unsigned value) {
		return setLong(value);
	}

	/**
	 * operatpr=(long double) same as setDouble()
	 */
	JsonHandle &operator=(long double value) {
		return setDouble(value);
	}

	/**
	 * operatpr=(double) same as setDouble()
	 */
	JsonHandle &operator=(double value) {
		return setDouble(value);
	}

	/**
	 * operatpr=(string &) same as setString(string &)
	 */
	JsonHandle &operator=(std::string const &value) {
		return setString(value);
	}

	/**
	 * operatpr=(char *) same as setString(char *)
	 */
	JsonHandle &operator=(const char *value) {
		return setString(value);
	}

	/**
	 * setBoolean: set the current handle to boolean
	 *   value.
	 */
	JsonHandle &setBoolean(bool value);

	/**
	 * setLong: set the current handle to int64_t
	 *   value.
	 */
	JsonHandle &setLong(int64_t value);

	/**
	 * setDouble: set the current handle to long double
	 *   value.
	 */
	JsonHandle &setDouble(long double value);

	/**
	 * setString: set the current handle to string
	 *   value.
	 */
	JsonHandle &setString(std::string const &value);

	/**
	 * setString: set the current handle to string
	 *   value.
	 */
	JsonHandle &setString(const char *value);

	/**
	 * booleanValue: gets the boolean value.
	 *   If the current type is string,
	 *   the type will change to boolean,
	 *   looking for true, false, yes, no, on, off.
	 * @returns
	 *   value or defaultValue if not set or conversion failed.
	 */
	bool booleanValue(bool defaultValue = false) const;

	/**
	 * longValue: gets the long value.
	 *   If the current type is string,
	 *   the type will change to number,
	 * @returns
	 *   value or defaultValue if not set or conversion failed.
	 */
	int64_t longValue(int64_t defaultValue = 0) const;

	/**
	 * doubleValue: gets the double value.
	 *   If the current type is string,
	 *   the type will change to number,
	 * @returns
	 *   value or defaultValue if not set or conversion failed.
	 */
	long double doubleValue(long double defaultValue = 0) const;

	/**
	 * stringValue: gets the string value.
	 *   If the current type is boolean
	 *   or number type will change to string,
	 * @returns
	 *   value or defaultValue if not set or conversion failed.
	 */
	const std::string &stringValue(std::string const &defaultValue =
			EMPTY_STRING) const;

	/**
	 * stringValuePtr: gets the string value.
	 *   If the current type is boolean
	 *   or number type will change to string,
	 * @returns
	 *   value or defaultValue if not set or conversion failed.
	 */
	const char *stringValuePtr(const char *defaultValue = NULL) const;

	/**
	 * copy: deep copies the contents one node to another.  Existing
	 *   contents will be released.
	 */
	JsonHandle &copy(const JsonHandle &from);

	/**
	 * fromString: appends the current contents with data decoded
	 *   from the provided string.
	 */
	JsonHandle &fromString(const std::string &from);

	/**
	 * fromString: appends the current contents with data decoded
	 *   from the provided string.
	 */
	JsonHandle &fromString(const char *from);

	/**
	 * fromString: appends the current contents with data decoded
	 *   from the provided file.
	 */
	JsonHandle &fromFile(const char *file);

	/**
	 * toString: JSON encodes the contents into the provided string buffer
	 *   WITH whitespace.  See also  toCompactString().
	 */
	std::string &toString(std::string &buffer,
			int precision = defaultPrecision) const;

	/**
	 * toFile: JSON encodes the contents into the provided file
	 *   WITH whitespace.  See also  toCompactFile().
	 */
	bool toFile(const char *file, int precision = defaultPrecision) const;

	/**
	 * toCompactString: JSON encodes the contents into the provided string buffer
	 *   WITHOUT whitespace.  See also  toString().
	 */
	std::string &toCompactString(std::string &buffer, int precision =
			defaultPrecision) const;

	/**
	 * toCompactFile: JSON encodes the contents into the provided string buffer
	 *   WITHOUT whitespace.  See also  toFile().
	 */
	bool toCompactFile(const char *file, int precision = defaultPrecision) const;

	/**
	 * fromStream - JSON decodes the handle data from the stream.  If conversion
	 *   fails, the failbit will be set on the stream.  No Extra characters will
	 *   be pulled from the stream allowing other data to exist.
	 */
	std::istream &fromStream(std::istream &stream);
	friend std::istream &operator>>(std::istream &stream, JsonHandle &ob);

	/**
	 * toStream - JSON encodes the handle data to the stream.  Conversion will
	 *   always be compact without whitespace.
	 */
	std::ostream &toStream(std::ostream &stream);
	friend std::ostream &operator<<(std::ostream &stream, JsonHandle ob);

	/**
	 * In the cases where precision is not provided, this is the default precision
	 * for double conversions.
	 */
	static int defaultPrecision;

	/**
	 * EMPTY_STRING - handy item used for dealing with empty strings without allocation.
	 */
	static const std::string EMPTY_STRING;

	/**
	 * JSON_ERROR - item returned with errors occur.
	 */
	static JsonHandle JSON_ERROR;

	/**
	 * operator ==  shallow equality.  Does not check actual  data.  Only compares
	 *   pointers to see if they are the same.  Very useful for error checking
	 *   (foo() == JsonHandle:JSON_ERROR)
	 */
	bool operator ==(const JsonHandle &other) const {
		return (this == &other) ? true :
				vapor.node == other.vapor.node;
	}


	/**
	 * Used for developing JsonHandle itself to show the references counting,
	 * allocations and deallocations.  Only works when HAS_JSON_STATS is defined
	 * during compilation.
	 */
	static void dumpStats();

protected:
	JsonHandle(_JS0 *node);
	JsonHandle(_JS0 *node, _JS0 *parent);
	JsonHandle(_JS0 *parent, const std::string &key);
	JsonHandle(_JS0 *parent, const char *key);
	JsonHandle(_JS0 *parent, int index);
	void makeVaporNode(const char *foo);

	_JS0 *parent;
	int state;
	struct {
		union {
			_JS0 *node;
			const std::string *stlkey;
			const char *key;
			int index;
		};
	} vapor;
};

inline std::ostream &operator<<(std::ostream &stream, JsonHandle ob) {
	return ob.toStream(stream);
}

inline std::istream &operator>>(std::istream &stream, JsonHandle &ob) {
	return ob.fromStream(stream);
}

#endif /* __JSONHANDLE_H__ */
