// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: message.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "message.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace {

const ::google::protobuf::Descriptor* ChatMessage_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  ChatMessage_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_message_2eproto() GOOGLE_ATTRIBUTE_COLD;
void protobuf_AssignDesc_message_2eproto() {
  protobuf_AddDesc_message_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "message.proto");
  GOOGLE_CHECK(file != NULL);
  ChatMessage_descriptor_ = file->message_type(0);
  static const int ChatMessage_offsets_[3] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ChatMessage, sender_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ChatMessage, text_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ChatMessage, data_),
  };
  ChatMessage_reflection_ =
    ::google::protobuf::internal::GeneratedMessageReflection::NewGeneratedMessageReflection(
      ChatMessage_descriptor_,
      ChatMessage::default_instance_,
      ChatMessage_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ChatMessage, _has_bits_[0]),
      -1,
      -1,
      sizeof(ChatMessage),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(ChatMessage, _internal_metadata_),
      -1);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_message_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
      ChatMessage_descriptor_, &ChatMessage::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_message_2eproto() {
  delete ChatMessage::default_instance_;
  delete ChatMessage_reflection_;
}

void protobuf_AddDesc_message_2eproto() GOOGLE_ATTRIBUTE_COLD;
void protobuf_AddDesc_message_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\rmessage.proto\"9\n\013ChatMessage\022\016\n\006Sender"
    "\030\001 \002(\t\022\014\n\004Text\030\002 \002(\t\022\014\n\004Data\030\003 \001(\t", 74);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "message.proto", &protobuf_RegisterTypes);
  ChatMessage::default_instance_ = new ChatMessage();
  ChatMessage::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_message_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_message_2eproto {
  StaticDescriptorInitializer_message_2eproto() {
    protobuf_AddDesc_message_2eproto();
  }
} static_descriptor_initializer_message_2eproto_;

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int ChatMessage::kSenderFieldNumber;
const int ChatMessage::kTextFieldNumber;
const int ChatMessage::kDataFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

ChatMessage::ChatMessage()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:ChatMessage)
}

void ChatMessage::InitAsDefaultInstance() {
}

ChatMessage::ChatMessage(const ChatMessage& from)
  : ::google::protobuf::Message(),
    _internal_metadata_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:ChatMessage)
}

void ChatMessage::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  sender_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  text_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  data_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

ChatMessage::~ChatMessage() {
  // @@protoc_insertion_point(destructor:ChatMessage)
  SharedDtor();
}

void ChatMessage::SharedDtor() {
  sender_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  text_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  data_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (this != default_instance_) {
  }
}

void ChatMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* ChatMessage::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return ChatMessage_descriptor_;
}

const ChatMessage& ChatMessage::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_message_2eproto();
  return *default_instance_;
}

ChatMessage* ChatMessage::default_instance_ = NULL;

ChatMessage* ChatMessage::New(::google::protobuf::Arena* arena) const {
  ChatMessage* n = new ChatMessage;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void ChatMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:ChatMessage)
  if (_has_bits_[0 / 32] & 7u) {
    if (has_sender()) {
      sender_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    }
    if (has_text()) {
      text_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    }
    if (has_data()) {
      data_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  if (_internal_metadata_.have_unknown_fields()) {
    mutable_unknown_fields()->Clear();
  }
}

bool ChatMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:ChatMessage)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required string Sender = 1;
      case 1: {
        if (tag == 10) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_sender()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->sender().data(), this->sender().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "ChatMessage.Sender");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(18)) goto parse_Text;
        break;
      }

      // required string Text = 2;
      case 2: {
        if (tag == 18) {
         parse_Text:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_text()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->text().data(), this->text().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "ChatMessage.Text");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(26)) goto parse_Data;
        break;
      }

      // optional string Data = 3;
      case 3: {
        if (tag == 26) {
         parse_Data:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_data()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->data().data(), this->data().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "ChatMessage.Data");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:ChatMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:ChatMessage)
  return false;
