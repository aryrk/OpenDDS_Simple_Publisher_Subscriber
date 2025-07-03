#include "MessageReaderListener.h"
#include <ace/streams.h>

void MessageReaderListener::on_data_available(DDS::DataReader_ptr reader) {
  Messager::MessageDataReader_var msg_reader = Messager::MessageDataReader::_narrow(reader);
  if (!msg_reader) return;

  Messager::Message msg;
  DDS::SampleInfo info;
  while (msg_reader->take_next_sample(msg, info) == DDS::RETCODE_OK) {
    if (info.valid_data) {
      ACE_DEBUG((LM_INFO, ACE_TEXT("Received: id=%d, content=%C, sender=%C\n"),
        msg.id, msg.content.in(), msg.sender.in()));
    }
  }
}