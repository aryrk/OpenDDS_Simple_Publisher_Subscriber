#include "MessagerTypeSupportImpl.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <ace/streams.h>
#include <ace/OS_NS_unistd.h>
#include <orbsvcs/Time_Utilities.h>

DDS::DomainId_t domain_id = 42;
const char* EXCHANGE_EVT_TOPIC_NAME = "ExchangeEventTopic";

// Helper to get current timestamp
TimeBase::TimeT get_timestamp() {
    TimeBase::TimeT retval;
    ACE_hrtime_t t = ACE_OS::gethrtime();
    ORBSVCS_Time::hrtime_to_TimeT(retval, t);
    return retval;
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[]) {
    try {
        // Create DomainParticipant
        DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);
        DDS::DomainParticipant_var participant =
            dpf->create_participant(domain_id,
                                    PARTICIPANT_QOS_DEFAULT,
                                    DDS::DomainParticipantListener::_nil(),
                                    ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(participant.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DomainParticipant creation failed.\n")), 1);

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

        // Create publisher and datawriter
        DDS::Publisher_var publisher =
            participant->create_publisher(PUBLISHER_QOS_DEFAULT,
                                          DDS::PublisherListener::_nil(),
                                          ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (CORBA::is_nil(publisher.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Publisher creation failed.\n")), 1);

        DDS::DataWriterQos dw_qos;
        publisher->get_default_datawriter_qos(dw_qos);
        DDS::DataWriter_var exchange_evt_writer =
            publisher->create_datawriter(exchange_evt_topic.in(),
                                         dw_qos,
                                         DDS::DataWriterListener::_nil(),
                                         ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (CORBA::is_nil(exchange_evt_writer.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DataWriter creation failed.\n")), 1);

        Messager::MessageDataWriter_var writer =
            Messager::MessageDataWriter::_narrow(exchange_evt_writer.in());
        if (CORBA::is_nil(writer.in()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Narrowing DataWriter failed.\n")), 1);

        // Publish messages
        for (int i = 0; i < 10; ++i) {
            Messager::Message message;
            message.id = i + 1;
            message.content = "Hello, OpenDDS!";
            message.sender = "Publisher";
            message.timestamp = get_timestamp();

            DDS::ReturnCode_t ret = writer->write(message, DDS::HANDLE_NIL);
            if (ret != DDS::RETCODE_OK)
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Failed to write message: %d\n"), ret), 1);

            ACE_DEBUG((LM_INFO, ACE_TEXT("INFO: Message %d written successfully.\n"), i + 1));
            ACE_OS::sleep(1);
        }

        // Cleanup
        participant->delete_contained_entities();
        dpf->delete_participant(participant.in());
        TheServiceParticipant->shutdown();
    }
    catch (const CORBA::Exception& ex) {
        ex._tao_print_exception("ERROR: Exception caught:");
        return 1;
    }
    return 0;
}