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

#include "_JS0.h"
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif

using namespace std;

static const char indentBuffer[] =
		"\n                                                  ";
#define MAX_INDENT ((int)sizeof(indentBuffer)-2)
#define INDENT(s,n)  s.append(indentBuffer,1 + (((n) < MAX_INDENT) ? (n) : MAX_INDENT))

#define PRINTERR(foo,msg) fprintf(stderr,"_JS0::%s: %s\n", foo, msg)

#define HEX(c) ((c >= '0' && c <= '9') ? (c - '0') : (c >= 'a' && c <= 'f') ? ((c - 'a') + 10) : (c >= 'A' && c <= 'F') ? ((c - 'F') + 10) :  -1)

#ifdef HAS_JSON_STATS
struct _JS0::stats _JS0::stats;
#endif

void _JS0::reset() {
	if (type != _JS0::UNDEFINED) {
		switch (type) {
		case _JS0::OBJECT: {
			for (int i = value.obj->size() - 1; i >= 0; i--) {
				struct entry *e = value.obj->at(i);
				e->node->release();
				delete e;
			}
			delete value.obj;
		}
			break;
		case _JS0::ARRAY: {
			for (int i = value.arr->size() - 1; i >= 0; i--)
				value.arr->at(i)->release();
			delete value.arr;
		}
			break;
		case _JS0::STRING:
			delete value.str;
			break;
		default:
			break;
		}
		type = _JS0::UNDEFINED;
	}
}

int _JS0::length() const {
	switch (type) {
	case _JS0::OBJECT:
		return value.obj->size();
	case _JS0::ARRAY:
		return value.arr->size();
	default:
		return 0;
	}
}

_JS0 *
_JS0::getByIndex(int index) const {
	if (type == _JS0::ARRAY) {
		if (index >= 0 && index < (int) value.arr->size())
			return value.arr->at(index);
	} else if (type == _JS0::OBJECT) {
		if (index >= 0 && index < (int) value.obj->size())
			return value.obj->at(index)->node;
	}
	return (_JS0 *) 0;
}

_JS0 *
_JS0::getByKey(std::string const &key) const {
	if (type == _JS0::OBJECT) {
		for (int i = value.obj->size() - 1; i >= 0; i--) {
			struct entry *e = value.obj->at(i);
			if (key.compare(e->key) == 0)
				return e->node;
		}
	}
	return (_JS0 *) 0;
}

_JS0 *
_JS0::getByKey(const char *key) const {
	if (type == _JS0::OBJECT) {
		for (int i = value.obj->size() - 1; i >= 0; i--) {
			struct entry *e = value.obj->at(i);
			if (e->key.compare(key) == 0)
				return e->node;
		}
	}
	return (_JS0 *) 0;
}

bool _JS0::booleanValue() const {
	return value.boo;
}

int64_t _JS0::longValue() const {
	return (type == _JS0::NUMBER_LONG) ? value.lng :
			(type == _JS0::NUMBER_DOUBLE) ? (int64_t) value.dbl :
			(type == _JS0::BOOLEAN && value.boo) ? 1 : 0;
}

long double _JS0::doubleValue() const {
	return (type == _JS0::NUMBER_DOUBLE) ? value.dbl :
			(type == _JS0::NUMBER_LONG) ? (long double) value.lng :
			(type == _JS0::BOOLEAN && value.boo) ? 1 : 0;
}

const std::string &
_JS0::getKeyAtIndex(int index) const {
	if (type == _JS0::OBJECT) {
		if (index >= 0 && index < (int) value.obj->size())
			return value.obj->at(index)->key;
	}
	return *(std::string *) 0;
}

const std::string &
_JS0::stringValue() const {
	return (type == _JS0::STRING) ? *value.str : *(std::string*) 0;
}

_JS0 *
_JS0::clone() const {
	_JS0 *node = new _JS0();
	node->setType(type);
	switch (type) {
	case _JS0::NUMBER_LONG:
		node->value.lng = value.lng;
		break;
	case _JS0::NUMBER_DOUBLE:
		node->value.dbl = value.dbl;
		break;
	case _JS0::BOOLEAN:
		node->value.boo = value.boo;
		break;
	case _JS0::STRING:
		*(node->value.str) = *value.str;
		break;
	case _JS0::OBJECT: {
		int len = value.obj->size();
		for (int i = 0; i < len; i++) {
			struct entry *ae = value.obj->at(i);
			struct entry *be = new struct entry;
			be->key = ae->key;
			be->node = ae->node->clone();
			be->node->reserve();
			node->value.obj->push_back(be);
		}
		break;
	}
	case _JS0::ARRAY: {
		int len = value.arr->size();
		for (int i = 0; i < len; i++) {
			_JS0 *n = value.arr->at(i)->clone();
			n->reserve();
			node->value.arr->push_back(n);
		}
		break;
	}
	}
	return node;
}

