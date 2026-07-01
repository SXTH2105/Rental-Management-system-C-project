#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "auth.h"
#include "room.h"
#include "guest.h"
#include "billing.h"
#include "utils.h"
#include "invoice.h"
#include "report.h"

static void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

Screen ui_screen_login(User *out_user) {
    printf("\n===================================\n");
    printf("      RENTAL MANAGEMENT SYSTEM     \n");
    printf("===================================\n");
    printf("1. Login\n");
    printf("2. Exit\n");
    printf("Choose an option: ");
    
    int choice = 0;
    if (scanf("%d", &choice) != 1) {
        clear_input_buffer();
        return SCREEN_LOGIN;
    }
    clear_input_buffer();

    if (choice == 2) {
        return SCREEN_EXIT;
    } else if (choice != 1) {
        printf("Invalid option!\n");
        return SCREEN_LOGIN;
    }

    printf("\n--- Login ---\n");
    printf("Enter Username: ");
    char username[50] = {0};
    if (scanf(" %49[^\n]", username) != 1) {
        clear_input_buffer();
        return SCREEN_LOGIN;
    }
    clear_input_buffer();

    printf("Enter Phone: ");
    char phone[20] = {0};
    if (scanf(" %19[^\n]", phone) != 1) {
        clear_input_buffer();
        return SCREEN_LOGIN;
    }
    clear_input_buffer();

    User user;
    if (auth_login(username, phone, &user)) {
        printf("\nLogin successful! Welcome, %s.\n", user.username);
        if (out_user) {
            *out_user = user;
        }
        if (user.role == ROLE_LANDLORD) {
            return SCREEN_LANDLORD_DASHBOARD;
        } else {
            return SCREEN_GUEST_STAY;
        }
    } else {
        printf("\nLogin failed! Invalid username or phone number.\n");
        return SCREEN_LOGIN;
    }
}

Screen ui_screen_landlord_dashboard(const User *user) {
    printf("\n--- Landlord Dashboard (%s) ---\n", user->username);
    
    // Display Rooms Grid Overview
    Room rooms[MAX_ROOMS];
    int room_count = 0;
    printf("Rooms Overview:\n");
    if (room_load(rooms, &room_count) && room_count > 0) {
        for (int i = 0; i < room_count; i++) {
            const char *status_str = "\033[1;32mAvailable\033[0m";
            if (rooms[i].status == ROOM_OCCUPIED) {
                status_str = "\033[1;31mOccupied\033[0m";
            } else if (rooms[i].status == ROOM_MAINTENANCE) {
                status_str = "\033[1;33mMaintenance\033[0m";
            }
            printf("[%s: %s]  ", rooms[i].name, status_str);
            if ((i + 1) % 5 == 0) {
                printf("\n");
            }
        }
        if (room_count % 5 != 0) {
            printf("\n");
        }
    } else {
        printf("[No rooms registered]\n");
    }
    
    printf("\nMenu Options:\n");
    printf("1. Manage Rooms\n");
    printf("2. Manage Guests\n");
    printf("3. Billing\n");
    printf("4. Invoices\n");
    printf("5. Problem Reports\n");
    printf("6. Logout\n");
    printf("Choose option: ");
    
    int choice = 0;
    if (scanf("%d", &choice) != 1) {
        clear_input_buffer();
        return SCREEN_LANDLORD_DASHBOARD;
    }
    clear_input_buffer();

    switch (choice) {
        case 1: return SCREEN_LANDLORD_ROOMS;
        case 2: return SCREEN_LANDLORD_GUESTS;
        case 3: return SCREEN_LANDLORD_BILLING;
        case 4: return SCREEN_LANDLORD_INVOICE;
        case 5: return SCREEN_LANDLORD_REPORTS;
        case 6: return SCREEN_LOGIN;
        default: printf("Invalid choice!\n"); return SCREEN_LANDLORD_DASHBOARD;
    }
}

