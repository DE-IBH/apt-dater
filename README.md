README for apt-dater
====================

With apt-dater you can easily keep one or more (Debian) GNU/Linux
hosts up to date.

Pre-configuration on your management server:
--------------------------------------------

  Copy and modify the apt-dater.conf:

    cp conf/apt-dater.conf.example $HOME/.config/apt-dater/apt-dater.conf

  Copy and modify the hosts.conf:

    cp conf/hosts.conf.example $HOME/.config/apt-dater/hosts.conf

  Copy and modify the screenrc:

    cp conf/screenrc.example $HOME/.config/apt-dater/screenrc


Managed hosts by apt-dater:
---------------------------

  You need a SSH server and sudo installed. Create a user which will
  be used to install updates (using root is NOT recommended).
  Modify the /etc/sudoers:

	Defaults env_reset,env_keep=MAINTAINER
	the-user ALL=NOPASSWD: /usr/bin/apt-get, /usr/sbin/needrestart


At your management server:
--------------------------

  Create a user on your management server which perform updates on your
  hosts.

  Generate a SSH keypair:

    ssh-keygen [-t TYPE] [..] -f ~/.ssh/apt-dater

  Distribute the public key(s) e.g.:

    ssh-copy-id -i ~/.ssh/apt-dater.pub the-user@managed-host