void _JS0::copy(const _JS0 &from) {
	setType(from.type);
	switch (from.type) {
	case _JS0::NUMBER_LONG:
		value.lng = from.value.lng;
		break;
	case _JS0::NUMBER_DOUBLE:
		value.dbl = from.value.dbl;
		break;
	case _JS0::BOOLEAN:
		value.boo = from.value.boo;
		break;
	case _JS0::STRING:
		*(value.str) = *(from.value.str);
		break;
	case _JS0::OBJECT: {
		int olen = value.obj->size();
		int len = from.value.obj->size();
		for (int i = 0; i < len; i++) {
			struct entry *ae = from.value.obj->at(i);
			struct entry *be = new struct entry;
			be->key = ae->key;
			be->node = ae->node->clone();
			be->node->reserve();
			value.obj->push_back(be);
		}
		if (olen > 0) {
			for (int i = 0; i < olen; i++) {
				struct entry * e = value.obj->at(i);
				e->node->release();
				delete e;
			}
			value.obj->erase(value.obj->begin(), value.obj->begin() + olen);
		}
		break;
	}
	case _JS0::ARRAY: {
		int olen = value.arr->size();
		int len = from.value.arr->size();
		for (int i = 0; i < len; i++) {
			_JS0 *n = from.value.arr->at(i)->clone();
			n->reserve();
			value.arr->push_back(n);
		}
		if (olen > 0) {
			for (int i = 0; i < olen; i++) {
				value.arr->at(i)->release();
			}
			value.arr->erase(value.arr->begin(), value.arr->begin() + olen);
		}
		break;
	}
	}
}

void _JS0::appendJsonString(std::string &buffer, bool whiteSpace, int precision,
		int depth) const {
	switch (type) {
	case _JS0::STRING:
		quoteJsonString(*value.str, buffer);
		break;
	case _JS0::NUMBER_LONG:
	case _JS0::NUMBER_DOUBLE:
	case _JS0::BOOLEAN:
	case _JS0::NULLVALUE:
		appendStringValue(buffer, precision);
		break;
	case _JS0::OBJECT: {
		buffer.push_back('{');
		int len = value.obj->size();
		for (int i = 0; i < len; i++) {
			struct entry *e = value.obj->at(i);
			if (i > 0)
				buffer.push_back(',');
			if (whiteSpace)
				INDENT(buffer, depth+1);
			quoteJsonString(e->key, buffer);
			buffer.push_back(':');
			if (whiteSpace)
				buffer.push_back(' ');
			e->node->appendJsonString(buffer, whiteSpace, precision, depth + 1);
		}
		if (whiteSpace)
			INDENT(buffer, depth);
		buffer.push_back('}');
	}
		break;
	case _JS0::ARRAY: {
		buffer.push_back('[');
		int len = value.arr->size();
		for (int i = 0; i < len; i++) {
			if (i > 0)
				buffer.push_back(',');
			if (whiteSpace)
				INDENT(buffer, depth+1);
			value.arr->at(i)->appendJsonString(buffer, whiteSpace, precision,
					depth + 1);
		}
		if (whiteSpace)
			INDENT(buffer, depth);
		buffer.push_back(']');
	}
		break;
	}
}

