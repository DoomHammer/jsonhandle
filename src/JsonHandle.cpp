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

#include "JsonHandle.h"
#include "_JS0.h"
#include <stdio.h>
#include <ios>

#ifdef HAS_JSON_STATS
static struct stats {
	unsigned allocs;
	unsigned frees;
} stats;
#endif

#define PRINTERR(foo,msg) fprintf(stderr,"JsonHandle::%s: %s\n", foo, msg)

#define STATE_NEW_NODE 0
#define STATE_HAS_NODE 1
#define STATE_NEW_CHILD_BY_KEY 2
#define STATE_NEW_CHILD_BY_STLKEY 3
#define STATE_NEW_CHILD_BY_INDEX 4
#define STATE_ORPHAN 5

const std::string JsonHandle::EMPTY_STRING;
int JsonHandle::defaultPrecision = 4;

JsonHandle JsonHandle::JSON_ERROR(NULL);

JsonHandle::~JsonHandle() {
#ifdef HAS_JSON_STATS
	stats.frees++;
#endif
	if (parent) {
		parent->release();
		parent = 0;
	}
	if (state == STATE_HAS_NODE) {
		vapor.node->release();
		state = STATE_ORPHAN;
	}
}

JsonHandle::JsonHandle(const JsonHandle &from) :
		parent(from.parent), state(from.state), vapor(from.vapor) {
#ifdef HAS_JSON_STATS
	stats.allocs++;
#endif
	if (state == STATE_HAS_NODE
	)
		vapor.node->reserve();
	if (parent)
		parent->reserve();
}

JsonHandle::JsonHandle() :
		parent(0), state(STATE_NEW_NODE) {
#ifdef HAS_JSON_STATS
	stats.allocs++;
#endif
}

JsonHandle::JsonHandle(_JS0 *node) :
		parent(0) {
#ifdef HAS_JSON_STATS
	stats.allocs++;
#endif
	if (node) {
		node->reserve();
		vapor.node = node;
		state = STATE_HAS_NODE;
	} else {
		vapor.node = (_JS0 *) -1;
		state = STATE_ORPHAN;
	}
	if (parent)
		parent->reserve();
}

JsonHandle::JsonHandle(_JS0 *node, _JS0 *parent) :
		parent(parent) {
#ifdef HAS_JSON_STATS
	stats.allocs++;
#endif
	if (node) {
		node->reserve();
		vapor.node = node;
		state = STATE_HAS_NODE;
	} else {
		state = STATE_ORPHAN;
	}
	if (parent)
		parent->reserve();
}

JsonHandle::JsonHandle(_JS0 *parent, int index) :
		parent(parent) {
#ifdef HAS_JSON_STATS
	stats.allocs++;
#endif
	parent->reserve();
	state = STATE_NEW_CHILD_BY_INDEX;
	vapor.index = index;
}

JsonHandle::JsonHandle(_JS0 *parent, const char *key) :
		parent(parent) {
#ifdef HAS_JSON_STATS
	stats.allocs++;
#endif
	parent->reserve();
	state = STATE_NEW_CHILD_BY_KEY;
	vapor.key = key;
}

JsonHandle::JsonHandle(_JS0 *parent, const std::string &key) :
		parent(parent) {
#ifdef HAS_JSON_STATS
	stats.allocs++;
#endif
	parent->reserve();
	state = STATE_NEW_CHILD_BY_STLKEY;
	vapor.stlkey = &key;
}

JsonHandle &
JsonHandle::copy(const JsonHandle &from) {
	if (state != STATE_HAS_NODE) {
		makeVaporNode("copy()");
		if (state != STATE_HAS_NODE
		)
			return *this;
	}
	if (from.state == STATE_HAS_NODE
	)
		vapor.node->copy(*(from.vapor.node));
	else
		vapor.node->setBoolean(false);
	return *this;
}

