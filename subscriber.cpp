#include "MessagerTypeSupportImpl.h"
#include "MessageReaderListener.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <ace/streams.h>
#include <ace/OS_NS_unistd.h>

DDS::DomainId_t domain_id = 0;
const char *TRACK_DATA_TOPIC_NAME = "ObjectTrackDataTopic";

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
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

        // Create Subscriber
        DDS::Subscriber_var subscriber =
            participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT,
                                           DDS::SubscriberListener::_nil(),
                                           ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (CORBA::is_nil(subscriber.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Subscriber creation failed.\n")), 1);

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

        // Create DataReader with listener
        DDS::DataReaderQos dr_qos;
        subscriber->get_default_datareader_qos(dr_qos);

        // dr_qos.ownership.kind = DDS::EXCLUSIVE_OWNERSHIP_QOS;
        dr_qos.ownership.kind = DDS::SHARED_OWNERSHIP_QOS;

        // Enhanced history QoS for monitoring - must match or be compatible with publisher
        dr_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
        dr_qos.history.depth = 100; // Match publisher's depth for monitoring

        // Reliability for guaranteed delivery
        dr_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;

        // Durability for late-joining subscribers
        dr_qos.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;

        DDS::DataReaderListener_var listener(new MessageReaderListener);

        DDS::DataReader_var track_data_reader =
            subscriber->create_datareader(track_data_topic.in(),
                                          dr_qos,
                                          listener.in(),
                                          ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(track_data_reader.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DataReader creation failed.\n")), 1);

        ACE_DEBUG((LM_INFO, ACE_TEXT("INFO: Subscriber running. Waiting for data...\n")));
        while (true)
        {
            ACE_OS::sleep(1);
        }
    }
    catch (const CORBA::Exception &ex)
    {
        ex._tao_print_exception("ERROR: Exception caught:");
        return 1;
    }
    return 0;
}