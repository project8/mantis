// Generated by the protocol buffer compiler.  DO NOT EDIT!

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "response.hh"

#include <algorithm>

#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace mantis {

namespace {

const ::google::protobuf::Descriptor* response_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  response_reflection_ = NULL;
const ::google::protobuf::EnumDescriptor* response_status_t_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_response_2eproto() {
  protobuf_AddDesc_response_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "response.proto");
  GOOGLE_CHECK(file != NULL);
  response_descriptor_ = file->message_type(0);
  static const int response_offsets_[17] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, rate_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, duration_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, file_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, date_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, description_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, status_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, reader_records_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, reader_acquisitions_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, reader_live_time_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, reader_dead_time_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, reader_megabytes_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, reader_rate_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, writer_records_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, writer_acquisitions_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, writer_live_time_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, writer_megabytes_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, writer_rate_),
  };
  response_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      response_descriptor_,
      response::default_instance_,
      response_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(response, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(response));
  response_status_t_descriptor_ = response_descriptor_->enum_type(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_response_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    response_descriptor_, &response::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_response_2eproto() {
  delete response::default_instance_;
  delete response_reflection_;
}

void protobuf_AddDesc_response_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\016response.proto\022\006mantis\"\351\003\n\010response\022\014\n"
    "\004rate\030\001 \002(\001\022\020\n\010duration\030\002 \002(\001\022\014\n\004file\030\003 "
    "\002(\t\022\014\n\004date\030\004 \002(\t\022\023\n\013description\030\005 \002(\t\022)"
    "\n\006status\030\006 \002(\0162\031.mantis.response.status_"
    "t\022\026\n\016reader_records\030\007 \001(\r\022\033\n\023reader_acqu"
    "isitions\030\010 \001(\r\022\030\n\020reader_live_time\030\t \001(\001"
    "\022\030\n\020reader_dead_time\030\n \001(\001\022\030\n\020reader_meg"
    "abytes\030\013 \001(\001\022\023\n\013reader_rate\030\014 \001(\001\022\026\n\016wri"
    "ter_records\030\r \001(\r\022\033\n\023writer_acquisitions"
    "\030\016 \001(\r\022\030\n\020writer_live_time\030\017 \001(\001\022\030\n\020writ"
    "er_megabytes\030\020 \001(\001\022\023\n\013writer_rate\030\021 \001(\001\""
    "K\n\010status_t\022\020\n\014acknowledged\020\000\022\013\n\007waiting"
    "\020\001\022\t\n\005ready\020\002\022\013\n\007running\020\003\022\010\n\004done\020\004", 516);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "response.proto", &protobuf_RegisterTypes);
  response::default_instance_ = new response();
  response::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_response_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_response_2eproto {
  StaticDescriptorInitializer_response_2eproto() {
    protobuf_AddDesc_response_2eproto();
  }
} static_descriptor_initializer_response_2eproto_;


// ===================================================================

const ::google::protobuf::EnumDescriptor* response_status_t_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return response_status_t_descriptor_;
}
bool response_status_t_IsValid(int value) {
  switch(value) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      return true;
    default:
      return false;
  }
}

#ifndef _MSC_VER
const response_status_t response::acknowledged;
const response_status_t response::waiting;
const response_status_t response::ready;
const response_status_t response::running;
const response_status_t response::done;
const response_status_t response::status_t_MIN;
const response_status_t response::status_t_MAX;
const int response::status_t_ARRAYSIZE;
#endif  // _MSC_VER
#ifndef _MSC_VER
const int response::kRateFieldNumber;
const int response::kDurationFieldNumber;
const int response::kFileFieldNumber;
const int response::kDateFieldNumber;
const int response::kDescriptionFieldNumber;
const int response::kStatusFieldNumber;
const int response::kReaderRecordsFieldNumber;
const int response::kReaderAcquisitionsFieldNumber;
const int response::kReaderLiveTimeFieldNumber;
const int response::kReaderDeadTimeFieldNumber;
const int response::kReaderMegabytesFieldNumber;
const int response::kReaderRateFieldNumber;
const int response::kWriterRecordsFieldNumber;
const int response::kWriterAcquisitionsFieldNumber;
const int response::kWriterLiveTimeFieldNumber;
const int response::kWriterMegabytesFieldNumber;
const int response::kWriterRateFieldNumber;
#endif  // !_MSC_VER

response::response()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void response::InitAsDefaultInstance() {
}

