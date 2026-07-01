#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "auth.h"

typedef enum {
    SCREEN_LOGIN = 0,
    SCREEN_LANDLORD_DASHBOARD,
    SCREEN_LANDLORD_ROOMS,
    SCREEN_LANDLORD_GUESTS,
    SCREEN_LANDLORD_BILLING,
    SCREEN_LANDLORD_INVOICE,
    SCREEN_LANDLORD_REPORTS,
    SCREEN_GUEST_STAY,
    SCREEN_GUEST_INVOICE,
    SCREEN_GUEST_REPORT,
    SCREEN_EXIT
} Screen;

enum {
    FLD_NONE = 0,
    FLD_LGN_USER, FLD_LGN_PHONE,
    FLD_RM_NAME,  FLD_RM_PRICE, FLD_RM_STATUS,
    FLD_GU_NAME,  FLD_GU_PHONE, FLD_GU_ROOM,
    FLD_BL_GUEST,
    FLD_BL_EPREV, FLD_BL_ECURR,
    FLD_BL_WPREV, FLD_BL_WCURR,
    FLD_BL_DUE,
    FLD_RT_ELEC,  FLD_RT_WATER,
    FLD_RP_DESC,  FLD_RP_STAT
};

typedef struct {
    Screen  current_screen;
    User    current_user;

    char login_username[50];
    char login_phone[20];

    char room_name[50];
    char room_price_str[20];
    char room_status_str[10];

    char guest_name[50];
    char guest_phone[20];
    char guest_room_str[10];

    char bill_guest_str[10];
    char bill_eprev_str[20];
    char bill_ecurr_str[20];
    char bill_wprev_str[20];
    char bill_wcurr_str[20];
    char bill_due_str[20];
    char rate_elec_str[20];
    char rate_water_str[20];

    char report_desc[200];
    char report_stat_str[5];

    int  active_field;
    char message[200];
    bool message_is_error;
    int  message_timer;

    int  sub_mode;
    int  selected_id;
    int  scroll_index;
} AppState;

void ui_init(AppState *state);
void ui_unload(AppState *state);

void ui_screen_login(AppState *state);
void ui_screen_landlord_dashboard(AppState *state);
void ui_screen_landlord_rooms(AppState *state);
void ui_screen_landlord_guests(AppState *state);
void ui_screen_landlord_billing(AppState *state);
void ui_screen_landlord_invoice(AppState *state);
void ui_screen_landlord_reports(AppState *state);
void ui_screen_guest_stay(AppState *state);
void ui_screen_guest_invoice(AppState *state);
void ui_screen_guest_report(AppState *state);

#endif
