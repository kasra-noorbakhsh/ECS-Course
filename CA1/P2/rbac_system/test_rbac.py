from permissions import is_command_allowed

user = "normal_user"
command = "df -h"

if is_command_allowed(user, command):
    print(f"{user} is allowed to run: {command}")
else:
    print(f"{user} is NOT allowed to run: {command}")

