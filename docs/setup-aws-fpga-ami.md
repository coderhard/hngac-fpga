# AWS FPGA Developer AMI Setup

**Last updated:** 2026-04-13  
**Purpose:** provision an AWS-hosted Linux environment with AMD FPGA tools already installed, avoid the local WSL memory ceiling, and keep an F1 fallback profile disabled until you explicitly choose to use it.

## Recommended path

For this repo, start with the AWS FPGA Developer AMI in `us-east-1` and use a non-FPGA build instance first.

Recommended defaults:

- Region: `us-east-1`
- AMI: AWS FPGA Developer AMI `1.19.0`, Ubuntu 24.04
- AMI ID in `us-east-1`: `ami-06691812f80312a04`
- Build instance type: `c7i.4xlarge`
- Root volume: `200 GiB gp3`
- Security: `22/tcp` from your IP only, optional `8443/tcp` for DCV from your IP only

Why this path:

- AWS ships the FPGA Developer AMI with AMD tools preinstalled.
- AWS's F2 documentation says the AMD tools work best with at least `4 vCPU` and `32 GiB` memory, and explicitly recommends compute-optimized or memory-optimized instances for synthesis.
- You do **not** need an FPGA-backed instance just to run the first `vitis_hls` synthesis for this repo.

## Instance profiles in this repo

The launcher in [scripts/aws/launch_fpga_dev_instance.sh](/mnt/c/Users/nomadic/projects/hngac-fpga/scripts/aws/launch_fpga_dev_instance.sh:1) supports three targets:

- `f2-dev`
  - default
  - AMI: `ami-06691812f80312a04`
  - instance type: `c7i.4xlarge`
  - use this for the first HLS synthesis and general tool work
- `f2-hardware`
  - AMI: `ami-06691812f80312a04`
  - instance type: `f2.6xlarge`
  - use this later if you need actual F2 hardware on the box
- `f1-retired`
  - disabled by default
  - intended only as a legacy fallback
  - blocked unless `AWS_FPGA_ENABLE_F1=true`

## F1 status

AWS's official `aws-fpga` repository states that Amazon EC2 `F1` reached end of life on **December 20, 2025**.

It also states:

- only existing F1 customers who ran F1 instances between **December 2023 and December 2024** can restart or launch new F1 instances
- by default, AWS accounts historically had an `F1` launch limit of `0`

That is why the repo's launcher keeps the F1 profile disabled until you explicitly enable it.

## AWS CLI prerequisites

You need:

1. AWS CLI v2 installed locally
2. an authenticated AWS CLI profile
3. EC2 permissions for:
   - describing VPCs, subnets, key pairs, instances, and security groups
   - creating security groups
   - authorizing security-group ingress
   - creating key pairs if you enable that option
   - launching EC2 instances
   - tagging instances

If you use IAM Identity Center, the clean path is:

```bash
aws configure sso --profile hngac-aws
aws sso login --profile hngac-aws
```

If you use long-lived access keys instead:

```bash
aws configure --profile hngac-aws
```

## Repo automation files

- env template: [scripts/aws/fpga-dev.env.example](/mnt/c/Users/nomadic/projects/hngac-fpga/scripts/aws/fpga-dev.env.example:1)
- launcher: [scripts/aws/launch_fpga_dev_instance.sh](/mnt/c/Users/nomadic/projects/hngac-fpga/scripts/aws/launch_fpga_dev_instance.sh:1)

The launcher does all of the following:

- validates AWS CLI authentication
- resolves the default VPC and a default subnet unless you override them
- creates or reuses a security group
- restricts SSH to your current public IP or an explicit CIDR you provide
- optionally opens `8443/tcp` for Amazon DCV
- reuses or creates an EC2 key pair
- launches the instance with a `gp3` root volume
- waits for `instance-running` and optionally `instance-status-ok`
- prints the public IP/DNS and an SSH command

