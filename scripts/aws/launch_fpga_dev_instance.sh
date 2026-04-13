#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

fail() {
  echo "ERROR: $*" >&2
  exit 1
}

note() {
  echo "==> $*"
}

bool_true() {
  case "${1:-}" in
    1|true|TRUE|yes|YES|on|ON) return 0 ;;
    *) return 1 ;;
  esac
}

normalize_text() {
  local value="${1:-}"
  if [[ "${value}" == "None" || "${value}" == "null" ]]; then
    echo ""
  else
    echo "${value}"
  fi
}

aws_text() {
  local output
  output="$(aws --region "${AWS_REGION}" "$@" --output text)"
  normalize_text "${output}"
}

authorize_ingress() {
  local group_id="$1"
  local port="$2"
  local cidr="$3"
  local description="$4"
  local output
  local status

  set +e
  output="$(
    aws --region "${AWS_REGION}" ec2 authorize-security-group-ingress \
      --group-id "${group_id}" \
      --ip-permissions "[{\"IpProtocol\":\"tcp\",\"FromPort\":${port},\"ToPort\":${port},\"IpRanges\":[{\"CidrIp\":\"${cidr}\",\"Description\":\"${description}\"}]}]" \
      2>&1
  )"
  status=$?
  set -e

  if [[ ${status} -ne 0 && "${output}" != *"InvalidPermission.Duplicate"* ]]; then
    echo "${output}" >&2
    exit "${status}"
  fi
}