JsonHandle &
JsonHandle::operator=(const JsonHandle &from) {
	if (&from && &from != this && from.state == STATE_HAS_NODE) {

		if (state != STATE_HAS_NODE) {
			if (state != STATE_HAS_NODE) {
				makeVaporNode("operator[](int)");
				if (state != STATE_HAS_NODE
				)
					return JsonHandle::JSON_ERROR;
			}
		}
		vapor.node->copy(*from.vapor.node);
	}
	return *this;
}

JsonHandle JsonHandle::operator[](int index) {
	if (state != STATE_HAS_NODE) {
		makeVaporNode("operator[](int)");
		if (state != STATE_HAS_NODE
		)
			return JsonHandle::JSON_ERROR;
		vapor.node->setArray();
	}
	int len = size();
	if (index == -1 || index == len) // may be trying to append item
		return JsonHandle(vapor.node, len);
	if (index < 0 || index > len) {
		PRINTERR("operator[](int)", "index out of bounds");
		return JsonHandle::JSON_ERROR;
	}
	_JS0 *n = vapor.node->getByIndex(index);
	return n ? JsonHandle(n, vapor.node) : JsonHandle(vapor.node, index);
}

JsonHandle JsonHandle::operator[](const char *key) {
	if (key == NULL)
	{
		PRINTERR("operator[](char *)", "null key pointer");
		return JsonHandle::JSON_ERROR;
	}
	if (state != STATE_HAS_NODE) {
		makeVaporNode("operator[](char *)");
		if (state != STATE_HAS_NODE
		)
			return JsonHandle::JSON_ERROR;
		vapor.node->setObject();
	}
	_JS0 *n = vapor.node->getByKey(key);
	return n ? JsonHandle(n, vapor.node) : JsonHandle(vapor.node, key);
}

JsonHandle JsonHandle::operator[](const std::string &key) {
	if (&key == NULL)
	{
		PRINTERR("operator[](string &)", "null key");
		return JsonHandle::JSON_ERROR;
	}
	if (state != STATE_HAS_NODE) {
		makeVaporNode("operator[](string &)");
		if (state != STATE_HAS_NODE
		)
			return JsonHandle::JSON_ERROR;
		vapor.node->setObject();
	}
	_JS0 *n = vapor.node->getByKey(key);
	return n ? JsonHandle(n, vapor.node) : JsonHandle(vapor.node, key);
}

JsonHandle JsonHandle::at(int index) {
	if (state != STATE_HAS_NODE) {
		makeVaporNode("at(int)");
		if (state != STATE_HAS_NODE
		)
			return JsonHandle::JSON_ERROR;
		vapor.node->setArray();
	}
	int len = size();
	if (index == -1 || index == len) // may be trying to append item
		return JsonHandle(vapor.node, len);
	if (index < 0 || index > len) {
		PRINTERR("at(int)", "index out of bounds");
		return JsonHandle::JSON_ERROR;
	}
	_JS0 *n = vapor.node->getByIndex(index);
	return n ? JsonHandle(n, vapor.node) : JsonHandle(vapor.node, index);
}

JsonHandle JsonHandle::at(const char *key) {
	if (key == NULL)
	{
		PRINTERR("at(char *)", "null key pointer");
		return JsonHandle::JSON_ERROR;
	}
	if (state != STATE_HAS_NODE) {
		makeVaporNode("at(char *)");
		if (state != STATE_HAS_NODE
		)
			return JsonHandle::JSON_ERROR;
		vapor.node->setObject();
	}
	_JS0 *n = vapor.node->getByKey(key);
	return n ? JsonHandle(n, vapor.node) : JsonHandle(vapor.node, key);
}

JsonHandle JsonHandle::at(const std::string &key) {
	if (&key == NULL)
	{
		PRINTERR("at(string &)", "null key");
		return JsonHandle::JSON_ERROR;
	}
	if (state != STATE_HAS_NODE) {
		makeVaporNode("operator[](string &)");
		if (state != STATE_HAS_NODE
		)
			return JsonHandle::JSON_ERROR;
		vapor.node->setObject();
	}
	_JS0 *n = vapor.node->getByKey(key);
	return n ? JsonHandle(n, vapor.node) : JsonHandle(vapor.node, key);
}