Screen ui_screen_landlord_rooms(const User *user) {
    (void)user;
    Room rooms[MAX_ROOMS];
    int room_count = 0;
    room_load(rooms, &room_count);

    printf("\n--- Manage Rooms ---\n");
    printf("1. List All Rooms (Detailed)\n");
    printf("2. Add Room\n");
    printf("3. Edit Room\n");
    printf("4. Delete Room\n");
    printf("5. Back to Dashboard\n");
    printf("Choose option: ");

    int choice = 0;
    if (scanf("%d", &choice) != 1) {
        clear_input_buffer();
        return SCREEN_LANDLORD_ROOMS;
    }
    clear_input_buffer();

    switch (choice) {
        case 1: {
            printf("\nID  | Room Name      | Price      | Status\n");
            printf("-------------------------------------------\n");
            for (int i = 0; i < room_count; i++) {
                const char *status_str = "\033[1;32mAvailable\033[0m";
                if (rooms[i].status == ROOM_OCCUPIED) status_str = "\033[1;31mOccupied\033[0m";
                else if (rooms[i].status == ROOM_MAINTENANCE) status_str = "\033[1;33mMaintenance\033[0m";
                printf("%-3d | %-14s | %-10.2f | %s\n", rooms[i].id, rooms[i].name, rooms[i].price, status_str);
            }
            printf("\nEnter any character + Enter to return: ");
            char dummy[10];
            scanf("%9s", dummy);
            clear_input_buffer();
            return SCREEN_LANDLORD_ROOMS;
        }
        case 2: {
            if (room_count >= MAX_ROOMS) {
                printf("Error: Maximum room limit reached!\n");
                return SCREEN_LANDLORD_ROOMS;
            }
            int new_id = 1;
            if (room_count > 0) {
                int ids[MAX_ROOMS];
                for (int i = 0; i < room_count; i++) {
                    ids[i] = rooms[i].id;
                }
                new_id = utils_next_id(ids, room_count);
            }

            Room r;
            memset(&r, 0, sizeof(Room));
            r.id = new_id;

            printf("Enter Room Name: ");
            if (scanf(" %49[^\n]", r.name) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_ROOMS;
            }
            clear_input_buffer();
            utils_trim(r.name);

            printf("Enter Monthly Price: ");
            if (scanf("%f", &r.price) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_ROOMS;
            }
            clear_input_buffer();

            printf("Enter Status (0=Available, 1=Occupied, 2=Maintenance): ");
            if (scanf("%d", &r.status) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_ROOMS;
            }
            clear_input_buffer();

            room_add(rooms, &room_count, &r);
            if (room_save(rooms, room_count)) {
                printf("Room added successfully with ID %d!\n", new_id);
            } else {
                printf("Error: Failed to save changes!\n");
            }
            return SCREEN_LANDLORD_ROOMS;
        }
        case 3: {
            printf("Enter Room ID to edit: ");
            int target_id = 0;
            if (scanf("%d", &target_id) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_ROOMS;
            }
            clear_input_buffer();

            int idx = room_find_by_id(rooms, room_count, target_id);
            if (idx == -1) {
                printf("Error: Room ID %d not found!\n", target_id);
                return SCREEN_LANDLORD_ROOMS;
            }

            printf("Editing Room [%s]\n", rooms[idx].name);
            
            printf("Enter New Name (leave blank to keep current): ");
            char new_name[50] = {0};
            if (fgets(new_name, sizeof(new_name), stdin)) {
                utils_trim(new_name);
                if (strlen(new_name) > 0) {
                    strcpy(rooms[idx].name, new_name);
                }
            }

            printf("Enter New Monthly Price (current: %.2f): ", rooms[idx].price);
            char price_buf[32] = {0};
            if (fgets(price_buf, sizeof(price_buf), stdin)) {
                utils_trim(price_buf);
                if (strlen(price_buf) > 0) {
                    rooms[idx].price = utils_str_to_float(price_buf);
                }
            }

            printf("Enter New Status (0=Available, 1=Occupied, 2=Maintenance, current: %d): ", rooms[idx].status);
            char status_buf[32] = {0};
            if (fgets(status_buf, sizeof(status_buf), stdin)) {
                utils_trim(status_buf);
                if (strlen(status_buf) > 0) {
                    rooms[idx].status = utils_str_to_int(status_buf);
                }
            }

            if (room_save(rooms, room_count)) {
                printf("Room updated successfully!\n");
            } else {
                printf("Error: Failed to save changes!\n");
            }
            return SCREEN_LANDLORD_ROOMS;
        }
        case 4: {
            printf("Enter Room ID to delete: ");
            int target_id = 0;
            if (scanf("%d", &target_id) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_ROOMS;
            }
            clear_input_buffer();

            int idx = room_find_by_id(rooms, room_count, target_id);
            if (idx == -1) {
                printf("Error: Room ID %d not found!\n", target_id);
                return SCREEN_LANDLORD_ROOMS;
            }

            printf("Are you sure you want to delete room [%s]? (1=Yes, 0=No): ", rooms[idx].name);
            int confirm = 0;
            if (scanf("%d", &confirm) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_ROOMS;
            }
            clear_input_buffer();

            if (confirm == 1) {
                room_delete(rooms, &room_count, target_id);
                if (room_save(rooms, room_count)) {
                    printf("Room deleted successfully!\n");
                } else {
                    printf("Error: Failed to save changes!\n");
                }
            } else {
                printf("Deletion cancelled.\n");
            }
            return SCREEN_LANDLORD_ROOMS;
        }
        case 5:
            return SCREEN_LANDLORD_DASHBOARD;
        default:
            printf("Invalid choice!\n");
            return SCREEN_LANDLORD_ROOMS;
    }
}

