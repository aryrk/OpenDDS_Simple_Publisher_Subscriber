#include "MessagerTypeSupportImpl.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/PublisherImpl.h>
#include <ace/streams.h>
#include <ace/OS_NS_unistd.h>
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

        DDS::Publisher_var publisher =
            participant->create_publisher(PUBLISHER_QOS_DEFAULT,
                                          DDS::PublisherListener::_nil(),
                                          ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(publisher.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: Publisher creation failed.\n")), 1);

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

        DDS::DataWriterQos dw_qos;
        publisher->get_default_datawriter_qos(dw_qos);

        DDS::DataWriter_var writer =
            publisher->create_datawriter(topic.in(),
                                         dw_qos,
                                         DDS::DataWriterListener::_nil(),
                                         ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(writer.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: DataWriter creation failed.\n")), 1);

        DDS::DataWriter_var exchange_evt_writer =
            publisher->create_datawriter(exchange_evt_topic.in(),
                                         dw_qos,
                                         DDS::DataWriterListener::_nil(),
                                         ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(exchange_evt_writer.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: Exchange Event DataWriter creation failed.\n")), 1);

        Messager::MessageDataWriter_var exchange_evt_data_writer =
            Messager::MessageDataWriter::_narrow(exchange_evt_writer.in());
        if (CORBA::is_nil(exchange_evt_data_writer.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: Narrowing Exchange Event DataWriter failed.\n")), 1);

        for (int i = 0; i < 10; ++i)
        {
            Messager::Message message;
            message.id = i + 1;
            message.content = "Hello, OpenDDS!";
            message.sender = "Publisher";
            message.timestamp = get_timestamp();

            DDS::ReturnCode_t ret = exchange_evt_data_writer->write(message, DDS::HANDLE_NIL);
            if (ret != DDS::RETCODE_OK)
            {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: Failed to write message: %d\n"), ret), 1);
            }

            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) INFO: Message %d written successfully.\n"), i + 1));

            ACE_OS::sleep(1);
        }
    }
    catch (const CORBA::Exception &ex)
    {
        ex._tao_print_exception("ERROR: Exception caught in DomainParticipant creation:");
        return 1;
    }

    // clean up
    try
    {
        if (!CORBA::is_nil(participant.in()))
        {
            participant->delete_contained_entities();
            dpf->delete_participant(participant.in());
        }
    }
    catch (const CORBA::Exception &ex)
    {
        ex._tao_print_exception("ERROR: Exception caught during cleanup:");
        return 1;
    }

    TheServiceParticipant->shutdown();
    return 0;
}