int JsonHandle::size() {
	return (state == STATE_HAS_NODE
			&& (vapor.node->getType() == _JS0::ARRAY
					|| vapor.node->getType() == _JS0::OBJECT)) ?
			vapor.node->length() : 0;
}

JsonHandle&
JsonHandle::setLong(int64_t x) {
	if (state != STATE_HAS_NODE) {
		makeVaporNode("setLong()");
		if (state != STATE_HAS_NODE
		)
			return *this;
	}
	vapor.node->setLong(x);
	return *this;
}

JsonHandle&
JsonHandle::setBoolean(bool x) {
	if (state != STATE_HAS_NODE) {
		makeVaporNode("setBoolean()");
		if (state != STATE_HAS_NODE
		)
			return *this;
	}
	vapor.node->setBoolean(x);
	return *this;
}

JsonHandle&
JsonHandle::setDouble(long double x) {
	if (state != STATE_HAS_NODE) {
		makeVaporNode("setDouble()");
		if (state != STATE_HAS_NODE
		)
			return *this;
	}
	vapor.node->setDouble(x);
	return *this;
}

JsonHandle&
JsonHandle::setString(const char *x) {
	if (state != STATE_HAS_NODE) {
		makeVaporNode("setString(char *)");
		if (state != STATE_HAS_NODE
		)
			return *this;
	}
	if (x == NULL
	)
		vapor.node->setNull();
	else
		vapor.node->setString(x);
	return *this;
}

JsonHandle&
JsonHandle::setString(const std::string &x) {
	if (state != STATE_HAS_NODE) {
		makeVaporNode("setString(string &)");
		if (state != STATE_HAS_NODE
		)
			return *this;
	}
	if (&x == NULL
	)
		vapor.node->setNull();
	else
		vapor.node->setString(x);
	return *this;
}

void JsonHandle::makeVaporNode(const char *foo) {
	if (!parent && state == STATE_ORPHAN) {
		PRINTERR(foo, "null reference error");
		return;
	}
	if (parent && (state == STATE_ORPHAN)) {
		PRINTERR(foo, "internal error, no vapor");
		return;
	}
	_JS0 *n = new _JS0();
	if (n == 0) {
		PRINTERR(foo, "error: cannot allocate node");
		return;
	}
	n->reserve();
	if (parent) {
		if (state == STATE_NEW_CHILD_BY_STLKEY) {
			parent->setByKey(*(vapor.stlkey), n);
		} else if (state == STATE_NEW_CHILD_BY_KEY) {
			parent->setByKey(vapor.key, n);
		} else {
			parent->setByIndex(vapor.index, n);
		}
	}
	vapor.node = n;
	state = STATE_HAS_NODE;
}

static bool parseLong(const std::string &s, int64_t &value) {
	const char *p = s.c_str();
	bool isneg = (*p == '-');
	if (isneg)
		p++;
	if (*p < '0' || *p > '9')
		return false;
	value = *p++ - '0';
	while (*p) {
		if (*p < '0' || *p > '9')
			return false;
		value = value * 10 + (*p++ - '0');
	}
	if (isneg)
		value = -value;
	return true;
}

static bool parseDouble(const std::string &s, long double &value) {
	const char *p = s.c_str();
	bool isneg = (*p == '-');
	if (isneg)
		p++;
	if (*p < '0' || *p > '9')
		return false;
	int64_t ip = *p++ - '0';
	while (*p) {
		if (*p < '0' || *p > '9') {
			if (*p == '.')
				break;
			return false;
		}
		ip = ip * 10 + (*p++ - '0');
	}
	value = (long double) ip;
	if (*p == '.') {
		p++;
		ip = 10;
		while (*p) {
			if (*p < '0' || *p > '9')
				return false;
			value += (long double) (*p++ - '0') / (long double) ip;
			ip *= 10;
		}
	}
	if (isneg)
		value = -value;
	return true;
}

