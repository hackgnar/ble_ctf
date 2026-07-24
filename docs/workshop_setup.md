# Workshop Setup

Welcome to **Learning to Hack Bluetooth Low Energy with BLE CTF**. This document walks you
through getting a working BLE client setup *before* you arrive so we can spend the workshop
hacking instead of debugging drivers.

## TLDR

You need a machine that can talk to a Bluetooth Low Energy peripheral and at least one BLE
client tool installed and tested. The **hardware target is provided** — every attendee gets a
preflashed ESP32 BLE CTF module at the workshop — so all you have to bring is a laptop (or Pi,
or phone) that can scan, connect, read, and write GATT characteristics.

There are several ways to get there. Pick the one that fits you:

| Approach | Support level | Good for |
| --- | --- | --- |
| **nRF Connect on your phone** (Option 3) | ✅ Easiest | Total beginners / your first BLE CTF |
| **Raspberry Pi + gatttool** (Option 2) | ✅ Supported | Beginners who want the full challenge set |
| **gratttool on any Linux** (Option 1) | ✅ Recommended tool | Comfortable in a terminal |
| **WHAD** | ⚠️ Unsupported | Experts, on your own |
| **Coding libs (bleak, etc.) / LLM** | ⚠️ Unsupported | Coders, on your own |

### New to Bluetooth hacking? Start here

You do **not** need to be a Linux expert to do this workshop. If you're a beginner, pick one of
these two gentle on-ramps and feel free to ignore the other options for now:

- **Easiest — nRF Connect on your phone (Option 3).** Install a free app and start scanning and
  poking at Bluetooth devices with taps instead of typed commands. It's the friendliest way in,
  and it's plenty to learn the core ideas and clear a good chunk of the challenges. You can move
  up to a laptop tool later if you catch the bug.
- **Fuller experience, still beginner-friendly — a Raspberry Pi (Option 2).** A Raspberry Pi is a
  small, inexpensive computer we've already tested end to end. You set it up once by flashing a
  single SD card with a point-and-click tool, then you have the full command-line toolset — the
  same capabilities as the recommended laptop setup — without gambling on whether your laptop's
  Bluetooth cooperates.

Comfortable typing commands in a Linux terminal and want the full toolset on your own laptop?
Head to **gratttool (Option 1)** below — it's the most capable path, just a bit more hands-on.

**On macOS or Windows?** The two recommended tools (gratttool and gatttool) are Linux-only — they
talk to BlueZ. For this workshop the recommended path is to run **Linux in a VM** and pass a USB
Bluetooth dongle through to the guest, which gives you the full toolset. Prefer to stay native?
**nRF Connect** (Option 3) and the **coding-library/LLM** path (bleak, etc.) both run directly on
macOS and Windows — see those sections for the tradeoffs. VM notes: on Intel machines VirtualBox
or VMware works; on Apple Silicon use UTM or VMware Fusion. Pass a **USB Bluetooth dongle** to the
guest rather than relying on built-in Bluetooth, which rarely passes through cleanly. On Windows,
use a real VM — **WSL2 doesn't do Bluetooth**.

