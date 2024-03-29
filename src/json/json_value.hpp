#pragma once

#include <typeinfo>
#include <string>
#include <vector>
#include <map>

#include "json_assert.hpp"

namespace pson {

// Json type
enum JSON_TYPE { JSON_NULL, JSON_BOOL, JSON_NUMBER, JSON_STRING, JSON_ARRAY, JSON_OBJECT };

struct null_type {};

class Value;
class Array;
class Object;

using Null = null_type;
using Bool = bool;
using Number = double;
using String = std::string;
using ArrayContainer = std::vector<Value>;
using ObjectContainer = std::map<std::string, Value>;

// some util...
template <typename T, typename K>
struct is_same {
  enum { value = false };
};

template <typename T>
struct is_same<T, T> {
  enum { value = true };
};

template <typename T, typename... Args>
struct TypesCmp {
  enum { value = false };
};

template <typename T, typename U, typename... Args>
struct TypesCmp<T, U, Args...> {
  enum { value = false || TypesCmp<T, Args...>::value };
};

template <typename T, typename... Args>
struct TypesCmp<T, T, Args...> {
  enum { value = true || TypesCmp<T, Args...>::value };
};

#define IS_JSON_TYPE(type)                                                                                             \
  (std::is_trivial<type>::value ||                                                                                     \
      pson::                                                                                                           \
          TypesCmp<type, pson::Null, pson::Bool, pson::Number, pson::String, pson::Array, pson::Object, pson::Value>:: \
              value)

class Object {
private:
  ObjectContainer value_map_;

public:
  Object() = default;
  ~Object() = default;

  Object(const Object &o);

  Object(Object &&o);

  Object &operator=(const Object &o);
  Object &operator=(Object &&o);

  template <typename T>
  void insert(std::string name, T t);

  // You could use move to zero over head,
  // eg. obj.insert(std::move(name), std::move(val));
  // Copy eg. obj.insert(name, val);
  void insert(std::string name, Value v);
  void erase(const std::string &name);
  Value &operator[](const std::string &name);
  Value &at(const std::string &name);

  bool empty() const;
  bool has(const std::string &name) const;
  std::size_t size() const;

  ObjectContainer &value_map();
};

class Array {
private:
  ArrayContainer values_;

public:
  Array() = default;
  ~Array() = default;

  Array(const Array &a);
  Array(Array &&a);

  Array &operator=(const Array &a);
  Array &operator=(Array &&a);

  Value &operator[](int p);
  void push_back(Value v);
  void pop_back();
  Value &back();
  Value &at(int i);

  bool has(int i) const;
  std::size_t size() const;
  bool empty() const;

  ArrayContainer &values();
};

class Value {
private:
  JSON_TYPE type_;
  union holder {
    Null null_;
    Number number_;
    Bool bool_;
    String *string_;
    Array *array_;
    Object *object_;
  } val_;

  inline bool judge_type(const JSON_TYPE &t) const
  {
    return type_ == t;
  }

  void init_base(const JSON_TYPE &t);

  inline void swap_value(Value &v)
  {
    std::swap(this->val_, v.val_);
    std::swap(this->type_, v.type_);
  }

  void copy_value(const Value &v);

public:
  Value();
  Value(bool b);
  Value(const char *s);
  Value(String s);
  Value(Number n);
  Value(int32_t n);
  Value(uint32_t n);
  Value(int64_t n);
  Value(uint64_t n);

  Value(const Array &array);
  Value(const Object &obj);

  Value(const JSON_TYPE &);

  // Copy
  Value(const Value &);

  // Move
  Value(Value &&);

  ~Value();

  // Copy
  Value &operator=(const Value &);

  // Move
  Value &operator=(Value &&);

  void reset();

  inline void reset_as(JSON_TYPE t)
  {
    reset();
    init_base(t);
  }

  template <typename T>
  void reset_as();

  inline void swap(Value &v)
  {
    swap_value(v);
  }

  inline JSON_TYPE type() const
  {
    return type_;
  }

  inline bool is_null() const
  {
    return judge_type(JSON_NULL);
  }
  inline bool is_bool() const
  {
    return judge_type(JSON_BOOL);
  }
  inline bool is_number() const
  {
    return judge_type(JSON_NUMBER);
  }
  inline bool is_string() const
  {
    return judge_type(JSON_STRING);
  }
  inline bool is_array() const
  {
    return judge_type(JSON_ARRAY);
  }
  inline bool is_object() const
  {
    return judge_type(JSON_OBJECT);
  }