void _JS0::appendStream(std::ostream &stream) const {
	if (!stream.good())
		return;
	std::string buf;
	switch (type) {
	case _JS0::STRING: {
		quoteJsonString(*value.str, buf);
		stream.write(buf.data(), buf.length());
		break;
	}
	case _JS0::NUMBER_LONG:
	case _JS0::NUMBER_DOUBLE:
	case _JS0::BOOLEAN:
	case _JS0::NULLVALUE: {
		appendStringValue(buf, stream.precision());
		stream.write(buf.data(), buf.length());
		break;
	}
	case _JS0::OBJECT: {
		stream.put('{');
		int len = value.obj->size();
		for (int i = 0; i < len && stream.good(); i++) {
			struct entry *e = value.obj->at(i);
			if (i > 0)
				stream.put(',');
			buf.clear();
			quoteJsonString(e->key, buf);
			stream.write(buf.data(), buf.length());
			stream.put(':');
			e->node->appendStream(stream);
		}
		stream.put('}');
	}
		break;
	case _JS0::ARRAY: {
		stream.put('[');
		int len = value.arr->size();
		for (int i = 0; i < len && stream.good(); i++) {
			if (i > 0)
				stream.put(',');
			value.arr->at(i)->appendStream(stream);
		}
		stream.put(']');
	}
		break;
	}
}

void _JS0::quoteJsonString(const std::string &src, std::string &dest) {
	const char *value = src.c_str();
	int b = 0, len = src.length();

	dest.push_back('"');

#define START_ESCAPE \
	if (b != i) dest.append(value + b, i - b); \
	b = i + 1; \
	dest.push_back('\\');

	for (int i = 0; i < len; i++) {
		char ch = value[i];
		switch (ch) {
		case '\\':
			START_ESCAPE
			;
			dest.push_back('\\');
			break;
		case '\n':
			START_ESCAPE
			;
			dest.push_back('n');
			break;
		case '"':
			START_ESCAPE
			;
			dest.push_back('\"');
			break;
		case '\b':
			START_ESCAPE
			;
			dest.push_back('b');
			break;
		case '\f':
			START_ESCAPE
			;
			dest.push_back('f');
			break;
		case '\r':
			START_ESCAPE
			;
			dest.push_back('r');
			break;
		case '\t':
			START_ESCAPE
			;
			dest.push_back('t');
			break;
		default:
			break;
		}
	}
	if (b < len)
		dest.append(value + b, len - b);

	dest.push_back('"');
}

void _JS0::toJsonString(std::string &buffer, bool whiteSpace,
		int precision) const {
	buffer.clear();
	_JS0::appendJsonString(buffer, whiteSpace, precision, 0);
}

void _JS0::appendStringValue(std::string &buffer, int precision) const {
	if (type == _JS0::NUMBER_LONG) {
		char buf[32];
		int len = snprintf(buf, sizeof(buf), "%lld", (long long)value.lng);
		buffer.append(buf, len);
	} else if (type == _JS0::NUMBER_DOUBLE) {
		char buf[64];
		int len = snprintf(buf, sizeof(buf), "%.*Lf", precision, value.dbl);
		if (len > precision + 1) { // get rid of trailing zeros
			char *p;
			for (p = buf + len - 1; p > buf && *p == '0'; p--)
				;
			if (*p == '.')
				p--;
			*(++p) = 0;
			len = (p - buf);
		}
		buffer.append(buf, len);
	} else if (type == _JS0::STRING) {
		buffer.append(*value.str);
	} else if (type == _JS0::BOOLEAN) {
		buffer.append(value.boo ? "true" : "false");
	} else if (type == _JS0::NULLVALUE) {
		buffer.append("null");
	}
}

void _JS0::setLong(int64_t val) {
	setType(_JS0::NUMBER_LONG);
	value.lng = val;
}

void _JS0::setDouble(long double val) {
	setType(_JS0::NUMBER_DOUBLE);
	value.dbl = val;
}

void _JS0::setBoolean(bool val) {
	setType(_JS0::BOOLEAN);
	value.boo = val;
}

void _JS0::setString(const std::string &val) {
	setType(_JS0::STRING);
	*value.str = val;
}

void _JS0::setString(const char *val) {
	setType(_JS0::STRING);
	if (val != NULL) {
		*value.str = val;
	}
}

void _JS0::setNull() {
	setType(_JS0::NULLVALUE);
}

void _JS0::setObject() {
	setType(_JS0::OBJECT);
}

void _JS0::setArray() {
	setType(_JS0::ARRAY);
}

void _JS0::clear() {
	if (type == _JS0::OBJECT) {
		for (int i = value.obj->size() - 1; i >= 0; i--) {
			struct entry *e = value.obj->at(i);
			e->node->release();
			delete e;
		}
		value.obj->clear();
	} else if (type == _JS0::ARRAY) {
		for (int i = value.arr->size() - 1; i >= 0; i--) {
			value.arr->at(i)->release();
		}
		value.arr->clear();
	}
}

