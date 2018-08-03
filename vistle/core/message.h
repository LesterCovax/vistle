#ifndef MESSAGE_H
#define MESSAGE_H

#include <array>
#include <cassert>

#include <util/enum.h>
#include <util/directory.h> // ModuleNameLength
#include "uuid.h"
#include "export.h"

#pragma pack(push)
#pragma pack(1)

namespace vistle {

namespace message {

DEFINE_ENUM_WITH_STRING_CONVERSIONS(CompressionMode,
                                    (CompressionNone)
                                    (CompressionLz4)
                                    (CompressionZstd)
                                    (CompressionSnappy))

DEFINE_ENUM_WITH_STRING_CONVERSIONS(Type,
      (INVALID) // keep 1st
      (ANY) //< for Trace: enables tracing of all message types -- keep 2nd
      (IDENTIFY)
      (ADDHUB)
      (REMOVESLAVE)
      (SETID)
      (TRACE)
      (SPAWN)
      (SPAWNPREPARED)
      (KILL)
      (DEBUG)
      (QUIT)
      (STARTED)
      (MODULEEXIT)
      (BUSY)
      (IDLE)
      (EXECUTIONPROGRESS)
      (EXECUTE)
      (CANCELEXECUTE)
      (ADDOBJECT)
      (ADDOBJECTCOMPLETED)
      (DATATRANSFERSTATE)
      (ADDPORT)
      (REMOVEPORT)
      (CONNECT)
      (DISCONNECT)
      (ADDPARAMETER)
      (REMOVEPARAMETER)
      (SETPARAMETER)
      (SETPARAMETERCHOICES)
      (PING)
      (PONG)
      (BARRIER)
      (BARRIERREACHED)
      (SENDTEXT)
      (UPDATESTATUS)
      (OBJECTRECEIVEPOLICY)
      (SCHEDULINGPOLICY)
      (REDUCEPOLICY)
      (MODULEAVAILABLE)
      (LOCKUI)
      (REPLAYFINISHED)
      (REQUESTTUNNEL)
      (REQUESTOBJECT)
      (SENDOBJECT)
      (REMOTERENDERING)
      (FILEQUERY)
      (FILEQUERYRESULT)
      (NumMessageTypes) // keep last
)
V_ENUM_OUTPUT_OP(Type, ::vistle::message)

struct Id {

   enum Reserved {
      ModuleBase = 1, //< >= ModuleBase: modules
      Invalid = 0,
      Vistle = -1, //< session parameters
      Broadcast = -2, //< master is broadcasting to all modules/hubs
      ForBroadcast = -3, //< to master for broadcasting
      NextHop = -4,
      UI = -5,
      LocalManager = -6,
      LocalHub = -7,
      MasterHub = -8, //< < MasterHub: slave hubs
   };

   static bool isHub(int id) { return id <= Id::LocalHub; }
   static bool isModule(int id) { return id >= Id::ModuleBase; }
};

class V_COREEXPORT DefaultSender {

   public:
      DefaultSender();
      static void init(int id, int rank);
      static const DefaultSender &instance();
      static int id();
      static int rank();

   private:
      int m_id;
      int m_rank;
      static DefaultSender s_instance;
};

typedef std::array<char, ModuleNameLength> module_name_t;
typedef std::array<char, 32> port_name_t;
typedef std::array<char, 32> param_name_t;
typedef std::array<char, 256> param_value_t;
typedef std::array<char, 512> param_desc_t;
typedef std::array<char, 50> param_choice_t;
const int param_num_choices = 18;
typedef std::array<char, 300> shmsegname_t;
typedef std::array<char, 800> text_t;
typedef std::array<char, 300> address_t;

typedef boost::uuids::uuid uuid_t;


class V_COREEXPORT Message {
   // this is POD

 public:
   static const size_t MESSAGE_SIZE = 1024; // fixed message size is imposed by boost::interprocess::message_queue

   Message(const Type type, const unsigned int size);
   // Message (or its subclasses) may not require destructors

   //! processing flags for messages of a type, composed of RoutingFlags
   unsigned long typeFlags() const;

   //! set message uuid
   void setUuid(const uuid_t &uuid);
   //! message uuid
   const uuid_t &uuid() const;
   //! set uuid of message which this message reacts to
   void setReferrer(const uuid_t &ref);
   //! message this message refers to
   const uuid_t &referrer() const;
   //! message type
   Type type() const;
   //! sender ID
   int senderId() const;
   //! set sender ID
   void setSenderId(int id);
   //! sender rank
   int rank() const;
   //! set sender rank
   void setRank(int rank);
   //! UI id, if sent from a UI
   int uiId() const;
   //! messge size
   size_t size() const;
   //! broadacast to all ranks?
   bool isBroadcast() const;
   //! mark message for broadcast to all ranks on destination
   void setBroadcast(bool enable=true);
   //! message is not a request for action, just a notification that an action has been taken
   bool isNotification() const;
   //! mark message as notification
   void setNotify(bool enable);