#undef DO_
}

void ChatMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:ChatMessage)
  // required string Sender = 1;
  if (has_sender()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->sender().data(), this->sender().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "ChatMessage.Sender");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->sender(), output);
  }

  // required string Text = 2;
  if (has_text()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->text().data(), this->text().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "ChatMessage.Text");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->text(), output);
  }

  // optional string Data = 3;
  if (has_data()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->data().data(), this->data().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "ChatMessage.Data");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      3, this->data(), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:ChatMessage)
}

::google::protobuf::uint8* ChatMessage::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:ChatMessage)
  // required string Sender = 1;
  if (has_sender()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->sender().data(), this->sender().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "ChatMessage.Sender");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->sender(), target);
  }

  // required string Text = 2;
  if (has_text()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->text().data(), this->text().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "ChatMessage.Text");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->text(), target);
  }

  // optional string Data = 3;
  if (has_data()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->data().data(), this->data().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "ChatMessage.Data");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        3, this->data(), target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:ChatMessage)
  return target;
}

int ChatMessage::RequiredFieldsByteSizeFallback() const {
// @@protoc_insertion_point(required_fields_byte_size_fallback_start:ChatMessage)
  int total_size = 0;

  if (has_sender()) {
    // required string Sender = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->sender());
  }

  if (has_text()) {
    // required string Text = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->text());
  }

  return total_size;
}
int ChatMessage::ByteSize() const {
// @@protoc_insertion_point(message_byte_size_start:ChatMessage)
  int total_size = 0;

  if (((_has_bits_[0] & 0x00000003) ^ 0x00000003) == 0) {  // All required fields are present.
    // required string Sender = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->sender());

    // required string Text = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->text());

  } else {
    total_size += RequiredFieldsByteSizeFallback();
  }
  // optional string Data = 3;
  if (has_data()) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->data());
  }

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void ChatMessage::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:ChatMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) {
    ::google::protobuf::internal::MergeFromFail(__FILE__, __LINE__);
  }
  const ChatMessage* source = 
      ::google::protobuf::internal::DynamicCastToGenerated<const ChatMessage>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:ChatMessage)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:ChatMessage)
    MergeFrom(*source);
  }
}

void ChatMessage::MergeFrom(const ChatMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:ChatMessage)
  if (GOOGLE_PREDICT_FALSE(&from == this)) {
    ::google::protobuf::internal::MergeFromFail(__FILE__, __LINE__);
  }
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_sender()) {
      set_has_sender();
      sender_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.sender_);
    }
    if (from.has_text()) {
      set_has_text();
      text_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.text_);
    }
    if (from.has_data()) {
      set_has_data();
      data_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.data_);
    }
  }
  if (from._internal_metadata_.have_unknown_fields()) {
    mutable_unknown_fields()->MergeFrom(from.unknown_fields());
  }
}

void ChatMessage::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:ChatMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void ChatMessage::CopyFrom(const ChatMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:ChatMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool ChatMessage::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000003) != 0x00000003) return false;

  return true;
}

