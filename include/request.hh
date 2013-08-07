// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: request.proto

#ifndef PROTOBUF_request_2eproto__INCLUDED
#define PROTOBUF_request_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2004001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
// @@protoc_insertion_point(includes)

namespace mantis {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_request_2eproto();
void protobuf_AssignDesc_request_2eproto();
void protobuf_ShutdownFile_request_2eproto();

class request;

// ===================================================================

class request : public ::google::protobuf::Message {
 public:
  request();
  virtual ~request();
  
  request(const request& from);
  
  inline request& operator=(const request& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const request& default_instance();
  
  void Swap(request* other);
  
  // implements Message ----------------------------------------------
  
  request* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const request& from);
  void MergeFrom(const request& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // required double rate = 1;
  inline bool has_rate() const;
  inline void clear_rate();
  static const int kRateFieldNumber = 1;
  inline double rate() const;
  inline void set_rate(double value);
  
  // required double duration = 2;
  inline bool has_duration() const;
  inline void clear_duration();
  static const int kDurationFieldNumber = 2;
  inline double duration() const;
  inline void set_duration(double value);
  
  // required string file = 3;
  inline bool has_file() const;
  inline void clear_file();
  static const int kFileFieldNumber = 3;
  inline const ::std::string& file() const;
  inline void set_file(const ::std::string& value);
  inline void set_file(const char* value);
  inline void set_file(const char* value, size_t size);
  inline ::std::string* mutable_file();
  inline ::std::string* release_file();
  
  // required string date = 4;
  inline bool has_date() const;
  inline void clear_date();
  static const int kDateFieldNumber = 4;
  inline const ::std::string& date() const;
  inline void set_date(const ::std::string& value);
  inline void set_date(const char* value);
  inline void set_date(const char* value, size_t size);
  inline ::std::string* mutable_date();
  inline ::std::string* release_date();
  
  // required string description = 5;
  inline bool has_description() const;
  inline void clear_description();
  static const int kDescriptionFieldNumber = 5;
  inline const ::std::string& description() const;
  inline void set_description(const ::std::string& value);
  inline void set_description(const char* value);
  inline void set_description(const char* value, size_t size);
  inline ::std::string* mutable_description();
  inline ::std::string* release_description();
  
  // @@protoc_insertion_point(class_scope:mantis.request)
 private:
  inline void set_has_rate();
  inline void clear_has_rate();
  inline void set_has_duration();
  inline void clear_has_duration();
  inline void set_has_file();
  inline void clear_has_file();
  inline void set_has_date();
  inline void clear_has_date();
  inline void set_has_description();
  inline void clear_has_description();
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  double rate_;
  double duration_;
  ::std::string* file_;
  ::std::string* date_;
  ::std::string* description_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(5 + 31) / 32];
  
  friend void  protobuf_AddDesc_request_2eproto();
  friend void protobuf_AssignDesc_request_2eproto();
  friend void protobuf_ShutdownFile_request_2eproto();
  
