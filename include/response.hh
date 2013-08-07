// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: response.proto

#ifndef PROTOBUF_response_2eproto__INCLUDED
#define PROTOBUF_response_2eproto__INCLUDED

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
void  protobuf_AddDesc_response_2eproto();
void protobuf_AssignDesc_response_2eproto();
void protobuf_ShutdownFile_response_2eproto();

class response;

enum response_status_t {
  response_status_t_acknowledged = 0,
  response_status_t_waiting = 1,
  response_status_t_ready = 2,
  response_status_t_running = 3,
  response_status_t_done = 4
};
bool response_status_t_IsValid(int value);
const response_status_t response_status_t_status_t_MIN = response_status_t_acknowledged;
const response_status_t response_status_t_status_t_MAX = response_status_t_done;
const int response_status_t_status_t_ARRAYSIZE = response_status_t_status_t_MAX + 1;

const ::google::protobuf::EnumDescriptor* response_status_t_descriptor();
inline const ::std::string& response_status_t_Name(response_status_t value) {
  return ::google::protobuf::internal::NameOfEnum(
    response_status_t_descriptor(), value);
}
inline bool response_status_t_Parse(
    const ::std::string& name, response_status_t* value) {
  return ::google::protobuf::internal::ParseNamedEnum<response_status_t>(
    response_status_t_descriptor(), name, value);
}
// ===================================================================

class response : public ::google::protobuf::Message {
 public:
  response();
  virtual ~response();
  
  response(const response& from);
  
  inline response& operator=(const response& from) {
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
  static const response& default_instance();
  
  void Swap(response* other);
  
  // implements Message ----------------------------------------------
  
  response* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const response& from);
  void MergeFrom(const response& from);
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
  
  typedef response_status_t status_t;
  static const status_t acknowledged = response_status_t_acknowledged;
  static const status_t waiting = response_status_t_waiting;
  static const status_t ready = response_status_t_ready;
  static const status_t running = response_status_t_running;
  static const status_t done = response_status_t_done;
  static inline bool status_t_IsValid(int value) {
    return response_status_t_IsValid(value);
  }
  static const status_t status_t_MIN =
    response_status_t_status_t_MIN;
  static const status_t status_t_MAX =
    response_status_t_status_t_MAX;
  static const int status_t_ARRAYSIZE =
    response_status_t_status_t_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  status_t_descriptor() {
    return response_status_t_descriptor();
  }
  static inline const ::std::string& status_t_Name(status_t value) {
    return response_status_t_Name(value);
  }
  static inline bool status_t_Parse(const ::std::string& name,
      status_t* value) {
    return response_status_t_Parse(name, value);
  }
  
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
  
  // required .mantis.response.status_t status = 6;
  inline bool has_status() const;
  inline void clear_status();
  static const int kStatusFieldNumber = 6;
  inline ::mantis::response_status_t status() const;
  inline void set_status(::mantis::response_status_t value);
  
  // optional uint32 reader_records = 7;
  inline bool has_reader_records() const;
  inline void clear_reader_records();
  static const int kReaderRecordsFieldNumber = 7;
  inline ::google::protobuf::uint32 reader_records() const;
  inline void set_reader_records(::google::protobuf::uint32 value);
  
  // optional uint32 reader_acquisitions = 8;
  inline bool has_reader_acquisitions() const;
  inline void clear_reader_acquisitions();
  static const int kReaderAcquisitionsFieldNumber = 8;
  inline ::google::protobuf::uint32 reader_acquisitions() const;
  inline void set_reader_acquisitions(::google::protobuf::uint32 value);
  
  // optional double reader_live_time = 9;
  inline bool has_reader_live_time() const;
  inline void clear_reader_live_time();
  static const int kReaderLiveTimeFieldNumber = 9;
  inline double reader_live_time() const;
  inline void set_reader_live_time(double value);
  
  // optional double reader_dead_time = 10;
  inline bool has_reader_dead_time() const;
  inline void clear_reader_dead_time();
  static const int kReaderDeadTimeFieldNumber = 10;
  inline double reader_dead_time() const;
  inline void set_reader_dead_time(double value);
  