void ChatMessage::Swap(ChatMessage* other) {
  if (other == this) return;
  InternalSwap(other);
}
void ChatMessage::InternalSwap(ChatMessage* other) {
  sender_.Swap(&other->sender_);
  text_.Swap(&other->text_);
  data_.Swap(&other->data_);
  std::swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata ChatMessage::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = ChatMessage_descriptor_;
  metadata.reflection = ChatMessage_reflection_;
  return metadata;
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// ChatMessage

// required string Sender = 1;
bool ChatMessage::has_sender() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
void ChatMessage::set_has_sender() {
  _has_bits_[0] |= 0x00000001u;
}
void ChatMessage::clear_has_sender() {
  _has_bits_[0] &= ~0x00000001u;
}
void ChatMessage::clear_sender() {
  sender_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_sender();
}
 const ::std::string& ChatMessage::sender() const {
  // @@protoc_insertion_point(field_get:ChatMessage.Sender)
  return sender_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void ChatMessage::set_sender(const ::std::string& value) {
  set_has_sender();
  sender_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:ChatMessage.Sender)
}
 void ChatMessage::set_sender(const char* value) {
  set_has_sender();
  sender_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:ChatMessage.Sender)
}
 void ChatMessage::set_sender(const char* value, size_t size) {
  set_has_sender();
  sender_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:ChatMessage.Sender)
}
 ::std::string* ChatMessage::mutable_sender() {
  set_has_sender();
  // @@protoc_insertion_point(field_mutable:ChatMessage.Sender)
  return sender_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 ::std::string* ChatMessage::release_sender() {
  // @@protoc_insertion_point(field_release:ChatMessage.Sender)
  clear_has_sender();
  return sender_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void ChatMessage::set_allocated_sender(::std::string* sender) {
  if (sender != NULL) {
    set_has_sender();
  } else {
    clear_has_sender();
  }
  sender_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), sender);
  // @@protoc_insertion_point(field_set_allocated:ChatMessage.Sender)
}

// required string Text = 2;
bool ChatMessage::has_text() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
void ChatMessage::set_has_text() {
  _has_bits_[0] |= 0x00000002u;
}
void ChatMessage::clear_has_text() {
  _has_bits_[0] &= ~0x00000002u;
}
void ChatMessage::clear_text() {
  text_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_text();
}
 const ::std::string& ChatMessage::text() const {
  // @@protoc_insertion_point(field_get:ChatMessage.Text)
  return text_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void ChatMessage::set_text(const ::std::string& value) {
  set_has_text();
  text_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:ChatMessage.Text)
}
 void ChatMessage::set_text(const char* value) {
  set_has_text();
  text_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:ChatMessage.Text)
}
 void ChatMessage::set_text(const char* value, size_t size) {
  set_has_text();
  text_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:ChatMessage.Text)
}
 ::std::string* ChatMessage::mutable_text() {
  set_has_text();
  // @@protoc_insertion_point(field_mutable:ChatMessage.Text)
  return text_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 ::std::string* ChatMessage::release_text() {
  // @@protoc_insertion_point(field_release:ChatMessage.Text)
  clear_has_text();
  return text_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void ChatMessage::set_allocated_text(::std::string* text) {
  if (text != NULL) {
    set_has_text();
  } else {
    clear_has_text();
  }
  text_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), text);
  // @@protoc_insertion_point(field_set_allocated:ChatMessage.Text)
}

// optional string Data = 3;
bool ChatMessage::has_data() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
void ChatMessage::set_has_data() {
  _has_bits_[0] |= 0x00000004u;
}
void ChatMessage::clear_has_data() {
  _has_bits_[0] &= ~0x00000004u;
}
void ChatMessage::clear_data() {
  data_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_data();
}
 const ::std::string& ChatMessage::data() const {
  // @@protoc_insertion_point(field_get:ChatMessage.Data)
  return data_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void ChatMessage::set_data(const ::std::string& value) {
  set_has_data();
  data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:ChatMessage.Data)
}
 void ChatMessage::set_data(const char* value) {
  set_has_data();
  data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:ChatMessage.Data)
}
 void ChatMessage::set_data(const char* value, size_t size) {
  set_has_data();
  data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:ChatMessage.Data)
}
 ::std::string* ChatMessage::mutable_data() {
  set_has_data();
  // @@protoc_insertion_point(field_mutable:ChatMessage.Data)
  return data_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 ::std::string* ChatMessage::release_data() {
  // @@protoc_insertion_point(field_release:ChatMessage.Data)
  clear_has_data();
  return data_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
 void ChatMessage::set_allocated_data(::std::string* data) {
  if (data != NULL) {
    set_has_data();
  } else {
    clear_has_data();
  }
  data_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), data);
  // @@protoc_insertion_point(field_set_allocated:ChatMessage.Data)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)
