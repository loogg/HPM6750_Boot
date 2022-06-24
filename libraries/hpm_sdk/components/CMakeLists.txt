# Copyright 2021 hpmicro
# SPDX-License-Identifier: BSD-3-Clause

sdk_inc(serial_nor)
sdk_inc(usb)
sdk_inc(enet_phy/rtl8211)

add_subdirectory(debug_console)
add_subdirectory_ifdef(CONFIG_CAMERA camera)
add_subdirectory_ifdef(CONFIG_CODEC codec)
add_subdirectory_ifdef(CONFIG_USB_HOST usb/host)
add_subdirectory_ifdef(CONFIG_USB_DEVICE usb/device)
add_subdirectory_ifdef(CONFIG_ENET_RTL8211_MASTER enet_phy/rtl8211)
add_subdirectory_ifdef(CONFIG_ENET_DP83867_MASTER enet_phy/dp83867)
add_subdirectory_ifdef(CONFIG_ENET_DP83848_MASTER enet_phy/dp83848)
add_subdirectory_ifdef(CONFIG_TOUCH touch)
