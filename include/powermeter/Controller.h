// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <powermeter/Provider.h>
#include <TaskSchedulerDeclarations.h>
#include <memory>
#include <mutex>
#include <optional>

namespace PowerMeters {

class Controller {
public:
    void init(Scheduler& scheduler);

    void updateSettings();

    float getPowerTotal() const;
    std::optional<float> getPowerL1() const;
    std::optional<float> getPowerL2() const;
    std::optional<float> getPowerL3() const;
    uint32_t getLastUpdate() const;
    bool isDataValid() const;

private:
    void loop();

    Task _loopTask;
    mutable std::mutex _mutex;
    std::unique_ptr<Provider> _upProvider = nullptr;
};

} // namespace PowerMeters

extern PowerMeters::Controller PowerMeter;
