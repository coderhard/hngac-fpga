# Vitis on WSL Setup

**Last updated:** 2026-04-13  
**Purpose:** install the minimum AMD toolchain needed to run the first HLS synthesis for `hngac-fpga` without putting tools or build products on the Windows-backed tree.

## Goal

Set up AMD Vitis inside the WSL filesystem, keep synthesis work under Linux-native paths such as `~/tools`, `~/downloads`, and `/tmp`, and then run:

```bash
vitis_hls -f /mnt/c/Users/nomadic/projects/hngac-fpga/fpga/hls/scripts/vitis_hls.tcl
```

## What needs to be installed

Install these items:

1. AMD `Vitis` for Linux
2. AMD post-install Linux dependency script from the Vitis install tree: `installLibs.sh`

Do **not** install these yet unless a later step explicitly needs them:

- XRT
- board platform packages
- random extra Ubuntu packages not requested by AMD's installer script

For the current repo state, the immediate next task is HLS `csim` plus `csynth`, not deployment.

## Recommended install locations

Keep everything on the WSL filesystem, not under `/mnt/c`:

- installer download: `~/downloads/amd`
- extracted installer: `~/downloads/amd/<installer-dir>`
- Vitis install root: `~/tools/amd`
- HLS work directory for this repo: `/tmp/hngac-fpga-hls`

Avoid these locations:

- `/mnt/c/Users/...`
- any OneDrive-backed path
- the repo checkout itself for large generated HLS projects

## What is already present on this machine

Verified in WSL on 2026-04-13:

- `cmake`
- `make`
- `gcc`
- `g++`
- `git`
- `zip`
- `unzip`
- `perl`
- `python3`
- `sqlite3`

That means no extra general-purpose C/C++ build tool install is needed before Vitis.

## Memory and disk guidance

AMD's Vitis installation guide lists these Linux-side minimums for embedded/HLS development:

- Ubuntu 24.04 LTS is supported
- 32 GB RAM minimum
- 25 GB disk for `Vitis Embedded Development`
- 200 GB disk for full `Vitis`

This WSL instance reported about `19 GiB` RAM available on 2026-04-13, which is below AMD's stated minimum. Disk is not a problem on the current Linux filesystem.

If the Windows host has enough RAM, increase the WSL memory cap with `%UserProfile%\.wslconfig`:

```ini
[wsl2]
memory=32GB
swap=16GB
processors=8
```

Then restart WSL from Windows:

```powershell
wsl --shutdown
```

Reopen the distro after that.

## Install steps

### 1. Download the Linux installer

From the AMD Adaptive Computing downloads site, download the Linux `Vitis` installer into `~/downloads/amd`.

Example prep:

```bash
mkdir -p ~/downloads/amd ~/tools/amd
cd ~/downloads/amd
```

### 2. Extract and launch the installer inside WSL

Use the extracted Linux installer, not the Windows installer:

```bash
tar -xf <downloaded-installer>.tar.gz
cd <extracted-installer-dir>
./xsetup
```

### 3. Installer choices

Inside the installer:

1. Sign in with your AMD account if prompted.
2. Choose `Vitis`.
3. Set the destination directory to `~/tools/amd`.
4. Keep `Vitis HLS` selected.
5. If device-family selection is shown, select only the family you actually plan to target.

Use the full `Vitis` product for this repo, because the full Vitis install includes `Vitis HLS`, which is the tool the repo's Tcl flow expects.

### 4. Run AMD's post-install dependency script

After the installer finishes:

```bash
sudo ~/tools/amd/<release>/Vitis/scripts/installLibs.sh
```

Replace `<release>` with the version you installed, for example `2025.1`.

If the script reports a missing package that it does not install automatically, install only that specific package and rerun the script.

## Shell setup

For the current shell:

```bash
source ~/tools/amd/<release>/Vitis/settings64.sh
which vitis_hls
vitis_hls -version
```

If that succeeds, make it persistent:

```bash
echo 'source ~/tools/amd/<release>/Vitis/settings64.sh' >> ~/.bashrc
```

## First repo run

Once `vitis_hls` is available:

```bash
source ~/tools/amd/<release>/Vitis/settings64.sh
export HNGAC_HLS_PART=<your-target-part>
export HNGAC_HLS_CLOCK_NS=5.0
export HNGAC_HLS_WORKDIR=/tmp/hngac-fpga-hls
vitis_hls -f /mnt/c/Users/nomadic/projects/hngac-fpga/fpga/hls/scripts/vitis_hls.tcl
```

Notes:

- `HNGAC_HLS_PART` is required by the Tcl script.
- `HNGAC_HLS_WORKDIR=/tmp/hngac-fpga-hls` keeps generated synthesis outputs off the repo checkout.
- the current script supports optional `HNGAC_HLS_COSIM=1` and `HNGAC_HLS_EXPORT=1`, but the first pass should stay at plain `csim` plus `csynth`.

## What not to install yet

Do not spend time on these before the first `csynth` pass:

- XRT
- Alveo runtime drivers
- board/platform repositories
- OPA
- any database or RPC stack for the benchmark harness

Those are not needed to get the first synthesis reports for `hngac_authorize`.

## Troubleshooting

If `vitis_hls` is not found after install:

```bash
find ~/tools/amd -path '*/Vitis/settings64.sh'
find ~/tools/amd -name vitis_hls
```

Then source the matching `settings64.sh` manually.

If synthesis fails immediately with environment errors:

- confirm `HNGAC_HLS_PART` is set
- confirm `vitis_hls -version` works in the same shell
- confirm the install is under the WSL filesystem, not `/mnt/c`

If the tool starts but crashes or is killed:

- re-check WSL RAM limits
- ensure `/tmp` has space
- avoid running the HLS project from the Windows-backed filesystem

If AMD licensing errors appear:

- resolve them through the AMD licensing flow documented in UG973
- do not guess at license file placement; follow the release-specific AMD guidance

## References

- AMD Vitis product page: <https://www.amd.com/en/products/software/adaptive-socs-and-fpgas/vitis.html>
- AMD Vitis Release Notes and Installation Guide, UG1742:
  - environment setup: <https://docs.amd.com/r/2025.1-English/ug1742-vitis-release-notes/Setting-Up-the-Environment-to-Run-the-Vitis-Software-Platform>
  - install requirements and install flow: <https://docs.amd.com/r/en-US/ug1742-vitis-release-notes/Vitis-Embedded-Platforms>
- AMD licensing overview, UG973: <https://docs.amd.com/r/2024.2-English/ug973-vivado-release-notes-install-license/Product-Licensing-Account-Overview>
- Microsoft WSL advanced config: <https://learn.microsoft.com/en-us/windows/wsl/wsl-config>
