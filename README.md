steamlink keyboard watcher utility

functionality
-------------

- double printscreen reboots steamlink
- double numlock switches kvm port (with cheap TK-207K USB KVM)

building
--------

- download steam link SDK
- cd /path/to/steamlink/sdk
- source ./setenv.sh
- cd /path/to/kvmkeywatcher
- make
- enable ssh on steamlink: https://github.com/ValveSoftware/steamlink-sdk#ssh-access
- scp S99kvmkeywatcher-arm root@ip.of.steamlink:/etc/init.d/startup/
- power cycle steamlink -- functionality should then be active

optionally disable ssh on steamlink by:

- (unplug ssh enabling usb stick if still plugged in)
- ssh root@ip.of.steamlink
- rm /mnt/config/system/enable_ssh.txt
- reboot
