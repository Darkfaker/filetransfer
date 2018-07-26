/*
 * cppjson - JSON (de)serialization library for C++ and STL
 *
 * Copyright 2012 Janne Kulmala <janne.t.kulmala@iki.fi>
 *
 * Program code is licensed with GNU LGPL 2.1. See COPYING.LGPL file.
 */
#ifndef __cppjson_h
#define __cppjson_h

#include <string>
#include <map>
#include <vector>
#include <istream>
#include <ostream>
#include <stdint.h>
#include <stdexcept>
#include <assert.h>

namespace json {

class decode_error: public std::runtime_error {
public:
	decode_error(const std::string &what) :
		std::runtime_error(what)
	{}
};

class type_error: public std::runtime_error {
public:
	type_error(const std::string &what) :
		std::runtime_error(what)
	{}
};

/* Must use the same order as type_names[] */
enum Type {
	JSON_NULL,
	JSON_STRING,
	JSON_INTEGER,
	JSON_FLOATING,
	JSON_BOOLEAN,
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_LAZY_ARRAY,
};

struct LazyArray;

class Value;
typedef std::map<std::string, Value> object_map_t;

class Value {
public:
	Value(Type type = JSON_NULL);
	Value(const std::string &s);
	Value(const char *s);
	Value(int i);
	Value(double d);
	Value(bool b);
	Value(const object_map_t &object);
	Value(const std::vector<Value> &array);
	Value(const Value &from);
	~Value();

	Type type() const { return m_type; }

	std::string as_string() const
	{
		verify_type(JSON_STRING);
		return *m_value.string;
	}
	int as_integer() const
	{
		verify_type(JSON_INTEGER);
		if (int(m_value.integer) != m_value.integer) {
			throw type_error("Too large a integer");
		}
		return m_value.integer;
	}
	int64_t as_int64() const
	{
		verify_type(JSON_INTEGER);
		return m_value.integer;
	}
	double as_double() const
	{
		/* treat integers as numbers too */
		if (m_type == JSON_INTEGER)
			return m_value.integer;
		verify_type(JSON_FLOATING);
		return m_value.floating;
	}
	bool as_boolean() const
	{
		verify_type(JSON_BOOLEAN);
		return m_value.boolean;
	}
	const object_map_t &as_object() const
	{
		verify_type(JSON_OBJECT);
		return *m_value.object;
	}
	const std::vector<Value> &as_array() const
	{
		verify_type(JSON_ARRAY);
		return *m_value.array;
	}
	const object_map_t &as_const_object()
	{
		verify_type(JSON_OBJECT);
		return *m_value.object;
	}
	const std::vector<Value> &as_const_array()
	{
		verify_type(JSON_ARRAY);
		return *m_value.array;
	}
	object_map_t &as_object()
	{
		verify_type(JSON_OBJECT);
		return *m_value.object;
	}
	std::vector<Value> &as_array()
	{
		verify_type(JSON_ARRAY);
		return *m_value.array;
	}

	/* Used to iterate lazy-loaded arrays */
	Value load_next(bool *eof = NULL, bool lazy = false);

	const Value &get(const std::string &s) const
	{
		static Value null;
		verify_type(JSON_OBJECT);
		object_map_t::iterator i = m_value.object->find(s);
		if (i == m_value.object->end()) {
			return null;
		}
		return i->second;
	}
	Value &get(const std::string &s)
	{
		static Value null;
		verify_type(JSON_OBJECT);
		object_map_t::iterator i = m_value.object->find(s);
		if (i == m_value.object->end()) {
			return null;
		}
		return i->second;
	}
	void set(const std::string &s, const Value &val)
	{
		verify_type(JSON_OBJECT);
		m_value.object->insert(std::make_pair(s, val));
	}

	void append(const Value &val)
	{
		verify_type(JSON_ARRAY);
		m_value.array->push_back(val);
	}

	void operator = (const Value &from);

	bool operator == (const Value &other) const;
	bool operator != (const Value &other) const;

	void load(std::istream &is, bool lazy = false);
	void load_all(std::istream &is, bool lazy = false);

	void write(std::ostream &os, int indent=0) const;

private:
	Type m_type;
	union {
		std::string *string;
		int64_t integer;
		double floating;
		bool boolean;
		object_map_t *object;
		std::vector<Value> *array;
		LazyArray *lazy;
	} m_value;

	void destroy();

	void verify_type(Type type) const;
};

}

#endif
