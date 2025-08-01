#include "MessageReaderListener.h"
#include <ace/streams.h>

void MessageReaderListener::on_data_available(DDS::DataReader_ptr reader)
{
  ObjectTrackData::MessageDataReader_var track_reader = ObjectTrackData::MessageDataReader::_narrow(reader);
  if (!track_reader)
    return;

  // Use read() instead of take() for monitoring - data remains available for re-reading
  ObjectTrackData::MessageSeq track_msgs;
  DDS::SampleInfoSeq infos;

  DDS::ReturnCode_t ret = track_reader->read(track_msgs, infos, DDS::LENGTH_UNLIMITED,
                                             DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);

  if (ret == DDS::RETCODE_OK)
  {
    for (CORBA::ULong i = 0; i < track_msgs.length(); ++i)
    {
      if (infos[i].valid_data)
      {
        const ObjectTrackData::Message &track_msg = track_msgs[i];

        ACE_DEBUG((LM_INFO, ACE_TEXT("Received Track Data:\n")));
        ACE_DEBUG((LM_INFO, ACE_TEXT("  Source ID: %d, Source Name: %C\n"),
                   track_msg.sourceId(), track_msg.sourceName().c_str()));
        ACE_DEBUG((LM_INFO, ACE_TEXT("  Track ID: %d, Sensor Track ID: %C\n"),
                   track_msg.trackId(), track_msg.sensorTrackId().c_str()));

        ACE_DEBUG((LM_INFO, ACE_TEXT("  Position: Lat=%.6f, Lon=%.6f\n"),
                   track_msg.latitude(), track_msg.longitude()));

        ACE_DEBUG((LM_INFO, ACE_TEXT("  Bearing: %.2f°, Range: %.2f\n"),
                   track_msg.bearing(), track_msg.range()));

        ACE_DEBUG((LM_INFO, ACE_TEXT("  Altitude: %.2f, Course: %.2f°\n"),
                   track_msg.altitude(), track_msg.course()));

        ACE_DEBUG((LM_INFO, ACE_TEXT("  Speed: %.2f kts, Vertical Speed: %.2f\n"),
                   track_msg.speed(), track_msg.verticalSpeed()));

        ACE_DEBUG((LM_INFO, ACE_TEXT("  Time: %lld\n"), track_msg.time()));
        ACE_DEBUG((LM_INFO, ACE_TEXT("---\n")));
      }
    }

    // Return the loaned samples
    track_reader->return_loan(track_msgs, infos);
  }
}