   //! id of message destination
   int destId() const;
   //! set id of message destination
   void setDestId(int id);

   //! rank of message destination
   int destRank() const;
   //! set rank of destination
   void setDestRank(int r);
   //! id of destination UI
   int destUiId() const;
   //! set id of destination UI
   void setDestUiId(int id);
   //! number of additional data bytes following message
   size_t payloadSize() const;
   //! set payload size
   void setPayloadSize(size_t size);
   //! compression method for payload
   CompressionMode payloadCompression() const;
   //! set compression method for payload
   void setPayloadCompression(CompressionMode mode);
   //! number of uncompressed payload bytes
   size_t payloadRawSize() const;
   //! set number of uncompressed payload bytes
   void setPayloadRawSize(size_t size);

   template<class SomeMessage>
   SomeMessage &as() { SomeMessage *m = static_cast<SomeMessage *>(this); assert(m->type()==SomeMessage::s_type); return *m; }
   template<class SomeMessage>
   SomeMessage const &as() const { const SomeMessage *m = static_cast<const SomeMessage *>(this); assert(m->type()==SomeMessage::s_type); return *m; }

 private:
   //! message type
   Type m_type;
   //! message size
   unsigned int m_size;
   //! sender ID
   int m_senderId;
   //! sender rank
   int m_rank;
   //! destination ID
   int m_destId;
   //! destination rank - -1: dependent on message, most often current rank
   int m_destRank;
   //! message uuid
   uuid_t m_uuid;
   //! uuid of triggering message
   uuid_t m_referrer;
  protected:
   //! payload size
   uint64_t m_payloadSize;
   //! raw (uncompressed) payload size
   uint64_t m_payloadRawSize;
   //! payload compression method
   int m_payloadCompression;
   //! broadcast to all ranks?
   bool m_broadcast;
   //! message is not a request for action
   bool m_notification;
   //! pad message to multiple of 8 bytes
   char m_pad[2];
};
// ensure alignment
static_assert(sizeof(Message) % sizeof(double)==0, "not padded to ensure double alignment");

class V_COREEXPORT Buffer: public Message {

  public:
   Buffer(): Message(ANY, Message::MESSAGE_SIZE) {
      memset(payload.data(), 0, payload.size());
   }
   Buffer(const Message &message): Message(message) {
      memset(payload.data(), 0, payload.size());
      memcpy(payload.data(), (char *)&message+sizeof(Message), message.size()-sizeof(Message));
   }
   const Buffer &operator=(const Buffer &rhs) {
      *static_cast<Message *>(this) = rhs;
      memcpy(payload.data(), rhs.payload.data(), payload.size());
      return *this;
   }

   template<class SomeMessage>
   SomeMessage &as() { SomeMessage *m = static_cast<SomeMessage *>(static_cast<Message *>(this)); assert(m->type()==SomeMessage::s_type); return *m; }
   template<class SomeMessage>
   SomeMessage const &as() const { const SomeMessage *m = static_cast<const SomeMessage *>(static_cast<const Message *>(this)); assert(m->type()==SomeMessage::s_type); return *m; }

   size_t bufferSize() const { return Message::MESSAGE_SIZE; }
   size_t size() const { return Message::size(); }
   char *data() { return static_cast<char *>(static_cast<void *>(this)); }
   const char *data() const { return static_cast<const char *>(static_cast<const void *>(this)); }

  private:
   std::array<char, Message::MESSAGE_SIZE - sizeof(Message)> payload;

};
static_assert(sizeof(Buffer) == Message::MESSAGE_SIZE, "message too large");

template<class MessageClass, Type MessageType>
class MessageBase: public Message {
public:
    static const Type s_type = MessageType;
protected:
    MessageBase(): Message(MessageType, sizeof(MessageClass)) {
    }
};

V_COREEXPORT std::vector<char> compressPayload(vistle::message::CompressionMode mode, Message &msg, std::vector<char> &raw, int speed=-1 /* algorithm default */);
V_COREEXPORT std::vector<char> decompressPayload(const Message &msg, std::vector<char> &compressed);

V_COREEXPORT std::ostream &operator<<(std::ostream &s, const Message &msg);

class V_COREEXPORT MessageSender {
public:
    virtual bool sendMessage(const Message &msg) const = 0;
};

} // namespace message
} // namespace vistle

#pragma pack(pop)
#endif
