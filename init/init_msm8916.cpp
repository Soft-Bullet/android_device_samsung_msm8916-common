/*
   Copyright (c) 2017, The LineageOS Project

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <android-base/file.h>
#include <android-base/strings.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#define SIMSLOT_FILE "/proc/simslot_count"

#include <init_msm8916.h>

using android::base::ReadFileToString;
using android::base::Trim;
using ::android::init::property_set;

#define SERIAL_NUMBER_FILE "/efs/FactoryApp/serial_no"

__attribute__ ((weak))
void init_target_properties()
{
}

constexpr const char* RO_PROP_SOURCES[] = {
        nullptr, "product.", "product_services.", "odm.", "vendor.", "system.", "bootimage.",
};

void property_override(char const prop[], char const value[]) {
    prop_info* pi = (prop_info*)__system_property_find(prop);
    if (pi) {
        __system_property_update(pi, value, strlen(value));
    }
}

/* Read the file at filename and returns the integer
 * value in the file.
 *
 * @prereq: Assumes that integer is non-negative.
 *
 * @return: integer value read if succesful, -1 otherwise. */
int read_integer(const char* filename)
{
    int retval;
    FILE * file;

    /* open the file */
    if (!(file = fopen(filename, "r"))) {
        return -1;
    }
    /* read the value from the file */
    fscanf(file, "%d", &retval);
    fclose(file);

    return retval;
}

void set_cdma_properties(const char *operator_alpha, const char *operator_numeric, const char * network)
{
    /* Dynamic CDMA Properties */
    property_set("ro.cdma.home.operator.alpha", operator_alpha);
    property_set("ro.cdma.home.operator.numeric", operator_numeric);
    property_set("ro.telephony.default_network", network);

    /* Static CDMA Properties */
    property_set("ril.subscription.types", "NV,RUIM");
    property_set("ro.telephony.default_cdma_sub", "0");
    property_set("ro.telephony.get_imsi_from_sim", "true");
    property_set("ro.telephony.ril.config", "newDriverCallU,newDialCode");
    property_set("telephony.lteOnCdmaDevice", "1");
}

void set_dsds_properties()
{
    property_set("ro.multisim.simslotcount", "2");
    property_set("ro.telephony.ril.config", "simactivation");
    property_set("persist.radio.multisim.config", "dsds");
    property_set("rild.libpath2", "/system/lib/libsec-ril-dsds.so");
}

void set_gsm_properties()
{
    property_set("telephony.lteOnCdmaDevice", "0");
    property_set("ro.telephony.default_network", "9");
}

void set_lte_properties()
{
    property_set("persist.radio.lte_vrte_ltd", "1");
    property_set("telephony.lteOnCdmaDevice", "0");
    property_set("telephony.lteOnGsmDevice", "1");
    property_set("ro.telephony.default_network", "10");
}

void set_wifi_properties()
{
    property_set("ro.carrier", "wifi-only");
    property_set("ro.radio.noril", "1");
}

void set_target_properties(const char *device, const char *model)
{
    const auto ro_prop_override = [](const char* source, const char* prop, const char* value,
                                     bool product) {
        std::string prop_name = "ro.";

        if (product) prop_name += "product.";
        if (source != nullptr) prop_name += source;
        if (!product) prop_name += "build.";
        prop_name += prop;

        property_override(prop_name.c_str(), value);
    };

    for (const auto& source : RO_PROP_SOURCES) {
        ro_prop_override(source, "device",device, true);
        ro_prop_override(source, "model", model, true);
    }

    property_set("ro.ril.telephony.mqanelements", "6");

    /* check for multi-sim devices */

    /* check if the simslot count file exists */
    if (access(SIMSLOT_FILE, F_OK) == 0) {
        int sim_count = read_integer(SIMSLOT_FILE);

        /* set the dual sim props */
        if (sim_count == 2)
            set_dsds_properties();
    }

    char const *serial_number_file = SERIAL_NUMBER_FILE;
    std::string serial_number;

    if (ReadFileToString(serial_number_file, &serial_number)) {
        serial_number = Trim(serial_number);
        property_override("ro.serialno", serial_number.c_str());
    }
}

void vendor_load_properties(void)
{
    /* set the device properties */
    init_target_properties();
}
