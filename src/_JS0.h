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

#ifndef ___JS0_H__
#define ___JS0_H__

#include <string>
#include <vector>
#include <istream>
#include <ostream>
#include <stdint.h>

/*
 * class: _JS0 (private, for use by class JsonHandle only)
 *
 * This class is a data container.  It encapsulates changeable type data.
 * In addition it handles streaming plus encoding and decoding.
 * Reference counting is used to ensure proper cleanup and prevent unneeded
 * allocations.
 */
class _JS0 {
	friend class JsonHandle;
private:

	// list of types
	static const unsigned short UNDEFINED = 0;
	static const unsigned short NULLVALUE = 1;
	static const unsigned short BOOLEAN = 2;
	static const unsigned short OBJECT = 3;
	static const unsigned short ARRAY = 4;
	static const unsigned short STRING = 5;
	static const unsigned short NUMBER_LONG = 6;
	static const unsigned short NUMBER_DOUBLE = 7;

	_JS0() :
			type(UNDEFINED), ref_count(0) {
#ifdef HAS_JSON_STATS
		_JS0::stats.allocs++;
#endif
	}

	~_JS0() {
#ifdef HAS_JSON_STATS
		_JS0::stats.frees++;
#endif
		reset();
	}

	void reset();

	int length() const;

	_JS0 *getByIndex(int index) const;

	_JS0 *getByKey(std::string const &key) const;

	_JS0 *getByKey(const char *key) const;

	bool booleanValue() const;

	int64_t longValue() const;

	long double doubleValue() const;

	const std::string &stringValue() const;

	void toJsonString(std::string &buffer, bool whiteSpace,
			int precision) const;

	void appendStringValue(std::string &buffer, int precision) const;

	void appendJsonString(std::string &buffer, bool whiteSpace, int precision,
			int depth) const;

	void appendStream(std::ostream &stream) const;

	static void quoteJsonString(const std::string &src, std::string &dest);

	const std::string &getKeyAtIndex(int index) const;

	void erase(int index);

	void erase(const char *key);

	void erase(const std::string &key);

	void setLong(int64_t value);

	void setDouble(long double value);

	void setBoolean(bool value);

	void setString(const std::string &value);

	void setString(const char *value);

	void setNull();

	void setObject();

	void setArray();

	void clear();

	void setByKey(const std::string &key, _JS0 *value);

	void setByKey(const char *key, _JS0 *value);

	void setByIndex(int index, _JS0 *value);

	void setType(int type);

	bool setFromJsonStlString(const std::string &from);

	bool setFromJsonString(const char *&from);

	bool setFromStream(std::istream &from);

	static bool parseJsonString(const char *&from, std::string &to);

	bool parseJsonNumberIntoSelf(const char *&from);

	bool parseJsonNumberIntoSelf(std::istream &from);

	static bool parseJsonString(std::istream &from, std::string &to);

	static bool fetchNextType(const char *&buffer, unsigned short &type,
			bool &isComma, bool &isColon, bool &isEnd);

	static bool fetchNextType(std::istream &stream, unsigned short &type,
			bool &isComma, bool &isColon, bool &isEnd);

	int getType() {
		return type;
	}

	void reserve() {
#ifdef HAS_JSON_STATS
		_JS0::stats.reserves++;
#endif
		ref_count++;
	}

	void release() {
#ifdef HAS_JSON_STATS
		_JS0::stats.releases++;
#endif
		if (--ref_count == 0)
			delete this;
	}

	_JS0 *clone() const;

	void copy(const _JS0 &from);

	struct entry {
		std::string key;
		_JS0 *node;
	};

	mutable unsigned short type;
	mutable unsigned short ref_count;
	mutable union {
		int64_t lng;
		bool boo;
		long double dbl;
		std::vector<_JS0 *> *arr;
		std::vector<struct entry *> *obj;
		std::string *str;
	} value;

#ifdef HAS_JSON_STATS
	static struct stats {
		unsigned allocs;
		unsigned frees;
		unsigned reserves;
		unsigned releases;
	} stats;
#endif

};

#endif /* ___JS0_H__ */