Screen ui_screen_landlord_guests(const User *user) {
    (void)user;
    Guest guests[MAX_GUESTS];
    int guest_count = 0;
    guest_load(guests, &guest_count);

    Room rooms[MAX_ROOMS];
    int room_count = 0;
    room_load(rooms, &room_count);

    printf("\n--- Manage Guests ---\n");
    printf("1. List All Guests\n");
    printf("2. Register Guest\n");
    printf("3. Delete Guest (Check-out)\n");
    printf("4. Back to Dashboard\n");
    printf("Choose option: ");

    int choice = 0;
    if (scanf("%d", &choice) != 1) {
        clear_input_buffer();
        return SCREEN_LANDLORD_GUESTS;
    }
    clear_input_buffer();

    switch (choice) {
        case 1: {
            printf("\nID  | Guest Name           | Phone        | Room Name      | Check-in\n");
            printf("----------------------------------------------------------------------\n");
            for (int i = 0; i < guest_count; i++) {
                char room_name[50] = "None";
                int r_idx = room_find_by_id(rooms, room_count, guests[i].room_id);
                if (r_idx != -1) {
                    strcpy(room_name, rooms[r_idx].name);
                }
                printf("%-3d | %-20s | %-12s | %-14s | %s\n", 
                       guests[i].id, guests[i].name, guests[i].phone, room_name, guests[i].check_in);
            }
            printf("\nEnter any character + Enter to return: ");
            char dummy[10];
            scanf("%9s", dummy);
            clear_input_buffer();
            return SCREEN_LANDLORD_GUESTS;
        }
        case 2: {
            if (guest_count >= MAX_GUESTS) {
                printf("Error: Maximum guest limit reached!\n");
                return SCREEN_LANDLORD_GUESTS;
            }

            Guest g;
            memset(&g, 0, sizeof(Guest));
            
            printf("Enter Guest Name: ");
            if (scanf(" %49[^\n]", g.name) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_GUESTS;
            }
            clear_input_buffer();
            utils_trim(g.name);

            printf("Enter Guest Phone: ");
            if (scanf(" %19[^\n]", g.phone) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_GUESTS;
            }
            clear_input_buffer();
            utils_trim(g.phone);

            printf("\nAvailable Rooms:\n");
            int available_count = 0;
            for (int i = 0; i < room_count; i++) {
                if (rooms[i].status == ROOM_AVAILABLE) {
                    printf("  - ID: %d | %s (Price: %.2f)\n", rooms[i].id, rooms[i].name, rooms[i].price);
                    available_count++;
                }
            }
            if (available_count == 0) {
                printf("[No available rooms! Please add or free a room first.]\n");
                return SCREEN_LANDLORD_GUESTS;
            }

            printf("Enter Room ID to assign: ");
            int target_room_id = 0;
            if (scanf("%d", &target_room_id) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_GUESTS;
            }
            clear_input_buffer();

            int r_idx = room_find_by_id(rooms, room_count, target_room_id);
            if (r_idx == -1 || rooms[r_idx].status != ROOM_AVAILABLE) {
                printf("Error: Room ID is invalid or not available!\n");
                return SCREEN_LANDLORD_GUESTS;
            }

            int new_id = 1;
            if (guest_count > 0) {
                int ids[MAX_GUESTS];
                for (int i = 0; i < guest_count; i++) {
                    ids[i] = guests[i].id;
                }
                new_id = utils_next_id(ids, guest_count);
            }
            g.id = new_id;
            g.room_id = target_room_id;
            utils_today(g.check_in);
            strcpy(g.check_out, "N/A");

            guest_add(guests, &guest_count, &g);
            rooms[r_idx].status = ROOM_OCCUPIED;

            if (guest_save(guests, guest_count) && room_save(rooms, room_count)) {
                printf("Guest registered successfully with ID %d and room %s assigned!\n", g.id, rooms[r_idx].name);
                
                // Add the guest to auth database so they can log in!
                User users[MAX_USERS];
                int user_count = 0;
                auth_load_users(users, &user_count);
                if (user_count < MAX_USERS) {
                    User u;
                    u.id = g.id + 10; // offset id
                    strcpy(u.username, g.name);
                    strcpy(u.phone, g.phone);
                    u.role = ROLE_GUEST;
                    users[user_count++] = u;
                    auth_save_users(users, user_count);
                }
            } else {
                printf("Error: Failed to save changes!\n");
            }
            return SCREEN_LANDLORD_GUESTS;
        }
        case 3: {
            printf("Enter Guest ID to check-out/delete: ");
            int target_id = 0;
            if (scanf("%d", &target_id) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_GUESTS;
            }
            clear_input_buffer();

            int idx = guest_find_by_id(guests, guest_count, target_id);
            if (idx == -1) {
                printf("Error: Guest ID %d not found!\n", target_id);
                return SCREEN_LANDLORD_GUESTS;
            }

            printf("Are you sure you want to check-out guest [%s]? (1=Yes, 0=No): ", guests[idx].name);
            int confirm = 0;
            if (scanf("%d", &confirm) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_GUESTS;
            }
            clear_input_buffer();

            if (confirm == 1) {
                int r_idx = room_find_by_id(rooms, room_count, guests[idx].room_id);
                if (r_idx != -1) {
                    rooms[r_idx].status = ROOM_AVAILABLE;
                }
                
                // Also remove from auth database
                User users[MAX_USERS];
                int user_count = 0;
                auth_load_users(users, &user_count);
                int u_idx = -1;
                for (int i = 0; i < user_count; i++) {
                    if (strcmp(users[i].username, guests[idx].name) == 0 && strcmp(users[i].phone, guests[idx].phone) == 0) {
                        u_idx = i;
                        break;
                    }
                }
                if (u_idx != -1) {
                    for (int i = u_idx; i < user_count - 1; i++) {
                        users[i] = users[i+1];
                    }
                    user_count--;
                    auth_save_users(users, user_count);
                }

                guest_delete(guests, &guest_count, target_id);

                if (guest_save(guests, guest_count) && room_save(rooms, room_count)) {
                    printf("Guest checked out and deleted successfully!\n");
                } else {
                    printf("Error: Failed to save changes!\n");
                }
            } else {
                printf("Check-out cancelled.\n");
            }
            return SCREEN_LANDLORD_GUESTS;
        }
        case 4:
            return SCREEN_LANDLORD_DASHBOARD;
        default:
            printf("Invalid choice!\n");
            return SCREEN_LANDLORD_GUESTS;
    }
}

