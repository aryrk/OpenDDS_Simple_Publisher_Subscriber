#pragma once
#include "MessagerTypeSupportC.h"
#include <dds/DCPS/LocalObject.h>
#include <dds/DCPS/Marked_Default_Qos.h>

class MessageReaderListener
    : public virtual OpenDDS::DCPS::LocalObject<DDS::DataReaderListener>
{
public:
    void on_data_available(DDS::DataReader_ptr reader) override;

    void on_requested_deadline_missed(
        DDS::DataReader_ptr,
        const DDS::RequestedDeadlineMissedStatus&) override {}

    void on_requested_incompatible_qos(
        DDS::DataReader_ptr,
        const DDS::RequestedIncompatibleQosStatus&) override {}

    void on_sample_rejected(
        DDS::DataReader_ptr,
        const DDS::SampleRejectedStatus&) override {}

    void on_liveliness_changed(
        DDS::DataReader_ptr,
        const DDS::LivelinessChangedStatus&) override {}

    void on_subscription_matched(
        DDS::DataReader_ptr,
        const DDS::SubscriptionMatchedStatus&) override {}

    void on_sample_lost(
        DDS::DataReader_ptr,
        const DDS::SampleLostStatus&) override {}
};