void _JS0::setByKey(const char *key, _JS0 *val) {
	setType(_JS0::OBJECT);
	if (val)
		val->reserve();
	for (int i = value.obj->size() - 1; i >= 0; i--) {
		struct entry *e = value.obj->at(i);
		if (e->key.compare(key) == 0) {
			e->node->release();
			e->node = val;
			if (val == NULL) {
				delete e;
				value.obj->erase(value.obj->begin() + i,
						value.obj->begin() + i + 1);
			}
			return;
		}
	}
	if (val) {
		struct entry *e = new struct entry;
		e->node = val;
		e->key = key;
		value.obj->push_back(e);
	}
}

void _JS0::setByKey(const std::string &key, _JS0 *val) {
	setType(_JS0::OBJECT);
	if (val)
		val->reserve();
	for (int i = value.obj->size() - 1; i >= 0; i--) {
		struct entry *e = value.obj->at(i);
		if (e->key.compare(key) == 0) {
			e->node->release();
			e->node = val;
			if (val == NULL) {
				delete e;
				value.obj->erase(value.obj->begin() + i,
						value.obj->begin() + i + 1);
			}
			return;
		}
	}
	if (val) {
		struct entry *e = new struct entry;
		e->node = val;
		e->key = key;
		value.obj->push_back(e);
	}
}

void _JS0::setByIndex(int index, _JS0 *val) {
	if (val && type != _JS0::ARRAY)
		setType(_JS0::ARRAY);
	if (val == NULL) {
		if (index >= 0 && index < (int) value.arr->size()) {
			value.arr->at(index)->release();
			value.arr->erase(value.arr->begin() + index,
					value.arr->begin() + index + 1);
		}
	} else {
		val->reserve();
		if (index == (int) value.arr->size()) {
			value.arr->push_back(val);
		} else if (index >= 0 && index < (int) value.arr->size()) {
			_JS0 *node = value.arr->at(index);
			value.arr->assign(index, val);
			node->release();
		} else {
			// index out of range
			PRINTERR("setByIndex()", "index out of range");
			val->release();
		}
	}

}

void _JS0::setType(int type) {
	if (this->type == type)
		return;
	reset();
	this->type = type;
	switch (type) {
	case _JS0::OBJECT:
		value.obj = new vector<struct entry *>();
		break;
	case _JS0::ARRAY:
		value.arr = new vector<_JS0 *>();
		break;
	case _JS0::STRING:
		value.str = new string();
		break;
	}
}

void _JS0::erase(int index) {
	if (type == _JS0::ARRAY) {
		int len = value.arr->size();
		if (index == -1)
			index = len - 1;
		if (index >= 0 && index < len) {
			value.arr->at(index)->release();
			value.arr->erase(value.arr->begin() + index,
					value.arr->begin() + index + 1);
		}
	} else if (type == _JS0::OBJECT) {
		int len = value.obj->size();
		if (index == -1)
			index = len - 1;
		if (index >= 0 && index < len) {
			struct entry *e = value.obj->at(index);
			e->node->release();
			delete e;
			value.obj->erase(value.obj->begin() + index,
					value.obj->begin() + index + 1);
		}
	}
}

void _JS0::erase(const char *key) {
	if (key != 0 && type == _JS0::OBJECT) {
		for (int i = value.obj->size() - 1; i >= 0; i--) {
			struct entry *e = value.obj->at(i);
			if (e->key.compare(key) == 0) {
				e->node->release();
				delete e;
				value.obj->erase(value.obj->begin() + i,
						value.obj->begin() + i + 1);
				return;
			}
		}
	}
}

void _JS0::erase(const std::string &key) {
	if (&key != 0 && type == _JS0::OBJECT) {
		for (int i = value.obj->size() - 1; i >= 0; i--) {
			struct entry *e = value.obj->at(i);
			if (e->key.compare(key) == 0) {
				e->node->release();
				delete e;
				value.obj->erase(value.obj->begin() + i,
						value.obj->begin() + i + 1);
				return;
			}
		}
	}
}

bool _JS0::setFromJsonStlString(const std::string &buffer) {
	const char *p = buffer.c_str();
	return setFromJsonString(p);
}