Screen ui_screen_landlord_billing(const User *user) {
    (void)user;
    Bill bills[MAX_BILLS];
    int bill_count = 0;
    billing_load(bills, &bill_count);

    Guest guests[MAX_GUESTS];
    int guest_count = 0;
    guest_load(guests, &guest_count);

    Room rooms[MAX_ROOMS];
    int room_count = 0;
    room_load(rooms, &room_count);

    printf("\n--- Billing Management ---\n");
    printf("1. List All Bills\n");
    printf("2. Create Bill\n");
    printf("3. Mark Bill as Paid\n");
    printf("4. Back to Dashboard\n");
    printf("Choose option: ");

    int choice = 0;
    if (scanf("%d", &choice) != 1) {
        clear_input_buffer();
        return SCREEN_LANDLORD_BILLING;
    }
    clear_input_buffer();

    switch (choice) {
        case 1: {
            printf("\nID  | Guest Name           | Room Name      | RoomFee  | Elec    | Water   | Total    | Due Date   | Status\n");
            printf("----------------------------------------------------------------------------------------------------------\n");
            for (int i = 0; i < bill_count; i++) {
                char guest_name[50] = "Unknown";
                char room_name[50] = "Unknown";
                int g_idx = guest_find_by_id(guests, guest_count, bills[i].guest_id);
                if (g_idx != -1) {
                    strcpy(guest_name, guests[g_idx].name);
                    int r_idx = room_find_by_id(rooms, room_count, guests[g_idx].room_id);
                    if (r_idx != -1) strcpy(room_name, rooms[r_idx].name);
                }
                const char *status_str = (bills[i].paid == BILL_PAID)
                    ? "\033[1;32mPaid\033[0m  "
                    : "\033[1;31mUnpaid\033[0m";
                printf("%-3d | %-20s | %-14s | %8.2f | %7.2f | %7.2f | %8.2f | %-10s | %s\n",
                       bills[i].id, guest_name, room_name,
                       bills[i].room_fee, bills[i].electric, bills[i].water, bills[i].total,
                       bills[i].due_date, status_str);
            }
            printf("\nPress Enter to return: ");
            char dummy[10];
            scanf("%9s", dummy);
            clear_input_buffer();
            return SCREEN_LANDLORD_BILLING;
        }
        case 2: {
            if (guest_count == 0) {
                printf("No guests registered! Please add a guest first.\n");
                return SCREEN_LANDLORD_BILLING;
            }
            printf("\nGuests:\n");
            for (int i = 0; i < guest_count; i++) {
                char room_name[50] = "None";
                int r_idx = room_find_by_id(rooms, room_count, guests[i].room_id);
                if (r_idx != -1) strcpy(room_name, rooms[r_idx].name);
                printf("  - ID: %d | %s (%s)\n", guests[i].id, guests[i].name, room_name);
            }
            printf("Enter Guest ID: ");
            int target_guest_id = 0;
            if (scanf("%d", &target_guest_id) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_BILLING;
            }
            clear_input_buffer();

            int g_idx = guest_find_by_id(guests, guest_count, target_guest_id);
            if (g_idx == -1) {
                printf("Error: Guest ID %d not found!\n", target_guest_id);
                return SCREEN_LANDLORD_BILLING;
            }

            Bill b;
            memset(&b, 0, sizeof(Bill));
            b.guest_id = target_guest_id;

            int r_idx = room_find_by_id(rooms, room_count, guests[g_idx].room_id);
            if (r_idx != -1) {
                b.room_fee = rooms[r_idx].price;
                printf("Room fee auto-set from room price: %.2f\n", b.room_fee);
            } else {
                printf("Enter Room Fee: ");
                if (scanf("%f", &b.room_fee) != 1) { clear_input_buffer(); return SCREEN_LANDLORD_BILLING; }
                clear_input_buffer();
            }

            printf("Enter Electric Fee: ");
            if (scanf("%f", &b.electric) != 1) { clear_input_buffer(); return SCREEN_LANDLORD_BILLING; }
            clear_input_buffer();

            printf("Enter Water Fee: ");
            if (scanf("%f", &b.water) != 1) { clear_input_buffer(); return SCREEN_LANDLORD_BILLING; }
            clear_input_buffer();

            printf("Enter Due Date (DD/MM/YYYY): ");
            if (scanf(" %19[^\n]", b.due_date) != 1) { clear_input_buffer(); return SCREEN_LANDLORD_BILLING; }
            clear_input_buffer();
            utils_trim(b.due_date);

            int ids[MAX_BILLS];
            for (int i = 0; i < bill_count; i++) ids[i] = bills[i].id;
            b.id = utils_next_id(ids, bill_count);
            b.paid = BILL_UNPAID;

            billing_add(bills, &bill_count, &b);
            if (billing_save(bills, bill_count)) {
                printf("Bill created! ID=%d, Total=%.2f\n", b.id, bills[bill_count-1].total);
            } else {
                printf("Error: Failed to save bill!\n");
            }
            return SCREEN_LANDLORD_BILLING;
        }
        case 3: {
            printf("Enter Bill ID to mark as paid: ");
            int target_bill_id = 0;
            if (scanf("%d", &target_bill_id) != 1) {
                clear_input_buffer();
                return SCREEN_LANDLORD_BILLING;
            }
            clear_input_buffer();

            int found = 0;
            for (int i = 0; i < bill_count; i++) {
                if (bills[i].id == target_bill_id) { found = 1; break; }
            }
            if (!found) {
                printf("Error: Bill ID %d not found!\n", target_bill_id);
                return SCREEN_LANDLORD_BILLING;
            }
            billing_mark_paid(bills, bill_count, target_bill_id);
            if (billing_save(bills, bill_count)) {
                printf("Bill ID %d marked as paid!\n", target_bill_id);
            } else {
                printf("Error: Failed to save changes!\n");
            }
            return SCREEN_LANDLORD_BILLING;
        }
        case 4:
            return SCREEN_LANDLORD_DASHBOARD;
        default:
            printf("Invalid choice!\n");
            return SCREEN_LANDLORD_BILLING;
    }
}