It also supports an optional `AWS_FPGA_USER_DATA_FILE` hook if you later want EC2 user-data to bootstrap a repo checkout or kick off a build automatically.

## Recommended first launch

1. Copy the env template:

```bash
cp scripts/aws/fpga-dev.env.example scripts/aws/fpga-dev.env
```

2. Edit `scripts/aws/fpga-dev.env` and set at least:

- `AWS_FPGA_KEY_NAME`
- `AWS_FPGA_CREATE_KEY_PAIR=true` if you want the script to make one
- `AWS_FPGA_ENABLE_DCV=true` if you want GUI access
- `AWS_FPGA_TAG_OWNER` if you want an owner tag

3. Launch:

```bash
AWS_PROFILE=hngac-aws scripts/aws/launch_fpga_dev_instance.sh scripts/aws/fpga-dev.env
```

The default result is:

- target: `f2-dev`
- AMI: `ami-06691812f80312a04`
- instance type: `c7i.4xlarge`

## After login

On the new instance:

```bash
source /etc/profile.d/default_module.sh
vitis_hls -version
vivado -version
```

Then clone your repo onto the instance's Linux filesystem and run:

```bash
export HNGAC_HLS_PART=<your-target-part>
export HNGAC_HLS_CLOCK_NS=5.0
export HNGAC_HLS_WORKDIR=/tmp/hngac-fpga-hls
vitis_hls -f fpga/hls/scripts/vitis_hls.tcl
```

## About the target part

This repo's local HLS Tcl flow currently expects `HNGAC_HLS_PART` to be set explicitly. The launcher does **not** guess the part value for you because that decision depends on whether you are doing:

- a generic local HLS estimate
- an AWS F2 integration path
- a different on-prem target part

Keep that decision explicit.

## Optional automation hook

If you want the EC2 instance to run custom bootstrap actions at first boot, set:

```bash
AWS_FPGA_USER_DATA_FILE=/absolute/path/to/your-user-data.sh
```

in `scripts/aws/fpga-dev.env`.

The launcher passes that through as:

```bash
--user-data file://<path>
```

That is the clean place to automate:

- package installs beyond the base AMI
- repo cloning
- artifact volume mounting
- post-launch build startup

The repo does not force a default user-data build because repo access, secrets, and target part selection are project-specific.

## F1 fallback

If you later decide to pursue the legacy F1 path anyway:

1. open `scripts/aws/fpga-dev.env`
2. set:

```bash
AWS_FPGA_TARGET=f1-retired
AWS_FPGA_ENABLE_F1=true
```

3. confirm your AWS account is still eligible to launch F1
4. rerun the launcher

The default legacy placeholders are:

- AMI ID: `ami-092fc5deb8f3c0f7d`
- instance type: `f1.2xlarge`

Treat this profile as best-effort only.

## References

- AWS F2 developer kit overview and current FPGA Developer AMI IDs:
  <https://awsdocs-fpga-f2.readthedocs-hosted.com/latest/User-Guide-AWS-EC2-FPGA-Development-Kit.html>
- AWS F2 `f2.6xlarge` availability announcement:
  <https://aws.amazon.com/about-aws/whats-new/2025/02/amazon-ec2-f2-6xlarge-new-f2-instance-size/>
- AWS EC2 compute optimized instance specs:
  <https://docs.aws.amazon.com/ec2/latest/instancetypes/co.html>
- AWS EC2 memory optimized instance specs:
  <https://docs.aws.amazon.com/ec2/latest/instancetypes/mo.html>
- AWS CLI IAM Identity Center setup:
  <https://docs.aws.amazon.com/cli/latest/userguide/cli-configure-sso.html>
- AWS CLI `aws configure sso`:
  <https://docs.aws.amazon.com/cli/latest/reference/configure/sso.html>
- AWS CLI `aws sso login`:
  <https://docs.aws.amazon.com/cli/latest/reference/sso/login.html>
- AWS official `aws-fpga` repository with the F1 retirement notice:
  <https://github.com/aws/aws-fpga>