static bool parseBoolean(const std::string &s, bool &value) {

#define IS_LCC(i,c) ((((unsigned)(s[i]))|32)==(unsigned)(c))

	if (s.length() == 5) {
		if (IS_LCC(0,'f') && IS_LCC(1,'a') && IS_LCC(2,'l') && IS_LCC(3,'s')
				&& IS_LCC(3,'e')) {
			value = false;
			return true;

		}
	}
	if (s.length() == 4) {
		if (IS_LCC(0,'t') && IS_LCC(1,'r') && IS_LCC(2,'u') && IS_LCC(3,'e')) {
			value = true;
			return true;

		}
	}
	if (s.length() == 3) {
		if (IS_LCC(0,'y') && IS_LCC(1,'e') && IS_LCC(2,'s')) {
			value = true;
			return true;
		}
		if (IS_LCC(0,'o') && IS_LCC(1,'f') && IS_LCC(2,'f')) {
			value = false;
			return true;
		}
	}
	if (s.length() == 2) {
		if (IS_LCC(0,'o') && IS_LCC(1,'n')) {
			value = true;
			return true;
		}
	}
	if (s.length() == 1) {
		if (IS_LCC(0,'t') || IS_LCC(0,'1') || IS_LCC(0,'y')) {
			value = true;
			return true;
		}
		if (IS_LCC(0,'f') || IS_LCC(0,'0') || IS_LCC(0,'n')) {
			value = false;
			return true;
		}
	}
	return false;
}

int64_t JsonHandle::longValue(int64_t defaultValue) const {
	if (state == STATE_HAS_NODE) {
		if (vapor.node->getType() == _JS0::NUMBER_LONG
				|| vapor.node->getType() == _JS0::NUMBER_DOUBLE)
			return vapor.node->longValue();
		if (vapor.node->getType() == _JS0::BOOLEAN)
			return vapor.node->booleanValue() ? 1 : 0;
		if (vapor.node->getType() == _JS0::STRING) {
			const std::string &s = vapor.node->stringValue();
			int64_t value;
			if (&s && parseLong(s, value)) {
				vapor.node->setLong(value);
				return value;
			}
		}
	}
	return defaultValue;
}

bool JsonHandle::booleanValue(bool defaultValue) const {
	if (state == STATE_HAS_NODE) {
		if (vapor.node->getType() == _JS0::BOOLEAN)
			return vapor.node->booleanValue();
		if (vapor.node->getType() == _JS0::NUMBER_LONG
				|| vapor.node->getType() == _JS0::NUMBER_DOUBLE)
			return vapor.node->longValue() ? true : false;
		if (vapor.node->getType() == _JS0::STRING) {
			const std::string &s = vapor.node->stringValue();
			bool value;
			if (&s && parseBoolean(s, value)) {
				vapor.node->setBoolean(value);
				return value;
			}
		}
	}
	return defaultValue;
}

long double JsonHandle::doubleValue(long double defaultValue) const {
	if (state == STATE_HAS_NODE) {
		if (vapor.node->getType() == _JS0::NUMBER_DOUBLE
				|| vapor.node->getType() == _JS0::NUMBER_LONG)
			return vapor.node->doubleValue();
		if (vapor.node->getType() == _JS0::BOOLEAN)
			return vapor.node->booleanValue() ? 1 : 0;
		if (vapor.node->getType() == _JS0::STRING) {
			const std::string &s = vapor.node->stringValue();
			long double value;
			if (&s && parseDouble(s, value)) {
				vapor.node->setDouble(value);
				return value;
			}
		}
	}
	return defaultValue;
}

