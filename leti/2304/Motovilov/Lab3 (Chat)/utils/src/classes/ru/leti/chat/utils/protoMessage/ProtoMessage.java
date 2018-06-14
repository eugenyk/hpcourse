// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: proto_message.proto

package ru.leti.chat.utils.protoMessage;

public final class ProtoMessage {
  private ProtoMessage() {}
  public static void registerAllExtensions(
      com.google.protobuf.ExtensionRegistryLite registry) {
  }

  public static void registerAllExtensions(
      com.google.protobuf.ExtensionRegistry registry) {
    registerAllExtensions(
        (com.google.protobuf.ExtensionRegistryLite) registry);
  }
  public interface ChatMessageOrBuilder extends
      // @@protoc_insertion_point(interface_extends:ChatMessage)
      com.google.protobuf.MessageOrBuilder {

    /**
     * <pre>
     *&#47;&lt; Идентификатор отправителя
     * </pre>
     *
     * <code>required string Sender = 1;</code>
     */
    boolean hasSender();
    /**
     * <pre>
     *&#47;&lt; Идентификатор отправителя
     * </pre>
     *
     * <code>required string Sender = 1;</code>
     */
    java.lang.String getSender();
    /**
     * <pre>
     *&#47;&lt; Идентификатор отправителя
     * </pre>
     *
     * <code>required string Sender = 1;</code>
     */
    com.google.protobuf.ByteString
        getSenderBytes();

    /**
     * <pre>
     *&#47;&lt; Содержание сообщения
     * </pre>
     *
     * <code>required string Text = 2;</code>
     */
    boolean hasText();
    /**
     * <pre>
     *&#47;&lt; Содержание сообщения
     * </pre>
     *
     * <code>required string Text = 2;</code>
     */
    java.lang.String getText();
    /**
     * <pre>
     *&#47;&lt; Содержание сообщения
     * </pre>
     *
     * <code>required string Text = 2;</code>
     */
    com.google.protobuf.ByteString
        getTextBytes();

    /**
     * <pre>
     *&#47;&lt; Internal field.
     * </pre>
     *
     * <code>optional string Data = 3;</code>
     */
    boolean hasData();
    /**
     * <pre>
     *&#47;&lt; Internal field.
     * </pre>
     *
     * <code>optional string Data = 3;</code>
     */
    java.lang.String getData();
    /**
     * <pre>
     *&#47;&lt; Internal field.
     * </pre>
     *
     * <code>optional string Data = 3;</code>
     */
    com.google.protobuf.ByteString
        getDataBytes();
  }
  /**
   * Protobuf type {@code ChatMessage}
   */
  public  static final class ChatMessage extends
      com.google.protobuf.GeneratedMessageV3 implements
      // @@protoc_insertion_point(message_implements:ChatMessage)
      ChatMessageOrBuilder {
  private static final long serialVersionUID = 0L;
    // Use ChatMessage.newBuilder() to construct.
    private ChatMessage(com.google.protobuf.GeneratedMessageV3.Builder<?> builder) {
      super(builder);
    }
    private ChatMessage() {
      sender_ = "";
      text_ = "";
      data_ = "";
    }

    @java.lang.Override
    public final com.google.protobuf.UnknownFieldSet
    getUnknownFields() {
      return this.unknownFields;
    }
    private ChatMessage(
        com.google.protobuf.CodedInputStream input,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws com.google.protobuf.InvalidProtocolBufferException {
      this();
      if (extensionRegistry == null) {
        throw new java.lang.NullPointerException();
      }
      int mutable_bitField0_ = 0;
      com.google.protobuf.UnknownFieldSet.Builder unknownFields =
          com.google.protobuf.UnknownFieldSet.newBuilder();
      try {
        boolean done = false;
        while (!done) {
          int tag = input.readTag();
          switch (tag) {
            case 0:
              done = true;
              break;
            default: {
              if (!parseUnknownField(
                  input, unknownFields, extensionRegistry, tag)) {
                done = true;
              }
              break;
            }
            case 10: {
              com.google.protobuf.ByteString bs = input.readBytes();
              bitField0_ |= 0x00000001;
              sender_ = bs;
              break;
            }
            case 18: {
              com.google.protobuf.ByteString bs = input.readBytes();
              bitField0_ |= 0x00000002;
              text_ = bs;
              break;
            }
            case 26: {
              com.google.protobuf.ByteString bs = input.readBytes();
              bitField0_ |= 0x00000004;
              data_ = bs;
              break;
            }
          }
        }
      } catch (com.google.protobuf.InvalidProtocolBufferException e) {
        throw e.setUnfinishedMessage(this);
      } catch (java.io.IOException e) {
        throw new com.google.protobuf.InvalidProtocolBufferException(
            e).setUnfinishedMessage(this);
      } finally {
        this.unknownFields = unknownFields.build();
        makeExtensionsImmutable();
      }
    }
    public static final com.google.protobuf.Descriptors.Descriptor
        getDescriptor() {
      return ru.leti.chat.utils.protoMessage.ProtoMessage.internal_static_ChatMessage_descriptor;
    }

    protected com.google.protobuf.GeneratedMessageV3.FieldAccessorTable
        internalGetFieldAccessorTable() {
      return ru.leti.chat.utils.protoMessage.ProtoMessage.internal_static_ChatMessage_fieldAccessorTable
          .ensureFieldAccessorsInitialized(
              ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage.class, ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage.Builder.class);
    }

    private int bitField0_;
    public static final int SENDER_FIELD_NUMBER = 1;
    private volatile java.lang.Object sender_;
    /**
     * <pre>
     *&#47;&lt; Идентификатор отправителя
     * </pre>
     *
     * <code>required string Sender = 1;</code>
     */
    public boolean hasSender() {
      return ((bitField0_ & 0x00000001) == 0x00000001);
    }
    /**
     * <pre>
     *&#47;&lt; Идентификатор отправителя
     * </pre>
     *
     * <code>required string Sender = 1;</code>
     */
    public java.lang.String getSender() {
      java.lang.Object ref = sender_;
      if (ref instanceof java.lang.String) {
        return (java.lang.String) ref;
      } else {
        com.google.protobuf.ByteString bs = 
            (com.google.protobuf.ByteString) ref;
        java.lang.String s = bs.toStringUtf8();
        if (bs.isValidUtf8()) {
          sender_ = s;
        }
        return s;
      }
    }
    /**
     * <pre>
     *&#47;&lt; Идентификатор отправителя
     * </pre>
     *
     * <code>required string Sender = 1;</code>
     */
    public com.google.protobuf.ByteString
        getSenderBytes() {
      java.lang.Object ref = sender_;
      if (ref instanceof java.lang.String) {
        com.google.protobuf.ByteString b = 
            com.google.protobuf.ByteString.copyFromUtf8(
                (java.lang.String) ref);
        sender_ = b;
        return b;
      } else {
        return (com.google.protobuf.ByteString) ref;
      }
    }

    public static final int TEXT_FIELD_NUMBER = 2;
    private volatile java.lang.Object text_;
    /**
     * <pre>
     *&#47;&lt; Содержание сообщения
     * </pre>
     *
     * <code>required string Text = 2;</code>
     */
    public boolean hasText() {
      return ((bitField0_ & 0x00000002) == 0x00000002);
    }
    /**
     * <pre>
     *&#47;&lt; Содержание сообщения
     * </pre>
     *
     * <code>required string Text = 2;</code>
     */
    public java.lang.String getText() {
      java.lang.Object ref = text_;
      if (ref instanceof java.lang.String) {
        return (java.lang.String) ref;
      } else {
        com.google.protobuf.ByteString bs = 
            (com.google.protobuf.ByteString) ref;
        java.lang.String s = bs.toStringUtf8();
        if (bs.isValidUtf8()) {
          text_ = s;
        }
        return s;
      }
    }
    /**
     * <pre>
     *&#47;&lt; Содержание сообщения
     * </pre>
     *
     * <code>required string Text = 2;</code>
     */
    public com.google.protobuf.ByteString
        getTextBytes() {
      java.lang.Object ref = text_;
      if (ref instanceof java.lang.String) {
        com.google.protobuf.ByteString b = 
            com.google.protobuf.ByteString.copyFromUtf8(
                (java.lang.String) ref);
        text_ = b;
        return b;
      } else {
        return (com.google.protobuf.ByteString) ref;
      }
    }

    public static final int DATA_FIELD_NUMBER = 3;
    private volatile java.lang.Object data_;
    /**
     * <pre>
     *&#47;&lt; Internal field.
     * </pre>
     *
     * <code>optional string Data = 3;</code>
     */
    public boolean hasData() {
      return ((bitField0_ & 0x00000004) == 0x00000004);
    }
    /**
     * <pre>
     *&#47;&lt; Internal field.
     * </pre>
     *
     * <code>optional string Data = 3;</code>
     */
    public java.lang.String getData() {
      java.lang.Object ref = data_;
      if (ref instanceof java.lang.String) {
        return (java.lang.String) ref;
      } else {
        com.google.protobuf.ByteString bs = 
            (com.google.protobuf.ByteString) ref;
        java.lang.String s = bs.toStringUtf8();
        if (bs.isValidUtf8()) {
          data_ = s;
        }
        return s;
      }
    }
    /**
     * <pre>
     *&#47;&lt; Internal field.
     * </pre>
     *
     * <code>optional string Data = 3;</code>
     */
    public com.google.protobuf.ByteString
        getDataBytes() {
      java.lang.Object ref = data_;
      if (ref instanceof java.lang.String) {
        com.google.protobuf.ByteString b = 
            com.google.protobuf.ByteString.copyFromUtf8(
                (java.lang.String) ref);
        data_ = b;
        return b;
      } else {
        return (com.google.protobuf.ByteString) ref;
      }
    }

    private byte memoizedIsInitialized = -1;
    public final boolean isInitialized() {
      byte isInitialized = memoizedIsInitialized;
      if (isInitialized == 1) return true;
      if (isInitialized == 0) return false;

      if (!hasSender()) {
        memoizedIsInitialized = 0;
        return false;
      }
      if (!hasText()) {
        memoizedIsInitialized = 0;
        return false;
      }
      memoizedIsInitialized = 1;
      return true;
    }

    public void writeTo(com.google.protobuf.CodedOutputStream output)
                        throws java.io.IOException {
      if (((bitField0_ & 0x00000001) == 0x00000001)) {
        com.google.protobuf.GeneratedMessageV3.writeString(output, 1, sender_);
      }
      if (((bitField0_ & 0x00000002) == 0x00000002)) {
        com.google.protobuf.GeneratedMessageV3.writeString(output, 2, text_);
      }
      if (((bitField0_ & 0x00000004) == 0x00000004)) {
        com.google.protobuf.GeneratedMessageV3.writeString(output, 3, data_);
      }
      unknownFields.writeTo(output);
    }

    public int getSerializedSize() {
      int size = memoizedSize;
      if (size != -1) return size;

      size = 0;
      if (((bitField0_ & 0x00000001) == 0x00000001)) {
        size += com.google.protobuf.GeneratedMessageV3.computeStringSize(1, sender_);
      }
      if (((bitField0_ & 0x00000002) == 0x00000002)) {
        size += com.google.protobuf.GeneratedMessageV3.computeStringSize(2, text_);
      }
      if (((bitField0_ & 0x00000004) == 0x00000004)) {
        size += com.google.protobuf.GeneratedMessageV3.computeStringSize(3, data_);
      }
      size += unknownFields.getSerializedSize();
      memoizedSize = size;
      return size;
    }

    @java.lang.Override
    public boolean equals(final java.lang.Object obj) {
      if (obj == this) {
       return true;
      }
      if (!(obj instanceof ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage)) {
        return super.equals(obj);
      }
      ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage other = (ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage) obj;

      boolean result = true;
      result = result && (hasSender() == other.hasSender());
      if (hasSender()) {
        result = result && getSender()
            .equals(other.getSender());
      }
      result = result && (hasText() == other.hasText());
      if (hasText()) {
        result = result && getText()
            .equals(other.getText());
      }
      result = result && (hasData() == other.hasData());
      if (hasData()) {
        result = result && getData()
            .equals(other.getData());
      }
      result = result && unknownFields.equals(other.unknownFields);
      return result;
    }

    @java.lang.Override
    public int hashCode() {
      if (memoizedHashCode != 0) {
        return memoizedHashCode;
      }
      int hash = 41;
      hash = (19 * hash) + getDescriptor().hashCode();
      if (hasSender()) {
        hash = (37 * hash) + SENDER_FIELD_NUMBER;
        hash = (53 * hash) + getSender().hashCode();
      }
      if (hasText()) {
        hash = (37 * hash) + TEXT_FIELD_NUMBER;
        hash = (53 * hash) + getText().hashCode();
      }
      if (hasData()) {
        hash = (37 * hash) + DATA_FIELD_NUMBER;
        hash = (53 * hash) + getData().hashCode();
      }
      hash = (29 * hash) + unknownFields.hashCode();
      memoizedHashCode = hash;
      return hash;
    }

    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseFrom(
        java.nio.ByteBuffer data)
        throws com.google.protobuf.InvalidProtocolBufferException {
      return PARSER.parseFrom(data);
    }
    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseFrom(
        java.nio.ByteBuffer data,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws com.google.protobuf.InvalidProtocolBufferException {
      return PARSER.parseFrom(data, extensionRegistry);
    }
    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseFrom(
        com.google.protobuf.ByteString data)
        throws com.google.protobuf.InvalidProtocolBufferException {
      return PARSER.parseFrom(data);
    }
    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseFrom(
        com.google.protobuf.ByteString data,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws com.google.protobuf.InvalidProtocolBufferException {
      return PARSER.parseFrom(data, extensionRegistry);
    }
    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseFrom(byte[] data)
        throws com.google.protobuf.InvalidProtocolBufferException {
      return PARSER.parseFrom(data);
    }
    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseFrom(
        byte[] data,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws com.google.protobuf.InvalidProtocolBufferException {
      return PARSER.parseFrom(data, extensionRegistry);
    }
    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseFrom(java.io.InputStream input)
        throws java.io.IOException {
      return com.google.protobuf.GeneratedMessageV3
          .parseWithIOException(PARSER, input);
    }
    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseFrom(
        java.io.InputStream input,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws java.io.IOException {
      return com.google.protobuf.GeneratedMessageV3
          .parseWithIOException(PARSER, input, extensionRegistry);
    }
    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseDelimitedFrom(java.io.InputStream input)
        throws java.io.IOException {
      return com.google.protobuf.GeneratedMessageV3
          .parseDelimitedWithIOException(PARSER, input);
    }
    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseDelimitedFrom(
        java.io.InputStream input,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws java.io.IOException {
      return com.google.protobuf.GeneratedMessageV3
          .parseDelimitedWithIOException(PARSER, input, extensionRegistry);
    }
    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseFrom(
        com.google.protobuf.CodedInputStream input)
        throws java.io.IOException {
      return com.google.protobuf.GeneratedMessageV3
          .parseWithIOException(PARSER, input);
    }
    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parseFrom(
        com.google.protobuf.CodedInputStream input,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws java.io.IOException {
      return com.google.protobuf.GeneratedMessageV3
          .parseWithIOException(PARSER, input, extensionRegistry);
    }

    public Builder newBuilderForType() { return newBuilder(); }
    public static Builder newBuilder() {
      return DEFAULT_INSTANCE.toBuilder();
    }
    public static Builder newBuilder(ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage prototype) {
      return DEFAULT_INSTANCE.toBuilder().mergeFrom(prototype);
    }
    public Builder toBuilder() {
      return this == DEFAULT_INSTANCE
          ? new Builder() : new Builder().mergeFrom(this);
    }

    @java.lang.Override
    protected Builder newBuilderForType(
        com.google.protobuf.GeneratedMessageV3.BuilderParent parent) {
      Builder builder = new Builder(parent);
      return builder;
    }
    /**
     * Protobuf type {@code ChatMessage}
     */
    public static final class Builder extends
        com.google.protobuf.GeneratedMessageV3.Builder<Builder> implements
        // @@protoc_insertion_point(builder_implements:ChatMessage)
        ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessageOrBuilder {
      public static final com.google.protobuf.Descriptors.Descriptor
          getDescriptor() {
        return ru.leti.chat.utils.protoMessage.ProtoMessage.internal_static_ChatMessage_descriptor;
      }

      protected com.google.protobuf.GeneratedMessageV3.FieldAccessorTable
          internalGetFieldAccessorTable() {
        return ru.leti.chat.utils.protoMessage.ProtoMessage.internal_static_ChatMessage_fieldAccessorTable
            .ensureFieldAccessorsInitialized(
                ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage.class, ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage.Builder.class);
      }

      // Construct using ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage.newBuilder()
      private Builder() {
        maybeForceBuilderInitialization();
      }

      private Builder(
          com.google.protobuf.GeneratedMessageV3.BuilderParent parent) {
        super(parent);
        maybeForceBuilderInitialization();
      }
      private void maybeForceBuilderInitialization() {
        if (com.google.protobuf.GeneratedMessageV3
                .alwaysUseFieldBuilders) {
        }
      }
      public Builder clear() {
        super.clear();
        sender_ = "";
        bitField0_ = (bitField0_ & ~0x00000001);
        text_ = "";
        bitField0_ = (bitField0_ & ~0x00000002);
        data_ = "";
        bitField0_ = (bitField0_ & ~0x00000004);
        return this;
      }

      public com.google.protobuf.Descriptors.Descriptor
          getDescriptorForType() {
        return ru.leti.chat.utils.protoMessage.ProtoMessage.internal_static_ChatMessage_descriptor;
      }

      public ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage getDefaultInstanceForType() {
        return ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage.getDefaultInstance();
      }

      public ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage build() {
        ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage result = buildPartial();
        if (!result.isInitialized()) {
          throw newUninitializedMessageException(result);
        }
        return result;
      }

      public ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage buildPartial() {
        ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage result = new ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage(this);
        int from_bitField0_ = bitField0_;
        int to_bitField0_ = 0;
        if (((from_bitField0_ & 0x00000001) == 0x00000001)) {
          to_bitField0_ |= 0x00000001;
        }
        result.sender_ = sender_;
        if (((from_bitField0_ & 0x00000002) == 0x00000002)) {
          to_bitField0_ |= 0x00000002;
        }
        result.text_ = text_;
        if (((from_bitField0_ & 0x00000004) == 0x00000004)) {
          to_bitField0_ |= 0x00000004;
        }
        result.data_ = data_;
        result.bitField0_ = to_bitField0_;
        onBuilt();
        return result;
      }

      public Builder clone() {
        return (Builder) super.clone();
      }
      public Builder setField(
          com.google.protobuf.Descriptors.FieldDescriptor field,
          java.lang.Object value) {
        return (Builder) super.setField(field, value);
      }
      public Builder clearField(
          com.google.protobuf.Descriptors.FieldDescriptor field) {
        return (Builder) super.clearField(field);
      }
      public Builder clearOneof(
          com.google.protobuf.Descriptors.OneofDescriptor oneof) {
        return (Builder) super.clearOneof(oneof);
      }
      public Builder setRepeatedField(
          com.google.protobuf.Descriptors.FieldDescriptor field,
          int index, java.lang.Object value) {
        return (Builder) super.setRepeatedField(field, index, value);
      }
      public Builder addRepeatedField(
          com.google.protobuf.Descriptors.FieldDescriptor field,
          java.lang.Object value) {
        return (Builder) super.addRepeatedField(field, value);
      }
      public Builder mergeFrom(com.google.protobuf.Message other) {
        if (other instanceof ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage) {
          return mergeFrom((ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage)other);
        } else {
          super.mergeFrom(other);
          return this;
        }
      }

      public Builder mergeFrom(ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage other) {
        if (other == ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage.getDefaultInstance()) return this;
        if (other.hasSender()) {
          bitField0_ |= 0x00000001;
          sender_ = other.sender_;
          onChanged();
        }
        if (other.hasText()) {
          bitField0_ |= 0x00000002;
          text_ = other.text_;
          onChanged();
        }
        if (other.hasData()) {
          bitField0_ |= 0x00000004;
          data_ = other.data_;
          onChanged();
        }
        this.mergeUnknownFields(other.unknownFields);
        onChanged();
        return this;
      }

      public final boolean isInitialized() {
        if (!hasSender()) {
          return false;
        }
        if (!hasText()) {
          return false;
        }
        return true;
      }

      public Builder mergeFrom(
          com.google.protobuf.CodedInputStream input,
          com.google.protobuf.ExtensionRegistryLite extensionRegistry)
          throws java.io.IOException {
        ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage parsedMessage = null;
        try {
          parsedMessage = PARSER.parsePartialFrom(input, extensionRegistry);
        } catch (com.google.protobuf.InvalidProtocolBufferException e) {
          parsedMessage = (ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage) e.getUnfinishedMessage();
          throw e.unwrapIOException();
        } finally {
          if (parsedMessage != null) {
            mergeFrom(parsedMessage);
          }
        }
        return this;
      }
      private int bitField0_;

      private java.lang.Object sender_ = "";
      /**
       * <pre>
       *&#47;&lt; Идентификатор отправителя
       * </pre>
       *
       * <code>required string Sender = 1;</code>
       */
      public boolean hasSender() {
        return ((bitField0_ & 0x00000001) == 0x00000001);
      }
      /**
       * <pre>
       *&#47;&lt; Идентификатор отправителя
       * </pre>
       *
       * <code>required string Sender = 1;</code>
       */
      public java.lang.String getSender() {
        java.lang.Object ref = sender_;
        if (!(ref instanceof java.lang.String)) {
          com.google.protobuf.ByteString bs =
              (com.google.protobuf.ByteString) ref;
          java.lang.String s = bs.toStringUtf8();
          if (bs.isValidUtf8()) {
            sender_ = s;
          }
          return s;
        } else {
          return (java.lang.String) ref;
        }
      }
      /**
       * <pre>
       *&#47;&lt; Идентификатор отправителя
       * </pre>
       *
       * <code>required string Sender = 1;</code>
       */
      public com.google.protobuf.ByteString
          getSenderBytes() {
        java.lang.Object ref = sender_;
        if (ref instanceof String) {
          com.google.protobuf.ByteString b = 
              com.google.protobuf.ByteString.copyFromUtf8(
                  (java.lang.String) ref);
          sender_ = b;
          return b;
        } else {
          return (com.google.protobuf.ByteString) ref;
        }
      }
      /**
       * <pre>
       *&#47;&lt; Идентификатор отправителя
       * </pre>
       *
       * <code>required string Sender = 1;</code>
       */
      public Builder setSender(
          java.lang.String value) {
        if (value == null) {
    throw new NullPointerException();
  }
  bitField0_ |= 0x00000001;
        sender_ = value;
        onChanged();
        return this;
      }
      /**
       * <pre>
       *&#47;&lt; Идентификатор отправителя
       * </pre>
       *
       * <code>required string Sender = 1;</code>
       */
      public Builder clearSender() {
        bitField0_ = (bitField0_ & ~0x00000001);
        sender_ = getDefaultInstance().getSender();
        onChanged();
        return this;
      }
      /**
       * <pre>
       *&#47;&lt; Идентификатор отправителя
       * </pre>
       *
       * <code>required string Sender = 1;</code>
       */
      public Builder setSenderBytes(
          com.google.protobuf.ByteString value) {
        if (value == null) {
    throw new NullPointerException();
  }
  bitField0_ |= 0x00000001;
        sender_ = value;
        onChanged();
        return this;
      }

      private java.lang.Object text_ = "";
      /**
       * <pre>
       *&#47;&lt; Содержание сообщения
       * </pre>
       *
       * <code>required string Text = 2;</code>
       */
      public boolean hasText() {
        return ((bitField0_ & 0x00000002) == 0x00000002);
      }
      /**
       * <pre>
       *&#47;&lt; Содержание сообщения
       * </pre>
       *
       * <code>required string Text = 2;</code>
       */
      public java.lang.String getText() {
        java.lang.Object ref = text_;
        if (!(ref instanceof java.lang.String)) {
          com.google.protobuf.ByteString bs =
              (com.google.protobuf.ByteString) ref;
          java.lang.String s = bs.toStringUtf8();
          if (bs.isValidUtf8()) {
            text_ = s;
          }
          return s;
        } else {
          return (java.lang.String) ref;
        }
      }
      /**
       * <pre>
       *&#47;&lt; Содержание сообщения
       * </pre>
       *
       * <code>required string Text = 2;</code>
       */
      public com.google.protobuf.ByteString
          getTextBytes() {
        java.lang.Object ref = text_;
        if (ref instanceof String) {
          com.google.protobuf.ByteString b = 
              com.google.protobuf.ByteString.copyFromUtf8(
                  (java.lang.String) ref);
          text_ = b;
          return b;
        } else {
          return (com.google.protobuf.ByteString) ref;
        }
      }
      /**
       * <pre>
       *&#47;&lt; Содержание сообщения
       * </pre>
       *
       * <code>required string Text = 2;</code>
       */
      public Builder setText(
          java.lang.String value) {
        if (value == null) {
    throw new NullPointerException();
  }
  bitField0_ |= 0x00000002;
        text_ = value;
        onChanged();
        return this;
      }
      /**
       * <pre>
       *&#47;&lt; Содержание сообщения
       * </pre>
       *
       * <code>required string Text = 2;</code>
       */
      public Builder clearText() {
        bitField0_ = (bitField0_ & ~0x00000002);
        text_ = getDefaultInstance().getText();
        onChanged();
        return this;
      }
      /**
       * <pre>
       *&#47;&lt; Содержание сообщения
       * </pre>
       *
       * <code>required string Text = 2;</code>
       */
      public Builder setTextBytes(
          com.google.protobuf.ByteString value) {
        if (value == null) {
    throw new NullPointerException();
  }
  bitField0_ |= 0x00000002;
        text_ = value;
        onChanged();
        return this;
      }

      private java.lang.Object data_ = "";
      /**
       * <pre>
       *&#47;&lt; Internal field.
       * </pre>
       *
       * <code>optional string Data = 3;</code>
       */
      public boolean hasData() {
        return ((bitField0_ & 0x00000004) == 0x00000004);
      }
      /**
       * <pre>
       *&#47;&lt; Internal field.
       * </pre>
       *
       * <code>optional string Data = 3;</code>
       */
      public java.lang.String getData() {
        java.lang.Object ref = data_;
        if (!(ref instanceof java.lang.String)) {
          com.google.protobuf.ByteString bs =
              (com.google.protobuf.ByteString) ref;
          java.lang.String s = bs.toStringUtf8();
          if (bs.isValidUtf8()) {
            data_ = s;
          }
          return s;
        } else {
          return (java.lang.String) ref;
        }
      }
      /**
       * <pre>
       *&#47;&lt; Internal field.
       * </pre>
       *
       * <code>optional string Data = 3;</code>
       */
      public com.google.protobuf.ByteString
          getDataBytes() {
        java.lang.Object ref = data_;
        if (ref instanceof String) {
          com.google.protobuf.ByteString b = 
              com.google.protobuf.ByteString.copyFromUtf8(
                  (java.lang.String) ref);
          data_ = b;
          return b;
        } else {
          return (com.google.protobuf.ByteString) ref;
        }
      }
      /**
       * <pre>
       *&#47;&lt; Internal field.
       * </pre>
       *
       * <code>optional string Data = 3;</code>
       */
      public Builder setData(
          java.lang.String value) {
        if (value == null) {
    throw new NullPointerException();
  }
  bitField0_ |= 0x00000004;
        data_ = value;
        onChanged();
        return this;
      }
      /**
       * <pre>
       *&#47;&lt; Internal field.
       * </pre>
       *
       * <code>optional string Data = 3;</code>
       */
      public Builder clearData() {
        bitField0_ = (bitField0_ & ~0x00000004);
        data_ = getDefaultInstance().getData();
        onChanged();
        return this;
      }
      /**
       * <pre>
       *&#47;&lt; Internal field.
       * </pre>
       *
       * <code>optional string Data = 3;</code>
       */
      public Builder setDataBytes(
          com.google.protobuf.ByteString value) {
        if (value == null) {
    throw new NullPointerException();
  }
  bitField0_ |= 0x00000004;
        data_ = value;
        onChanged();
        return this;
      }
      public final Builder setUnknownFields(
          final com.google.protobuf.UnknownFieldSet unknownFields) {
        return super.setUnknownFields(unknownFields);
      }

      public final Builder mergeUnknownFields(
          final com.google.protobuf.UnknownFieldSet unknownFields) {
        return super.mergeUnknownFields(unknownFields);
      }


      // @@protoc_insertion_point(builder_scope:ChatMessage)
    }

    // @@protoc_insertion_point(class_scope:ChatMessage)
    private static final ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage DEFAULT_INSTANCE;
    static {
      DEFAULT_INSTANCE = new ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage();
    }

    public static ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage getDefaultInstance() {
      return DEFAULT_INSTANCE;
    }

    @java.lang.Deprecated public static final com.google.protobuf.Parser<ChatMessage>
        PARSER = new com.google.protobuf.AbstractParser<ChatMessage>() {
      public ChatMessage parsePartialFrom(
          com.google.protobuf.CodedInputStream input,
          com.google.protobuf.ExtensionRegistryLite extensionRegistry)
          throws com.google.protobuf.InvalidProtocolBufferException {
        return new ChatMessage(input, extensionRegistry);
      }
    };

    public static com.google.protobuf.Parser<ChatMessage> parser() {
      return PARSER;
    }

    @java.lang.Override
    public com.google.protobuf.Parser<ChatMessage> getParserForType() {
      return PARSER;
    }

    public ru.leti.chat.utils.protoMessage.ProtoMessage.ChatMessage getDefaultInstanceForType() {
      return DEFAULT_INSTANCE;
    }

  }

  private static final com.google.protobuf.Descriptors.Descriptor
    internal_static_ChatMessage_descriptor;
  private static final 
    com.google.protobuf.GeneratedMessageV3.FieldAccessorTable
      internal_static_ChatMessage_fieldAccessorTable;

  public static com.google.protobuf.Descriptors.FileDescriptor
      getDescriptor() {
    return descriptor;
  }
  private static  com.google.protobuf.Descriptors.FileDescriptor
      descriptor;
  static {
    java.lang.String[] descriptorData = {
      "\n\023proto_message.proto\"9\n\013ChatMessage\022\016\n\006" +
      "Sender\030\001 \002(\t\022\014\n\004Text\030\002 \002(\t\022\014\n\004Data\030\003 \001(\t" +
      "B/\n\037ru.leti.chat.utils.protoMessageB\014Pro" +
      "toMessage"
    };
    com.google.protobuf.Descriptors.FileDescriptor.InternalDescriptorAssigner assigner =
        new com.google.protobuf.Descriptors.FileDescriptor.    InternalDescriptorAssigner() {
          public com.google.protobuf.ExtensionRegistry assignDescriptors(
              com.google.protobuf.Descriptors.FileDescriptor root) {
            descriptor = root;
            return null;
          }
        };
    com.google.protobuf.Descriptors.FileDescriptor
      .internalBuildGeneratedFileFrom(descriptorData,
        new com.google.protobuf.Descriptors.FileDescriptor[] {
        }, assigner);
    internal_static_ChatMessage_descriptor =
      getDescriptor().getMessageTypes().get(0);
    internal_static_ChatMessage_fieldAccessorTable = new
      com.google.protobuf.GeneratedMessageV3.FieldAccessorTable(
        internal_static_ChatMessage_descriptor,
        new java.lang.String[] { "Sender", "Text", "Data", });
  }

  // @@protoc_insertion_point(outer_class_scope)
}