bool _JS0::setFromJsonString(const char *&buffer) {
	unsigned short s_type;
	std::string str;
	bool isComma, isColon, isEnd;
	if (buffer == 0)
		return false;
	if (!fetchNextType(buffer, s_type, isComma, isColon, isEnd) || isComma
			|| isColon || isEnd) {
		if (*buffer != 0)
			PRINTERR("setFromJsonString()", "syntax error: missing value");
		return false;
	}
	switch (s_type) {
	case _JS0::NUMBER_LONG:
		if (!parseJsonNumberIntoSelf(buffer)) {
			PRINTERR("setFromJsonString()", "syntax error: invalid number");
			return false;
		}
		return true;
	case _JS0::STRING:
		setType(_JS0::STRING);
		if (!parseJsonString(buffer, *value.str)) {
			PRINTERR("setFromJsonString()", "syntax error: invalid string");
			return false;
		}
		return true;
	case _JS0::NULLVALUE:
		if (buffer[0] == 'n' && buffer[1] == 'u' && buffer[2] == 'l'
				&& buffer[3] == 'l') {
			setType(_JS0::NULLVALUE);
			buffer += 4;
		} else {
			PRINTERR("setFromJsonString()", "syntax error: invalid null value");
			return false;
		}
		return true;
	case _JS0::BOOLEAN:
		if (buffer[0] == 't' && buffer[1] == 'r' && buffer[2] == 'u'
				&& buffer[3] == 'e') {
			setType(_JS0::BOOLEAN);
			value.boo = true;
			buffer += 4;
		} else if (buffer[0] == 'f' && buffer[1] == 'a' && buffer[2] == 'l'
				&& buffer[3] == 's' && buffer[4] == 'e') {
			setType(_JS0::BOOLEAN);
			value.boo = false;
			buffer += 5;
		} else {
			PRINTERR("setFromJsonString()",
					"syntax error: invalid boolean value");
			return false;
		}
		return true;
	case _JS0::OBJECT:
		setType(_JS0::OBJECT);
		buffer++;
		while (1) {
			if (!fetchNextType(buffer, s_type, isComma, isColon, isEnd)) {
				PRINTERR("setFromJsonString()",
						"syntax error: object not closed");
				return false;
			}
			if (isEnd) {
				if (s_type != _JS0::OBJECT) {
					PRINTERR("setFromJsonString()",
							"syntax error: object not closed");
					return false;
				}
				buffer++;
				return true;
			}
			if (s_type != _JS0::STRING) {
				PRINTERR("setFromJsonString()",
						"syntax error: object key must be a string");
				return false;
			}
			struct entry *e = new struct entry;
			if (!parseJsonString(buffer, e->key)) {
				PRINTERR("setFromJsonString()",
						"syntax error: invalid string for object key");
				delete e;
				return false;
			}
			if (!fetchNextType(buffer, s_type, isComma, isColon, isEnd)
					|| !isColon) {
				PRINTERR("setFromJsonString()",
						"syntax error: object colon expected");
				delete e;
				return false;
			}
			buffer++;
			e->node = new _JS0();
			e->node->reserve();
			if (!e->node->setFromJsonString(buffer)) {
				e->node->release();
				delete e;
				return false;
			}
			if (!fetchNextType(buffer, s_type, isComma, isColon, isEnd)
					|| !(isComma || (isEnd && s_type == _JS0::OBJECT))) {
				PRINTERR("setFromJsonString()",
						"syntax error: object comma or end expected");
				e->node->release();
				delete e;
				return false;
			}
			buffer++;
			value.obj->push_back(e);
			if (isEnd) {
				return true;
			}
		} // while
		break;
	case _JS0::ARRAY:
		setType(_JS0::ARRAY);
		buffer++;
		while (1) {
			if (!fetchNextType(buffer, s_type, isComma, isColon, isEnd)) {
				PRINTERR("setFromJsonString()",
						"syntax error: array not closed");
				return false;
			}
			if (isEnd) {
				if (s_type != _JS0::ARRAY) {
					PRINTERR("setFromJsonString()",
							"syntax error: array not closed");
					return false;
				}
				buffer++;
				return true;
			}
			_JS0 *n = new _JS0();
			n->reserve();
			if (!n->setFromJsonString(buffer)) {
				n->release();
				return false;
			}
			if (!fetchNextType(buffer, s_type, isComma, isColon, isEnd)
					|| !(isComma || (isEnd && s_type == _JS0::ARRAY))) {
				PRINTERR("setFromJsonString()",
						"syntax error: array comma or end expected");
				n->release();
				return false;
			}
			buffer++;
			value.arr->push_back(n);
			if (isEnd) {
				return true;
			}
		} // while
		break;
	} // switch
	return false;
}

