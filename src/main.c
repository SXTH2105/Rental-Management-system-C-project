#include "raylib.h"
#include "ui.h"

int main(void) {
    InitWindow(1024, 680, "Rental Management System");
    SetTargetFPS(60);
    SetExitKey(0);

    AppState state;
    ui_init(&state);

    while (!WindowShouldClose() && state.current_screen != SCREEN_EXIT) {
        BeginDrawing();
        ClearBackground((Color){246, 248, 255, 255});

        switch (state.current_screen) {
            case SCREEN_LOGIN:              ui_screen_login(&state);              break;
            case SCREEN_LANDLORD_DASHBOARD: ui_screen_landlord_dashboard(&state); break;
            case SCREEN_LANDLORD_ROOMS:     ui_screen_landlord_rooms(&state);     break;
            case SCREEN_LANDLORD_GUESTS:    ui_screen_landlord_guests(&state);    break;
            case SCREEN_LANDLORD_BILLING:   ui_screen_landlord_billing(&state);   break;
            case SCREEN_LANDLORD_INVOICE:   ui_screen_landlord_invoice(&state);   break;
            case SCREEN_LANDLORD_REPORTS:   ui_screen_landlord_reports(&state);   break;
            case SCREEN_GUEST_STAY:         ui_screen_guest_stay(&state);         break;
            case SCREEN_GUEST_INVOICE:      ui_screen_guest_invoice(&state);      break;
            case SCREEN_GUEST_REPORT:       ui_screen_guest_report(&state);       break;
            default:                        state.current_screen = SCREEN_EXIT;   break;
        }

        EndDrawing();
    }

    ui_unload(&state);
    CloseWindow();

    return 0;
}
