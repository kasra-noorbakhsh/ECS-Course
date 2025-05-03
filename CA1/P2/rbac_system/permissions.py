import re
from rbac_config import RBAC

def is_command_allowed(user, command):
    if user not in RBAC:
        return False
    allowed = RBAC[user]["allowed_commands"]
    if "*" in allowed:
        return True
    for allowed_cmd in allowed:
        if command.strip().startswith(allowed_cmd):
            return True 
    return False