detect_ssh_cidr() {
  if [[ -n "${AWS_FPGA_SSH_CIDR:-}" ]]; then
    if [[ "${AWS_FPGA_SSH_CIDR}" == */* ]]; then
      echo "${AWS_FPGA_SSH_CIDR}"
    else
      echo "${AWS_FPGA_SSH_CIDR}/32"
    fi
    return 0
  fi

  command -v curl >/dev/null 2>&1 || fail "Set AWS_FPGA_SSH_CIDR or install curl to autodetect your public IP."
  local detected_ip
  detected_ip="$(curl -fsSL https://checkip.amazonaws.com | tr -d '[:space:]')"
  [[ -n "${detected_ip}" ]] || fail "Unable to detect your public IP. Set AWS_FPGA_SSH_CIDR explicitly."
  echo "${detected_ip}/32"
}

CONFIG_FILE="${1:-${AWS_FPGA_CONFIG_FILE:-${SCRIPT_DIR}/fpga-dev.env}}"
if [[ -f "${CONFIG_FILE}" ]]; then
  # shellcheck source=/dev/null
  source "${CONFIG_FILE}"
elif [[ $# -gt 0 ]]; then
  fail "Config file not found: ${CONFIG_FILE}"
fi

: "${AWS_REGION:=us-east-1}"
: "${AWS_FPGA_TARGET:=f2-dev}"
: "${AWS_FPGA_INSTANCE_NAME:=hngac-fpga-dev}"
: "${AWS_FPGA_VOLUME_SIZE_GB:=200}"
: "${AWS_FPGA_SECURITY_GROUP_NAME:=hngac-fpga-dev-sg}"
: "${AWS_FPGA_ENABLE_DCV:=false}"
: "${AWS_FPGA_WAIT_FOR_OK:=true}"
: "${AWS_FPGA_KEY_NAME:=hngac-fpga-dev}"
: "${AWS_FPGA_CREATE_KEY_PAIR:=false}"
: "${AWS_FPGA_TAG_PROJECT:=hngac-fpga}"
: "${AWS_FPGA_TAG_OWNER:=}"
: "${AWS_FPGA_F2_UBUNTU_AMI_ID:=ami-06691812f80312a04}"
: "${AWS_FPGA_F2_ROCKY_AMI_ID:=ami-0648ba8008e805297}"
: "${AWS_FPGA_F2_DEV_INSTANCE_TYPE:=c7i.4xlarge}"
: "${AWS_FPGA_F2_HW_INSTANCE_TYPE:=f2.6xlarge}"
: "${AWS_FPGA_ENABLE_F1:=false}"
: "${AWS_FPGA_F1_AMI_ID:=ami-092fc5deb8f3c0f7d}"
: "${AWS_FPGA_F1_INSTANCE_TYPE:=f1.2xlarge}"

command -v aws >/dev/null 2>&1 || fail "AWS CLI not found. Install it first."
aws sts get-caller-identity >/dev/null 2>&1 || fail "AWS CLI is not authenticated. Run aws configure sso / aws sso login or aws configure first."

case "${AWS_FPGA_TARGET}" in
  f2-dev)
    : "${AWS_FPGA_AMI_ID:=${AWS_FPGA_F2_UBUNTU_AMI_ID}}"
    : "${AWS_FPGA_INSTANCE_TYPE:=${AWS_FPGA_F2_DEV_INSTANCE_TYPE}}"
    ;;
  f2-hardware)
    : "${AWS_FPGA_AMI_ID:=${AWS_FPGA_F2_UBUNTU_AMI_ID}}"
    : "${AWS_FPGA_INSTANCE_TYPE:=${AWS_FPGA_F2_HW_INSTANCE_TYPE}}"
    ;;
  f1-retired)
    bool_true "${AWS_FPGA_ENABLE_F1}" || fail "F1 is disabled by default. Set AWS_FPGA_ENABLE_F1=true only when you are ready to use the retired F1 path."
    : "${AWS_FPGA_AMI_ID:=${AWS_FPGA_F1_AMI_ID}}"
    : "${AWS_FPGA_INSTANCE_TYPE:=${AWS_FPGA_F1_INSTANCE_TYPE}}"
    ;;
  *)
    fail "Unknown AWS_FPGA_TARGET: ${AWS_FPGA_TARGET}"
    ;;
esac

AWS_FPGA_SSH_CIDR="$(detect_ssh_cidr)"

if [[ -z "${AWS_FPGA_VPC_ID:-}" ]]; then
  AWS_FPGA_VPC_ID="$(aws_text ec2 describe-vpcs --filters Name=isDefault,Values=true --query 'Vpcs[0].VpcId')"
fi
[[ -n "${AWS_FPGA_VPC_ID}" ]] || fail "No default VPC found. Set AWS_FPGA_VPC_ID explicitly."

if [[ -z "${AWS_FPGA_SUBNET_ID:-}" ]]; then
  AWS_FPGA_SUBNET_ID="$(aws_text ec2 describe-subnets --filters Name=vpc-id,Values="${AWS_FPGA_VPC_ID}" Name=default-for-az,Values=true --query 'Subnets[0].SubnetId')"
fi
[[ -n "${AWS_FPGA_SUBNET_ID}" ]] || fail "No default subnet found. Set AWS_FPGA_SUBNET_ID explicitly."

AWS_FPGA_SECURITY_GROUP_ID="$(
  aws_text ec2 describe-security-groups \
    --filters Name=vpc-id,Values="${AWS_FPGA_VPC_ID}" Name=group-name,Values="${AWS_FPGA_SECURITY_GROUP_NAME}" \
    --query 'SecurityGroups[0].GroupId'
)"
if [[ -z "${AWS_FPGA_SECURITY_GROUP_ID}" ]]; then
  note "Creating security group ${AWS_FPGA_SECURITY_GROUP_NAME} in ${AWS_FPGA_VPC_ID}"
  AWS_FPGA_SECURITY_GROUP_ID="$(
    aws_text ec2 create-security-group \
      --group-name "${AWS_FPGA_SECURITY_GROUP_NAME}" \
      --description "hngac-fpga developer access" \
      --vpc-id "${AWS_FPGA_VPC_ID}" \
      --query 'GroupId'
  )"
else
  note "Reusing security group ${AWS_FPGA_SECURITY_GROUP_NAME} (${AWS_FPGA_SECURITY_GROUP_ID})"
fi

authorize_ingress "${AWS_FPGA_SECURITY_GROUP_ID}" 22 "${AWS_FPGA_SSH_CIDR}" "SSH from operator"
if bool_true "${AWS_FPGA_ENABLE_DCV}"; then
  authorize_ingress "${AWS_FPGA_SECURITY_GROUP_ID}" 8443 "${AWS_FPGA_SSH_CIDR}" "DCV from operator"
fi

if bool_true "${AWS_FPGA_CREATE_KEY_PAIR}"; then
  : "${AWS_FPGA_KEY_PATH:=${HOME}/.ssh/${AWS_FPGA_KEY_NAME}.pem}"
  if [[ -e "${AWS_FPGA_KEY_PATH}" ]]; then
    fail "Refusing to overwrite existing key file: ${AWS_FPGA_KEY_PATH}"
  fi
  if [[ -n "$(aws_text ec2 describe-key-pairs --key-names "${AWS_FPGA_KEY_NAME}" --query 'KeyPairs[0].KeyName' 2>/dev/null || true)" ]]; then
    fail "Key pair ${AWS_FPGA_KEY_NAME} already exists in AWS. Reuse it or choose a new name."
  fi
  note "Creating key pair ${AWS_FPGA_KEY_NAME}"
  mkdir -p "$(dirname "${AWS_FPGA_KEY_PATH}")"
  aws --region "${AWS_REGION}" ec2 create-key-pair \
    --key-name "${AWS_FPGA_KEY_NAME}" \
    --query 'KeyMaterial' \
    --output text > "${AWS_FPGA_KEY_PATH}"
  chmod 600 "${AWS_FPGA_KEY_PATH}"
else
  [[ -n "$(aws_text ec2 describe-key-pairs --key-names "${AWS_FPGA_KEY_NAME}" --query 'KeyPairs[0].KeyName' 2>/dev/null || true)" ]] \
    || fail "EC2 key pair ${AWS_FPGA_KEY_NAME} not found. Create it first or set AWS_FPGA_CREATE_KEY_PAIR=true."
fi

RUN_ARGS=(
  --region "${AWS_REGION}"
  ec2 run-instances
  --image-id "${AWS_FPGA_AMI_ID}"
  --instance-type "${AWS_FPGA_INSTANCE_TYPE}"
  --count 1
  --key-name "${AWS_FPGA_KEY_NAME}"
  --security-group-ids "${AWS_FPGA_SECURITY_GROUP_ID}"
  --subnet-id "${AWS_FPGA_SUBNET_ID}"
  --metadata-options "HttpTokens=required"
  --block-device-mappings "DeviceName=/dev/sda1,Ebs={VolumeSize=${AWS_FPGA_VOLUME_SIZE_GB},VolumeType=gp3,DeleteOnTermination=true}"
  --tag-specifications "ResourceType=instance,Tags=[{Key=Name,Value=${AWS_FPGA_INSTANCE_NAME}},{Key=Project,Value=${AWS_FPGA_TAG_PROJECT}},{Key=FpgaTarget,Value=${AWS_FPGA_TARGET}},{Key=ManagedBy,Value=launch_fpga_dev_instance.sh}]"
)
if [[ -n "${AWS_FPGA_USER_DATA_FILE:-}" ]]; then
  [[ -f "${AWS_FPGA_USER_DATA_FILE}" ]] || fail "User-data file not found: ${AWS_FPGA_USER_DATA_FILE}"
  RUN_ARGS+=(--user-data "file://${AWS_FPGA_USER_DATA_FILE}")
fi

note "Launching ${AWS_FPGA_TARGET} instance in ${AWS_REGION}"
AWS_FPGA_INSTANCE_ID="$(
  aws "${RUN_ARGS[@]}" --query 'Instances[0].InstanceId' --output text
)"

if [[ -n "${AWS_FPGA_TAG_OWNER}" ]]; then
  aws --region "${AWS_REGION}" ec2 create-tags \
    --resources "${AWS_FPGA_INSTANCE_ID}" \
    --tags "Key=Owner,Value=${AWS_FPGA_TAG_OWNER}"
fi

note "Waiting for instance-running: ${AWS_FPGA_INSTANCE_ID}"
aws --region "${AWS_REGION}" ec2 wait instance-running --instance-ids "${AWS_FPGA_INSTANCE_ID}"
if bool_true "${AWS_FPGA_WAIT_FOR_OK}"; then
  note "Waiting for instance-status-ok: ${AWS_FPGA_INSTANCE_ID}"
  aws --region "${AWS_REGION}" ec2 wait instance-status-ok --instance-ids "${AWS_FPGA_INSTANCE_ID}"
fi

INSTANCE_AZ="$(
  aws_text ec2 describe-instances --instance-ids "${AWS_FPGA_INSTANCE_ID}" --query 'Reservations[0].Instances[0].Placement.AvailabilityZone'
)"
INSTANCE_PUBLIC_IP="$(
  aws_text ec2 describe-instances --instance-ids "${AWS_FPGA_INSTANCE_ID}" --query 'Reservations[0].Instances[0].PublicIpAddress'
)"
INSTANCE_PUBLIC_DNS="$(
  aws_text ec2 describe-instances --instance-ids "${AWS_FPGA_INSTANCE_ID}" --query 'Reservations[0].Instances[0].PublicDnsName'
)"

echo
echo "Instance launch complete."
echo "  Instance ID:      ${AWS_FPGA_INSTANCE_ID}"
echo "  Target profile:   ${AWS_FPGA_TARGET}"
echo "  Region:           ${AWS_REGION}"
echo "  AvailabilityZone: ${INSTANCE_AZ}"
echo "  AMI ID:           ${AWS_FPGA_AMI_ID}"
echo "  Instance type:    ${AWS_FPGA_INSTANCE_TYPE}"
echo "  Subnet ID:        ${AWS_FPGA_SUBNET_ID}"
echo "  Security group:   ${AWS_FPGA_SECURITY_GROUP_ID}"
echo "  SSH CIDR:         ${AWS_FPGA_SSH_CIDR}"
if [[ -n "${INSTANCE_PUBLIC_IP}" ]]; then
  echo "  Public IP:        ${INSTANCE_PUBLIC_IP}"
fi
if [[ -n "${INSTANCE_PUBLIC_DNS}" ]]; then
  echo "  Public DNS:       ${INSTANCE_PUBLIC_DNS}"
fi
if [[ -n "${AWS_FPGA_KEY_PATH:-}" ]]; then
  echo "  Key path:         ${AWS_FPGA_KEY_PATH}"
  if [[ -n "${INSTANCE_PUBLIC_DNS}" ]]; then
    echo "  SSH:              ssh -i ${AWS_FPGA_KEY_PATH} ubuntu@${INSTANCE_PUBLIC_DNS}"
  fi
else
  echo "  Key pair:         ${AWS_FPGA_KEY_NAME}"
fi