Screen ui_screen_landlord_invoice(const User *user) {
    (void)user;
    Guest guests[MAX_GUESTS];
    int guest_count = 0;
    guest_load(guests, &guest_count);

    Room rooms[MAX_ROOMS];
    int room_count = 0;
    room_load(rooms, &room_count);

    Bill bills[MAX_BILLS];
    int bill_count = 0;
    billing_load(bills, &bill_count);

    printf("\n--- Landlord Invoice Panel ---\n");
    if (guest_count == 0) {
        printf("No guests registered.\n");
        printf("\nEnter any character + Enter to return: ");
        char dummy[10];
        scanf("%9s", dummy);
        clear_input_buffer();
        return SCREEN_LANDLORD_DASHBOARD;
    }

    printf("\nList of Guests:\n");
    printf("ID  | Guest Name           | Room Name      | Phone\n");
    printf("---------------------------------------------------\n");
    for (int i = 0; i < guest_count; i++) {
        char room_name[50] = "None";
        int r_idx = room_find_by_id(rooms, room_count, guests[i].room_id);
        if (r_idx != -1) {
            strcpy(room_name, rooms[r_idx].name);
        }
        printf("%-3d | %-20s | %-14s | %s\n", guests[i].id, guests[i].name, room_name, guests[i].phone);
    }

    printf("\nEnter Guest ID to generate invoice: ");
    int target_guest_id = 0;
    if (scanf("%d", &target_guest_id) != 1) {
        clear_input_buffer();
        return SCREEN_LANDLORD_DASHBOARD;
    }
    clear_input_buffer();

    int g_idx = guest_find_by_id(guests, guest_count, target_guest_id);
    if (g_idx == -1) {
        printf("Error: Guest ID %d not found!\n", target_guest_id);
        printf("\nEnter any character + Enter to return: ");
        char dummy[10];
        scanf("%9s", dummy);
        clear_input_buffer();
        return SCREEN_LANDLORD_DASHBOARD;
    }

    int b_idx = billing_find_by_guest(bills, bill_count, target_guest_id);
    if (b_idx == -1) {
        printf("Error: No bill found for guest [%s]!\n", guests[g_idx].name);
        printf("\nEnter any character + Enter to return: ");
        char dummy[10];
        scanf("%9s", dummy);
        clear_input_buffer();
        return SCREEN_LANDLORD_DASHBOARD;
    }

    char today[20];
    utils_today(today);
    Invoice inv = invoice_build(&guests[g_idx], &bills[b_idx], today);

    printf("\n--- Invoice Preview ---\n");
    invoice_preview(&inv);

    printf("\nSave invoice to file? (1=Yes, 0=No): ");
    int save_choice = 0;
    if (scanf("%d", &save_choice) == 1 && save_choice == 1) {
        if (invoice_save_txt(&inv, "invoices")) {
            printf("Invoice saved successfully to 'invoices' folder!\n");
        } else {
            printf("Error saving invoice!\n");
        }
    }
    clear_input_buffer();

    printf("\nEnter any character + Enter to return: ");
    char dummy[10];
    scanf("%9s", dummy);
    clear_input_buffer();
    return SCREEN_LANDLORD_DASHBOARD;
}

