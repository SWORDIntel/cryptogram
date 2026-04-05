#!/bin/bash
# auto_setup_ssh_vnc.sh - Automates setting up SSH keys over VNC using vncdotool

# Activate the virtual environment with vncdotool
source venv/bin/activate

# Your public SSH key
PUB_KEY="ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIM1gCSPG/gRGjZJOCck2JOOqMc5HA94MHZM0xYbZ2w+V john@rightbox"

echo "Select a server to automate SSH setup:"
echo "1) PHOENIX1 (23.88.3.239:6045)"
echo "2) PHOENIX2 (205.209.111.190:6011)"
echo "3) PHOENIX3 (205.209.111.190:6090)"
read -p "Choice [1-3]: " choice

case $choice in
    1) HOST="23.88.3.239"; PORT="6045"; VNCPASS="MYgQDBO2";;
    2) HOST="205.209.111.190"; PORT="6011"; VNCPASS="CfGCxz9K";;
    3) HOST="205.209.111.190"; PORT="6090"; VNCPASS="pcEGrVRA";;
    *) echo "Invalid choice"; exit 1;;
esac

read -p "Enter the root password for the server (to login via VNC console): " ROOT_PASS

echo "Connecting to $HOST:$PORT..."
echo "Typing login credentials and setting up SSH key..."

# We use vncdo to script the keystrokes.
# Note: typing long strings can sometimes drop characters over slow networks.
# We will create the ~/.ssh directory, then echo the key into authorized_keys.

vncdo -s $HOST::$PORT -p $VNCPASS \
    key enter pause 1 \
    type "root" key enter pause 2 \
    type "$ROOT_PASS" key enter pause 2 \
    type "mkdir -p ~/.ssh && chmod 700 ~/.ssh" key enter pause 1 \
    type "echo '$PUB_KEY' > ~/.ssh/authorized_keys" key enter pause 1 \
    type "chmod 600 ~/.ssh/authorized_keys" key enter pause 1 \
    type "systemctl restart ssh || service sshd restart" key enter pause 1 \
    type "exit" key enter

echo "Done! Try connecting via SSH now."
