cp tmux.tar.gz /tmp
mount -o remount,exec /tmp
cd /tmp
tar -xvf tmux.tar.gz

export LD_LIBRARY_PATH=/tmp/tmux/lib

/tmp/tmux/bin/tmux