Screen ui_screen_landlord_reports(const User *user) {
    (void)user;
    Report reports[MAX_REPORTS];
    int report_count = 0;
    report_load(reports, &report_count);

    Guest guests[MAX_GUESTS];
    int guest_count = 0;
    guest_load(guests, &guest_count);

    Room rooms[MAX_ROOMS];
    int room_count = 0;
    room_load(rooms, &room_count);

    printf("\n--- Landlord Problem Reports ---\n");
    if (report_count == 0) {
        printf("No reports submitted.\n");
        printf("\nEnter any character + Enter to return: ");
        char dummy[10];
        scanf("%9s", dummy);
        clear_input_buffer();
        return SCREEN_LANDLORD_DASHBOARD;
    }

    printf("\nAll Submitted Reports:\n");
    printf("ID  | Guest Name           | Room Name      | Date       | Status      | Description\n");
    printf("------------------------------------------------------------------------------------\n");
    for (int i = 0; i < report_count; i++) {
        char guest_name[50] = "Unknown";
        int g_idx = guest_find_by_id(guests, guest_count, reports[i].guest_id);
        if (g_idx != -1) strcpy(guest_name, guests[g_idx].name);

        char room_name[50] = "Unknown";
        int r_idx = room_find_by_id(rooms, room_count, reports[i].room_id);
        if (r_idx != -1) strcpy(room_name, rooms[r_idx].name);

        const char *status_str = "Pending";
        if (reports[i].status == REPORT_IN_PROGRESS) {
            status_str = "\033[1;33mIn Progress\033[0m";
        } else if (reports[i].status == REPORT_RESOLVED) {
            status_str = "\033[1;32mResolved\033[0m";
        } else {
            status_str = "\033[1;31mPending\033[0m";
        }

        printf("%-3d | %-20s | %-14s | %-10s | %-11s | %s\n", 
               reports[i].id, guest_name, room_name, reports[i].date, status_str, reports[i].description);
    }

    printf("\nOptions:\n");
    printf("1. Update Report Status\n");
    printf("2. Back to Dashboard\n");
    printf("Choose option: ");

    int choice = 0;
    if (scanf("%d", &choice) != 1) {
        clear_input_buffer();
        return SCREEN_LANDLORD_REPORTS;
    }
    clear_input_buffer();

    if (choice == 1) {
        printf("Enter Report ID to update: ");
        int target_id = 0;
        if (scanf("%d", &target_id) != 1) {
            clear_input_buffer();
            return SCREEN_LANDLORD_REPORTS;
        }
        clear_input_buffer();

        int found_idx = -1;
        for (int i = 0; i < report_count; i++) {
            if (reports[i].id == target_id) {
                found_idx = i;
                break;
            }
        }

        if (found_idx == -1) {
            printf("Error: Report ID %d not found!\n", target_id);
            printf("\nEnter any character + Enter to return: ");
            char dummy[10];
            scanf("%9s", dummy);
            clear_input_buffer();
            return SCREEN_LANDLORD_REPORTS;
        }

        printf("Enter New Status (0=Pending, 1=In Progress, 2=Resolved): ");
        int new_status = 0;
        if (scanf("%d", &new_status) != 1) {
            clear_input_buffer();
            return SCREEN_LANDLORD_REPORTS;
        }
        clear_input_buffer();

        if (new_status < 0 || new_status > 2) {
            printf("Error: Invalid status!\n");
            printf("\nEnter any character + Enter to return: ");
            char dummy[10];
            scanf("%9s", dummy);
            clear_input_buffer();
            return SCREEN_LANDLORD_REPORTS;
        }

        report_update_status(reports, report_count, target_id, new_status);
        if (report_save(reports, report_count)) {
            printf("Report ID %d status updated successfully!\n", target_id);
        } else {
            printf("Error: Failed to save changes!\n");
        }

        printf("\nEnter any character + Enter to return: ");
        char dummy[10];
        scanf("%9s", dummy);
        clear_input_buffer();
        return SCREEN_LANDLORD_REPORTS;

    } else if (choice == 2) {
        return SCREEN_LANDLORD_DASHBOARD;
    } else {
        printf("Invalid option!\n");
        return SCREEN_LANDLORD_REPORTS;
    }
}

