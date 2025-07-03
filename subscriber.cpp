#include "MessagerTypeSupportImpl.h"
#include "MessageReaderListener.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/SubscriberImpl.h>
#include <dds/DCPS/BuiltInTopicUtils.h>
#include <ace/streams.h>
#include "ace/OS_NS_unistd.h"
#include <orbsvcs/Time_Utilities.h>

#include "dds/DCPS/StaticIncludes.h"
DDS::DomainId_t domain_id = 42;
const char *MESSAGER_TOPIC_NAME = "MessageTopic";
const char *EXCHANGE_EVT_TOPIC_NAME = "ExchangeEventTopic";

TimeBase::TimeT get_timestamp()
{
    TimeBase::TimeT retval;
    ACE_hrtime_t t = ACE_OS::gethrtime();
    ORBSVCS_Time::hrtime_to_TimeT(retval, t);
    return retval;
}

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{

    DDS::DomainParticipantFactory_var dpf = DDS::DomainParticipantFactory::_nil();
    DDS::DomainParticipant_var participant = DDS::DomainParticipant::_nil();

    try
    {
        dpf = TheParticipantFactoryWithArgs(argc, argv);
        participant = dpf->create_participant(domain_id,
                                              PARTICIPANT_QOS_DEFAULT,
                                              DDS::DomainParticipantListener::_nil(),
                                              ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(participant.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: DomainParticipant creation failed.\n")), 1);

        DDS::Subscriber_var subscriber =
            participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT,
                                           DDS::SubscriberListener::_nil(),
                                           ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (CORBA::is_nil(subscriber.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: Subscriber creation failed.\n")), 1);


        Messager::MessageTypeSupport_var mts =
            new Messager::MessageTypeSupportImpl();
        if (mts->register_type(participant.in(), "") != DDS::RETCODE_OK)
        {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: Failed to register type.\n")), 1);
        }

        DDS::TopicQos topic_qos;
        participant->get_default_topic_qos(topic_qos);

        DDS::Topic_var topic =
            participant->create_topic(MESSAGER_TOPIC_NAME,
                                      mts->get_type_name(),
                                      topic_qos,
                                      DDS::TopicListener::_nil(),
                                      ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(topic.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: Topic creation failed.\n")), 1);

        DDS::Topic_var exchange_evt_topic =
            participant->create_topic(EXCHANGE_EVT_TOPIC_NAME,
                                      mts->get_type_name(),
                                      topic_qos,
                                      DDS::TopicListener::_nil(),
                                      ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(exchange_evt_topic.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: Exchange Event Topic creation failed.\n")), 1);

        DDS::DataReaderQos dr_qos;
        subscriber->get_default_datareader_qos(dr_qos);

        DDS::DataReaderListener_var listener(new MessageReaderListener);

        DDS::DataReader_var exchange_evt_reader =
            subscriber->create_datareader(exchange_evt_topic.in(),
                                          dr_qos,
                                          listener.in(),
                                          ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(exchange_evt_reader.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: Exchange Event DataReader creation failed.\n")), 1);

        // Wait for data (run forever, or until interrupted)
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) INFO: Subscriber running. Waiting for data...\n")));
        while (true)
        {
            ACE_OS::sleep(1);
        }
    }
    catch (const CORBA::Exception &ex)
    {
        ex._tao_print_exception("ERROR: Exception caught in DomainParticipant creation:");
        return 1;
    }

    return 0;
}