#include "MessagerTypeSupportImpl.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <ace/streams.h>
#include <ace/OS_NS_unistd.h>
#include <ace/Time_Value.h>
#include <cmath>

DDS::DomainId_t domain_id = 0;
const char *TRACK_DATA_TOPIC_NAME = "ObjectTrackDataTopic";

// Helper to get current timestamp in milliseconds
long long get_current_time_ms()
{
    ACE_Time_Value tv = ACE_OS::gettimeofday();
    return tv.sec() * 1000LL + tv.usec() / 1000;
}

// Generate random source name
std::string random_source_name()
{
    const char *sources[] = {"RADAR_01", "SONAR_02", "LIDAR_03", "CAMERA_04", "SENSOR_05"};
    return sources[rand() % 5];
}

// Generate random sensor track ID
std::string random_sensor_track_id()
{
    const char charset[] = "0123456789ABCDEF";
    std::string id = "TRK_";
    for (int i = 0; i < 6; ++i)
    {
        id += charset[rand() % (sizeof(charset) - 1)];
    }
    return id;
}

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
    srand(static_cast<unsigned int>(time(nullptr)));
    try
    {
        // Create DomainParticipant
        DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);

        // Configure participant QoS for XTypes
        DDS::DomainParticipantQos participant_qos;
        dpf->get_default_participant_qos(participant_qos);

        DDS::DomainParticipant_var participant =
            dpf->create_participant(domain_id,
                                    participant_qos,
                                    DDS::DomainParticipantListener::_nil(),
                                    ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(participant.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DomainParticipant creation failed.\n")), 1);

        // Register type
        ObjectTrackData::MessageTypeSupport_var mts = new ObjectTrackData::MessageTypeSupportImpl();
        if (mts->register_type(participant.in(), "") != DDS::RETCODE_OK)
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Failed to register type.\n")), 1);

        // Create topic
        DDS::TopicQos topic_qos;
        participant->get_default_topic_qos(topic_qos);
        DDS::Topic_var track_data_topic =
            participant->create_topic(TRACK_DATA_TOPIC_NAME,
                                      mts->get_type_name(),
                                      topic_qos,
                                      DDS::TopicListener::_nil(),
                                      ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(track_data_topic.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Track Data Topic creation failed.\n")), 1);

        // Create publisher and datawriter
        DDS::Publisher_var publisher =
            participant->create_publisher(PUBLISHER_QOS_DEFAULT,
                                          DDS::PublisherListener::_nil(),
                                          ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (CORBA::is_nil(publisher.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Publisher creation failed.\n")), 1);

        DDS::DataWriterQos dw_qos;
        publisher->get_default_datawriter_qos(dw_qos);

        // dw_qos.ownership.kind = DDS::EXCLUSIVE_OWNERSHIP_QOS;
        dw_qos.ownership.kind = DDS::SHARED_OWNERSHIP_QOS;

        // Enhanced history QoS for monitoring
        dw_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
        dw_qos.history.depth = 100; // Increased to keep more samples for monitoring

        // Reliability for guaranteed delivery
        dw_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
        dw_qos.reliability.max_blocking_time.sec = 1;
        dw_qos.reliability.max_blocking_time.nanosec = 0;

        // Durability for late-joining subscribers
        dw_qos.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;

        int strength = 0;
        for (int i = 0; i < argc; ++i)
        {
            if (ACE_OS::strcmp(argv[i], ACE_TEXT("-strength")) == 0 && i + 1 < argc)
            {
                strength = ACE_OS::atoi(argv[i + 1]);
            }
        }
        dw_qos.ownership_strength.value = strength;

        DDS::DataWriter_var track_data_writer =
            publisher->create_datawriter(track_data_topic.in(),
                                         dw_qos,
                                         DDS::DataWriterListener::_nil(),
                                         ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (CORBA::is_nil(track_data_writer.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DataWriter creation failed.\n")), 1);

        ObjectTrackData::MessageDataWriter_var writer =
            ObjectTrackData::MessageDataWriter::_narrow(track_data_writer.in());
        if (CORBA::is_nil(writer.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Narrowing DataWriter failed.\n")), 1);

        // Publish track data messages
        std::string source_name = random_source_name();
        unsigned short source_id = 1 + (rand() % 10);
        unsigned short track_id = 1;

        while (true)
        {
            ObjectTrackData::Message track_message;

            // Set basic track info
            track_message.sourceId(source_id);
            track_message.sourceName(random_source_name());
            track_message.trackId(track_id++);
            track_message.sensorTrackId(random_sensor_track_id());
            track_message.time(get_current_time_ms());

            // Simulate some moving object data (treated as regular doubles despite @optional)
            static double base_lat = 37.7749; // San Francisco area
            static double base_lon = -122.4194;
            static double current_course = 45.0; // degrees
            static double current_speed = 25.0;  // knots

            // track_message.latitude(base_lat + (sin(track_id * 0.1) * 0.01));
            // track_message.longitude(base_lon + (cos(track_id * 0.1) * 0.01));
            track_message.bearing(fmod(track_id * 5.0, 360.0));
            track_message.range(1000.0 + (track_id * 50.0));
            // track_message.altitude(1000.0 + (sin(track_id * 0.2) * 500.0));
            // track_message.course(current_course);
            // track_message.speed(current_speed + (sin(track_id * 0.3) * 5.0));
            // track_message.verticalSpeed(cos(track_id * 0.15) * 10.0);

            DDS::ReturnCode_t ret = writer->write(track_message, DDS::HANDLE_NIL);
            if (ret != DDS::RETCODE_OK)
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Failed to write track message: %d\n"), ret), 1);

            ACE_DEBUG((LM_INFO, ACE_TEXT("INFO: Track message written - Source: %C, Track ID: %d, Sensor Track: %C\n"),
                       track_message.sourceName().c_str(), track_message.trackId(), track_message.sensorTrackId().c_str()));
            ACE_OS::sleep(1);
        }

        // Cleanup
        participant->delete_contained_entities();
        dpf->delete_participant(participant.in());
        TheServiceParticipant->shutdown();
    }
    catch (const CORBA::Exception &ex)
    {
        ex._tao_print_exception("ERROR: Exception caught:");
        return 1;
    }
    return 0;
}