const std::string &
JsonHandle::stringValue(std::string const &defaultValue) const {
	if (state == STATE_HAS_NODE) {
		if (vapor.node->getType() == _JS0::STRING)
			return vapor.node->stringValue();
		if (vapor.node->getType() == _JS0::BOOLEAN) {
			vapor.node->setString(
					vapor.node->booleanValue() ? "true" : "false");
			return vapor.node->stringValue();
		}
		if (vapor.node->getType() == _JS0::NUMBER_LONG
				|| vapor.node->getType() == _JS0::NUMBER_DOUBLE) {
			std::string s;
			vapor.node->appendStringValue(s, defaultPrecision);
			vapor.node->setString(s);
			return vapor.node->stringValue();
		}
	}
	return defaultValue;
}

const char *
JsonHandle::stringValuePtr(const char *defaultValue) const {
	if (state == STATE_HAS_NODE) {
		if (vapor.node->getType() == _JS0::STRING) {
			const std::string &s = vapor.node->stringValue();
			return (&s == 0) ? NULL : s.c_str();
		}
		if (vapor.node->getType() == _JS0::BOOLEAN) {
			vapor.node->setString(
					vapor.node->booleanValue() ? "true" : "false");
			const std::string &s = vapor.node->stringValue();
			return (&s == 0) ? NULL : s.c_str();
		}
		if (vapor.node->getType() == _JS0::NUMBER_LONG
				|| vapor.node->getType() == _JS0::NUMBER_DOUBLE) {
			std::string st;
			vapor.node->appendStringValue(st, defaultPrecision);
			vapor.node->setString(st);
			const std::string &s = vapor.node->stringValue();
			return (&s == 0) ? NULL : s.c_str();
		}
	}
	return defaultValue;
}

std::string &
JsonHandle::toString(std::string &buffer, int precision) const {
	if (state == STATE_HAS_NODE
	)
		vapor.node->appendJsonString(buffer, true, precision, 0);
	return buffer;
}

const std::string &
JsonHandle::key(int index) const {
	return (state == STATE_HAS_NODE) ?
			vapor.node->getKeyAtIndex(index) : *(std::string*) 0;
}

bool
JsonHandle::exists(std::string const &key) const {
	if (&key == NULL)
	{
		return false;
	}
	_JS0 *n = vapor.node->getByKey(key);
  return n != NULL;
}

std::string &
JsonHandle::toCompactString(std::string &buffer, int precision) const {
	if (state == STATE_HAS_NODE
	)
		vapor.node->appendJsonString(buffer, false, precision, 0);
	return buffer;
}

JsonHandle &
JsonHandle::clear() {
	if (state == STATE_HAS_NODE
	)
		vapor.node->clear();
	return *this;
}

JsonHandle &
JsonHandle::erase(int index) {
	if (state == STATE_HAS_NODE
	)
		vapor.node->erase(index);
	return *this;
}

JsonHandle &
JsonHandle::erase(const char *key) {
	if (state == STATE_HAS_NODE
	)
		vapor.node->erase(key);
	return *this;
}

JsonHandle &
JsonHandle::erase(const std::string &key) {
	if (state == STATE_HAS_NODE
	)
		vapor.node->erase(key);
	return *this;
}

bool JsonHandle::isNull() const {
	return state == STATE_HAS_NODE && vapor.node->getType() == _JS0::NULLVALUE;
}
bool JsonHandle::isArray() const {
	return state == STATE_HAS_NODE && vapor.node->getType() == _JS0::ARRAY;
}
bool JsonHandle::isObject() const {
	return state == STATE_HAS_NODE && vapor.node->getType() == _JS0::OBJECT;
}
bool JsonHandle::isString() const {
	return state == STATE_HAS_NODE && vapor.node->getType() == _JS0::STRING;
}
bool JsonHandle::isNumber() const {
	return state == STATE_HAS_NODE
			&& (vapor.node->getType() == _JS0::NUMBER_LONG
					|| vapor.node->getType() == _JS0::NUMBER_DOUBLE);
}
bool JsonHandle::isBoolean() const {
	return state == STATE_HAS_NODE && vapor.node->getType() == _JS0::BOOLEAN;
}