Screen ui_screen_guest_stay(const User *user) {
    printf("\n--- Guest Stay (%s) ---\n", user->username);

    Guest guests[MAX_GUESTS];
    int guest_count = 0;
    guest_load(guests, &guest_count);

    Room rooms[MAX_ROOMS];
    int room_count = 0;
    room_load(rooms, &room_count);

    Bill bills[MAX_BILLS];
    int bill_count = 0;
    billing_load(bills, &bill_count);

    // Find this guest's record by matching username and phone
    int my_guest_idx = -1;
    for (int i = 0; i < guest_count; i++) {
        if (strcmp(guests[i].name, user->username) == 0 &&
            strcmp(guests[i].phone, user->phone) == 0) {
            my_guest_idx = i;
            break;
        }
    }

    if (my_guest_idx != -1) {
        Guest *g = &guests[my_guest_idx];
        char room_name[50] = "Unknown";
        float room_price = 0.0f;
        int r_idx = room_find_by_id(rooms, room_count, g->room_id);
        if (r_idx != -1) {
            strcpy(room_name, rooms[r_idx].name);
            room_price = rooms[r_idx].price;
        }
        printf("Room    : %s (%.2f/month)\n", room_name, room_price);
        printf("Check-in: %s\n", g->check_in);

        int b_idx = billing_find_by_guest(bills, bill_count, g->id);
        if (b_idx != -1) {
            Bill *b = &bills[b_idx];
            const char *status_str = (b->paid == BILL_PAID)
                ? "\033[1;32mPaid\033[0m"
                : "\033[1;31mUnpaid\033[0m";
            printf("\n--- My Current Bill ---\n");
            printf("  Room Fee : %.2f\n", b->room_fee);
            printf("  Electric : %.2f\n", b->electric);
            printf("  Water    : %.2f\n", b->water);
            printf("  Total    : %.2f\n", b->total);
            printf("  Due Date : %s\n", b->due_date);
            printf("  Status   : %s\n", status_str);
        } else {
            printf("\n[No bill issued yet]\n");
        }
    } else {
        printf("[Stay information not found]\n");
    }

    printf("\n1. View Invoice\n");
    printf("2. Submit Problem Report\n");
    printf("3. Logout\n");
    printf("Choose option: ");

    int choice = 0;
    if (scanf("%d", &choice) != 1) {
        clear_input_buffer();
        return SCREEN_GUEST_STAY;
    }
    clear_input_buffer();

    switch (choice) {
        case 1: return SCREEN_GUEST_INVOICE;
        case 2: return SCREEN_GUEST_REPORT;
        case 3: return SCREEN_LOGIN;
        default: printf("Invalid choice!\n"); return SCREEN_GUEST_STAY;
    }
}

Screen ui_screen_guest_invoice(const User *user) {
    Guest guests[MAX_GUESTS];
    int guest_count = 0;
    guest_load(guests, &guest_count);

    Bill bills[MAX_BILLS];
    int bill_count = 0;
    billing_load(bills, &bill_count);

    printf("\n--- View My Invoice ---\n");

    int my_guest_idx = -1;
    for (int i = 0; i < guest_count; i++) {
        if (strcmp(guests[i].name, user->username) == 0 &&
            strcmp(guests[i].phone, user->phone) == 0) {
            my_guest_idx = i;
            break;
        }
    }

    if (my_guest_idx != -1) {
        int b_idx = billing_find_by_guest(bills, bill_count, guests[my_guest_idx].id);
        if (b_idx != -1) {
            char today[20];
            utils_today(today);
            Invoice inv = invoice_build(&guests[my_guest_idx], &bills[b_idx], today);

            printf("\n--- Invoice Preview ---\n");
            invoice_preview(&inv);

            printf("\nSave invoice to file? (1=Yes, 0=No): ");
            int save_choice = 0;
            if (scanf("%d", &save_choice) == 1 && save_choice == 1) {
                if (invoice_save_txt(&inv, "invoices")) {
                    printf("Invoice saved successfully to 'invoices' folder!\n");
                } else {
                    printf("Error saving invoice!\n");
                }
            }
            clear_input_buffer();
        } else {
            printf("\nNo bill has been issued for you yet. Cannot generate invoice.\n");
        }
    } else {
        printf("\nStay information not found!\n");
    }

    printf("\nEnter any character + Enter to return: ");
    char dummy[10];
    scanf("%9s", dummy);
    clear_input_buffer();
    return SCREEN_GUEST_STAY;
}

