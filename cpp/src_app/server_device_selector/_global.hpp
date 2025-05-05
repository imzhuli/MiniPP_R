#pragma once
#include "./device_context_manager.hpp"
#include "./device_selector.hpp"

#include <pp_common/base.hpp>

extern xNetAddress BindAddress;
extern xNetAddress DeviceDispatcherAddress;

extern xDS_DeviceContextManager  DeviceContextManager;
extern xDS_DeviceSelectorService DeviceSelectorService;
extern xDS_DeviceObserver        DeviceObserver;

extern void LoadConfig(const char * filename);