JsonHandle &
JsonHandle::fromString(const std::string &from) {
	if (state != STATE_HAS_NODE) {
		makeVaporNode("fromString()");
		if (state != STATE_HAS_NODE
		)
			return JsonHandle::JSON_ERROR;
	}
	if (&from == 0 || !vapor.node->setFromJsonStlString(from)) {
		return JsonHandle::JSON_ERROR;
	}
	return *this;

}

JsonHandle &
JsonHandle::fromString(const char *from) {
	if (state != STATE_HAS_NODE) {
		makeVaporNode("fromString()");
		if (state != STATE_HAS_NODE
		)
			return JsonHandle::JSON_ERROR;
	}
	if (from == 0 || !vapor.node->setFromJsonString(from)) {
		return JsonHandle::JSON_ERROR;
	}
	return *this;

}

void JsonHandle::dumpStats() {
#ifdef HAS_JSON_STATS
	fprintf(stderr, "\n+------------------------------+\n");
	fprintf(stderr, "|         JsonHandle Stats     |\n");
	fprintf(stderr, "|------------------------------|\n");
	fprintf(stderr, "| handle allocs   | %10u |\n", stats.allocs);
	fprintf(stderr, "|        frees    | %10u |\n", stats.frees);
	fprintf(stderr, "| node   allocs   | %10u |\n", _JS0::stats.allocs);
	fprintf(stderr, "|        frees    | %10u |\n", _JS0::stats.frees);
	fprintf(stderr, "|        reserves | %10u |\n", _JS0::stats.reserves);
	fprintf(stderr, "|        releases | %10u |\n", _JS0::stats.releases);
	fprintf(stderr, "+------------------------------+\n\n");
#else
	fprintf(stderr,"JsonHandle: dumpStats() not compiled in with -DHAS_JSON_STATS\n");
#endif
}

JsonHandle &
JsonHandle::fromFile(const char *file) {

#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
	FILE* fp = 0;
	if ( fopen_s( &fp, file, "r" ) )
	fp = 0;
#else
	FILE *fp = fopen(file, "r");
#endif
	if (fp == 0) {
		PRINTERR("fromFile()", "could not read file");
		return JsonHandle::JSON_ERROR;
	}
	long length = 0;
	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *buf = new char[length + 1];
	if (buf == 0) {
		// could not allocate memory, file too large?
		PRINTERR("fromFile()", "could not allocate memory");
		fclose(fp);
		return JsonHandle::JSON_ERROR;
	}

	if (!fread(buf, length, 1, fp)) length = 0;
	fclose(fp);
	buf[length] = 0;

	JsonHandle &ret = fromString(buf);

	delete []buf;
	return ret;
}

bool JsonHandle::toFile(const char *file, int precision) const {

#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
	FILE* fp = 0;
	if ( fopen_s( &fp, file, "w" ) )
	fp = 0;
#else
	FILE *fp = fopen(file, "w");
#endif
	if (fp == 0) {
		PRINTERR("toFile()", "could not write file");
		return false;
	}

	std::string buffer;
	toString(buffer, precision);
	fwrite(buffer.data(), 1, buffer.length(), fp);
	fclose(fp);
	return true;
}

std::istream &
JsonHandle::fromStream(std::istream &stream) {
	if (stream.rdstate() & std::ios_base::failbit)
		return stream;
	if (state != STATE_HAS_NODE) {
		makeVaporNode("fromStream()");
		if (state != STATE_HAS_NODE
		) {
			stream.setstate(stream.rdstate() | std::ios_base::failbit);
			return stream;
		}
	}
	if (!vapor.node->setFromStream(stream)) {
		stream.setstate(stream.rdstate() | std::ios_base::failbit);
		return stream;
	}
	return stream;
}

std::ostream &
JsonHandle::toStream(std::ostream &stream) {
	if (state == STATE_HAS_NODE
	) {
		std::string buf;
		vapor.node->appendStream(stream);
	}
	return stream;
}