  // optional double reader_megabytes = 11;
  inline bool has_reader_megabytes() const;
  inline void clear_reader_megabytes();
  static const int kReaderMegabytesFieldNumber = 11;
  inline double reader_megabytes() const;
  inline void set_reader_megabytes(double value);
  
  // optional double reader_rate = 12;
  inline bool has_reader_rate() const;
  inline void clear_reader_rate();
  static const int kReaderRateFieldNumber = 12;
  inline double reader_rate() const;
  inline void set_reader_rate(double value);
  
  // optional uint32 writer_records = 13;
  inline bool has_writer_records() const;
  inline void clear_writer_records();
  static const int kWriterRecordsFieldNumber = 13;
  inline ::google::protobuf::uint32 writer_records() const;
  inline void set_writer_records(::google::protobuf::uint32 value);
  
  // optional uint32 writer_acquisitions = 14;
  inline bool has_writer_acquisitions() const;
  inline void clear_writer_acquisitions();
  static const int kWriterAcquisitionsFieldNumber = 14;
  inline ::google::protobuf::uint32 writer_acquisitions() const;
  inline void set_writer_acquisitions(::google::protobuf::uint32 value);
  
  // optional double writer_live_time = 15;
  inline bool has_writer_live_time() const;
  inline void clear_writer_live_time();
  static const int kWriterLiveTimeFieldNumber = 15;
  inline double writer_live_time() const;
  inline void set_writer_live_time(double value);
  
  // optional double writer_megabytes = 16;
  inline bool has_writer_megabytes() const;
  inline void clear_writer_megabytes();
  static const int kWriterMegabytesFieldNumber = 16;
  inline double writer_megabytes() const;
  inline void set_writer_megabytes(double value);
  
  // optional double writer_rate = 17;
  inline bool has_writer_rate() const;
  inline void clear_writer_rate();
  static const int kWriterRateFieldNumber = 17;
  inline double writer_rate() const;
  inline void set_writer_rate(double value);
  
  // @@protoc_insertion_point(class_scope:mantis.response)
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
  inline void set_has_status();
  inline void clear_has_status();
  inline void set_has_reader_records();
  inline void clear_has_reader_records();
  inline void set_has_reader_acquisitions();
  inline void clear_has_reader_acquisitions();
  inline void set_has_reader_live_time();
  inline void clear_has_reader_live_time();
  inline void set_has_reader_dead_time();
  inline void clear_has_reader_dead_time();
  inline void set_has_reader_megabytes();
  inline void clear_has_reader_megabytes();
  inline void set_has_reader_rate();
  inline void clear_has_reader_rate();
  inline void set_has_writer_records();
  inline void clear_has_writer_records();
  inline void set_has_writer_acquisitions();
  inline void clear_has_writer_acquisitions();
  inline void set_has_writer_live_time();
  inline void clear_has_writer_live_time();
  inline void set_has_writer_megabytes();
  inline void clear_has_writer_megabytes();
  inline void set_has_writer_rate();
  inline void clear_has_writer_rate();
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  double rate_;
  double duration_;
  ::std::string* file_;
  ::std::string* date_;
  ::std::string* description_;
  int status_;
  ::google::protobuf::uint32 reader_records_;
  double reader_live_time_;
  double reader_dead_time_;
  ::google::protobuf::uint32 reader_acquisitions_;
  ::google::protobuf::uint32 writer_records_;
  double reader_megabytes_;
  double reader_rate_;
  double writer_live_time_;
  double writer_megabytes_;
  double writer_rate_;
  ::google::protobuf::uint32 writer_acquisitions_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(17 + 31) / 32];
  
  friend void  protobuf_AddDesc_response_2eproto();
  friend void protobuf_AssignDesc_response_2eproto();
  friend void protobuf_ShutdownFile_response_2eproto();
  
  void InitAsDefaultInstance();
  static response* default_instance_;
};
// ===================================================================


// ===================================================================

// response

// required double rate = 1;
inline bool response::has_rate() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void response::set_has_rate() {
  _has_bits_[0] |= 0x00000001u;
}
inline void response::clear_has_rate() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void response::clear_rate() {
  rate_ = 0;
  clear_has_rate();
}
inline double response::rate() const {
  return rate_;
}
inline void response::set_rate(double value) {
  set_has_rate();
  rate_ = value;
}

