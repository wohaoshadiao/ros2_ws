FROM ubuntu:22.04

RUN sed -i 's/ports.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list \
    && sed -i 's/ports.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list \
    && apt update && apt upgrade -y && DEBIAN_FRONTEND=noninteractive \
    apt install \
    tzdata \
    git \
    openssh-server \
    vim \
    zsh \
    silversearcher-ag \
    fzf \
    curl \
    tmux \
    -y \
    && sed -i 's/#Port 22/Port 2222/' /etc/ssh/sshd_config \
    && sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config \
    && chsh -s /bin/zsh \
    && sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" --unattended \
    && sed -i '/^plugins=(git)$/c\plugins=(\n    zsh-syntax-highlighting\n    zsh-autosuggestions\n    git\n    extract\n    ag\n)' ~/.zshrc \
    && git clone https://github.com/zsh-users/zsh-autosuggestions ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-autosuggestions \
    && git clone https://github.com/zsh-users/zsh-syntax-highlighting.git ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-syntax-highlighting \
    && git clone --depth=1 https://github.com/romkatv/powerlevel10k.git ${ZSH_CUSTOM:-$HOME/.oh-my-zsh/custom}/themes/powerlevel10k \
    && sed -i 's/ZSH_THEME="robbyrussell"/ZSH_THEME="powerlevel10k\/powerlevel10k"/' ~/.zshrc

RUN apt install software-properties-common -y \
    && add-apt-repository universe \
    && apt update \
    && curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | tee /etc/apt/sources.list.d/ros2.list > /dev/null \
    && apt update && apt upgrade -y \
    && apt install ros-humble-desktop python3-colcon-common-extensions python3-argcomplete -y \
    && echo "source /opt/ros/humble/setup.zsh" >> ~/.zshrc \
    && echo 'source /usr/share/colcon_cd/function/colcon_cd.sh' >> ~/.zshrc \
    && echo 'export _colcon_cd_root=/opt/ros/humble/' >> ~/.zshrc \
    && echo 'source /usr/share/colcon_argcomplete/hook/colcon-argcomplete.zsh' >> ~/.zshrc \
    && echo 'eval "$(register-python-argcomplete3 ros2)"' >> ~/.zshrc \
    && echo 'eval "$(register-python-argcomplete3 colcon)"' >> ~/.zshrc
