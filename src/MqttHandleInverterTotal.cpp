// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2026 Thomas Basler and others
 */
#include "MqttHandleInverterTotal.h"
#include "Configuration.h"
#include "Datastore.h"
#include "MqttSettings.h"
#include <Hoymiles.h>
#include <powermeter/Controller.h>

MqttHandleInverterTotalClass MqttHandleInverterTotal;

MqttHandleInverterTotalClass::MqttHandleInverterTotalClass()
    : _loopTask(TASK_IMMEDIATE, TASK_FOREVER, std::bind(&MqttHandleInverterTotalClass::loop, this))
{
}

void MqttHandleInverterTotalClass::init(Scheduler& scheduler)
{
    scheduler.addTask(_loopTask);
    _loopTask.setInterval(Configuration.get().Mqtt.PublishInterval * TASK_SECOND);
    _loopTask.enable();
}

void MqttHandleInverterTotalClass::loop()
{
    // Update interval from config
    _loopTask.setInterval(Configuration.get().Mqtt.PublishInterval * TASK_SECOND);

    if (!MqttSettings.getConnected() || !Hoymiles.isAllRadioIdle()) {
        _loopTask.forceNextIteration();
        return;
    }

    MqttSettings.publish("ac/power", String(Datastore.getTotalAcPowerEnabled(), Datastore.getTotalAcPowerDigits()));
    MqttSettings.publish("ac/yieldtotal", String(Datastore.getTotalAcYieldTotalEnabled(), Datastore.getTotalAcYieldTotalDigits()));
    MqttSettings.publish("ac/yieldday", String(Datastore.getTotalAcYieldDayEnabled(), Datastore.getTotalAcYieldDayDigits()));
    MqttSettings.publish("ac/is_valid", String(Datastore.getIsAllEnabledReachable()));
    MqttSettings.publish("dc/power", String(Datastore.getTotalDcPowerEnabled(), Datastore.getTotalDcPowerDigits()));
    MqttSettings.publish("dc/irradiation", String(Datastore.getTotalDcIrradiation(), 3));
    MqttSettings.publish("dc/is_valid", String(Datastore.getIsAllEnabledReachable()));

    // Phase power for multi-phase inverters
    float totalAcPower1 = 0;
    float totalAcPower2 = 0;
    float totalAcPower3 = 0;
    bool hasPhases = false;
    for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
        auto inv = Hoymiles.getInverterByPos(i);
        if (inv != nullptr && inv->getEnablePolling() && inv->Statistics()->hasChannelFieldValue(TYPE_AC, CH0, FLD_PAC_1)) {
            hasPhases = true;
            totalAcPower1 += inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_PAC_1);
            totalAcPower2 += inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_PAC_2);
            totalAcPower3 += inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_PAC_3);
        }
    }
    if (hasPhases) {
        MqttSettings.publish("ac/power1", String(totalAcPower1, 1));
        MqttSettings.publish("ac/power2", String(totalAcPower2, 1));
        MqttSettings.publish("ac/power3", String(totalAcPower3, 1));
        MqttSettings.publish("ac/power_1", String(totalAcPower1, 1));
        MqttSettings.publish("ac/power_2", String(totalAcPower2, 1));
        MqttSettings.publish("ac/power_3", String(totalAcPower3, 1));
        MqttSettings.publish("ac/power_l1", String(totalAcPower1, 1));
        MqttSettings.publish("ac/power_l2", String(totalAcPower2, 1));
        MqttSettings.publish("ac/power_l3", String(totalAcPower3, 1));
    }

    if (Configuration.get().PowerMeter.Enabled && PowerMeter.isDataValid()) {
        float consumption = PowerMeter.getPowerTotal() + Datastore.getTotalAcPowerEnabled();
        MqttSettings.publish("consumption/power", String(consumption, 1));
    }
}