Whatever you choose, **test it before you arrive** using the [Testing Your Setup](#testing-your-setup)
section at the bottom. If you can scan for a BLE device and read a characteristic off of it,
you're ready.

> **Using nRF Connect on your phone? You can skip this box entirely** — it only matters for the
> laptop and Raspberry Pi command-line tools.
>
> One important gotcha up front: **gratttool and gatttool want opposite things from the BlueZ
> daemon.** gratttool uses the modern BlueZ D-Bus API and needs `bluetoothd` **running**.
> gatttool talks to the controller over raw HCI sockets and needs `bluetoothd` **stopped**.
> Don't mix the two setups on the same machine without flipping the daemon back. This trips up
> more people than anything else.

---

## Supported Setups

These are the approaches the workshop staff and instructors recommend and will help you debug
in the room.

### Option 1: gratttool on any Linux distro (recommended tool · effort: moderate — comfortable in a terminal)

For almost everyone, **[gratttool](https://github.com/hackgnar/gratttool) is the setup to use.**
It's a modern, full-featured Rust reimplementation of gatttool that gives you the **exact same
capabilities** — it produces character-for-character identical output, so every gatttool command
in this CTF works unchanged — but unlike gatttool it **installs and runs cleanly on current Linux
distributions**. No Raspberry Pi, no hunting down a package your distro removed years ago. It
also throws in some genuinely handy extras (`--scan`, `--enumerate`, ASCII output, string writes,
MAC/address spoofing). If you don't have a specific reason to do otherwise, start here.

This runs natively on your own laptop's Linux install — Ubuntu, Fedora, Arch, Kali, and friends
all work.

**Pre-setup — before you install gratttool, make sure your BLE stack works:**

gratttool uses the modern **BlueZ D-Bus API**, so it needs the BlueZ daemon **running** (not
stopped).

1. Confirm you have a working BLE adapter your OS recognizes. We use `bluetoothctl` and `rfkill`
   here rather than the old `hciconfig`/`hcitool` commands — those BlueZ tools are deprecated and
   are **often not installed on current distros** (Ubuntu 22.04+, Fedora, etc.):

   ```bash
   rfkill list bluetooth     # make sure Bluetooth isn't soft/hard blocked
   bluetoothctl list         # should show at least one Controller
   ```

2. Make sure BlueZ is installed and the service is up:

   ```bash
   sudo systemctl start bluetooth
   systemctl status bluetooth   # should be active (running)
   ```

3. Power the adapter on. On **Kali** (and some hardened distros) it comes up blocked by default,
   so unblock first:

   ```bash
   sudo rfkill unblock bluetooth
   bluetoothctl power on        # modern equivalent of `hciconfig hci0 up`
   ```

   (LE is enabled by default under `bluetoothd`, so there's no separate `btmgmt le on` step
   needed on the gratttool path.)

   Note: **leave `bluetoothd` running** for gratttool. (This is the opposite of the gatttool/Pi
   setup in Option 2 below.)

**Install gratttool (prebuilt binary — recommended):**

gratttool ships **prebuilt Linux binaries** on its releases page, so you no longer need Rust or a
compiler. Grab the one that matches your CPU architecture, drop it on your PATH, and you're done.

1. Install the small runtime dependencies (BlueZ and libdbus). If Bluetooth already works on
   your machine you likely have these, but to be safe:

   ```bash
   # Debian / Ubuntu / Kali / Raspberry Pi OS
   sudo apt install bluetooth bluez libdbus-1-3

   # Fedora
   sudo dnf install bluez dbus-libs

   # Arch
   sudo pacman -S bluez dbus
   ```

2. Figure out which build you need:

   ```bash
   uname -m
   ```

   | `uname -m` prints | Release asset | Typical machine |
   | --- | --- | --- |
   | `x86_64` | `gratttool-linux-x86_64` | most laptops |
   | `aarch64` | `gratttool-linux-aarch64` | 64-bit Raspberry Pi OS (Pi 3/4/5, Zero 2 W) |
   | `armv7l` | `gratttool-linux-armv7` | 32-bit Raspberry Pi OS (Pi 2/3/4) |
   | `armv6l` | `gratttool-linux-armv6` | 32-bit Pi OS on Pi 1 / Pi Zero (W) |

3. Download the asset for your architecture from the latest release. The assets are **bare
   executables** — there's no archive to extract. For a typical laptop (`x86_64`):

   ```bash
   curl -LO https://github.com/hackgnar/gratttool/releases/latest/download/gratttool-linux-x86_64
   ```

   Swap `gratttool-linux-x86_64` for the row matching your `uname -m`. Each asset shows a
   `sha256:` digest on the [releases page](https://github.com/hackgnar/gratttool/releases/latest);
   to verify, compare it against `sha256sum gratttool-linux-x86_64`.

4. Mark it executable and put it on your PATH, renaming it to plain `gratttool` on the way in:

   ```bash
   chmod +x gratttool-linux-x86_64
   sudo install -m 0755 gratttool-linux-x86_64 /usr/local/bin/gratttool

   gratttool --scan   # smoke test — see Testing Your Setup below
   ```

**Optional one-liner** (download + install the right binary in one go). It maps `uname -m` to the
release asset name and uses GitHub's permanent "latest" download URL:

```bash
case "$(uname -m)" in
  x86_64)  A=x86_64 ;;
  aarch64) A=aarch64 ;;
  armv7l)  A=armv7 ;;
  armv6l)  A=armv6 ;;
  *) echo "No prebuilt binary for $(uname -m) — build from source (below)"; A= ;;
esac
if [ -n "$A" ]; then
  curl -L "https://github.com/hackgnar/gratttool/releases/latest/download/gratttool-linux-$A" -o gratttool
  chmod +x gratttool && sudo install -m 0755 gratttool /usr/local/bin/gratttool
  gratttool --scan
fi
```

**Fallback — build from source:** if there's no prebuilt binary for your platform, you can still
compile it yourself. Install Rust 1.70+ via [rustup](https://rustup.rs/) plus the build headers
(`libdbus-1-dev pkg-config` on Debian/Ubuntu, `dbus-devel pkgconf-pkg-config` on Fedora, `dbus`
on Arch), then `git clone` the repo and run `cargo build --release`. Full build instructions are
in the [gratttool README](https://github.com/hackgnar/gratttool#building).

**A few gratttool features worth knowing for the CTF:**

- `gratttool --scan` replaces `hcitool lescan` for finding the CTF device's MAC address.
- `gratttool -b <MAC> --enumerate` dumps every service, characteristic, and readable value in a
  single color-coded table — great for orienting yourself on a target.
- `-A` / `--ascii` prints read values as ASCII instead of hex; `-X` shows both side by side.
- `-S "text"` writes an ASCII string directly, so you can skip the classic
  `echo -n "..." | xxd -ps` dance (and the trailing-newline bug that comes with forgetting `-n`).
- Some CTF challenges send notifications on characteristics that don't advertise the NOTIFY bit,
  which BlueZ silently drops. Running `--listen` under `sudo` lets gratttool capture those.
- `sudo gratttool --bdaddr <ADDR>` changes your own (client) BLE address — handy for any challenge
  that keys off who's connecting. It needs `sudo`, and only works on adapters whose chipset
  supports address changes (see Common Problems).

Full flag reference and examples live in the [gratttool README](https://github.com/hackgnar/gratttool).

**Following the challenge instructions:** the flag write-ups in the main BLE CTF project (and most
older walkthroughs online) use `gatttool` commands. gratttool is a drop-in replacement — every one
of those commands runs verbatim, so you can follow any `gatttool …` example as-is, or swap in
gratttool's nicer flags (like `-S` for string writes). You don't need gatttool installed.

### Option 2: gatttool via Raspberry Pi (effort: beginner-friendly — one-time setup)

Prefer the original `gatttool`? It's still a perfectly good tool for this CTF — but most Linux
distributions deprecated and removed it from BlueZ years ago, which is exactly why gratttool
(Option 1) exists. If you specifically want the original, or you already have a Pi-based
workflow, the most painless way to keep using it is a **Raspberry Pi running a stock, up-to-date
Raspberry Pi OS image**. The workshop staff has tested and verified gatttool as fully functional
on the latest Raspberry Pi OS. (If you just want something that works on your laptop, use
gratttool instead — it gives you the same capabilities.)

**What you need:**

- A Raspberry Pi with onboard Bluetooth (Zero 2 W, 3, 4, 5, etc.) or a BLE USB dongle
- A fresh install of the latest Raspberry Pi OS
- A way to access the Pi during the workshop (see below)

**One-time prep — install gatttool:**

Recent Raspberry Pi OS images may not ship `gatttool` by default. Install it via `bluez-tools`
if it isn't already present:

```bash
sudo apt update
sudo apt install bluez bluez-tools
which gatttool   # confirm it's on your PATH
```

**Bring the adapter up (run these each session, in this order):**

```bash
sudo rfkill unblock all       # clear any soft/hard radio blocks
sudo hciconfig hci0 up        # bring the HCI interface up
sudo btmgmt le on             # make sure LE mode is enabled
sudo systemctl stop bluetooth # stop bluetoothd so it doesn't hold the adapter
```

That last command is the important one. `gatttool` talks to the controller directly over raw
HCI sockets, and a running `bluetoothd` will fight it for the adapter. Stop the daemon and
gatttool gets exclusive access. (If you later want to switch to gratttool or `bluetoothctl`,
start it back up with `sudo systemctl start bluetooth`.)

**Accessing the Pi during the workshop.** The only real hassle with the Pi approach is *how you
talk to it* in the room. The cleanest path is to configure everything up front when you flash the
card, using the official **[Raspberry Pi Imager](https://www.raspberrypi.com/software/)**. It's
the recommended way to write Raspberry Pi OS now, and its OS-customization menu sets up SSH and
Wi-Fi for you through GUI menus — no config-file editing, no `wpa_supplicant.conf`, no
`touch /boot/ssh`.

When you write the card in Raspberry Pi Imager:

1. Choose your Pi model, select **Raspberry Pi OS**, pick your SD card, then click **Next**.
2. When it offers to apply **OS customization**, choose **Edit Settings**.
3. On the **General** tab, set a hostname and a username/password, and enter your **Wi-Fi SSID +
   password** (and Wi-Fi country).
4. On the **Services** tab, **enable SSH** (password or public-key auth).
5. Save, write the card, and boot. The Pi joins Wi-Fi and accepts SSH on first boot.

From your laptop: `ssh <username>@<hostname>.local`.

Conference Wi-Fi is chaotic, so **test this at home first.** If you'd rather not depend on the
room's network at all, two no-network fallbacks:

- **Keyboard + monitor plugged straight into the Pi.** Bring your own gear. Zero surprises.
- **SSH over a USB gadget/Ethernet link** between your laptop and the Pi. No network needed.
  See: <https://raspberrypi.stackexchange.com/questions/66431/headless-pi-zero-ssh-access-over-usb>

### Option 3: nRF Connect from your phone or computer (effort: easiest — phone app)

Nordic's **nRF Connect** app (iOS, Android, and desktop) is the lowest-friction way to play. No
drivers, no builds — install it from your app store, scan, and start poking at GATT. Workshop
staff support this approach.

**Keep in mind:** nRF Connect is a graphical BLE explorer, not a full attack tool. Some
challenges require capabilities it doesn't expose (raw handle-level tricks, MAC/address
spoofing, certain notification behaviors, scripted or bulk operations). Realistically you'll be
able to complete **around 60% of the exercises** with nRF Connect alone. It's a great backup or
a great "phone in one hand, laptop in the other" companion — just don't rely on it as your only
tool if you want to finish everything.

**iOS/macOS note (new for DEF CON 34):** Apple's Core Bluetooth identifies peripherals by an
opaque per-device UUID rather than exposing their real BLE MAC address. In past workshops this
made it hard for nRF Connect users on iPhones/iPads to tell *which* advertising device was their
CTF board. The **DEF CON 34 variant fixes this by giving the boards recognizable MAC-address
prefixes**, so you can reliably pick out and scan to your device from iOS/macOS.

Optional but useful even if you're on gratttool/gatttool: keep nRF Connect installed on a phone
as a quick sanity-check scanner.

---

## Unsupported Setups

The following are legitimately excellent approaches that the instructors use and love. But they
have a lot of moving parts, and we can't stop the class to debug them for you. If you go this
route: **you're on your own, or at the mercy of a clanker.** Bring your LLM of choice.

### WHAD

[WHAD](https://whad.io/) (Wireless HAcking Devices) is a fantastic, protocol-agnostic wireless
framework with a full set of BLE tools (`wble-central`, scanning, connecting, discovery, MitM,
sniffing, and more). Install is a one-liner:

```bash
pip install whad
```

To do more than talk to a local HCI adapter you'll generally want compatible hardware — e.g. a
Nordic/MakerDiary nRF52840 dongle flashed with WHAD's [ButteRFly](https://github.com/whad-team/butterfly)
firmware. That's more than we can troubleshoot mid-workshop, so consider WHAD a bring-it-if-you-
already-know-it option. Docs: <https://whad.readthedocs.io/>.

### Coding libraries / LLM-assisted (e.g. Python bleak)

Writing your own client with a BLE library is a great way to work through the CTF, and honestly
one of the more satisfying ones. [**bleak**](https://github.com/hbldh/bleak) is the go-to
cross-platform async Python BLE library:

```bash
pip install bleak
```

Pair it with an LLM (ChatGPT, Claude, etc.) and you can script solutions quickly. **You are
encouraged to use LLMs during the workshop** — but note the DEF CON 34 CTF variant is
specifically designed so that LLM assistance alone won't shortcut the challenges. You'll still
have to do the work.

Same caveat as WHAD: we won't provide support for library/scripting issues during class. You're
on your own, or at the mercy of a clanker.

Because bleak is cross-platform, this is also the one path that runs **natively on macOS and
Windows** with no Linux VM. Same Apple quirk as the nRF Connect note above — Core Bluetooth
surfaces devices by UUID rather than MAC — which is exactly what the DEF CON 34 MAC-prefix scheme
is designed to work around.

---

## Testing Your Setup

Do this **before** you show up. If it works, you're ready. You can test against *any* nearby BLE
peripheral — a fitness band, a spare phone advertising, another laptop — since the actual CTF
module is handed out at the workshop.

### If you're using gratttool (any Linux) — recommended

```bash
# daemon running (see Option 1)
sudo systemctl start bluetooth

# scan for nearby BLE devices
gratttool --scan

# connect to one and dump its GATT table (replace the MAC)
gratttool -b 11:22:33:44:55:66 --enumerate
```

A populated scan table plus a successful enumerate means you're ready.

### If you're using gatttool (Raspberry Pi)

```bash
# adapter up, daemon stopped (see Option 2)
sudo hciconfig hci0 up
sudo systemctl stop bluetooth

# scan for nearby BLE devices — Ctrl-C to stop
sudo hcitool lescan

# connect to one of the MACs from the scan and list its characteristics
# (replace 11:22:33:44:55:66; not every device is connectable — one success is enough)
gatttool -b 11:22:33:44:55:66 --characteristics
```

If you get a characteristics listing back without errors, you're good to go.

### If you're using nRF Connect

Open the app, run a scan, connect to any nearby device, and confirm you can browse its services
and read a characteristic value. That's the whole test.

---

## Common Problems

These are the issues attendees hit most often. Skimming them now will save you time later.

- **No BLE-capable adapter.** Some older laptops have pre-Bluetooth-4.0 radios that can't do
  Low Energy at all. Check *before* the workshop. The fix is cheap: any BLE-capable USB
  Bluetooth dongle will do.

- **Adapter blocked / won't come up (especially Kali).** This is almost always `rfkill`. On a
  modern laptop distro: `sudo rfkill unblock bluetooth`, then `bluetoothctl power on`. On the
  Raspberry Pi / gatttool path (where `hciconfig` is present): `sudo rfkill unblock all`, then
  `sudo hciconfig hci0 up`, then `sudo btmgmt le on`.

- **gratttool won't connect or can't find services.** gratttool needs the BlueZ daemon
  *running*. `sudo systemctl start bluetooth`.

- **gratttool won't start / "error while loading shared libraries".** The prebuilt binary needs
  BlueZ and libdbus present at runtime. Install them (see Option 1, step 1): on Debian-family,
  `sudo apt install bluetooth bluez libdbus-1-3`.

- **No prebuilt binary matches your architecture.** Fall back to building from source — Rust plus
  the `-dev` headers (see the fallback note at the end of Option 1).

- **gatttool "device busy" / can't connect.** Opposite problem: something is holding the adapter,
  usually `bluetoothd`. Stop it: `sudo systemctl stop bluetooth`.

- **Running Linux in a VM.** Make sure you do USB/device pass-through in the VM settings so the
  guest actually sees your Bluetooth adapter. Built-in laptop Bluetooth often doesn't pass
  through cleanly — a USB dongle you can explicitly assign to the VM is far more reliable.

- **Can't change your BLE (client) address.** Spoofing your own Bluetooth address requires an
  adapter whose chipset actually supports it. Many built-in laptop radios and cheap Realtek USB
  dongles don't — so a challenge that depends on this may not be doable on the hardware you
  brought. That's a known hardware limitation, not something you're doing wrong. The instructors
  bring a few spoofing-capable BLE dongles you can borrow during the workshop to finish it.

- **Expected a notification but saw nothing (gratttool).** Some CTF challenges notify on
  characteristics that don't advertise the NOTIFY property, and BlueZ drops those. Run
  `--listen` under `sudo` to capture them.

---

## Questions

Setup issues before the event? Open an issue on the [BLE CTF repo](https://github.com/hackgnar/ble_ctf)
or catch us at the workshop. See you at DEF CON 34.
