#include "../c1-ril-compat.h"
#include <assert.h>
#include <stdint.h>
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(C1RilAppStatus) == 52, "CMC221 app ABI");
static_assert(sizeof(C1RilCardStatus) == 440, "CMC221 SIM v6 ABI");
#endif
int main() {
    C1RilCardStatus input = {};
    RIL_CardStatus_v6 output = {};
    input.card_state = RIL_CARDSTATE_PRESENT;
    input.gsm_umts_subscription_app_index = 0;
    input.cdma_subscription_app_index = -1;
    input.ims_subscription_app_index = 1;
    input.num_applications = 2;
    input.applications[0].standard.app_type = RIL_APPTYPE_USIM;
    input.applications[1].standard.app_type = RIL_APPTYPE_ISIM;
    input.applications[0].samsung[0] = 0x1234;
    input.applications[1].samsung[4] = 0x5678;
    assert(convertC1CardStatus(&input, sizeof(input), &output));
    assert(output.card_state == RIL_CARDSTATE_PRESENT);
    assert(output.num_applications == 2 && output.ims_subscription_app_index == 1);
    assert(output.applications[0].app_type == RIL_APPTYPE_USIM);
    assert(output.applications[1].app_type == RIL_APPTYPE_ISIM);
    assert(!convertC1CardStatus(&input, sizeof(input)-1, &output));
    assert(!convertC1CardStatus(nullptr, sizeof(input), &output));
    input.num_applications = RIL_CARD_MAX_APPS + 1;
    assert(!convertC1CardStatus(&input, sizeof(input), &output));
    input.num_applications = -1;
    assert(!convertC1CardStatus(&input, sizeof(input), &output));
    input.num_applications = 0;
    assert(convertC1CardStatus(&input, sizeof(input), &output));
}