bool _JS0::parseJsonString(const char *&input, std::string &out) {
	out.clear();
	if (*input != '"')
		return false;

	for (int c = *(++input); c != 0; c = *(++input)) {
		if (c == '"') {
			input++;
			return true;
		}
		if (c != '\\') {
			out.push_back(c);
			continue;
		}
		c = *(++input);
		switch (c) {
		case 0:
			return false;
		case '"':
			out.push_back('"');
			break;
		case '\\':
			out.push_back('\\');
			break;
		case 'n':
			out.push_back('\n');
			break;
		case 't':
			out.push_back('\t');
			break;
		case 'r':
			out.push_back('\r');
			break;
		case 'f':
			out.push_back('\f');
			break;
		case 'b':
			out.push_back('\b');
			break;
		case '/':
			out.push_back('/');
			break;
		case 'u': {
			short h;
			unsigned short unicode;
			c = *(++input);
			if ((h = HEX(c)) == -1)
				return false;
			unicode = h;
			c = *(++input);
			if ((h = HEX(c)) == -1)
				return false;
			unicode = (unicode << 4) | h;
			c = *(++input);
			if ((h = HEX(c)) == -1)
				return false;
			unicode = (unicode << 4) | h;
			c = *(++input);
			if ((h = HEX(c)) == -1)
				return false;
			unicode = (unicode << 4) | h;
			// XXX: unicode support is not implemented, so
			// just cheat and reduce character width
			out.push_back((char) (unicode & 0xff));
			break;
		}
		default:
			// assume it was not a real escape sequence
			// as it is outside the JSON specification
			out.push_back('\\');
			out.push_back(c);
			break;
		}
	}
	return false;
}

bool _JS0::parseJsonString(std::istream &stream, std::string &out) {
	out.clear();
	char ch = stream.get();
	if (ch != '"' || !stream.good())
		return false;

	for (int c = stream.get(); stream.good(); c = stream.get()) {
		if (c == '"') {
			return true;
		}
		if (c != '\\') {
			out.push_back(c);
			continue;
		}
		c = stream.get();
		if (!stream.good())
			return false;
		switch (c) {
		case '"':
			out.push_back('"');
			break;
		case '\\':
			out.push_back('\\');
			break;
		case 'n':
			out.push_back('\n');
			break;
		case 't':
			out.push_back('\t');
			break;
		case 'r':
			out.push_back('\r');
			break;
		case 'f':
			out.push_back('\f');
			break;
		case 'b':
			out.push_back('\b');
			break;
		case '/':
			out.push_back('/');
			break;
		case 'u': {
			short h;
			unsigned short unicode;
			c = stream.get();
			if (!stream.good())
				return false;
			if ((h = HEX(c)) == -1)
				return false;
			unicode = h;
			c = stream.get();
			if (!stream.good())
				return false;
			if ((h = HEX(c)) == -1)
				return false;
			unicode = (unicode << 4) | h;
			c = stream.get();
			if (!stream.good())
				return false;
			if ((h = HEX(c)) == -1)
				return false;
			unicode = (unicode << 4) | h;
			c = stream.get();
			if (!stream.good())
				return false;
			if ((h = HEX(c)) == -1)
				return false;
			unicode = (unicode << 4) | h;
			// XXX: unicode support is not implemented, so
			// just cheat and reduce character width
			out.push_back((char) (unicode & 0xff));
			break;
		}
		default:
			// assume it was not a real escape sequence
			// as it is outside the JSON specification
			out.push_back('\\');
			out.push_back(c);
			break;
		}
	}
	return false;
}