  void InitAsDefaultInstance();
  static request* default_instance_;
};
// ===================================================================


// ===================================================================

// request

// required double rate = 1;
inline bool request::has_rate() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void request::set_has_rate() {
  _has_bits_[0] |= 0x00000001u;
}
inline void request::clear_has_rate() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void request::clear_rate() {
  rate_ = 0;
  clear_has_rate();
}
inline double request::rate() const {
  return rate_;
}
inline void request::set_rate(double value) {
  set_has_rate();
  rate_ = value;
}

// required double duration = 2;
inline bool request::has_duration() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void request::set_has_duration() {
  _has_bits_[0] |= 0x00000002u;
}
inline void request::clear_has_duration() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void request::clear_duration() {
  duration_ = 0;
  clear_has_duration();
}
inline double request::duration() const {
  return duration_;
}
inline void request::set_duration(double value) {
  set_has_duration();
  duration_ = value;
}

// required string file = 3;
inline bool request::has_file() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void request::set_has_file() {
  _has_bits_[0] |= 0x00000004u;
}
inline void request::clear_has_file() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void request::clear_file() {
  if (file_ != &::google::protobuf::internal::kEmptyString) {
    file_->clear();
  }
  clear_has_file();
}
inline const ::std::string& request::file() const {
  return *file_;
}
inline void request::set_file(const ::std::string& value) {
  set_has_file();
  if (file_ == &::google::protobuf::internal::kEmptyString) {
    file_ = new ::std::string;
  }
  file_->assign(value);
}
inline void request::set_file(const char* value) {
  set_has_file();
  if (file_ == &::google::protobuf::internal::kEmptyString) {
    file_ = new ::std::string;
  }
  file_->assign(value);
}
inline void request::set_file(const char* value, size_t size) {
  set_has_file();
  if (file_ == &::google::protobuf::internal::kEmptyString) {
    file_ = new ::std::string;
  }
  file_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* request::mutable_file() {
  set_has_file();
  if (file_ == &::google::protobuf::internal::kEmptyString) {
    file_ = new ::std::string;
  }
  return file_;
}
inline ::std::string* request::release_file() {
  clear_has_file();
  if (file_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = file_;
    file_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}

// required string date = 4;
inline bool request::has_date() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void request::set_has_date() {
  _has_bits_[0] |= 0x00000008u;
}
inline void request::clear_has_date() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void request::clear_date() {
  if (date_ != &::google::protobuf::internal::kEmptyString) {
    date_->clear();
  }
  clear_has_date();
}
inline const ::std::string& request::date() const {
  return *date_;
}
inline void request::set_date(const ::std::string& value) {
  set_has_date();
  if (date_ == &::google::protobuf::internal::kEmptyString) {
    date_ = new ::std::string;
  }
  date_->assign(value);
}
inline void request::set_date(const char* value) {
  set_has_date();
  if (date_ == &::google::protobuf::internal::kEmptyString) {
    date_ = new ::std::string;
  }
  date_->assign(value);
}
inline void request::set_date(const char* value, size_t size) {
  set_has_date();
  if (date_ == &::google::protobuf::internal::kEmptyString) {
    date_ = new ::std::string;
  }
  date_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* request::mutable_date() {
  set_has_date();
  if (date_ == &::google::protobuf::internal::kEmptyString) {
    date_ = new ::std::string;
  }
  return date_;
}
inline ::std::string* request::release_date() {
  clear_has_date();
  if (date_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = date_;
    date_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}

// required string description = 5;
inline bool request::has_description() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void request::set_has_description() {
  _has_bits_[0] |= 0x00000010u;
}
inline void request::clear_has_description() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void request::clear_description() {
  if (description_ != &::google::protobuf::internal::kEmptyString) {
    description_->clear();
  }
  clear_has_description();
}
inline const ::std::string& request::description() const {
  return *description_;
}
inline void request::set_description(const ::std::string& value) {
  set_has_description();
  if (description_ == &::google::protobuf::internal::kEmptyString) {
    description_ = new ::std::string;
  }
  description_->assign(value);
}
inline void request::set_description(const char* value) {
  set_has_description();
  if (description_ == &::google::protobuf::internal::kEmptyString) {
    description_ = new ::std::string;
  }
  description_->assign(value);
}
inline void request::set_description(const char* value, size_t size) {
  set_has_description();
  if (description_ == &::google::protobuf::internal::kEmptyString) {
    description_ = new ::std::string;
  }
  description_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* request::mutable_description() {
  set_has_description();
  if (description_ == &::google::protobuf::internal::kEmptyString) {
    description_ = new ::std::string;
  }
  return description_;
}
inline ::std::string* request::release_description() {
  clear_has_description();
  if (description_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = description_;
    description_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace mantis

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_request_2eproto__INCLUDED
