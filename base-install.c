#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NOB_IMPLEMENTATION
#include "./nob.h"

// ANSI color codes
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"

void color_printf(const char *color, const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("%s", color);
    vprintf(format, args);
    printf("%s", COLOR_RESET);
    printf("\n");
    va_end(args);
}

int execute_command(const char *command, const char *error_message) {
    if (system(command) != 0) {
        color_printf(COLOR_RED, "[ERROR] %s\n", error_message);
        return 1;
    }
    return 0;
}

int get_user_input(const char *prompt, char *buffer, size_t buffer_size) {
    color_printf(COLOR_BLUE, "%s", prompt);
    if (fgets(buffer, buffer_size, stdin) == NULL) {
        color_printf(COLOR_RED, "Input error.\n");
        return 1;
    }
    buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
    return 0;
}

int install_core_packages(void) {
    color_printf(COLOR_YELLOW, "Installing core packages...\n");
    return execute_command(
        "pacstrap /mnt base linux linux-firmware sof-firmware "
        "base-devel grub efibootmgr networkmanager",
        "Failed to install core packages."
    );
}

int configure_grub(const char *disk_name) {
    char *grub_install_cmd = nob_temp_sprintf("grub-install %s", disk_name);

    if (execute_command("mkdir -p /boot/grub", "Failed to add GRUB folder in boot.")) {
        return 1;
    }
    if (execute_command(grub_install_cmd, "Failed to install GRUB.")) {
        return 1;
    }
    if (execute_command("grub-mkconfig -o /boot/grub/grub.cfg", "Failed to configure GRUB.")) {
        return 1;
    }
    return 0;
}
int main(void) {
    color_printf(COLOR_GREEN, "Adjusting the time...\n");
    if (execute_command("hwclock --systohc", "Failed to adjust the system clock.")) {
        return 1;
    }

    char username[256];
    if (get_user_input("Enter a username: ", username, sizeof(username))) {
        return 1;
    }
    char *set_user = nob_temp_sprintf("useradd -G wheel -m %s", username);
    if (execute_command(set_user, "Failed to create user.")) {
        return 1;
    }
    char *set_password = nob_temp_sprintf("passwd %s", username);
    if (execute_command(set_password, "Failed to set user password.")) {
        return 1;
    }

    char timezone[256];
    if (get_user_input("Enter your timezone (Syntax: Continent/City): ", timezone, sizeof(timezone))) {
        return 1;
    }
    char *set_timezone = nob_temp_sprintf("ln -sf /usr/share/zoneinfo/%s /etc/localtime", timezone);
    if (execute_command(set_timezone, "Failed to set timezone.")) {
        return 1;
    }

    color_printf(COLOR_GREEN, "Enabling NetworkManager...\n");
    if (execute_command("systemctl enable NetworkManager", "Failed to enable NetworkManager.")) {
        return 1;
    }

    char disk_name[256];
    if (get_user_input("Enter your disk name (e.g., /dev/sda): ", disk_name, sizeof(disk_name))) {
        return 1;
    }
    if (configure_grub(disk_name)) {
        return 1;
    }

    color_printf(COLOR_GREEN, "TODOs for you now:\n");
    color_printf(COLOR_YELLOW, "- Enable sudo for your user (use `visudo`).\n");
    color_printf(COLOR_YELLOW, "- Set your language configuration:\n");
    color_printf(COLOR_YELLOW, "  /etc/locale.gen\n  locale-gen\n  /etc/locale.conf\n  /etc/vconsole.conf\n");
    color_printf(COLOR_YELLOW, "- Check GRUB_DISABLE_OS_PROBER for dual boot.\n");
    color_printf(COLOR_GREEN, "Base install complete!\n");
    return 0;
}