  template <typename T>
  bool is();

  inline Null as_null()
  {
    PSON_ASSERT(judge_type(JSON_NULL));
    return val_.null_;
  }

  inline Number &as_number()
  {
    PSON_ASSERT(judge_type(JSON_NUMBER));
    return val_.number_;
  }

  inline Bool &as_bool()
  {
    PSON_ASSERT(judge_type(JSON_BOOL));
    return val_.bool_;
  }

  inline String &as_string()
  {
    PSON_ASSERT(judge_type(JSON_STRING));
    return *val_.string_;
  }

  inline Array &as_array()
  {
    PSON_ASSERT(judge_type(JSON_ARRAY));
    return *val_.array_;
  }

  inline Object &as_object()
  {
    PSON_ASSERT(judge_type(JSON_OBJECT));
    return *val_.object_;
  }

  template <typename T>
  T &as();

  bool empty() const;
  std::size_t size() const;  // For array, object

  // operator for Array
  void push_back(Value v);
  void pop_back();
  Value &back();
  bool has(int i) const;
  Value &operator[](int i);
  Value &at(int i);

  // operator for Object
  void insert(std::string name, Value v);
  bool has(const std::string &name) const;
  Value &operator[](const std::string &name);
  Value &at(const std::string &name);
  void erase(const std::string &name);

  bool parse(const std::string &json_data);
  bool parse(char *json_data, std::size_t len);

