# Running BLE tools (gratttool) in WSL2

BLE work in this repo generally assumes a Linux box with a working Bluetooth
adapter. If you're on Windows and only have WSL2, getting BlueZ-based tools like
`gratttool` (or `gatttool`/`bluetoothctl`) working takes a few extra steps because
WSL2 has no Bluetooth support and no hardware access by default. This doc is a
general guide to get there, whether your adapter is a USB dongle or your laptop's
built-in radio.

Fill in these two values for your machine as you go — everything below refers back
to them:

| Placeholder | What it is                           | How to find it                      |
| ----------- | ------------------------------------- | ------------------------------------ |
| `<busid>`   | USB bus ID of your Bluetooth adapter  | `usbipd list` (Windows), see step 2 |
| `<user>`    | Your Windows username                 | Whatever `C:\Users\<user>` is       |

## Why it doesn't work out of the box

1. **BlueZ-based tools are Linux-only.** [gratttool](https://github.com/hackgnar/gratttool),
   like `gatttool` before it, talks to BlueZ over D-Bus (via the `bluer` crate).
   BlueZ is Linux's Bluetooth stack — there's no Windows build and none is coming,
   since Windows has its own separate Bluetooth API.

2. **The stock WSL2 kernel has no Bluetooth support at all** — not even as a
   module. Running `hciconfig` on a default WSL2 install fails with:

   ```
   Can't open HCI socket.: Address family not supported by protocol
   ```

   That means `AF_BLUETOOTH` sockets don't exist in the kernel, so BlueZ/`bluetoothd`
   can never start (`org.freedesktop.DBus.Error.TimedOut: Failed to activate
   service 'org.bluez'`).

3. **WSL2 has no USB access to hardware by default.** Even with a Bluetooth-capable
   kernel, an adapter still needs to be forwarded from Windows via `usbipd-win`.

This works for both external dongles and most laptops' built-in radios: on modern
Intel/Realtek/Qualcomm combo Wi-Fi+BT cards (the common "CNVi" design), Wi-Fi rides
over PCIe but Bluetooth is exposed as its own internal **USB** device — which is
why `usbipd-win` can see and forward it exactly like an external dongle. Only the
Bluetooth USB endpoint gets forwarded to WSL; Wi-Fi is a separate interface and is
never affected.

## One-time setup

### 1. Install usbipd-win (Windows, admin)

```powershell
winget install usbipd
```

### 2. Find and bind your adapter's USB bus ID (Windows, admin)

```powershell
usbipd list
```

Look for your Bluetooth radio in the `Connected` list (e.g. `Intel(R) Wireless
Bluetooth(R)`, a Realtek/Qualcomm equivalent, or an external dongle's name) and
note its `BUSID` — that's your `<busid>`.

```powershell
usbipd bind --busid <busid>
```

If it warns about an incompatible USB filter driver (e.g. Wireshark's `USBPcap`)
and refuses, re-run with `--force`:

```powershell
usbipd bind --busid <busid> --force
```

### 3. Build a Bluetooth-enabled WSL2 kernel (inside WSL)

The prebuilt Microsoft WSL2 kernel ships with Bluetooth stripped out. Build a
custom one with it enabled:

```bash
git clone --depth 1 https://github.com/microsoft/WSL2-Linux-Kernel.git ~/WSL2-Linux-Kernel
cd ~/WSL2-Linux-Kernel
zcat /proc/config.gz > .config      # start from your current running config
scripts/config --enable CONFIG_BT --enable CONFIG_BT_HCIBTUSB \
                --module CONFIG_BT_RFCOMM --module CONFIG_BT_BNEP \
                --enable CONFIG_BT_LE
make olddefconfig
make -j$(nproc)                     # a full build; no sudo needed
```

This produces `arch/x86/boot/bzImage`. `make` alone doesn't need root — you only
need `sudo` if you're also missing build tools (`build-essential flex bison
libssl-dev libelf-dev bc dwarves`).

> Run the build via a command you stay attached to (foreground, or backgrounded at
> the process-manager level outside WSL) rather than `nohup ... &` detached inside
> the shell — without `systemd` lingering enabled for your user, background jobs
> get killed as soon as the WSL login session that spawned them ends.

### 4. Install the kernel

Copy the built image somewhere Windows can see it and point `.wslconfig` at it:

```bash
mkdir -p "/mnt/c/Users/<user>/wsl-kernel"
cp ~/WSL2-Linux-Kernel/arch/x86/boot/bzImage "/mnt/c/Users/<user>/wsl-kernel/bzImage-bt"
```

`C:\Users\<user>\.wslconfig`:

```ini
[wsl2]
kernel=C:\\Users\\<user>\\wsl-kernel\\bzImage-bt
```

Then, from Windows:

```powershell
wsl --shutdown
```

The next `wsl` launch boots the custom kernel. Verify with:

```bash
uname -r                 # ends in ...-microsoft-standard-WSL2+  (note the +)
zcat /proc/config.gz | grep -E '^CONFIG_BT(=|_HCIBTUSB=)'   # both should be =y
```

### 5. Install your BLE tooling

Install `gratttool` (or `bluez`/`gatttool`, `bluetoothctl`, etc.) as normal for
your distro inside WSL. On distros with systemd support in WSL, `dbus` and
`bluetooth` start automatically once the kernel supports `AF_BLUETOOTH` — check
with `systemctl is-active dbus bluetooth`. If your WSL doesn't have systemd
enabled, start them manually: `sudo service dbus start && sudo service bluetooth start`.

## Every session: unlocking the adapter for WSL

Windows and WSL can't both hold the radio at once, and WSL only "sees" USB devices
that are actively attached via `usbipd`.

1. **Turn Bluetooth off in Windows** — Settings → Bluetooth & devices → Off, or
   the tray icon. This releases the OS's handle on the radio; skipping this makes
   `usbipd attach` fail with `Device busy (exported)`.

2. **Keep a WSL terminal open.** The WSL2 VM shuts down when nothing is attached
   to it, and `usbipd attach` needs a running instance to target.

3. **Attach the adapter** (Windows, admin PowerShell):

   ```powershell
   usbipd attach --wsl --busid <busid>
   ```

4. **Confirm it's up** (inside WSL — should already be automatic if systemd is
   managing `bluetooth`):

   ```bash
   hciconfig -a       # hci0: ... UP RUNNING
   ```

   If it's down: `sudo hciconfig hci0 up`.

5. **Run your BLE tools as normal**, e.g.:

   ```bash
   gratttool -b <target-mac> --enumerate
   ```

## Giving Bluetooth back to Windows

```powershell
usbipd detach --busid <busid>
```

then turn Windows Bluetooth back **on** so paired devices (mouse/keyboard/
headphones) reconnect.

## Reference

- gratttool: [hackgnar/gratttool](https://github.com/hackgnar/gratttool)
- usbipd-win: [dorssel/usbipd-win](https://github.com/dorssel/usbipd-win)
- WSL2 kernel source: [microsoft/WSL2-Linux-Kernel](https://github.com/microsoft/WSL2-Linux-Kernel)
- This repo's target device details: [mac.txt](mac.txt), [notes.md](notes.md)