bool _JS0::parseJsonNumberIntoSelf(const char *&p) {
	bool isneg = (*p == '-');
	if (isneg)
		p++;
	if (*p < '0' || *p > '9')
		return false;
	int64_t ip = *p++ - '0';
	for (; *p; p++) {
		char c = *p;
		if (c < '0' || c > '9')
			break;
		ip = ip * 10 + (c - '0');
	}
	if (*p == '.') {
		setType(_JS0::NUMBER_DOUBLE);
		value.dbl = ((long double) ip);
		ip = 10;
		p++;
		for (; *p; p++) {
			char c = *p;
			if (c < '0' || c > '9')
				break;
			if (c != '0') // save cycles
				value.dbl += (long double) (c - '0') / (long double) ip;
			ip *= 10;
		}
		if (isneg)
			value.dbl = -value.dbl;
	} else {
		setType(_JS0::NUMBER_LONG);
		value.lng = ip;
		if (isneg)
			value.lng = -value.lng;
	}
	return true;
}

bool _JS0::parseJsonNumberIntoSelf(std::istream &stream) {
	char ch = stream.get();
	if (!stream.good())
		return false;
	bool isneg = (ch == '-');
	if (isneg) {
		ch = stream.get();
		if (!stream.good())
			return false;
	}
	if (ch < '0' || ch > '9')
		return false;
	int64_t ip = ch - '0';
	ch = stream.get();
	for (; stream.good(); ch = stream.get()) {
		if (ch < '0' || ch > '9')
			break;
		ip = ip * 10 + (ch - '0');
	}
	if (ch == '.') {
		setType(_JS0::NUMBER_DOUBLE);
		value.dbl = ((long double) ip);
		ip = 10;
		ch = stream.get();
		for (; stream.good(); ch = stream.get()) {
			if (ch < '0' || ch > '9')
				break;
			if (ch != '0') // save cycles
				value.dbl += (long double) (ch - '0') / (long double) ip;
			ip *= 10;
		}
		if (isneg)
			value.dbl = -value.dbl;
	} else {
		setType(_JS0::NUMBER_LONG);
		value.lng = ip;
		if (isneg)
			value.lng = -value.lng;
	}
	if (stream.good())
		stream.unget();
	return true;
}

bool _JS0::fetchNextType(const char *&buffer, unsigned short &type,
		bool &isComma, bool &isColon, bool &isEnd) {
	isColon = isComma = isEnd = false;
	while (*buffer) {
		switch (*buffer) {
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			buffer++;
			continue;
		case '"':
			type = _JS0::STRING;
			return true;
		case '[':
			type = _JS0::ARRAY;
			return true;
		case ']':
			type = _JS0::ARRAY;
			isEnd = true;
			return true;
		case '{':
			type = _JS0::OBJECT;
			return true;
		case '}':
			type = _JS0::OBJECT;
			isEnd = true;
			return true;
		case ',':
			isComma = true;
			return true;
		case ':':
			isColon = true;
			return true;
		case '-':
		case '+':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			type = _JS0::NUMBER_LONG;
			return true;
		case 'n':
			type = _JS0::NULLVALUE;
			return true;
		case 't':
		case 'f':
			type = _JS0::BOOLEAN;
			return true;
		default:
			return false;
		}
	}
	return false;
}