  std::string print();
};

// Implement
// Object
inline Object::Object(const Object &o) : value_map_(o.value_map_)
{}

inline Object::Object(Object &&o) : value_map_(std::move(o.value_map_))
{}

inline Object &Object::operator=(const Object &o)
{
  value_map_ = o.value_map_;
  return *this;
}

inline Object &Object::operator=(Object &&o)
{
  value_map_ = std::move(o.value_map_);
  return *this;
}

inline Value &Object::operator[](const std::string &name)
{
  return value_map_.at(name);
}

inline bool Object::empty() const
{
  return value_map_.empty();
}

inline Value &Object::at(const std::string &name)
{
  return value_map_.at(name);
}

inline std::size_t Object::size() const
{
  return value_map_.size();
}

inline ObjectContainer &Object::value_map()
{
  return value_map_;
}

inline bool Object::has(const std::string &name) const
{
  return value_map_.find(name) != value_map_.end();
}

template <typename T>
inline void Object::insert(std::string name, T t)
{
  static_assert(IS_JSON_TYPE(T), "Object::insert(std::string, T) tparam not a json type");
  pson::Value v(t);
  insert(std::move(name), std::move(v));
}

inline void Object::insert(std::string name, Value v)
{
  value_map_.emplace(std::move(name), std::move(v));
}

inline void Object::erase(const std::string &name)
{
  value_map_.erase(name);
}

// Array

inline Array::Array(const Array &a) : values_(a.values_)
{}

inline Array::Array(Array &&a) : values_(std::move(a.values_))
{}

inline Array &Array::operator=(const Array &a)
{
  values_ = a.values_;
  return *this;
}

inline Array &Array::operator=(Array &&a)
{
  values_ = std::move(a.values_);
  return *this;
}

inline bool Array::empty() const
{
  return values_.empty();
}

inline ArrayContainer &Array::values()
{
  return values_;
}

inline Value &Array::back()
{
  return values_.back();
}

inline Value &Array::at(int i)
{
  return values_[i];
}

inline bool Array::has(int i) const
{
  return i >= 0 && values_.size() > i;
}

inline std::size_t Array::size() const
{
  return values_.size();
}

inline void Array::push_back(Value v)
{
  values_.emplace_back(std::move(v));
}

inline void Array::pop_back()
{
  values_.pop_back();
}

inline Value &Array::operator[](int p)
{
  return values_[p];
}

inline Value &Value::back()
{
  PSON_ASSERT(judge_type(JSON_ARRAY));
  return val_.array_->back();
}

inline void Value::pop_back()
{
  PSON_ASSERT(judge_type(JSON_ARRAY));
  val_.array_->pop_back();
}

// Value
inline Value::Value() : type_(JSON_NULL)
{
  val_.null_ = Null{};
}

inline Value::Value(bool b) : type_(JSON_BOOL)
{
  val_.bool_ = b;
}

inline Value::Value(Number n) : type_(JSON_NUMBER)
{
  val_.number_ = n;
}

inline Value::Value(int32_t n) : type_(JSON_NUMBER)
{
  val_.number_ = static_cast<Number>(n);
}

inline Value::Value(uint32_t n) : type_(JSON_NUMBER)
{
  val_.number_ = static_cast<Number>(n);
}

inline Value::Value(int64_t n) : type_(JSON_NUMBER)
{
  val_.number_ = static_cast<Number>(n);
}

inline Value::Value(uint64_t n) : type_(JSON_NUMBER)
{
  val_.number_ = static_cast<Number>(n);
}

inline Value::Value(const Array &a) : type_(JSON_ARRAY)
{
  val_.array_ = new Array(a);
}

inline Value::Value(const Object &o) : type_(JSON_OBJECT)
{
  val_.object_ = new Object(o);
}

inline Value::Value(const char *s) : type_(JSON_STRING)
{
  val_.string_ = new String(s);
}

inline Value::Value(String s) : type_(JSON_STRING)
{
  val_.string_ = new String(std::move(s));
}

inline Value::Value(const JSON_TYPE &t)
{
  init_base(t);
}

inline Value::Value(const Value &v) : type_(JSON_NULL)
{
  copy_value(v);
}

inline Value::Value(Value &&other)
{
  init_base(JSON_NULL);
  swap_value(other);
}

inline Value &Value::operator=(const Value &v)
{
  copy_value(v);
  return *this;
}

inline Value &Value::operator=(Value &&v)
{
  reset();
  swap_value(v);
  return *this;
}

inline Value::~Value()
{
  switch (type_) {
    case JSON_STRING:
      delete val_.string_;
      break;
    case JSON_ARRAY:
      delete val_.array_;
      break;
    case JSON_OBJECT:
      delete val_.object_;
      break;
    default:
      break;
  }
}

inline void Value::reset()
{
  switch (type_) {
    case JSON_STRING:
      delete val_.string_;
      break;
    case JSON_ARRAY:
      delete val_.array_;
      break;
    case JSON_OBJECT:
      delete val_.object_;
      break;
    default:
      break ;
  }
  type_ = JSON_NULL;
  val_.null_ = Null{};
}

inline void Value::init_base(const JSON_TYPE &t)
{
  type_ = t;
  switch (type_) {
    case JSON_NULL:
      val_.null_ = Null{};
      break;
    case JSON_BOOL:
      val_.bool_ = false;
      break;
    case JSON_NUMBER:
      val_.number_ = 0.0;
      break;
    case JSON_STRING:
      val_.string_ = new String();
      break;
    case JSON_ARRAY:
      val_.array_ = new Array();
      break;
    case JSON_OBJECT:
      val_.object_ = new Object();
      break;
  }
}

inline void Value::copy_value(const Value &v)
{
  if (type_ == v.type_) {
    switch (v.type_) {
      case JSON_NULL:
        val_.null_ = Null{};
        break;
      case JSON_BOOL:
        val_.bool_ = v.val_.bool_;
        break;
      case JSON_NUMBER:
        val_.number_ = v.val_.number_;
        break;
      case JSON_STRING:
        *val_.string_ = *v.val_.string_;
        break;
      case JSON_ARRAY:
        *val_.array_ = *v.val_.array_;
        break;
      case JSON_OBJECT:
        *val_.object_ = *v.val_.object_;
        break;
    }
  } else {
    switch (v.type_) {
      case JSON_NULL:
        val_.null_ = Null{};
        break;
      case JSON_BOOL:
        val_.bool_ = v.val_.bool_;
        break;
      case JSON_NUMBER:
        val_.number_ = v.val_.number_;
        break;
      case JSON_STRING:
        val_.string_ = new String(*v.val_.string_);
        break;
      case JSON_ARRAY:
        val_.array_ = new Array(*v.val_.array_);
        break;
      case JSON_OBJECT:
        val_.object_ = new Object(*v.val_.object_);
        break;
    }
    type_ = v.type_;
  }
}

// void Value::reset_as<T>()
template <>
inline void Value::reset_as<Null>()
{
  reset_as(JSON_NULL);
}

template <>
inline void Value::reset_as<Bool>()
{
  reset_as(JSON_BOOL);
}

template <>
inline void Value::reset_as<Number>()
{
  reset_as(JSON_NUMBER);
}

template <>
inline void Value::reset_as<String>()
{
  reset_as(JSON_STRING);
}

template <>
inline void Value::reset_as<Array>()
{
  reset_as(JSON_ARRAY);
}

template <>
inline void Value::reset_as<Object>()
{
  reset_as(JSON_OBJECT);
}

// bool Value::is<T>()
template <>
inline bool Value::is<Null>()
{
  return judge_type(JSON_NULL);
}

template <>
inline bool Value::is<Bool>()
{
  return judge_type(JSON_BOOL);
}

template <>
inline bool Value::is<Number>()
{
  return judge_type(JSON_NUMBER);
}

template <>
inline bool Value::is<String>()
{
  return judge_type(JSON_STRING);
}

template <>
inline bool Value::is<Array>()
{
  return judge_type(JSON_ARRAY);
}

template <>
inline bool Value::is<Object>()
{
  return judge_type(JSON_OBJECT);
}

// T& Value::as<T>()
template <>
inline Value &Value::as<Value>()
{
  return *this;
}

template <>
inline Null &Value::as<Null>()
{
  PSON_ASSERT(judge_type(JSON_NULL));
  return val_.null_;
}

template <>
inline Bool &Value::as<Bool>()
{
  PSON_ASSERT(judge_type(JSON_BOOL));
  return val_.bool_;
}

template <>
inline Number &Value::as<Number>()
{
  PSON_ASSERT(judge_type(JSON_NUMBER));
  return val_.number_;
}

template <>
inline String &Value::as<String>()
{
  PSON_ASSERT(judge_type(JSON_STRING));
  return *val_.string_;
}

template <>
inline Array &Value::as<Array>()
{
  PSON_ASSERT(judge_type(JSON_ARRAY));
  return *val_.array_;
}

template <>
inline Object &Value::as<Object>()
{
  PSON_ASSERT(judge_type(JSON_OBJECT));
  return *val_.object_;
}

inline bool Value::empty() const
{
  switch (type_) {
    case JSON_NULL:
      return true;
    case JSON_ARRAY:
      return val_.array_->empty();
    case JSON_OBJECT:
      return val_.object_->empty();
    default:
      PSON_ASSERT(false);
  }
  return false;  // Make happy
}

inline std::size_t Value::size() const
{
  switch (type_) {
    case JSON_STRING:
      return val_.string_->size();
    case JSON_ARRAY:
      return val_.array_->size();
    case JSON_OBJECT:
      return val_.object_->size();
    default:
      PSON_ASSERT(true);
  }
  return 0;
}

inline void Value::push_back(Value v)
{
  PSON_ASSERT(judge_type(JSON_ARRAY));
  val_.array_->push_back(std::move(v));
}

inline bool Value::has(int i) const
{
  PSON_ASSERT(judge_type(JSON_BOOL));
  return val_.array_->has(i);
}

inline Value &Value::operator[](int i)
{
  PSON_ASSERT(judge_type(JSON_ARRAY));
  return val_.array_->at(i);
}

inline Value &Value::at(int i)
{
  PSON_ASSERT(judge_type(JSON_ARRAY));
  return val_.array_->at(i);
}

inline void Value::insert(std::string name, Value v)
{
  PSON_ASSERT(judge_type(JSON_OBJECT));
  return val_.object_->insert(std::move(name), std::move(v));
}

inline bool Value::has(const std::string &name) const
{
  PSON_ASSERT(judge_type(JSON_OBJECT));
  return val_.object_->has(name);
}

inline Value &Value::operator[](const std::string &name)
{
  PSON_ASSERT(judge_type(JSON_OBJECT));
  return val_.object_->at(name);
}

inline Value &Value::at(const std::string &name)
{
  PSON_ASSERT(judge_type(JSON_OBJECT));
  return val_.object_->at(name);
}

inline void Value::erase(const std::string &name)
{
  PSON_ASSERT(judge_type(JSON_OBJECT));
  val_.object_->erase(name);
}

}  // namespace pson
