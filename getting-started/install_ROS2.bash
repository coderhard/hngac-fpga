#/usr/bin/bash
## 1. Setup Sources and Keys
sudo apt update && sudo apt install -y locales software-properties-common curl
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8

sudo add-apt-repository universe -y
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg

echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null

# 2. Install ROS 2 Jazzy, Gazebo, and Build Tools
sudo apt update
sudo apt install -y ros-jazzy-desktop \
                    ros-dev-tools \
                    build-essential \
                    cmake \
                    valgrind

# 3. Initialize ROS Dependencies
sudo rosdep init
rosdep update

# 4. Auto-source ROS 2 every time you open a terminal
echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc
source ~/.bashrc

echo "✅ Installation Complete. You are ready for Phase 1 and 2."