bool _JS0::setFromStream(std::istream &stream) {
	unsigned short s_type;
	std::string str;
	bool isComma, isColon, isEnd;
	if (!fetchNextType(stream, s_type, isComma, isColon, isEnd) || isComma
			|| isColon || isEnd) {
		if (!stream.eof())
			PRINTERR("setFromStream()", "syntax error: missing value");
		return false;
	}
	switch (s_type) {
	case _JS0::NUMBER_LONG:
		if (!parseJsonNumberIntoSelf(stream)) {
			PRINTERR("setFromStream()", "syntax error: invalid number");
			return false;
		}
		return true;
	case _JS0::STRING:
		setType(_JS0::STRING);
		if (!parseJsonString(stream, *value.str)) {
			PRINTERR("setFromStream()", "syntax error: invalid string");
			return false;
		}
		return true;
	case _JS0::NULLVALUE:
		if (stream.get() == 'n' && stream.get() == 'u' && stream.get() == 'l'
				&& stream.get() == 'l') {
			setType(_JS0::NULLVALUE);
		} else {
			PRINTERR("setFromStream()", "syntax error: invalid null value");
			return false;
		}
		return true;
	case _JS0::BOOLEAN: {
		char ch = stream.get();
		if (ch == 't' && stream.get() == 'r' && stream.get() == 'u'
				&& stream.get() == 'e') {
			setType(_JS0::BOOLEAN);
			value.boo = true;
		} else if (ch == 'f' && stream.get() == 'a' && stream.get() == 'l'
				&& stream.get() == 's' && stream.get() == 'e') {
			setType(_JS0::BOOLEAN);
			value.boo = false;
		} else {
			PRINTERR("setFromStream()", "syntax error: invalid boolean value");
			return false;
		}
		return true;
	}
	case _JS0::OBJECT:
		setType(_JS0::OBJECT);
		stream.get();
		while (1) {
			if (!fetchNextType(stream, s_type, isComma, isColon, isEnd)) {
				PRINTERR("setFromStream()", "syntax error: object not closed");
				return false;
			}
			if (isEnd) {
				if (s_type != _JS0::OBJECT) {
					PRINTERR("setFromStream()",
							"syntax error: object not closed");
					return false;
				}
				stream.get();
				return true;
			}
			if (s_type != _JS0::STRING) {
				PRINTERR("setFromStream()",
						"syntax error: object key must be a string");
				return false;
			}
			struct entry *e = new struct entry;
			if (!parseJsonString(stream, e->key)) {
				PRINTERR("setFromStream()",
						"syntax error: invalid string for object key");
				delete e;
				return false;
			}
			if (!fetchNextType(stream, s_type, isComma, isColon, isEnd)
					|| !isColon) {
				PRINTERR("setFromStream()",
						"syntax error: object colon expected");
				delete e;
				return false;
			}
			stream.get();
			e->node = new _JS0();
			e->node->reserve();
			if (!e->node->setFromStream(stream)) {
				e->node->release();
				delete e;
				return false;
			}
			if (!fetchNextType(stream, s_type, isComma, isColon, isEnd)
					|| !(isComma || (isEnd && s_type == _JS0::OBJECT))) {
				PRINTERR("setFromStream()",
						"syntax error: object comma or end expected");
				e->node->release();
				delete e;
				return false;
			}
			stream.get();
			value.obj->push_back(e);
			if (isEnd) {
				return true;
			}
		} // while
		break;
	case _JS0::ARRAY:
		setType(_JS0::ARRAY);
		stream.get();
		while (1) {
			if (!fetchNextType(stream, s_type, isComma, isColon, isEnd)) {
				PRINTERR("setFromStream()", "syntax error: array not closed");
				return false;
			}
			if (isEnd) {
				if (s_type != _JS0::ARRAY) {
					PRINTERR("setFromStream()",
							"syntax error: array not closed");
					return false;
				}
				stream.get();
				return true;
			}
			_JS0 *n = new _JS0();
			n->reserve();
			if (!n->setFromStream(stream)) {
				n->release();
				return false;
			}
			if (!fetchNextType(stream, s_type, isComma, isColon, isEnd)
					|| !(isComma || (isEnd && s_type == _JS0::ARRAY))) {
				PRINTERR("setFromStream()",
						"syntax error: array comma or end expected");
				n->release();
				return false;
			}
			stream.get();
			value.arr->push_back(n);
			if (isEnd) {
				return true;
			}
		} // while
		break;
	} // switch
	return false;
}

bool _JS0::fetchNextType(std::istream &stream, unsigned short &type,
		bool &isComma, bool &isColon, bool &isEnd) {
	isColon = isComma = isEnd = false;
	while (stream.good()) {
		char ch = stream.get();
		if (!stream.good())
			return false;
		switch (ch) {
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			continue;
		case '"':
			type = _JS0::STRING;
			stream.unget();
			return true;
		case '[':
			type = _JS0::ARRAY;
			stream.unget();
			return true;
		case ']':
			type = _JS0::ARRAY;
			stream.unget();
			isEnd = true;
			return true;
		case '{':
			type = _JS0::OBJECT;
			stream.unget();
			return true;
		case '}':
			type = _JS0::OBJECT;
			stream.unget();
			isEnd = true;
			return true;
		case ',':
			isComma = true;
			stream.unget();
			return true;
		case ':':
			stream.unget();
			isColon = true;
			return true;
		case '-':
		case '+':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			type = _JS0::NUMBER_LONG;
			stream.unget();
			return true;
		case 'n':
			type = _JS0::NULLVALUE;
			stream.unget();
			return true;
		case 't':
		case 'f':
			type = _JS0::BOOLEAN;
			stream.unget();
			return true;
		default:
			stream.unget();
			return false;
		}
	}
	return false;
}
