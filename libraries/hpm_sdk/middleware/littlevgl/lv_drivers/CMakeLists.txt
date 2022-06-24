# Copyright 2021 hpmicro
# SPDX-License-Identifier: BSD-3-Clause

sdk_inc(${SOC})
sdk_src(${SOC}/lv_port_disp.c)
sdk_src_ifdef(CONFIG_LV_INDEV ${SOC}/lv_port_indev.c)