Screen ui_screen_guest_report(const User *user) {
    Guest guests[MAX_GUESTS];
    int guest_count = 0;
    guest_load(guests, &guest_count);

    int my_guest_idx = -1;
    for (int i = 0; i < guest_count; i++) {
        if (strcmp(guests[i].name, user->username) == 0 &&
            strcmp(guests[i].phone, user->phone) == 0) {
            my_guest_idx = i;
            break;
        }
    }

    if (my_guest_idx == -1) {
        printf("\n[Stay information not found. Cannot manage reports.]\n");
        printf("Enter any character + Enter to return: ");
        char dummy[10];
        scanf("%9s", dummy);
        clear_input_buffer();
        return SCREEN_GUEST_STAY;
    }

    Guest *g = &guests[my_guest_idx];

    printf("\n--- Problem Reports ---\n");
    printf("1. List My Reports\n");
    printf("2. Submit New Report\n");
    printf("3. Back\n");
    printf("Choose option: ");

    int choice = 0;
    if (scanf("%d", &choice) != 1) {
        clear_input_buffer();
        return SCREEN_GUEST_REPORT;
    }
    clear_input_buffer();

    if (choice == 1) {
        Report reports[MAX_REPORTS];
        int report_count = 0;
        report_load(reports, &report_count);

        Room rooms[MAX_ROOMS];
        int room_count = 0;
        room_load(rooms, &room_count);

        printf("\nMy Submitted Reports:\n");
        printf("ID  | Room Name      | Date       | Status      | Description\n");
        printf("------------------------------------------------------------------------\n");
        int found = 0;
        for (int i = 0; i < report_count; i++) {
            if (reports[i].guest_id == g->id) {
                char room_name[50] = "Unknown";
                int r_idx = room_find_by_id(rooms, room_count, reports[i].room_id);
                if (r_idx != -1) strcpy(room_name, rooms[r_idx].name);

                const char *status_str = "Pending";
                if (reports[i].status == REPORT_IN_PROGRESS) {
                    status_str = "\033[1;33mIn Progress\033[0m";
                } else if (reports[i].status == REPORT_RESOLVED) {
                    status_str = "\033[1;32mResolved\033[0m";
                } else {
                    status_str = "\033[1;31mPending\033[0m";
                }

                printf("%-3d | %-14s | %-10s | %-20s | %s\n", 
                       reports[i].id, room_name, reports[i].date, status_str, reports[i].description);
                found++;
            }
        }
        if (found == 0) {
            printf("[No reports submitted yet]\n");
        }
        printf("\nEnter any character + Enter to return: ");
        char dummy[10];
        scanf("%9s", dummy);
        clear_input_buffer();
        return SCREEN_GUEST_REPORT;

    } else if (choice == 2) {
        Report reports[MAX_REPORTS];
        int report_count = 0;
        report_load(reports, &report_count);

        if (report_count >= MAX_REPORTS) {
            printf("Error: Maximum report limit reached!\n");
            printf("\nEnter any character + Enter to return: ");
            char dummy[10];
            scanf("%9s", dummy);
            clear_input_buffer();
            return SCREEN_GUEST_REPORT;
        }

        Report r;
        memset(&r, 0, sizeof(Report));

        printf("Enter problem description: ");
        if (scanf(" %199[^\n]", r.description) != 1) {
            clear_input_buffer();
            return SCREEN_GUEST_REPORT;
        }
        clear_input_buffer();
        utils_trim(r.description);

        int ids[MAX_REPORTS];
        for (int i = 0; i < report_count; i++) ids[i] = reports[i].id;
        r.id = utils_next_id(ids, report_count);
        r.guest_id = g->id;
        r.room_id = g->room_id;
        r.status = REPORT_PENDING;
        utils_today(r.date);

        report_add(reports, &report_count, &r);
        if (report_save(reports, report_count)) {
            printf("Report submitted successfully with ID %d!\n", r.id);
        } else {
            printf("Error: Failed to save report!\n");
        }

        printf("\nEnter any character + Enter to return: ");
        char dummy[10];
        scanf("%9s", dummy);
        clear_input_buffer();
        return SCREEN_GUEST_REPORT;

    } else if (choice == 3) {
        return SCREEN_GUEST_STAY;
    } else {
        printf("Invalid option!\n");
        return SCREEN_GUEST_REPORT;
    }
}
