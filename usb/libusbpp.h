#pragma once

#include <set>
#include <map>
#include <tuple>
#include <vector>
#include <list>
#include <functional>
#include <memory>

#include <libusb.h>

#include "events.h"
#include "usb_context.h"
#include "usb_device_handle.h"
#include "usb_device.h"

std::ostream& operator<<(std::ostream& os, const usbpp::usb_device& device);

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:
