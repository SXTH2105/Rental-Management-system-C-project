#include "raylib.h"
#include <stdlib.h>
#ifndef TextToFloat
#define TextToFloat(text) ((float)atof(text))
#endif
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "auth.h"
#include "room.h"
#include "guest.h"
#include "billing.h"
#include "invoice.h"
#include "report.h"
#include "utils.h"
#include "rates.h"

//color
static const Color C_PANEL  = {246, 248, 255, 255};
static const Color C_CARD   = {235, 240, 250, 255};
static const Color C_BTN    = {35,  206, 107, 255};
static const Color C_ACCENT = {35,  206, 107, 255};
static const Color C_GREEN  = {35,  206, 107, 255};
static const Color C_YELLOW = {210, 150, 0,   255};
static const Color C_RED    = {187, 45,  45,  255};
static const Color C_TEXT   = {39,  45,  45,  255};
static const Color C_DIM    = {80,  81,  79,  255};
static const Color C_BORDER = {180, 185, 200, 255};

//layout
#define WIN_W  1024
#define WIN_H   680
#define SBW     185
#define HDRH     60
#define P        10
#define ROW_H    36
#define BTN_H    34
#define INP_H    32
#define FS       15
#define FS_SM    12
#define FS_LG    18
#define FS_XL    22
#define MAX_VIS  12

#define LX  (SBW + P)
#define LY  (HDRH + P)
#define LW   496
#define LH   510
#define FX  (LX + LW + P)
#define FY   LY
#define FW  (WIN_W - FX - P)
#define FH   LH
#define ACTY (LY + LH + 8)
#define MSGY (WIN_H - 44)

// Custom font (loaded once in ui_init, used everywhere)
static Font g_font = {0};

// Drop-in replacements for DrawText / MeasureText that use g_font
static inline void TXT(const char *s, int x, int y, int sz, Color c) {
    DrawTextEx(g_font, s, (Vector2){(float)x, (float)y}, (float)sz, 1.0f, c);
}
static inline int TW(const char *s, int sz) {
    return (int)MeasureTextEx(g_font, s, (float)sz, 1.0f).x;
}


//helper functions

static void apply_theme(void) {
    GuiSetStyle(DEFAULT, TEXT_SIZE, FS);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL,   ColorToInt(C_TEXT));
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED,  ColorToInt(C_TEXT));
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(C_BORDER));
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED,ColorToInt(C_ACCENT));
    
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL,    ColorToInt(C_BTN));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED,   ColorToInt((Color){80, 81, 79, 255}));
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED,   ColorToInt(C_RED));
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL,    ColorToInt(C_TEXT));
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED,   ColorToInt(WHITE));
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED,   ColorToInt(WHITE));
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL,  ColorToInt(C_BORDER));
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, ColorToInt((Color){80, 81, 79, 255}));
    GuiSetStyle(BUTTON, BORDER_WIDTH,         1);
    
    GuiSetStyle(TEXTBOX, BASE_COLOR_NORMAL,    ColorToInt(C_PANEL));
    GuiSetStyle(TEXTBOX, BASE_COLOR_FOCUSED,   ColorToInt(WHITE));
    GuiSetStyle(TEXTBOX, TEXT_COLOR_NORMAL,    ColorToInt(C_TEXT));
    GuiSetStyle(TEXTBOX, TEXT_COLOR_FOCUSED,   ColorToInt(C_TEXT));
    GuiSetStyle(TEXTBOX, BORDER_COLOR_NORMAL,  ColorToInt(C_BORDER));
    GuiSetStyle(TEXTBOX, BORDER_COLOR_FOCUSED, ColorToInt(C_ACCENT));
    GuiSetStyle(TEXTBOX, BORDER_WIDTH,         1);
}

void ui_init(AppState *s) {
    memset(s, 0, sizeof(AppState));
    s->current_screen = SCREEN_LOGIN;
    g_font = LoadFontEx("resource/Inter-VariableFont_opsz,wght.ttf", 32, NULL, 0);
    if (!IsFontReady(g_font)) {
        TraceLog(LOG_WARNING, "Inter font not found, using raylib default");
        g_font = GetFontDefault();
    } else {
        SetTextureFilter(g_font.texture, TEXTURE_FILTER_BILINEAR);
    }
    GuiSetFont(g_font);
    apply_theme();
}

void ui_unload(AppState *s) {
    (void)s;
    if (IsFontReady(g_font) && g_font.texture.id != GetFontDefault().texture.id) {
        UnloadFont(g_font);
    }
}

static void set_msg(AppState *s, bool err, const char *msg) {
    strncpy(s->message, msg, sizeof(s->message) - 1);
    s->message[sizeof(s->message) - 1] = '\0';
    s->message_is_error = err;
    s->message_timer = 240;
}

static void draw_msg(AppState *s) {
    if (s->message_timer <= 0) return;
    s->message_timer--;
    Color c = s->message_is_error ? C_RED : C_GREEN;
    DrawRectangle(0, MSGY, WIN_W, 44, c);
    TXT(s->message, P * 2, MSGY + 14, FS, WHITE);
}

static void draw_hdr(const char *title, const char *uname) {
    DrawRectangle(0, 0, WIN_W, HDRH, C_PANEL);
    DrawLine(0, HDRH, WIN_W, HDRH, C_BORDER);
    TXT(title, P * 2, HDRH / 2 - FS_XL / 2, FS_XL, C_TEXT);
    if (uname && uname[0]) {
        char buf[80];
        snprintf(buf, sizeof(buf), "User: %s", uname);
        int tw = TW(buf, FS_SM);
        TXT(buf, WIN_W - tw - P * 2, HDRH / 2 - FS_SM / 2, FS_SM, C_DIM);
    }
}

static void draw_panel(int x, int y, int w, int h, const char *title) {
    DrawRectangle(x, y, w, h, C_PANEL);
    DrawRectangleLinesEx((Rectangle){x, y, w, h}, 1, C_BORDER);
    if (title && title[0]) {
        TXT(title, x + P, y + 12, FS_LG, C_ACCENT);
        DrawLine(x, y + 40, x + w, y + 40, C_BORDER);
    }
}

static bool danger_btn(Rectangle r, const char *lbl) {
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL,  ColorToInt(C_RED));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt((Color){200,50,70,255}));
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL,  ColorToInt(WHITE));
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, ColorToInt(WHITE));
    bool res = GuiButton(r, lbl);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL,  ColorToInt(C_BTN));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt((Color){80, 81, 79, 255}));
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL,  ColorToInt(C_TEXT));
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, ColorToInt(WHITE));
    return res;
}

static const char *room_stat(int status, Color *c) {
    if (status == ROOM_OCCUPIED)    { *c = C_RED;    return "Occupied"; }
    if (status == ROOM_MAINTENANCE) { *c = C_YELLOW; return "Maintenance"; }
    *c = C_GREEN; return "Available";
}

static const char *rep_stat(int status, Color *c) {
    if (status == REPORT_IN_PROGRESS) { *c = C_YELLOW; return "In Progress"; }
    if (status == REPORT_RESOLVED)    { *c = C_GREEN;  return "Resolved"; }
    *c = C_RED; return "Pending";
}

static void draw_sidebar(AppState *s, int active) {
    DrawRectangle(0, HDRH, SBW, WIN_H - HDRH, C_PANEL);
    DrawLine(SBW, HDRH, SBW, WIN_H, C_BORDER);

    const char *labels[] = {"Dashboard","Rooms","Guests","Billing","Invoices","Reports"};
    const Screen scr[]   = {
        SCREEN_LANDLORD_DASHBOARD, SCREEN_LANDLORD_ROOMS, SCREEN_LANDLORD_GUESTS,
        SCREEN_LANDLORD_BILLING,   SCREEN_LANDLORD_INVOICE, SCREEN_LANDLORD_REPORTS
    };

    for (int i = 0; i < 6; i++) {
        int by = HDRH + 14 + i * 50;
        Rectangle r = {8, by, SBW - 16, 38};
        bool hov = CheckCollisionPointRec(GetMousePosition(), r);
        Color bg = (i == active) ? C_ACCENT : (hov ? C_CARD : C_PANEL);
        Color tx = C_TEXT;
        DrawRectangleRec(r, bg);
        if (i == active) {
            DrawRectangleLinesEx(r, 1, C_BORDER);
        } else if (hov) {
            DrawRectangleLinesEx(r, 1, C_ACCENT);
        }
        int tw = TW(labels[i], FS);
        TXT(labels[i], 8 + (SBW - 16 - tw) / 2, by + (38 - FS) / 2, FS, tx);
        if (i != active && hov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            s->current_screen = scr[i];
            s->sub_mode = 0; s->selected_id = 0;
            s->active_field = FLD_NONE; s->scroll_index = 0;
        }
    }
    Rectangle lr = {8, WIN_H - 52, SBW - 16, 38};
    bool lhov = CheckCollisionPointRec(GetMousePosition(), lr);
    DrawRectangleRec(lr, lhov ? C_RED : C_PANEL);
    if (lhov) {
        DrawRectangleLinesEx(lr, 1, C_BORDER);
    }
    Color ltx = lhov ? WHITE : C_TEXT;
    int tw = TW("Logout", FS);
    TXT("Logout", 8 + (SBW - 16 - tw) / 2, WIN_H - 52 + (38 - FS) / 2, FS, ltx);
    if (lhov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        s->current_screen = SCREEN_LOGIN;
        s->sub_mode = 0; s->selected_id = 0; s->active_field = FLD_NONE;
    }
}

static void draw_col_hdr(int x, int y, int w, const char **cols, const int *cws, int n) {
    DrawRectangle(x, y, w, 26, C_CARD);
    int cx = x + 8;
    for (int i = 0; i < n; i++) {
        TXT(cols[i], cx, y + 6, FS_SM, C_DIM);
        cx += cws[i];
    }
    DrawLine(x, y + 26, x + w, y + 26, C_BORDER);
}

static bool draw_row(int x, int y, int w, int idx, bool sel) {
    Vector2 mp = GetMousePosition();
    bool hov = CheckCollisionPointRec(mp, (Rectangle){x, y, w, ROW_H - 2});
    Color bg = sel ? (Color){210, 245, 225, 255} : (hov ? (Color){230, 248, 238, 255} :
               (idx % 2 == 0 ? C_PANEL : C_CARD));
    DrawRectangle(x, y, w, ROW_H - 2, bg);
    if (sel) DrawRectangleLinesEx((Rectangle){x, y, w, ROW_H - 2}, 1, C_ACCENT);
    if (hov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return true;
    return false;
}

static void handle_scroll(AppState *s, int x, int y, int w, int h, int count) {
    if (count <= MAX_VIS) return;
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){x, y, w, h})) {
        float wh = GetMouseWheelMove();
        if (wh > 0 && s->scroll_index > 0) s->scroll_index--;
        if (wh < 0 && s->scroll_index < count - MAX_VIS) s->scroll_index++;
    }
}

//login page
void ui_screen_login(AppState *s) {
    const char *title = "RENTAL MANAGEMENT SYSTEM";
    int tw = TW(title, FS_XL + 4);
    TXT(title, (WIN_W - tw) / 2, 55, FS_XL + 4, C_TEXT);
    const char *sub = "Landlord & Guest Management Portal";
    int sw = TW(sub, FS);
    TXT(sub, (WIN_W - sw) / 2, 92, FS, C_DIM);
    DrawRectangle((WIN_W - 80) / 2, 118, 80, 3, C_ACCENT);

    int cw = 380, ch = 300, cx = (WIN_W - cw) / 2, cy = 148;
    DrawRectangle(cx, cy, cw, ch, C_PANEL);
    DrawRectangleLinesEx((Rectangle){cx, cy, cw, ch}, 1, C_BORDER);
    DrawRectangle(cx, cy, cw, 40, C_ACCENT);
    const char *lt = "SIGN IN";
    int ltw = TW(lt, FS_LG);
    TXT(lt, cx + (cw - ltw) / 2, cy + 11, FS_LG, C_TEXT);

    int fx = cx + 20, fw = cw - 40, fy = cy + 54;
    TXT("Username", fx, fy, FS_SM, C_DIM);
    if (GuiTextBox((Rectangle){fx, fy + 16, fw, INP_H}, s->login_username, 50,
                   s->active_field == FLD_LGN_USER))
        s->active_field = (s->active_field == FLD_LGN_USER) ? FLD_NONE : FLD_LGN_USER;
    fy += 68;
    TXT("Phone Number", fx, fy, FS_SM, C_DIM);
    if (GuiTextBox((Rectangle){fx, fy + 16, fw, INP_H}, s->login_phone, 20,
                   s->active_field == FLD_LGN_PHONE))
        s->active_field = (s->active_field == FLD_LGN_PHONE) ? FLD_NONE : FLD_LGN_PHONE;
    fy += 68;

    if (IsKeyPressed(KEY_TAB)) {
        if (s->active_field == FLD_LGN_USER)  s->active_field = FLD_LGN_PHONE;
        else                                   s->active_field = FLD_LGN_USER;
    }
    bool do_login = IsKeyPressed(KEY_ENTER) &&
        (s->active_field == FLD_LGN_USER || s->active_field == FLD_LGN_PHONE);

    if (GuiButton((Rectangle){fx, fy, fw, BTN_H + 4}, "LOGIN") || do_login) {
        User u;
        if (auth_login(s->login_username, s->login_phone, &u)) {
            s->current_user = u;
            memset(s->login_username, 0, sizeof(s->login_username));
            memset(s->login_phone, 0, sizeof(s->login_phone));
            s->active_field = FLD_NONE; s->sub_mode = 0;
            s->selected_id = 0; s->scroll_index = 0;
            s->current_screen = (u.role == ROLE_LANDLORD) ?
                SCREEN_LANDLORD_DASHBOARD : SCREEN_GUEST_STAY;
        } else {
            set_msg(s, true, "Login failed! Invalid username or phone.");
        }
    }
    fy += BTN_H + 12;
    const char *ex = "Exit Application";
    int etw = TW(ex, FS_SM);
    int ex2 = cx + (cw - etw) / 2;
    Color ec = CheckCollisionPointRec(GetMousePosition(),
               (Rectangle){ex2-4, fy, etw+8, FS_SM+6}) ? C_ACCENT : C_DIM;
    TXT(ex, ex2, fy + 2, FS_SM, ec);
    if (ec.r == C_ACCENT.r && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        s->current_screen = SCREEN_EXIT;
    draw_msg(s);
}

//dashboard page
void ui_screen_landlord_dashboard(AppState *s) {
    draw_hdr("Landlord Dashboard", s->current_user.username);
    draw_sidebar(s, 0);

    Room rooms[MAX_ROOMS]; int rc = 0;
    room_load(rooms, &rc);
    int avail = 0, occ = 0, maint = 0;
    for (int i = 0; i < rc; i++) {
        if (rooms[i].status == ROOM_AVAILABLE) avail++;
        else if (rooms[i].status == ROOM_OCCUPIED) occ++;
        else maint++;
    }

    int mx = SBW + P, my = HDRH + P, mw = WIN_W - SBW - P * 2;
    int sw = (mw - P * 2) / 3;
    int stats[] = {avail, occ, maint};
    const char *slbl[] = {"Available","Occupied","Maintenance"};
    Color scols[] = {C_GREEN, C_RED, C_YELLOW};
    for (int i = 0; i < 3; i++) {
        int sx = mx + i * (sw + P);
        DrawRectangle(sx, my, sw, 86, C_PANEL);
        DrawRectangleLinesEx((Rectangle){sx, my, sw, 86}, 2, scols[i]);
        TXT(slbl[i], sx + 12, my + 10, FS_SM, C_DIM);
        char num[8]; snprintf(num, sizeof(num), "%d", stats[i]);
        TXT(num, sx + 12, my + 28, 32, scols[i]);
        TXT("rooms", sx + 12, my + 64, FS_SM, C_DIM);
    }

    int gy = my + 100;
    TXT("Room Overview", mx, gy, FS_LG, C_TEXT); gy += 28;
    if (rc == 0) {
        TXT("No rooms yet. Go to Rooms to add some.", mx, gy, FS, C_DIM);
    } else {
        int gcols = 4, gcw = (mw - P * (gcols - 1)) / gcols, gch = 64;
        for (int i = 0; i < rc; i++) {
            int col = i % gcols, row = i / gcols;
            int gx = mx + col * (gcw + P), row_y = gy + row * (gch + 8);
            if (row_y + gch > WIN_H - 60) break;
            Color sc; const char *ss = room_stat(rooms[i].status, &sc);
            DrawRectangle(gx, row_y, gcw, gch, C_PANEL);
            DrawRectangleLinesEx((Rectangle){gx, row_y, gcw, gch}, 2, sc);
            TXT(rooms[i].name, gx + 10, row_y + 8, FS, C_TEXT);
            char ps[20]; snprintf(ps, sizeof(ps), "$%.0f/mo", rooms[i].price);
            TXT(ps, gx + 10, row_y + 28, FS_SM, C_DIM);
            TXT(ss, gx + 10, row_y + 46, FS_SM, sc);
        }
    }
    draw_msg(s);
}

//landlord page
void ui_screen_landlord_rooms(AppState *s) {
    draw_hdr("Room Management", s->current_user.username);
    draw_sidebar(s, 1);

    Room rooms[MAX_ROOMS]; int rc = 0;
    room_load(rooms, &rc);
    handle_scroll(s, LX, LY, LW, LH, rc);

    draw_panel(LX, LY, LW, LH, "Rooms");
    const char *cols[] = {"ID","Name","Price","Status"};
    const int   cws[]  = {35, 205, 110, 130};
    draw_col_hdr(LX, LY + 42, LW, cols, cws, 4);

    int ry0 = LY + 68, ve = s->scroll_index + MAX_VIS;
    if (ve > rc) ve = rc;
    for (int i = s->scroll_index; i < ve; i++) {
        int row = i - s->scroll_index, ry = ry0 + row * ROW_H;
        if (draw_row(LX, ry, LW, row, rooms[i].id == s->selected_id)) {
            s->selected_id = rooms[i].id; s->sub_mode = 0;
        }
        char b[32]; snprintf(b, sizeof(b), "%d", rooms[i].id);
        TXT(b, LX+8, ry+10, FS, C_TEXT);
        TXT(rooms[i].name, LX+43, ry+10, FS, C_TEXT);
        snprintf(b, sizeof(b), "%.2f", rooms[i].price);
        TXT(b, LX+248, ry+10, FS, C_TEXT);
        Color sc; const char *ss = room_stat(rooms[i].status, &sc);
        TXT(ss, LX+358, ry+10, FS, sc);
    }
    if (rc == 0) TXT("No rooms yet. Click '+ Add Room'.", LX+P, ry0+P, FS, C_DIM);
    if(rc>MAX_VIS){ TXT("Scroll: mouse wheel",LX+P,LY+LH-18,FS_SM,C_DIM); }

    if (GuiButton((Rectangle){LX, ACTY, 120, BTN_H}, "+ Add Room")) {
        s->sub_mode = 1;
        memset(s->room_name, 0, sizeof(s->room_name));
        memset(s->room_price_str, 0, sizeof(s->room_price_str));
        memset(s->room_status_str, 0, sizeof(s->room_status_str));
        s->active_field = FLD_NONE;
    }
    if (s->selected_id > 0) {
        if (GuiButton((Rectangle){LX+130, ACTY, 90, BTN_H}, "Edit")) {
            int idx = room_find_by_id(rooms, rc, s->selected_id);
            if (idx != -1) {
                s->sub_mode = 2;
                strncpy(s->room_name, rooms[idx].name, 49);
                snprintf(s->room_price_str, 20, "%.2f", rooms[idx].price);
                snprintf(s->room_status_str, 10, "%d", rooms[idx].status);
                s->active_field = FLD_NONE;
            }
        }
        if (danger_btn((Rectangle){LX+230, ACTY, 90, BTN_H}, "Delete"))
            s->sub_mode = 3;
    }

    if (s->sub_mode == 1 || s->sub_mode == 2) {
        draw_panel(FX, FY, FW, FH, s->sub_mode == 1 ? "Add Room" : "Edit Room");
        int fx = FX+P, fw = FW-P*2, fy = FY+50;
        TXT("Room Name", fx, fy, FS_SM, C_DIM);
        if (GuiTextBox((Rectangle){fx, fy+16, fw, INP_H}, s->room_name, 50,
                       s->active_field == FLD_RM_NAME))
            s->active_field = (s->active_field==FLD_RM_NAME)?FLD_NONE:FLD_RM_NAME;
        fy += 64;
        TXT("Monthly Price", fx, fy, FS_SM, C_DIM);
        if (GuiTextBox((Rectangle){fx, fy+16, fw, INP_H}, s->room_price_str, 20,
                       s->active_field == FLD_RM_PRICE))
            s->active_field = (s->active_field==FLD_RM_PRICE)?FLD_NONE:FLD_RM_PRICE;
        fy += 64;
        TXT("Status  0=Available  1=Occupied  2=Maintenance", fx, fy, FS_SM, C_DIM);
        if (GuiTextBox((Rectangle){fx, fy+16, fw, INP_H}, s->room_status_str, 10,
                       s->active_field == FLD_RM_STATUS))
            s->active_field = (s->active_field==FLD_RM_STATUS)?FLD_NONE:FLD_RM_STATUS;
        fy += 64;
        if (GuiButton((Rectangle){fx, fy, 100, BTN_H}, "Save")) {
            if (!strlen(s->room_name)) { set_msg(s, true, "Name required!"); }
            else if (!strlen(s->room_price_str)) { set_msg(s, true, "Price required!"); }
            else {
                int sv = utils_str_to_int(s->room_status_str);
                if (sv < 0 || sv > 2) { set_msg(s, true, "Status must be 0-2"); }
                else if (s->sub_mode == 1) {
                    int ids[MAX_ROOMS]; for (int i=0;i<rc;i++) ids[i]=rooms[i].id;
                    Room r = {0};
                    r.id = utils_next_id(ids, rc);
                    strncpy(r.name, s->room_name, 49); utils_trim(r.name);
                    r.price = utils_str_to_float(s->room_price_str);
                    r.status = sv;
                    room_add(rooms, &rc, &r);
                    room_save(rooms, rc) ? set_msg(s,false,"Room added!") : set_msg(s,true,"Save failed!");
                    s->sub_mode = 0;
                } else {
                    int idx = room_find_by_id(rooms, rc, s->selected_id);
                    if (idx != -1) {
                        strncpy(rooms[idx].name, s->room_name, 49); utils_trim(rooms[idx].name);
                        rooms[idx].price = utils_str_to_float(s->room_price_str);
                        rooms[idx].status = sv;
                        room_save(rooms, rc) ? set_msg(s,false,"Room updated!") : set_msg(s,true,"Save failed!");
                        s->sub_mode = 0;
                    }
                }
            }
        }
        if (GuiButton((Rectangle){fx+110, fy, 80, BTN_H}, "Cancel")) s->sub_mode = 0;
    } else if (s->sub_mode == 3) {
        draw_panel(FX, FY, FW, FH, "Confirm Delete");
        int idx = room_find_by_id(rooms, rc, s->selected_id);
        int fx = FX+P, fy = FY+60;
        if (idx != -1) {
            char b[80]; snprintf(b, sizeof(b), "Delete room '%s'?", rooms[idx].name);
            TXT(b, fx, fy, FS, C_TEXT);
            TXT("This cannot be undone.", fx, fy+24, FS_SM, C_DIM);
            if (danger_btn((Rectangle){fx, fy+60, 120, BTN_H}, "Yes, Delete")) {
                room_delete(rooms, &rc, s->selected_id);
                room_save(rooms, rc) ? set_msg(s,false,"Room deleted!") : set_msg(s,true,"Save failed!");
                s->selected_id = 0; s->sub_mode = 0;
            }
            if (GuiButton((Rectangle){fx+130, fy+60, 80, BTN_H}, "Cancel")) s->sub_mode = 0;
        }
    } else {
        draw_panel(FX, FY, FW, FH, "Room Details");
        int fx = FX+P, fy = FY+55;
        if (s->selected_id > 0) {
            int idx = room_find_by_id(rooms, rc, s->selected_id);
            if (idx != -1) {
                char b[80]; Room *r = &rooms[idx];
                snprintf(b,sizeof(b),"ID:     %d",r->id);       TXT(b,fx,fy,FS,C_TEXT); fy+=26;
                snprintf(b,sizeof(b),"Name:   %s",r->name);     TXT(b,fx,fy,FS,C_TEXT); fy+=26;
                snprintf(b,sizeof(b),"Price:  $%.2f/mo",r->price); TXT(b,fx,fy,FS,C_TEXT); fy+=26;
                Color sc; const char *ss = room_stat(r->status, &sc);
                TXT("Status: ", fx, fy, FS, C_TEXT);
                TXT(ss, fx + TW("Status: ", FS), fy, FS, sc);
            }
        } else {
            TXT("Select a room from the list,", fx, fy, FS, C_DIM);
            TXT("or click '+ Add Room'.", fx, fy+22, FS, C_DIM);
        }
    }
    draw_msg(s);
}

//landlord-guest
void ui_screen_landlord_guests(AppState *s) {
    draw_hdr("Guest Management", s->current_user.username);
    draw_sidebar(s, 2);

    Guest guests[MAX_GUESTS]; int gc = 0; guest_load(guests, &gc);
    Room rooms[MAX_ROOMS];    int rc = 0; room_load(rooms, &rc);
    handle_scroll(s, LX, LY, LW, LH, gc);

    draw_panel(LX, LY, LW, LH, "Guests");
    const char *cols[] = {"ID","Name","Phone","Room","Check-in"};
    const int   cws[]  = {35, 135, 105, 120, 95};
    draw_col_hdr(LX, LY+42, LW, cols, cws, 5);

    int ry0 = LY+68, ve = s->scroll_index + MAX_VIS;
    if (ve > gc) ve = gc;
    for (int i = s->scroll_index; i < ve; i++) {
        int row = i - s->scroll_index, ry = ry0 + row*ROW_H;
        if (draw_row(LX, ry, LW, row, guests[i].id == s->selected_id)) {
            s->selected_id = guests[i].id; s->sub_mode = 0;
        }
        char b[12]; snprintf(b, sizeof(b), "%d", guests[i].id);
        TXT(b,            LX+8,   ry+10, FS, C_TEXT);
        TXT(guests[i].name,  LX+43,  ry+10, FS, C_TEXT);
        TXT(guests[i].phone, LX+178, ry+10, FS, C_TEXT);
        char rn[50] = "None";
        int ri = room_find_by_id(rooms, rc, guests[i].room_id);
        if (ri != -1) strncpy(rn, rooms[ri].name, 49);
        TXT(rn,              LX+283, ry+10, FS, C_TEXT);
        TXT(guests[i].check_in, LX+403, ry+10, FS, C_DIM);
    }
    if (gc == 0) TXT("No guests. Click '+ Register Guest'.", LX+P, ry0+P, FS, C_DIM);
    if(gc>MAX_VIS){ TXT("Scroll: mouse wheel",LX+P,LY+LH-18,FS_SM,C_DIM); }


    if (GuiButton((Rectangle){LX, ACTY, 140, BTN_H}, "+ Register Guest")) {
        s->sub_mode = 1;
        memset(s->guest_name,  0, sizeof(s->guest_name));
        memset(s->guest_phone, 0, sizeof(s->guest_phone));
        memset(s->guest_room_str, 0, sizeof(s->guest_room_str));
        s->active_field = FLD_NONE;
    }
    if (s->selected_id > 0) {
        if (danger_btn((Rectangle){LX+150, ACTY, 130, BTN_H}, "Checkout Guest"))
            s->sub_mode = 2;
    }

    if (s->sub_mode == 1) {
        draw_panel(FX, FY, FW, FH, "Register Guest");
        int fx = FX+P, fw = FW-P*2, fy = FY+50;
        TXT("Guest Name", fx, fy, FS_SM, C_DIM);
        if (GuiTextBox((Rectangle){fx, fy+16, fw, INP_H}, s->guest_name, 50,
                       s->active_field==FLD_GU_NAME))
            s->active_field=(s->active_field==FLD_GU_NAME)?FLD_NONE:FLD_GU_NAME;
        fy += 64;
        TXT("Phone Number", fx, fy, FS_SM, C_DIM);
        if (GuiTextBox((Rectangle){fx, fy+16, fw, INP_H}, s->guest_phone, 20,
                       s->active_field==FLD_GU_PHONE))
            s->active_field=(s->active_field==FLD_GU_PHONE)?FLD_NONE:FLD_GU_PHONE;
        fy += 64;
        TXT("Available Rooms:", fx, fy, FS_SM, C_DIM); fy += 17;
        for (int i=0; i<rc && i<5; i++) {
            if (rooms[i].status != ROOM_AVAILABLE) continue;
            char b[100]; snprintf(b, sizeof(b), "  ID %d: %s ($%.0f)", rooms[i].id, rooms[i].name, rooms[i].price);
            TXT(b, fx, fy, FS_SM, C_GREEN); fy += 17;
        }
        fy += 4;
        TXT("Assign Room ID", fx, fy, FS_SM, C_DIM);
        if (GuiTextBox((Rectangle){fx, fy+16, fw, INP_H}, s->guest_room_str, 10,
                       s->active_field==FLD_GU_ROOM))
            s->active_field=(s->active_field==FLD_GU_ROOM)?FLD_NONE:FLD_GU_ROOM;
        fy += 64;
        if (GuiButton((Rectangle){fx, fy, 110, BTN_H}, "Register")) {
            if (!strlen(s->guest_name)||!strlen(s->guest_phone)) { set_msg(s,true,"Name and phone required!"); }
            else {
                int rid = utils_str_to_int(s->guest_room_str);
                int ri  = room_find_by_id(rooms, rc, rid);
                if (ri==-1||rooms[ri].status!=ROOM_AVAILABLE) { set_msg(s,true,"Invalid or unavailable room!"); }
                else {
                    int ids[MAX_GUESTS]; for (int i=0;i<gc;i++) ids[i]=guests[i].id;
                    Guest g = {0};
                    g.id = utils_next_id(ids, gc);
                    strncpy(g.name,  s->guest_name,  49); utils_trim(g.name);
                    strncpy(g.phone, s->guest_phone, 19); utils_trim(g.phone);
                    g.room_id = rid;
                    utils_today(g.check_in); strcpy(g.check_out, "N/A");
                    guest_add(guests, &gc, &g);
                    rooms[ri].status = ROOM_OCCUPIED;
                    if (guest_save(guests, gc) && room_save(rooms, rc)) {
                        User users[MAX_USERS]; int uc=0;
                        auth_load_users(users, &uc);
                        if (uc < MAX_USERS) {
                            User u={0}; u.id=g.id+10;
                            strncpy(u.username, g.name,  49);
                            strncpy(u.phone,    g.phone, 19);
                            u.role = ROLE_GUEST; users[uc++] = u;
                            auth_save_users(users, uc);
                        }
                        set_msg(s, false, "Guest registered and room assigned!");
                        s->sub_mode = 0;
                    } else { set_msg(s, true, "Failed to save!"); }
                }
            }
        }
        if (GuiButton((Rectangle){fx+120, fy, 80, BTN_H}, "Cancel")) s->sub_mode = 0;

    } else if (s->sub_mode == 2) {
        draw_panel(FX, FY, FW, FH, "Checkout Guest");
        int gi = guest_find_by_id(guests, gc, s->selected_id);
        int fx = FX+P, fy = FY+60;
        if (gi != -1) {
            char b[80]; snprintf(b, sizeof(b), "Checkout '%s'?", guests[gi].name);
            TXT(b, fx, fy, FS, C_TEXT);
            TXT("Frees the room and removes login access.", fx, fy+24, FS_SM, C_DIM);
            if (danger_btn((Rectangle){fx, fy+60, 120, BTN_H}, "Checkout")) {
                int ri = room_find_by_id(rooms, rc, guests[gi].room_id);
                if (ri != -1) rooms[ri].status = ROOM_AVAILABLE;
                User users[MAX_USERS]; int uc=0; auth_load_users(users,&uc);
                for (int i=0;i<uc;i++) {
                    if (strcmp(users[i].username,guests[gi].name)==0 &&
                        strcmp(users[i].phone,guests[gi].phone)==0) {
                        for (int j=i;j<uc-1;j++) users[j]=users[j+1];
                uc--; break;
                    }
                }
                auth_save_users(users, uc);
                guest_delete(guests, &gc, s->selected_id);
                (guest_save(guests,gc)&&room_save(rooms,rc)) ?
                    set_msg(s,false,"Guest checked out!") : set_msg(s,true,"Save failed!");
                s->selected_id=0; s->sub_mode=0;
            }
            if (GuiButton((Rectangle){fx+130, fy+60, 80, BTN_H}, "Cancel")) s->sub_mode=0;
        }
    } else {
        draw_panel(FX, FY, FW, FH, "Guest Details");
        int fx = FX+P, fy = FY+55;
        if (s->selected_id > 0) {
            int gi = guest_find_by_id(guests, gc, s->selected_id);
            if (gi != -1) {
                char b[80]; Guest *g=&guests[gi];
                snprintf(b,sizeof(b),"ID:       %d",g->id);        TXT(b,fx,fy,FS,C_TEXT); fy+=26;
                snprintf(b,sizeof(b),"Name:     %s",g->name);      TXT(b,fx,fy,FS,C_TEXT); fy+=26;
                snprintf(b,sizeof(b),"Phone:    %s",g->phone);     TXT(b,fx,fy,FS,C_TEXT); fy+=26;
                char rn[50]="Unknown"; int ri=room_find_by_id(rooms,rc,g->room_id);
                if (ri!=-1) strncpy(rn,rooms[ri].name,49);
                snprintf(b,sizeof(b),"Room:     %s",rn);           TXT(b,fx,fy,FS,C_TEXT); fy+=26;
                snprintf(b,sizeof(b),"Check-in: %s",g->check_in);  TXT(b,fx,fy,FS,C_TEXT);
            }
        } else { TXT("Select a guest to see details.", FX+P, FY+60, FS, C_DIM); }
    }
    draw_msg(s);
}

//landlord-billing
void ui_screen_landlord_billing(AppState *s) {
    draw_hdr("Billing Management", s->current_user.username);
    draw_sidebar(s, 3);

    Bill  bills[MAX_BILLS];   int bc=0; billing_load(bills,  &bc);
    Guest guests[MAX_GUESTS]; int gc=0; guest_load(guests, &gc);
    Room  rooms[MAX_ROOMS];   int rc=0; room_load(rooms,   &rc);
    Rates rates; rates_load(&rates);
    handle_scroll(s, LX, LY, LW, LH, bc);

    draw_panel(LX, LY, LW, LH, "Bills");
    const char *cols[] = {"ID","Guest","Total","Due Date","Status"};
    const int   cws[]  = {35, 150, 80, 115, 100};
    draw_col_hdr(LX, LY+42, LW, cols, cws, 5);

    int ry0=LY+68, ve=s->scroll_index+MAX_VIS; if(ve>bc)ve=bc;
    for (int i=s->scroll_index; i<ve; i++) {
        int row=i-s->scroll_index, ry=ry0+row*ROW_H;
        if (draw_row(LX,ry,LW,row,bills[i].id==s->selected_id)) { s->selected_id=bills[i].id; s->sub_mode=0; }
        char b[32]; snprintf(b,sizeof(b),"%d",bills[i].id); TXT(b,LX+8,ry+10,FS,C_TEXT);
        char gn[50]="Unknown"; int gi=guest_find_by_id(guests,gc,bills[i].guest_id);
        if(gi!=-1) strncpy(gn,guests[gi].name,49);
        TXT(gn,LX+43,ry+10,FS,C_TEXT);
        snprintf(b,sizeof(b),"%.2f",bills[i].total); TXT(b,LX+193,ry+10,FS,C_TEXT);
        TXT(bills[i].due_date,LX+273,ry+10,FS,C_DIM);
        Color sc=bills[i].paid?C_GREEN:C_RED;
        TXT(bills[i].paid?"Paid":"Unpaid",LX+388,ry+10,FS,sc);
    }
    if(bc==0) TXT("No bills. Click '+ Create Bill'.",LX+P,ry0+P,FS,C_DIM);
    if(bc>MAX_VIS){ TXT("Scroll: mouse wheel",LX+P,LY+LH-18,FS_SM,C_DIM); }

    /* ---- Action buttons row ---- */
    if (GuiButton((Rectangle){LX, ACTY, 130, BTN_H}, "+ Create Bill")) {
        s->sub_mode=1;
        memset(s->bill_guest_str, 0, sizeof(s->bill_guest_str));
        memset(s->bill_eprev_str, 0, sizeof(s->bill_eprev_str));
        memset(s->bill_ecurr_str, 0, sizeof(s->bill_ecurr_str));
        memset(s->bill_wprev_str, 0, sizeof(s->bill_wprev_str));
        memset(s->bill_wcurr_str, 0, sizeof(s->bill_wcurr_str));
        memset(s->bill_due_str,   0, sizeof(s->bill_due_str));
        s->active_field=FLD_NONE;
    }
    if (s->selected_id>0) {
        int bi=-1; for(int i=0;i<bc;i++) if(bills[i].id==s->selected_id){bi=i;break;}
        if (bi!=-1&&!bills[bi].paid)
            if (GuiButton((Rectangle){LX+140,ACTY,110,BTN_H},"Mark Paid")) s->sub_mode=2;
    }
    /* Rate Settings button (always available) */
    if (GuiButton((Rectangle){LX+260,ACTY,130,BTN_H},"Rate Settings")) {
        s->sub_mode=3;
        snprintf(s->rate_elec_str,  sizeof(s->rate_elec_str),  "%.4f", rates.elec_rate);
        snprintf(s->rate_water_str, sizeof(s->rate_water_str), "%.4f", rates.water_rate);
        s->active_field=FLD_NONE;
    }
    //right panel
    if (s->sub_mode==1) {
        //create bill
        draw_panel(FX,FY,FW,FH,"Create Bill");
        int fx=FX+P, fw=FW-P*2, fy=FY+48;

        //guest list
        TXT("Guests (ID: Name):",fx,fy,FS_SM,C_DIM); fy+=16;
        for(int i=0;i<gc&&i<5;i++){
            char b[60]; snprintf(b,sizeof(b),"  %d: %s",guests[i].id,guests[i].name);
            TXT(b,fx,fy,FS_SM,C_TEXT); fy+=15;
        }
        fy+=4;
        TXT("Guest ID",fx,fy,FS_SM,C_DIM);
        if(GuiTextBox((Rectangle){fx,fy+16,fw,INP_H},s->bill_guest_str,10,s->active_field==FLD_BL_GUEST))
            s->active_field=(s->active_field==FLD_BL_GUEST)?FLD_NONE:FLD_BL_GUEST;
        fy+=48;

        //elec. read
        DrawRectangle(fx-2, fy-4, fw+4, 92, C_CARD);
        DrawRectangleLinesEx((Rectangle){fx-2,fy-4,fw+4,92},1,C_BORDER);
        TXT("ELECTRICITY",fx+4,fy+2,FS_SM,C_ACCENT); fy+=20;
        //prev & current
        int hw=(fw-8)/2;
        TXT("Previous (kWh)",fx,fy,FS_SM,C_DIM);
        TXT("Current  (kWh)",fx+hw+8,fy,FS_SM,C_DIM);
        fy+=16;
        if(GuiTextBox((Rectangle){fx,fy,hw,INP_H},s->bill_eprev_str,20,s->active_field==FLD_BL_EPREV))
            s->active_field=(s->active_field==FLD_BL_EPREV)?FLD_NONE:FLD_BL_EPREV;
        if(GuiTextBox((Rectangle){fx+hw+8,fy,hw,INP_H},s->bill_ecurr_str,20,s->active_field==FLD_BL_ECURR))
            s->active_field=(s->active_field==FLD_BL_ECURR)?FLD_NONE:FLD_BL_ECURR;
        fy+=INP_H+6;
        //real  time fee update
        float ep=utils_str_to_float(s->bill_eprev_str);
        float ec=utils_str_to_float(s->bill_ecurr_str);
        float eu=ec-ep; if(eu<0)eu=0;
        char epreview[80];
        snprintf(epreview,sizeof(epreview),"Usage: %.2f kWh x %.4f = $%.2f",eu,rates.elec_rate,eu*rates.elec_rate);
        TXT(epreview,fx,fy,FS_SM,C_GREEN); fy+=14;

        fy+=8;
        //water read
        DrawRectangle(fx-2, fy-4, fw+4, 92, C_CARD);
        DrawRectangleLinesEx((Rectangle){fx-2,fy-4,fw+4,92},1,C_BORDER);
        TXT("WATER",fx+4,fy+2,FS_SM,(Color){79,195,247,255}); fy+=20;
        TXT("Previous (m³)",fx,fy,FS_SM,C_DIM);
        TXT("Current  (m³)",fx+hw+8,fy,FS_SM,C_DIM);
        fy+=16;
        if(GuiTextBox((Rectangle){fx,fy,hw,INP_H},s->bill_wprev_str,20,s->active_field==FLD_BL_WPREV))
            s->active_field=(s->active_field==FLD_BL_WPREV)?FLD_NONE:FLD_BL_WPREV;
        if(GuiTextBox((Rectangle){fx+hw+8,fy,hw,INP_H},s->bill_wcurr_str,20,s->active_field==FLD_BL_WCURR))
            s->active_field=(s->active_field==FLD_BL_WCURR)?FLD_NONE:FLD_BL_WCURR;
        fy+=INP_H+6;
        float wp=utils_str_to_float(s->bill_wprev_str);
        float wc=utils_str_to_float(s->bill_wcurr_str);
        float wu=wc-wp; if(wu<0)wu=0;
        char wpreview[80];
        snprintf(wpreview,sizeof(wpreview),"Usage: %.2f m³ x %.4f = $%.2f",wu,rates.water_rate,wu*rates.water_rate);
        TXT(wpreview,fx,fy,FS_SM,(Color){79,195,247,255}); fy+=14;

        fy+=12;
        TXT("Due Date (DD/MM/YYYY)",fx,fy,FS_SM,C_DIM);
        if(GuiTextBox((Rectangle){fx,fy+16,fw,INP_H},s->bill_due_str,20,s->active_field==FLD_BL_DUE))
            s->active_field=(s->active_field==FLD_BL_DUE)?FLD_NONE:FLD_BL_DUE;
        fy+=INP_H+22;

        if (GuiButton((Rectangle){fx,fy,100,BTN_H},"Create")) {
            int gid=utils_str_to_int(s->bill_guest_str);
            int gi=guest_find_by_id(guests,gc,gid);
            if(gi==-1){set_msg(s,true,"Invalid Guest ID!");}
            else if(!strlen(s->bill_due_str)){set_msg(s,true,"Due date required!");}
            else if(ec<ep){set_msg(s,true,"Electric current < previous!");}
            else if(wc<wp){set_msg(s,true,"Water current < previous!");}
            else {
                Bill b={0};
                int ids[MAX_BILLS]; for(int i=0;i<bc;i++) ids[i]=bills[i].id;
                b.id=utils_next_id(ids,bc); b.guest_id=gid;
                int ri=room_find_by_id(rooms,rc,guests[gi].room_id);
                b.room_fee=(ri!=-1)?rooms[ri].price:0.0f;
                b.elec_prev=ep; b.elec_curr=ec;
                b.water_prev=wp; b.water_curr=wc;
                b.electric=eu*rates.elec_rate;
                b.water=wu*rates.water_rate;
                b.total=b.room_fee+b.electric+b.water;
                strncpy(b.due_date,s->bill_due_str,19); utils_trim(b.due_date);
                b.paid=BILL_UNPAID;
                billing_add(bills,&bc,&b);
                billing_save(bills,bc)?set_msg(s,false,"Bill created!"):set_msg(s,true,"Save failed!");
                s->sub_mode=0;
            }
        }
        if(GuiButton((Rectangle){fx+110,fy,80,BTN_H},"Cancel")) s->sub_mode=0;

    } else if (s->sub_mode==2) {
        //mark paid
        draw_panel(FX,FY,FW,FH,"Mark as Paid");
        int bi=-1; for(int i=0;i<bc;i++) if(bills[i].id==s->selected_id){bi=i;break;}
        int fx=FX+P, fy=FY+60;
        if(bi!=-1){
            char b[80]; snprintf(b,sizeof(b),"Mark Bill ID %d as Paid?",bills[bi].id);
            TXT(b,fx,fy,FS,C_TEXT);
            snprintf(b,sizeof(b),"Total: $%.2f",bills[bi].total);
            TXT(b,fx,fy+24,FS_SM,C_DIM);
            if(GuiButton((Rectangle){fx,fy+60,120,BTN_H},"Confirm Paid")){
                billing_mark_paid(bills,bc,s->selected_id);
                billing_save(bills,bc)?set_msg(s,false,"Bill marked paid!"):set_msg(s,true,"Save failed!");
                s->sub_mode=0;
            }
            if(GuiButton((Rectangle){fx+130,fy+60,80,BTN_H},"Cancel")) s->sub_mode=0;
        }

    } else if (s->sub_mode==3) {
        //rate setting
        draw_panel(FX,FY,FW,FH,"Rate Settings");
        int fx=FX+P, fw=FW-P*2, fy=FY+52;

        TXT("Set the cost per unit for utilities.",fx,fy,FS_SM,C_DIM); fy+=26;
        TXT("These rates apply to ALL new bills.",fx,fy,FS_SM,C_DIM); fy+=28;

        DrawRectangle(fx-2,fy-4,fw+4,80,C_CARD);
        DrawRectangleLinesEx((Rectangle){fx-2,fy-4,fw+4,80},1,C_BORDER);
        TXT("ELECTRICITY RATE",fx+4,fy+2,FS_SM,C_ACCENT); fy+=20;
        TXT("Cost per 1 kWh ($)",fx,fy,FS_SM,C_DIM); fy+=16;
        if(GuiTextBox((Rectangle){fx,fy,fw,INP_H},s->rate_elec_str,20,s->active_field==FLD_RT_ELEC))
            s->active_field=(s->active_field==FLD_RT_ELEC)?FLD_NONE:FLD_RT_ELEC;
        fy+=INP_H+16;

        DrawRectangle(fx-2,fy-4,fw+4,80,C_CARD);
        DrawRectangleLinesEx((Rectangle){fx-2,fy-4,fw+4,80},1,C_BORDER);
        TXT("WATER RATE",fx+4,fy+2,FS_SM,(Color){79,195,247,255}); fy+=20;
        TXT("Cost per 1 m\xb3 ($)",fx,fy,FS_SM,C_DIM); fy+=16;
        if(GuiTextBox((Rectangle){fx,fy,fw,INP_H},s->rate_water_str,20,s->active_field==FLD_RT_WATER))
            s->active_field=(s->active_field==FLD_RT_WATER)?FLD_NONE:FLD_RT_WATER;
        fy+=INP_H+24;

        //summary preview
        char preview[120];
        snprintf(preview,sizeof(preview),"Current: $%.4f/kWh   $%.4f/m\xb3",rates.elec_rate,rates.water_rate);
        TXT(preview,fx,fy,FS_SM,C_DIM); fy+=20;

        if(GuiButton((Rectangle){fx,fy,110,BTN_H},"Save Rates")){
            float re=utils_str_to_float(s->rate_elec_str);
            float rw=utils_str_to_float(s->rate_water_str);
            if(re<=0){set_msg(s,true,"Electric rate must be > 0!");}
            else if(rw<=0){set_msg(s,true,"Water rate must be > 0!");}
            else{
                Rates nr; nr.elec_rate=re; nr.water_rate=rw;
                rates_save(&nr)?set_msg(s,false,"Rates saved!"):set_msg(s,true,"Save failed!");
                s->sub_mode=0;
            }
        }
        if(GuiButton((Rectangle){fx+120,fy,80,BTN_H},"Cancel")) s->sub_mode=0;

    } else {
        //bill detail
        draw_panel(FX,FY,FW,FH,"Bill Details");
        int fx=FX+P, fy=FY+55;
        if(s->selected_id>0){
            int bi=-1; for(int i=0;i<bc;i++) if(bills[i].id==s->selected_id){bi=i;break;}
            if(bi!=-1){
                char b[100]; Bill *bl=&bills[bi];
                char gn[50]="Unknown"; int gi=guest_find_by_id(guests,gc,bl->guest_id);
                if(gi!=-1) strncpy(gn,guests[gi].name,49);
                snprintf(b,sizeof(b),"Bill ID:  %d",bl->id);           TXT(b,fx,fy,FS,C_TEXT); fy+=24;
                snprintf(b,sizeof(b),"Guest:    %s",gn);               TXT(b,fx,fy,FS,C_TEXT); fy+=24;
                snprintf(b,sizeof(b),"Room Fee: $%.2f",bl->room_fee);  TXT(b,fx,fy,FS,C_TEXT); fy+=20;
                DrawLine(fx,fy+2,fx+FW-P*2,fy+2,C_BORDER); fy+=10;
                /* Electricity detail */
                TXT("ELECTRICITY",fx,fy,FS_SM,C_ACCENT); fy+=16;
                snprintf(b,sizeof(b),"  Prev: %.2f kWh  Curr: %.2f kWh",bl->elec_prev,bl->elec_curr);
                TXT(b,fx,fy,FS_SM,C_TEXT); fy+=16;
                float eu=bl->elec_curr-bl->elec_prev; if(eu<0)eu=0;
                snprintf(b,sizeof(b),"  Used: %.2f kWh x $%.4f = $%.2f",eu,rates.elec_rate,bl->electric);
                TXT(b,fx,fy,FS_SM,C_GREEN); fy+=20;
                //water detail
                TXT("WATER",fx,fy,FS_SM,(Color){79,195,247,255}); fy+=16;
                snprintf(b,sizeof(b),"  Prev: %.2f m3  Curr: %.2f m3",bl->water_prev,bl->water_curr);
                TXT(b,fx,fy,FS_SM,C_TEXT); fy+=16;
                float wu=bl->water_curr-bl->water_prev; if(wu<0)wu=0;
                snprintf(b,sizeof(b),"  Used: %.2f m3 x $%.4f = $%.2f",wu,rates.water_rate,bl->water);
                TXT(b,fx,fy,FS_SM,(Color){79,195,247,255}); fy+=20;
                DrawLine(fx,fy,fx+FW-P*2,fy,C_BORDER); fy+=10;
                snprintf(b,sizeof(b),"TOTAL:    $%.2f",bl->total);      TXT(b,fx,fy,FS_LG,C_TEXT); fy+=28;
                snprintf(b,sizeof(b),"Due:      %s",bl->due_date);      TXT(b,fx,fy,FS,C_TEXT); fy+=22;
                Color sc=bl->paid?C_GREEN:C_RED;
                TXT(bl->paid?"Status: Paid":"Status: Unpaid",fx,fy,FS,sc);
            }
        } else { TXT("Select a bill to see details.",FX+P,FY+60,FS,C_DIM); }
    }
    draw_msg(s);
}

//landlord-invoice
void ui_screen_landlord_invoice(AppState *s) {
    draw_hdr("Invoice Management", s->current_user.username);
    draw_sidebar(s, 4);

    Guest guests[MAX_GUESTS]; int gc=0; guest_load(guests,&gc);
    Bill  bills[MAX_BILLS];   int bc=0; billing_load(bills,&bc);
    Room  rooms[MAX_ROOMS];   int rc=0; room_load(rooms,&rc);
    handle_scroll(s, LX, LY, LW, LH, gc);

    draw_panel(LX,LY,LW,LH,"Guests");
    const char *cols[]={"ID","Name","Room","Bill Status"};
    const int   cws[] ={35,195,140,110};
    draw_col_hdr(LX,LY+42,LW,cols,cws,4);

    int ry0=LY+68, ve=s->scroll_index+MAX_VIS; if(ve>gc)ve=gc;
    for(int i=s->scroll_index;i<ve;i++){
        int row=i-s->scroll_index, ry=ry0+row*ROW_H;
        if(draw_row(LX,ry,LW,row,guests[i].id==s->selected_id)) { s->selected_id=guests[i].id; s->sub_mode=1; }
        char b[12]; snprintf(b,sizeof(b),"%d",guests[i].id); TXT(b,LX+8,ry+10,FS,C_TEXT);
        TXT(guests[i].name,LX+43,ry+10,FS,C_TEXT);
        char rn[50]="None"; int ri=room_find_by_id(rooms,rc,guests[i].room_id);
        if(ri!=-1) strncpy(rn,rooms[ri].name,49);
        TXT(rn,LX+238,ry+10,FS,C_TEXT);
        int bi=billing_find_by_guest(bills,bc,guests[i].id);
        if(bi!=-1){Color sc=bills[bi].paid?C_GREEN:C_RED;TXT(bills[bi].paid?"Paid":"Unpaid",LX+378,ry+10,FS,sc);}
        else TXT("No Bill",LX+378,ry+10,FS,C_DIM);
    }
    if(gc==0) TXT("No guests registered.",LX+P,ry0+P,FS,C_DIM);

    draw_panel(FX,FY,FW,FH,"Invoice Preview");
    if(s->selected_id>0&&s->sub_mode==1){
        int gi=guest_find_by_id(guests,gc,s->selected_id);
        int bi=(gi!=-1)?billing_find_by_guest(bills,bc,s->selected_id):-1;
        int fx=FX+P, fy=FY+48;
        if(gi!=-1&&bi!=-1){
            char today[20]; utils_today(today);
            Invoice inv=invoice_build(&guests[gi],&bills[bi],today);
            char rn[50]="Unknown"; int ri=room_find_by_id(rooms,rc,guests[gi].room_id);
            if(ri!=-1) strncpy(rn,rooms[ri].name,49);
            TXT("================================",fx,fy,FS_SM,C_DIM); fy+=18;
            TXT("       RENTAL INVOICE",fx,fy,FS_LG,C_ACCENT); fy+=24;
            TXT("================================",fx,fy,FS_SM,C_DIM); fy+=18;
            char b[80];
            snprintf(b,sizeof(b),"Invoice ID : %d",inv.invoice_id);TXT(b,fx,fy,FS_SM,C_TEXT);fy+=17;
            snprintf(b,sizeof(b),"Date       : %s",today);TXT(b,fx,fy,FS_SM,C_TEXT);fy+=17;
            snprintf(b,sizeof(b),"Guest      : %s",inv.guest.name);TXT(b,fx,fy,FS_SM,C_TEXT);fy+=17;
            snprintf(b,sizeof(b),"Room       : %s",rn);TXT(b,fx,fy,FS_SM,C_TEXT);fy+=17;
            snprintf(b,sizeof(b),"Check-in   : %s",inv.guest.check_in);TXT(b,fx,fy,FS_SM,C_TEXT);fy+=17;
            TXT("--------------------------------",fx,fy,FS_SM,C_DIM);fy+=17;
            snprintf(b,sizeof(b),"Room Fee   : %.2f",inv.bill.room_fee);TXT(b,fx,fy,FS_SM,C_TEXT);fy+=17;
            snprintf(b,sizeof(b),"Electric   : %.2f",inv.bill.electric);TXT(b,fx,fy,FS_SM,C_TEXT);fy+=17;
            snprintf(b,sizeof(b),"Water      : %.2f",inv.bill.water);TXT(b,fx,fy,FS_SM,C_TEXT);fy+=17;
            TXT("--------------------------------",fx,fy,FS_SM,C_DIM);fy+=17;
            snprintf(b,sizeof(b),"TOTAL      : %.2f",inv.bill.total);TXT(b,fx,fy,FS_LG,C_TEXT);fy+=24;
            snprintf(b,sizeof(b),"Due Date   : %s",inv.bill.due_date);TXT(b,fx,fy,FS_SM,C_TEXT);fy+=17;
            Color sc=inv.bill.paid?C_GREEN:C_RED;
            TXT(inv.bill.paid?"Status     : Paid":"Status     : Unpaid",fx,fy,FS_SM,sc);fy+=24;
            if(GuiButton((Rectangle){fx,fy,150,BTN_H},"Save to File")){
                char ok[80];
                invoice_save_txt(&inv,"invoices")?
                    (snprintf(ok,sizeof(ok),"Saved: invoices/invoice_%d_%d.txt",inv.guest.id,inv.bill.id),set_msg(s,false,ok)):
                    set_msg(s,true,"Failed to save invoice!");
            }
        } else {
            TXT(gi==-1?"Guest not found!":"No bill for this guest.",fx,fy+20,FS,C_DIM);
        }
    } else {
        TXT("Click a guest row to preview",FX+P,FY+60,FS,C_DIM);
        TXT("their invoice.",FX+P,FY+82,FS,C_DIM);
    }
    draw_msg(s);
}

//lanlord report
void ui_screen_landlord_reports(AppState *s) {
    draw_hdr("Problem Reports", s->current_user.username);
    draw_sidebar(s, 5);

    Report reports[MAX_REPORTS]; int rpc=0; report_load(reports,&rpc);
    Guest  guests[MAX_GUESTS];   int gc=0;  guest_load(guests,&gc);
    Room   rooms[MAX_ROOMS];     int rc=0;  room_load(rooms,&rc);
    handle_scroll(s, LX, LY, LW, LH, rpc);

    draw_panel(LX,LY,LW,LH,"Reports");
    const char *cols[]={"ID","Guest","Room","Status","Description"};
    const int   cws[] ={35,120,105,105,115};
    draw_col_hdr(LX,LY+42,LW,cols,cws,5);

    int ry0=LY+68, ve=s->scroll_index+MAX_VIS; if(ve>rpc)ve=rpc;
    for(int i=s->scroll_index;i<ve;i++){
        int row=i-s->scroll_index, ry=ry0+row*ROW_H;
        if(draw_row(LX,ry,LW,row,reports[i].id==s->selected_id)){
            s->selected_id=reports[i].id;
            snprintf(s->report_stat_str,sizeof(s->report_stat_str),"%d",reports[i].status);
        }
        char b[12]; snprintf(b,sizeof(b),"%d",reports[i].id); TXT(b,LX+8,ry+10,FS,C_TEXT);
        char gn[50]="?"; int gi=guest_find_by_id(guests,gc,reports[i].guest_id);
        if(gi!=-1) strncpy(gn,guests[gi].name,49);
        TXT(gn,LX+43,ry+10,FS,C_TEXT);
        char rn[50]="?"; int ri=room_find_by_id(rooms,rc,reports[i].room_id);
        if(ri!=-1) strncpy(rn,rooms[ri].name,49);
        TXT(rn,LX+163,ry+10,FS,C_TEXT);
        Color sc; const char *ss=rep_stat(reports[i].status,&sc); TXT(ss,LX+268,ry+10,FS,sc);
        char desc[18]; strncpy(desc,reports[i].description,17); desc[17]='\0';
        TXT(desc,LX+373,ry+10,FS_SM,C_DIM);
    }
    if(rpc==0) TXT("No reports submitted yet.",LX+P,ry0+P,FS,C_DIM);
    if(rpc>MAX_VIS){ TXT("Scroll: mouse wheel",LX+P,LY+LH-18,FS_SM,C_DIM); }

    draw_panel(FX,FY,FW,FH,"Update Status");
    int fx=FX+P, fw=FW-P*2, fy=FY+50;
    if(s->selected_id>0){
        int ri=-1; for(int i=0;i<rpc;i++) if(reports[i].id==s->selected_id){ri=i;break;}
        if(ri!=-1){
            char b[200];
            TXT("Description:",fx,fy,FS_SM,C_DIM); fy+=17;
            strncpy(b,reports[ri].description,60); b[60]='\0';
            if(strlen(reports[ri].description)>60){b[57]='.';b[58]='.';b[59]='.';b[60]='\0';}
            TXT(b,fx,fy,FS_SM,C_TEXT); fy+=20;
            snprintf(b,sizeof(b),"Date: %s",reports[ri].date);
            TXT(b,fx,fy,FS_SM,C_DIM); fy+=24;
            Color sc; const char *ss=rep_stat(reports[ri].status,&sc);
            TXT("Current: ",fx,fy,FS,C_TEXT);
            TXT(ss,fx+TW("Current: ",FS),fy,FS,sc); fy+=30;
            TXT("New Status  0=Pending  1=InProgress  2=Resolved",fx,fy,FS_SM,C_DIM); fy+=16;
            if(GuiTextBox((Rectangle){fx,fy,fw,INP_H},s->report_stat_str,5,s->active_field==FLD_RP_STAT))
                s->active_field=(s->active_field==FLD_RP_STAT)?FLD_NONE:FLD_RP_STAT;
            fy+=INP_H+12;
            if(GuiButton((Rectangle){fx,fy,130,BTN_H},"Update Status")){
                int ns=utils_str_to_int(s->report_stat_str);
                if(ns<0||ns>2){set_msg(s,true,"Status must be 0-2!");}
                else{
                    report_update_status(reports,rpc,s->selected_id,ns);
                    report_save(reports,rpc)?set_msg(s,false,"Status updated!"):set_msg(s,true,"Save failed!");
                }
            }
        }
    } else { TXT("Click a report to select it.",fx,fy,FS,C_DIM); }
    draw_msg(s);
}

//guest
void ui_screen_guest_stay(AppState *s) {
    draw_hdr("My Stay", s->current_user.username);
    Guest guests[MAX_GUESTS]; int gc=0; guest_load(guests,&gc);
    Room  rooms[MAX_ROOMS];   int rc=0; room_load(rooms,&rc);
    Bill  bills[MAX_BILLS];   int bc=0; billing_load(bills,&bc);

    int mi=-1;
    for(int i=0;i<gc;i++){
        if(strcmp(guests[i].name,s->current_user.username)==0&&
           strcmp(guests[i].phone,s->current_user.phone)==0){mi=i;break;}
    }

    int cy=HDRH+P;
    if(mi==-1){
        TXT("Guest record not found. Contact your landlord.",P*2,cy+20,FS,C_RED);
        if(GuiButton((Rectangle){P*2,cy+60,100,BTN_H},"Logout")) s->current_screen=SCREEN_LOGIN;
        draw_msg(s); return;
    }
    Guest *g=&guests[mi];

    int lw=480, lh=400;
    draw_panel(P*2, cy, lw, lh, "My Stay Information");
    char rn[50]="Unknown"; float rp=0;
    int ri=room_find_by_id(rooms,rc,g->room_id);
    if(ri!=-1){strncpy(rn,rooms[ri].name,49);rp=rooms[ri].price;}
    int iy=cy+55; char b[100];
    snprintf(b,sizeof(b),"Room:      %s",rn);            TXT(b,P*2+P,iy,FS,C_TEXT); iy+=26;
    snprintf(b,sizeof(b),"Monthly:   $%.2f",rp);         TXT(b,P*2+P,iy,FS,C_TEXT); iy+=26;
    snprintf(b,sizeof(b),"Check-in:  %s",g->check_in);   TXT(b,P*2+P,iy,FS,C_TEXT); iy+=34;
    DrawLine(P*2,iy-6,P*2+lw,iy-6,C_BORDER);
    TXT("Current Bill",P*2+P,iy,FS_LG,C_ACCENT); iy+=28;
    int bi=billing_find_by_guest(bills,bc,g->id);
    if(bi!=-1){
        Bill *bl=&bills[bi];
        snprintf(b,sizeof(b),"Room Fee:  %.2f",bl->room_fee); TXT(b,P*2+P,iy,FS,C_TEXT);iy+=22;
        snprintf(b,sizeof(b),"Electric:  %.2f",bl->electric);  TXT(b,P*2+P,iy,FS,C_TEXT);iy+=22;
        snprintf(b,sizeof(b),"Water:     %.2f",bl->water);      TXT(b,P*2+P,iy,FS,C_TEXT);iy+=22;
        snprintf(b,sizeof(b),"Total:     %.2f",bl->total);      TXT(b,P*2+P,iy,FS_LG,C_TEXT);iy+=28;
        snprintf(b,sizeof(b),"Due:       %s",bl->due_date);     TXT(b,P*2+P,iy,FS,C_TEXT);iy+=22;
        Color sc=bl->paid?C_GREEN:C_RED;
        TXT(bl->paid?"Status:    Paid":"Status:    Unpaid",P*2+P,iy,FS,sc);
    } else { TXT("No bill issued yet.",P*2+P,iy,FS,C_DIM); }

    int rx=P*2+lw+P, rw=WIN_W-rx-P;
    draw_panel(rx,cy,rw,lh,"Quick Actions");
    int ay=cy+60;
    if(GuiButton((Rectangle){rx+P,ay,rw-P*2,BTN_H+8},"View My Invoice")){
        s->current_screen=SCREEN_GUEST_INVOICE; s->sub_mode=0;
    }
    ay+=BTN_H+8+12;
    if(GuiButton((Rectangle){rx+P,ay,rw-P*2,BTN_H+8},"Problem Reports")){
        s->current_screen=SCREEN_GUEST_REPORT; s->sub_mode=0;
    }
    ay+=BTN_H+8+12;
    if(danger_btn((Rectangle){rx+P,ay,rw-P*2,BTN_H+8},"Logout")){
        s->current_screen=SCREEN_LOGIN; s->active_field=FLD_NONE;
    }
    draw_msg(s);
}

//guest-invoice
void ui_screen_guest_invoice(AppState *s) {
    draw_hdr("My Invoice", s->current_user.username);
    if(GuiButton((Rectangle){P,HDRH+P,110,BTN_H},"< Back")){s->current_screen=SCREEN_GUEST_STAY;return;}

    Guest guests[MAX_GUESTS]; int gc=0; guest_load(guests,&gc);
    Bill  bills[MAX_BILLS];   int bc=0; billing_load(bills,&bc);
    Room  rooms[MAX_ROOMS];   int rc=0; room_load(rooms,&rc);

    int mi=-1;
    for(int i=0;i<gc;i++) if(strcmp(guests[i].name,s->current_user.username)==0&&strcmp(guests[i].phone,s->current_user.phone)==0){mi=i;break;}

    int cw=400, cx=(WIN_W-cw)/2, cy=HDRH+56;
    draw_panel(cx-P,cy-P,cw+P*2,440,"");

    if(mi==-1){TXT("Guest record not found.",cx,cy+20,FS,C_RED);draw_msg(s);return;}
    int bi=billing_find_by_guest(bills,bc,guests[mi].id);
    if(bi==-1){TXT("No bill issued yet.",cx,cy+20,FS,C_DIM);TXT("Ask your landlord to create a bill.",cx,cy+42,FS_SM,C_DIM);draw_msg(s);return;}

    char today[20]; utils_today(today);
    Invoice inv=invoice_build(&guests[mi],&bills[bi],today);
    char rn[50]="Unknown"; int ri=room_find_by_id(rooms,rc,guests[mi].room_id);
    if(ri!=-1) strncpy(rn,rooms[ri].name,49);

    int iy=cy; char b[80];
    TXT("================================",cx,iy,FS_SM,C_DIM);iy+=18;
    TXT("       RENTAL INVOICE",cx,iy,FS_LG,C_ACCENT);iy+=24;
    TXT("================================",cx,iy,FS_SM,C_DIM);iy+=18;
    snprintf(b,sizeof(b),"Invoice ID : %d",inv.invoice_id);TXT(b,cx,iy,FS_SM,C_TEXT);iy+=17;
    snprintf(b,sizeof(b),"Date       : %s",today);TXT(b,cx,iy,FS_SM,C_TEXT);iy+=17;
    snprintf(b,sizeof(b),"Guest      : %s",inv.guest.name);TXT(b,cx,iy,FS_SM,C_TEXT);iy+=17;
    snprintf(b,sizeof(b),"Room       : %s",rn);TXT(b,cx,iy,FS_SM,C_TEXT);iy+=17;
    snprintf(b,sizeof(b),"Check-in   : %s",inv.guest.check_in);TXT(b,cx,iy,FS_SM,C_TEXT);iy+=17;
    TXT("--------------------------------",cx,iy,FS_SM,C_DIM);iy+=17;
    snprintf(b,sizeof(b),"Room Fee   : %.2f",inv.bill.room_fee);TXT(b,cx,iy,FS_SM,C_TEXT);iy+=17;
    snprintf(b,sizeof(b),"Electric   : %.2f",inv.bill.electric);TXT(b,cx,iy,FS_SM,C_TEXT);iy+=17;
    snprintf(b,sizeof(b),"Water      : %.2f",inv.bill.water);TXT(b,cx,iy,FS_SM,C_TEXT);iy+=17;
    TXT("--------------------------------",cx,iy,FS_SM,C_DIM);iy+=17;
    snprintf(b,sizeof(b),"TOTAL      : %.2f",inv.bill.total);TXT(b,cx,iy,FS_LG,C_TEXT);iy+=24;
    snprintf(b,sizeof(b),"Due Date   : %s",inv.bill.due_date);TXT(b,cx,iy,FS_SM,C_TEXT);iy+=17;
    Color sc=inv.bill.paid?C_GREEN:C_RED;
    TXT(inv.bill.paid?"Status     : Paid":"Status     : Unpaid",cx,iy,FS_SM,sc);iy+=24;
    if(GuiButton((Rectangle){cx,iy,160,BTN_H},"Save to File")){
        char ok[80];
        invoice_save_txt(&inv,"invoices")?
            (snprintf(ok,sizeof(ok),"Saved: invoice_%d_%d.txt",inv.guest.id,inv.bill.id),set_msg(s,false,ok)):
            set_msg(s,true,"Failed to save!");
    }
    draw_msg(s);
}

//guest-report
void ui_screen_guest_report(AppState *s) {
    draw_hdr("Problem Reports", s->current_user.username);
    if(GuiButton((Rectangle){P,HDRH+P,110,BTN_H},"< Back")){s->current_screen=SCREEN_GUEST_STAY;return;}

    Guest  guests[MAX_GUESTS];  int gc=0;  guest_load(guests,&gc);
    Report reports[MAX_REPORTS];int rpc=0; report_load(reports,&rpc);
    Room   rooms[MAX_ROOMS];    int rc=0;  room_load(rooms,&rc);

    int mi=-1;
    for(int i=0;i<gc;i++) if(strcmp(guests[i].name,s->current_user.username)==0&&strcmp(guests[i].phone,s->current_user.phone)==0){mi=i;break;}
    if(mi==-1){TXT("Guest record not found.",P*2,HDRH+60,FS,C_RED);draw_msg(s);return;}
    Guest *g=&guests[mi];

    int cy=HDRH+52, lw=530, lh=440;
    draw_panel(P,cy,lw,lh,"My Reports");
    const char *cols[]={"ID","Room","Date","Status"};
    const int   cws[] ={35,140,115,110};
    draw_col_hdr(P,cy+42,lw,cols,cws,4);
    int ry0=cy+68, cnt=0;
    for(int i=0;i<rpc&&cnt<10;i++){
        if(reports[i].guest_id!=g->id) continue;
        int ry=ry0+cnt*ROW_H;
        draw_row(P,ry,lw,cnt,false);
        char b[12]; snprintf(b,sizeof(b),"%d",reports[i].id); TXT(b,P+8,ry+10,FS,C_TEXT);
        char rn[50]="Unknown"; int ri=room_find_by_id(rooms,rc,reports[i].room_id);
        if(ri!=-1) strncpy(rn,rooms[ri].name,49);
        TXT(rn,P+43,ry+10,FS,C_TEXT);
        TXT(reports[i].date,P+183,ry+10,FS,C_DIM);
        Color sc; const char *ss=rep_stat(reports[i].status,&sc); TXT(ss,P+298,ry+10,FS,sc);
        cnt++;
    }
    if(cnt==0) TXT("No reports submitted yet.",P+P,ry0+P,FS,C_DIM);

    int rx=P+lw+P, rw=WIN_W-rx-P;
    draw_panel(rx,cy,rw,lh,"Submit New Report");
    int fx=rx+P, fw=rw-P*2, fy=cy+50;
    TXT("Describe the problem:",fx,fy,FS_SM,C_DIM); fy+=18;
    if(GuiTextBox((Rectangle){fx,fy,fw,INP_H*2+8},s->report_desc,200,s->active_field==FLD_RP_DESC))
        s->active_field=(s->active_field==FLD_RP_DESC)?FLD_NONE:FLD_RP_DESC;
    fy+=INP_H*2+8+16;
    if(GuiButton((Rectangle){fx,fy,fw,BTN_H+4},"Submit Report")){
        if(!strlen(s->report_desc)){set_msg(s,true,"Description cannot be empty!");}
        else if(rpc>=MAX_REPORTS){set_msg(s,true,"Report limit reached!");}
        else{
            Report r={0};
            int ids[MAX_REPORTS]; for(int i=0;i<rpc;i++) ids[i]=reports[i].id;
            r.id=utils_next_id(ids,rpc); r.guest_id=g->id; r.room_id=g->room_id;
            r.status=REPORT_PENDING;
            strncpy(r.description,s->report_desc,199); utils_trim(r.description);
            utils_today(r.date);
            report_add(reports,&rpc,&r);
            report_save(reports,rpc)?set_msg(s,false,"Report submitted!"):set_msg(s,true,"Save failed!");
            memset(s->report_desc,0,sizeof(s->report_desc)); s->active_field=FLD_NONE;
        }
    }
    draw_msg(s);
}
