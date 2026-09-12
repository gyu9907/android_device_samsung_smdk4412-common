// SPDX-License-Identifier: Apache-2.0
// CMC221 SIM layout from c1skt cm-14.1 include/telephony/ril.h.
#pragma once
#include <stddef.h>
#include <string.h>
#include <telephony/ril.h>

struct C1RilAppStatus {
    RIL_AppStatus standard;
    int samsung[5];
};
struct C1RilCardStatus {
    RIL_CardState card_state;
    RIL_PinState universal_pin_state;
    int gsm_umts_subscription_app_index;
    int cdma_subscription_app_index;
    int ims_subscription_app_index;
    int num_applications;
    C1RilAppStatus applications[RIL_CARD_MAX_APPS];
};

// The returned strings remain owned by the vendor RIL for this callback.
static inline bool convertC1CardStatus(const void *response, size_t length,
                                      RIL_CardStatus_v6 *out) {
    if (!response || !out || length != sizeof(C1RilCardStatus))
        return false;
    const C1RilCardStatus *in = static_cast<const C1RilCardStatus *>(response);
    if (in->num_applications < 0 || in->num_applications > RIL_CARD_MAX_APPS)
        return false;
    memset(out, 0, sizeof(*out));
    out->card_state = in->card_state;
    out->universal_pin_state = in->universal_pin_state;
    out->gsm_umts_subscription_app_index = in->gsm_umts_subscription_app_index;
    out->cdma_subscription_app_index = in->cdma_subscription_app_index;
    out->ims_subscription_app_index = in->ims_subscription_app_index;
    out->num_applications = in->num_applications;
    for (int i = 0; i < in->num_applications; ++i)
        out->applications[i] = in->applications[i].standard;
    return true;
}