response::response(const response& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void response::SharedCtor() {
  _cached_size_ = 0;
  rate_ = 0;
  duration_ = 0;
  file_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  date_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  description_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  status_ = 0;
  reader_records_ = 0u;
  reader_acquisitions_ = 0u;
  reader_live_time_ = 0;
  reader_dead_time_ = 0;
  reader_megabytes_ = 0;
  reader_rate_ = 0;
  writer_records_ = 0u;
  writer_acquisitions_ = 0u;
  writer_live_time_ = 0;
  writer_megabytes_ = 0;
  writer_rate_ = 0;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

response::~response() {
  SharedDtor();
}

void response::SharedDtor() {
  if (file_ != &::google::protobuf::internal::kEmptyString) {
    delete file_;
  }
  if (date_ != &::google::protobuf::internal::kEmptyString) {
    delete date_;
  }
  if (description_ != &::google::protobuf::internal::kEmptyString) {
    delete description_;
  }
  if (this != default_instance_) {
  }
}

void response::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* response::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return response_descriptor_;
}

const response& response::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_response_2eproto();  return *default_instance_;
}

response* response::default_instance_ = NULL;

response* response::New() const {
  return new response;
}

void response::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    rate_ = 0;
    duration_ = 0;
    if (has_file()) {
      if (file_ != &::google::protobuf::internal::kEmptyString) {
        file_->clear();
      }
    }
    if (has_date()) {
      if (date_ != &::google::protobuf::internal::kEmptyString) {
        date_->clear();
      }
    }
    if (has_description()) {
      if (description_ != &::google::protobuf::internal::kEmptyString) {
        description_->clear();
      }
    }
    status_ = 0;
    reader_records_ = 0u;
    reader_acquisitions_ = 0u;
  }
  if (_has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    reader_live_time_ = 0;
    reader_dead_time_ = 0;
    reader_megabytes_ = 0;
    reader_rate_ = 0;
    writer_records_ = 0u;
    writer_acquisitions_ = 0u;
    writer_live_time_ = 0;
    writer_megabytes_ = 0;
  }
  if (_has_bits_[16 / 32] & (0xffu << (16 % 32))) {
    writer_rate_ = 0;
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool response::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required double rate = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &rate_)));
          set_has_rate();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(17)) goto parse_duration;
        break;
      }
      
      // required double duration = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64) {
         parse_duration:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &duration_)));
          set_has_duration();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(26)) goto parse_file;
        break;
      }
      
      // required string file = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_file:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_file()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->file().data(), this->file().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(34)) goto parse_date;
        break;
      }
      
      // required string date = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_date:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_date()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->date().data(), this->date().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(42)) goto parse_description;
        break;
      }
      
      // required string description = 5;
      case 5: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_description:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_description()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->description().data(), this->description().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(48)) goto parse_status;
        break;
      }
      
      // required .mantis.response.status_t status = 6;
      case 6: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_status:
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::mantis::response_status_t_IsValid(value)) {
            set_status(static_cast< ::mantis::response_status_t >(value));
          } else {
            mutable_unknown_fields()->AddVarint(6, value);
          }
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(56)) goto parse_reader_records;
        break;
      }
      
      // optional uint32 reader_records = 7;
      case 7: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_reader_records:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &reader_records_)));
          set_has_reader_records();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(64)) goto parse_reader_acquisitions;
        break;
      }
      
      // optional uint32 reader_acquisitions = 8;
      case 8: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_reader_acquisitions:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &reader_acquisitions_)));
          set_has_reader_acquisitions();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(73)) goto parse_reader_live_time;
        break;
      }
      
      // optional double reader_live_time = 9;
      case 9: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64) {
         parse_reader_live_time:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &reader_live_time_)));
          set_has_reader_live_time();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(81)) goto parse_reader_dead_time;
        break;
      }
      
      // optional double reader_dead_time = 10;
      case 10: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64) {
         parse_reader_dead_time:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &reader_dead_time_)));
          set_has_reader_dead_time();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(89)) goto parse_reader_megabytes;
        break;
      }
      
      // optional double reader_megabytes = 11;
      case 11: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64) {
         parse_reader_megabytes:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &reader_megabytes_)));
          set_has_reader_megabytes();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(97)) goto parse_reader_rate;
        break;
      }
      
      // optional double reader_rate = 12;
      case 12: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64) {
         parse_reader_rate:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &reader_rate_)));
          set_has_reader_rate();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(104)) goto parse_writer_records;
        break;
      }
      
      // optional uint32 writer_records = 13;
      case 13: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_writer_records:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &writer_records_)));
          set_has_writer_records();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(112)) goto parse_writer_acquisitions;
        break;
      }
      
      // optional uint32 writer_acquisitions = 14;
      case 14: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_writer_acquisitions:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &writer_acquisitions_)));
          set_has_writer_acquisitions();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(121)) goto parse_writer_live_time;
        break;
      }
      
      // optional double writer_live_time = 15;
      case 15: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64) {
         parse_writer_live_time:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &writer_live_time_)));
          set_has_writer_live_time();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(129)) goto parse_writer_megabytes;
        break;
      }
      
      // optional double writer_megabytes = 16;
      case 16: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64) {
         parse_writer_megabytes:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &writer_megabytes_)));
          set_has_writer_megabytes();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(137)) goto parse_writer_rate;
        break;
      }
      
      // optional double writer_rate = 17;
      case 17: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64) {
         parse_writer_rate:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &writer_rate_)));
          set_has_writer_rate();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }
      
      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void response::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // required double rate = 1;
  if (has_rate()) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(1, this->rate(), output);
  }
  
  // required double duration = 2;
  if (has_duration()) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(2, this->duration(), output);
  }
  
  // required string file = 3;
  if (has_file()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->file().data(), this->file().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      3, this->file(), output);
  }
  
  // required string date = 4;
  if (has_date()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->date().data(), this->date().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      4, this->date(), output);
  }
  
  // required string description = 5;
  if (has_description()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->description().data(), this->description().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      5, this->description(), output);
  }
  
  // required .mantis.response.status_t status = 6;
  if (has_status()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      6, this->status(), output);
  }
  
  // optional uint32 reader_records = 7;
  if (has_reader_records()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(7, this->reader_records(), output);
  }
  
  // optional uint32 reader_acquisitions = 8;
  if (has_reader_acquisitions()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(8, this->reader_acquisitions(), output);
  }
  
  // optional double reader_live_time = 9;
  if (has_reader_live_time()) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(9, this->reader_live_time(), output);
  }
  
  // optional double reader_dead_time = 10;
  if (has_reader_dead_time()) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(10, this->reader_dead_time(), output);
  }
  
  // optional double reader_megabytes = 11;
  if (has_reader_megabytes()) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(11, this->reader_megabytes(), output);
  }
  
  // optional double reader_rate = 12;
  if (has_reader_rate()) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(12, this->reader_rate(), output);
  }
  
  // optional uint32 writer_records = 13;
  if (has_writer_records()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(13, this->writer_records(), output);
  }
  
  // optional uint32 writer_acquisitions = 14;
  if (has_writer_acquisitions()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(14, this->writer_acquisitions(), output);
  }
  
  // optional double writer_live_time = 15;
  if (has_writer_live_time()) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(15, this->writer_live_time(), output);
  }
  
  // optional double writer_megabytes = 16;
  if (has_writer_megabytes()) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(16, this->writer_megabytes(), output);
  }
  
  // optional double writer_rate = 17;
  if (has_writer_rate()) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(17, this->writer_rate(), output);
  }
  
  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* response::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required double rate = 1;
  if (has_rate()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(1, this->rate(), target);
  }
  
  // required double duration = 2;
  if (has_duration()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(2, this->duration(), target);
  }
  
  // required string file = 3;
  if (has_file()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->file().data(), this->file().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        3, this->file(), target);
  }
  
  // required string date = 4;
  if (has_date()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->date().data(), this->date().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        4, this->date(), target);
  }
  
  // required string description = 5;
  if (has_description()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->description().data(), this->description().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        5, this->description(), target);
  }
  
  // required .mantis.response.status_t status = 6;
  if (has_status()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      6, this->status(), target);
  }
  
  // optional uint32 reader_records = 7;
  if (has_reader_records()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(7, this->reader_records(), target);
  }
  
  // optional uint32 reader_acquisitions = 8;
  if (has_reader_acquisitions()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(8, this->reader_acquisitions(), target);
  }
  
  // optional double reader_live_time = 9;
  if (has_reader_live_time()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(9, this->reader_live_time(), target);
  }
  
  // optional double reader_dead_time = 10;
  if (has_reader_dead_time()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(10, this->reader_dead_time(), target);
  }
  
  // optional double reader_megabytes = 11;
  if (has_reader_megabytes()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(11, this->reader_megabytes(), target);
  }
  
  // optional double reader_rate = 12;
  if (has_reader_rate()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(12, this->reader_rate(), target);
  }
  
  // optional uint32 writer_records = 13;
  if (has_writer_records()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(13, this->writer_records(), target);
  }
  
  // optional uint32 writer_acquisitions = 14;
  if (has_writer_acquisitions()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(14, this->writer_acquisitions(), target);
  }
  
  // optional double writer_live_time = 15;
  if (has_writer_live_time()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(15, this->writer_live_time(), target);
  }
  
  // optional double writer_megabytes = 16;
  if (has_writer_megabytes()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(16, this->writer_megabytes(), target);
  }
  
  // optional double writer_rate = 17;
  if (has_writer_rate()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(17, this->writer_rate(), target);
  }
  
  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int response::ByteSize() const {
  int total_size = 0;
  
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required double rate = 1;
    if (has_rate()) {
      total_size += 1 + 8;
    }
    
    // required double duration = 2;
    if (has_duration()) {
      total_size += 1 + 8;
    }
    
    // required string file = 3;
    if (has_file()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->file());
    }
    
    // required string date = 4;
    if (has_date()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->date());
    }
    
    // required string description = 5;
    if (has_description()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->description());
    }
    
    // required .mantis.response.status_t status = 6;
    if (has_status()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->status());
    }
    
    // optional uint32 reader_records = 7;
    if (has_reader_records()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->reader_records());
    }
    
    // optional uint32 reader_acquisitions = 8;
    if (has_reader_acquisitions()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->reader_acquisitions());
    }
    
  }
  if (_has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    // optional double reader_live_time = 9;
    if (has_reader_live_time()) {
      total_size += 1 + 8;
    }
    
    // optional double reader_dead_time = 10;
    if (has_reader_dead_time()) {
      total_size += 1 + 8;
    }
    
    // optional double reader_megabytes = 11;
    if (has_reader_megabytes()) {
      total_size += 1 + 8;
    }
    
    // optional double reader_rate = 12;
    if (has_reader_rate()) {
      total_size += 1 + 8;
    }
    
    // optional uint32 writer_records = 13;
    if (has_writer_records()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->writer_records());
    }
    
    // optional uint32 writer_acquisitions = 14;
    if (has_writer_acquisitions()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->writer_acquisitions());
    }
    
    // optional double writer_live_time = 15;
    if (has_writer_live_time()) {
      total_size += 1 + 8;
    }
    
    // optional double writer_megabytes = 16;
    if (has_writer_megabytes()) {
      total_size += 2 + 8;
    }
    
  }
  if (_has_bits_[16 / 32] & (0xffu << (16 % 32))) {
    // optional double writer_rate = 17;
    if (has_writer_rate()) {
      total_size += 2 + 8;
    }
    
  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void response::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const response* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const response*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void response::MergeFrom(const response& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_rate()) {
      set_rate(from.rate());
    }
    if (from.has_duration()) {
      set_duration(from.duration());
    }
    if (from.has_file()) {
      set_file(from.file());
    }
    if (from.has_date()) {
      set_date(from.date());
    }
    if (from.has_description()) {
      set_description(from.description());
    }
    if (from.has_status()) {
      set_status(from.status());
    }
    if (from.has_reader_records()) {
      set_reader_records(from.reader_records());
    }
    if (from.has_reader_acquisitions()) {
      set_reader_acquisitions(from.reader_acquisitions());
    }
  }
  if (from._has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    if (from.has_reader_live_time()) {
      set_reader_live_time(from.reader_live_time());
    }
    if (from.has_reader_dead_time()) {
      set_reader_dead_time(from.reader_dead_time());
    }
    if (from.has_reader_megabytes()) {
      set_reader_megabytes(from.reader_megabytes());
    }
    if (from.has_reader_rate()) {
      set_reader_rate(from.reader_rate());
    }
    if (from.has_writer_records()) {
      set_writer_records(from.writer_records());
    }
    if (from.has_writer_acquisitions()) {
      set_writer_acquisitions(from.writer_acquisitions());
    }
    if (from.has_writer_live_time()) {
      set_writer_live_time(from.writer_live_time());
    }
    if (from.has_writer_megabytes()) {
      set_writer_megabytes(from.writer_megabytes());
    }
  }
  if (from._has_bits_[16 / 32] & (0xffu << (16 % 32))) {
    if (from.has_writer_rate()) {
      set_writer_rate(from.writer_rate());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void response::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void response::CopyFrom(const response& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool response::IsInitialized() const {
  if ((_has_bits_[0] & 0x0000003f) != 0x0000003f) return false;
  
  return true;
}

void response::Swap(response* other) {
  if (other != this) {
    std::swap(rate_, other->rate_);
    std::swap(duration_, other->duration_);
    std::swap(file_, other->file_);
    std::swap(date_, other->date_);
    std::swap(description_, other->description_);
    std::swap(status_, other->status_);
    std::swap(reader_records_, other->reader_records_);
    std::swap(reader_acquisitions_, other->reader_acquisitions_);
    std::swap(reader_live_time_, other->reader_live_time_);
    std::swap(reader_dead_time_, other->reader_dead_time_);
    std::swap(reader_megabytes_, other->reader_megabytes_);
    std::swap(reader_rate_, other->reader_rate_);
    std::swap(writer_records_, other->writer_records_);
    std::swap(writer_acquisitions_, other->writer_acquisitions_);
    std::swap(writer_live_time_, other->writer_live_time_);
    std::swap(writer_megabytes_, other->writer_megabytes_);
    std::swap(writer_rate_, other->writer_rate_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata response::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = response_descriptor_;
  metadata.reflection = response_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace mantis

// @@protoc_insertion_point(global_scope)
