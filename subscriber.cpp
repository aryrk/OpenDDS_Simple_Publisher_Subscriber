#include "MessagerTypeSupportImpl.h"
#include "MessageReaderListener.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <ace/streams.h>
#include <ace/OS_NS_unistd.h>

DDS::DomainId_t domain_id = 42;
const char *EXCHANGE_EVT_TOPIC_NAME = "ExchangeEventTopic";

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
    try
    {
        // Create DomainParticipant
        DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);
        DDS::DomainParticipant_var participant =
            dpf->create_participant(domain_id,
                                    PARTICIPANT_QOS_DEFAULT,
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
        Messager::MessageTypeSupport_var mts = new Messager::MessageTypeSupportImpl();
        if (mts->register_type(participant.in(), "") != DDS::RETCODE_OK)
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Failed to register type.\n")), 1);

        // Create topic
        DDS::TopicQos topic_qos;
        participant->get_default_topic_qos(topic_qos);
        DDS::Topic_var exchange_evt_topic =
            participant->create_topic(EXCHANGE_EVT_TOPIC_NAME,
                                      mts->get_type_name(),
                                      topic_qos,
                                      DDS::TopicListener::_nil(),
                                      ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(exchange_evt_topic.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Exchange Event Topic creation failed.\n")), 1);

        // Create DataReader with listener
        DDS::DataReaderQos dr_qos;
        subscriber->get_default_datareader_qos(dr_qos);

        dr_qos.deadline.period.sec = 2;
        dr_qos.deadline.period.nanosec = 0;

        DDS::DataReaderListener_var listener(new MessageReaderListener);

        DDS::DataReader_var exchange_evt_reader =
            subscriber->create_datareader(exchange_evt_topic.in(),
                                          dr_qos,
                                          listener.in(),
                                          ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(exchange_evt_reader.in()))
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