// required double duration = 2;
inline bool response::has_duration() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void response::set_has_duration() {
  _has_bits_[0] |= 0x00000002u;
}
inline void response::clear_has_duration() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void response::clear_duration() {
  duration_ = 0;
  clear_has_duration();
}
inline double response::duration() const {
  return duration_;
}
inline void response::set_duration(double value) {
  set_has_duration();
  duration_ = value;
}

// required string file = 3;
inline bool response::has_file() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void response::set_has_file() {
  _has_bits_[0] |= 0x00000004u;
}
inline void response::clear_has_file() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void response::clear_file() {
  if (file_ != &::google::protobuf::internal::kEmptyString) {
    file_->clear();
  }
  clear_has_file();
}
inline const ::std::string& response::file() const {
  return *file_;
}
inline void response::set_file(const ::std::string& value) {
  set_has_file();
  if (file_ == &::google::protobuf::internal::kEmptyString) {
    file_ = new ::std::string;
  }
  file_->assign(value);
}
inline void response::set_file(const char* value) {
  set_has_file();
  if (file_ == &::google::protobuf::internal::kEmptyString) {
    file_ = new ::std::string;
  }
  file_->assign(value);
}
inline void response::set_file(const char* value, size_t size) {
  set_has_file();
  if (file_ == &::google::protobuf::internal::kEmptyString) {
    file_ = new ::std::string;
  }
  file_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* response::mutable_file() {
  set_has_file();
  if (file_ == &::google::protobuf::internal::kEmptyString) {
    file_ = new ::std::string;
  }
  return file_;
}
inline ::std::string* response::release_file() {
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
inline bool response::has_date() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void response::set_has_date() {
  _has_bits_[0] |= 0x00000008u;
}
inline void response::clear_has_date() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void response::clear_date() {
  if (date_ != &::google::protobuf::internal::kEmptyString) {
    date_->clear();
  }
  clear_has_date();
}
inline const ::std::string& response::date() const {
  return *date_;
}
inline void response::set_date(const ::std::string& value) {
  set_has_date();
  if (date_ == &::google::protobuf::internal::kEmptyString) {
    date_ = new ::std::string;
  }
  date_->assign(value);
}
inline void response::set_date(const char* value) {
  set_has_date();
  if (date_ == &::google::protobuf::internal::kEmptyString) {
    date_ = new ::std::string;
  }
  date_->assign(value);
}
inline void response::set_date(const char* value, size_t size) {
  set_has_date();
  if (date_ == &::google::protobuf::internal::kEmptyString) {
    date_ = new ::std::string;
  }
  date_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* response::mutable_date() {
  set_has_date();
  if (date_ == &::google::protobuf::internal::kEmptyString) {
    date_ = new ::std::string;
  }
  return date_;
}
inline ::std::string* response::release_date() {
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
inline bool response::has_description() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void response::set_has_description() {
  _has_bits_[0] |= 0x00000010u;
}
inline void response::clear_has_description() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void response::clear_description() {
  if (description_ != &::google::protobuf::internal::kEmptyString) {
    description_->clear();
  }
  clear_has_description();
}
inline const ::std::string& response::description() const {
  return *description_;
}
inline void response::set_description(const ::std::string& value) {
  set_has_description();
  if (description_ == &::google::protobuf::internal::kEmptyString) {
    description_ = new ::std::string;
  }
  description_->assign(value);
}
inline void response::set_description(const char* value) {
  set_has_description();
  if (description_ == &::google::protobuf::internal::kEmptyString) {
    description_ = new ::std::string;
  }
  description_->assign(value);
}
inline void response::set_description(const char* value, size_t size) {
  set_has_description();
  if (description_ == &::google::protobuf::internal::kEmptyString) {
    description_ = new ::std::string;
  }
  description_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* response::mutable_description() {
  set_has_description();
  if (description_ == &::google::protobuf::internal::kEmptyString) {
    description_ = new ::std::string;
  }
  return description_;
}
inline ::std::string* response::release_description() {
  clear_has_description();
  if (description_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = description_;
    description_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}

// required .mantis.response.status_t status = 6;
inline bool response::has_status() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void response::set_has_status() {
  _has_bits_[0] |= 0x00000020u;
}
inline void response::clear_has_status() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void response::clear_status() {
  status_ = 0;
  clear_has_status();
}
inline ::mantis::response_status_t response::status() const {
  return static_cast< ::mantis::response_status_t >(status_);
}
inline void response::set_status(::mantis::response_status_t value) {
  GOOGLE_DCHECK(::mantis::response_status_t_IsValid(value));
  set_has_status();
  status_ = value;
}

// optional uint32 reader_records = 7;
inline bool response::has_reader_records() const {
  return (_has_bits_[0] & 0x00000040u) != 0;
}
inline void response::set_has_reader_records() {
  _has_bits_[0] |= 0x00000040u;
}
inline void response::clear_has_reader_records() {
  _has_bits_[0] &= ~0x00000040u;
}
inline void response::clear_reader_records() {
  reader_records_ = 0u;
  clear_has_reader_records();
}
inline ::google::protobuf::uint32 response::reader_records() const {
  return reader_records_;
}
inline void response::set_reader_records(::google::protobuf::uint32 value) {
  set_has_reader_records();
  reader_records_ = value;
}

// optional uint32 reader_acquisitions = 8;
inline bool response::has_reader_acquisitions() const {
  return (_has_bits_[0] & 0x00000080u) != 0;
}
inline void response::set_has_reader_acquisitions() {
  _has_bits_[0] |= 0x00000080u;
}
inline void response::clear_has_reader_acquisitions() {
  _has_bits_[0] &= ~0x00000080u;
}
inline void response::clear_reader_acquisitions() {
  reader_acquisitions_ = 0u;
  clear_has_reader_acquisitions();
}
inline ::google::protobuf::uint32 response::reader_acquisitions() const {
  return reader_acquisitions_;
}
inline void response::set_reader_acquisitions(::google::protobuf::uint32 value) {
  set_has_reader_acquisitions();
  reader_acquisitions_ = value;
}

// optional double reader_live_time = 9;
inline bool response::has_reader_live_time() const {
  return (_has_bits_[0] & 0x00000100u) != 0;
}
inline void response::set_has_reader_live_time() {
  _has_bits_[0] |= 0x00000100u;
}
inline void response::clear_has_reader_live_time() {
  _has_bits_[0] &= ~0x00000100u;
}
inline void response::clear_reader_live_time() {
  reader_live_time_ = 0;
  clear_has_reader_live_time();
}
inline double response::reader_live_time() const {
  return reader_live_time_;
}
inline void response::set_reader_live_time(double value) {
  set_has_reader_live_time();
  reader_live_time_ = value;
}

// optional double reader_dead_time = 10;
inline bool response::has_reader_dead_time() const {
  return (_has_bits_[0] & 0x00000200u) != 0;
}
inline void response::set_has_reader_dead_time() {
  _has_bits_[0] |= 0x00000200u;
}
inline void response::clear_has_reader_dead_time() {
  _has_bits_[0] &= ~0x00000200u;
}
inline void response::clear_reader_dead_time() {
  reader_dead_time_ = 0;
  clear_has_reader_dead_time();
}
inline double response::reader_dead_time() const {
  return reader_dead_time_;
}
inline void response::set_reader_dead_time(double value) {
  set_has_reader_dead_time();
  reader_dead_time_ = value;
}

// optional double reader_megabytes = 11;
inline bool response::has_reader_megabytes() const {
  return (_has_bits_[0] & 0x00000400u) != 0;
}
inline void response::set_has_reader_megabytes() {
  _has_bits_[0] |= 0x00000400u;
}
inline void response::clear_has_reader_megabytes() {
  _has_bits_[0] &= ~0x00000400u;
}
inline void response::clear_reader_megabytes() {
  reader_megabytes_ = 0;
  clear_has_reader_megabytes();
}
inline double response::reader_megabytes() const {
  return reader_megabytes_;
}
inline void response::set_reader_megabytes(double value) {
  set_has_reader_megabytes();
  reader_megabytes_ = value;
}

// optional double reader_rate = 12;
inline bool response::has_reader_rate() const {
  return (_has_bits_[0] & 0x00000800u) != 0;
}
inline void response::set_has_reader_rate() {
  _has_bits_[0] |= 0x00000800u;
}
inline void response::clear_has_reader_rate() {
  _has_bits_[0] &= ~0x00000800u;
}
inline void response::clear_reader_rate() {
  reader_rate_ = 0;
  clear_has_reader_rate();
}
inline double response::reader_rate() const {
  return reader_rate_;
}
inline void response::set_reader_rate(double value) {
  set_has_reader_rate();
  reader_rate_ = value;
}

// optional uint32 writer_records = 13;
inline bool response::has_writer_records() const {
  return (_has_bits_[0] & 0x00001000u) != 0;
}
inline void response::set_has_writer_records() {
  _has_bits_[0] |= 0x00001000u;
}
inline void response::clear_has_writer_records() {
  _has_bits_[0] &= ~0x00001000u;
}
inline void response::clear_writer_records() {
  writer_records_ = 0u;
  clear_has_writer_records();
}
inline ::google::protobuf::uint32 response::writer_records() const {
  return writer_records_;
}
inline void response::set_writer_records(::google::protobuf::uint32 value) {
  set_has_writer_records();
  writer_records_ = value;
}

// optional uint32 writer_acquisitions = 14;
inline bool response::has_writer_acquisitions() const {
  return (_has_bits_[0] & 0x00002000u) != 0;
}
inline void response::set_has_writer_acquisitions() {
  _has_bits_[0] |= 0x00002000u;
}
inline void response::clear_has_writer_acquisitions() {
  _has_bits_[0] &= ~0x00002000u;
}
inline void response::clear_writer_acquisitions() {
  writer_acquisitions_ = 0u;
  clear_has_writer_acquisitions();
}
inline ::google::protobuf::uint32 response::writer_acquisitions() const {
  return writer_acquisitions_;
}
inline void response::set_writer_acquisitions(::google::protobuf::uint32 value) {
  set_has_writer_acquisitions();
  writer_acquisitions_ = value;
}

// optional double writer_live_time = 15;
inline bool response::has_writer_live_time() const {
  return (_has_bits_[0] & 0x00004000u) != 0;
}
inline void response::set_has_writer_live_time() {
  _has_bits_[0] |= 0x00004000u;
}
inline void response::clear_has_writer_live_time() {
  _has_bits_[0] &= ~0x00004000u;
}
inline void response::clear_writer_live_time() {
  writer_live_time_ = 0;
  clear_has_writer_live_time();
}
inline double response::writer_live_time() const {
  return writer_live_time_;
}
inline void response::set_writer_live_time(double value) {
  set_has_writer_live_time();
  writer_live_time_ = value;
}

// optional double writer_megabytes = 16;
inline bool response::has_writer_megabytes() const {
  return (_has_bits_[0] & 0x00008000u) != 0;
}
inline void response::set_has_writer_megabytes() {
  _has_bits_[0] |= 0x00008000u;
}
inline void response::clear_has_writer_megabytes() {
  _has_bits_[0] &= ~0x00008000u;
}
inline void response::clear_writer_megabytes() {
  writer_megabytes_ = 0;
  clear_has_writer_megabytes();
}
inline double response::writer_megabytes() const {
  return writer_megabytes_;
}
inline void response::set_writer_megabytes(double value) {
  set_has_writer_megabytes();
  writer_megabytes_ = value;
}

// optional double writer_rate = 17;
inline bool response::has_writer_rate() const {
  return (_has_bits_[0] & 0x00010000u) != 0;
}
inline void response::set_has_writer_rate() {
  _has_bits_[0] |= 0x00010000u;
}
inline void response::clear_has_writer_rate() {
  _has_bits_[0] &= ~0x00010000u;
}
inline void response::clear_writer_rate() {
  writer_rate_ = 0;
  clear_has_writer_rate();
}
inline double response::writer_rate() const {
  return writer_rate_;
}
inline void response::set_writer_rate(double value) {
  set_has_writer_rate();
  writer_rate_ = value;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace mantis

#ifndef SWIG
namespace google {
namespace protobuf {

template <>
inline const EnumDescriptor* GetEnumDescriptor< ::mantis::response_status_t>() {
  return ::mantis::response_status_t_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_response_2eproto__INCLUDED
