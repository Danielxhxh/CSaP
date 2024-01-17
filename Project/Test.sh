#!/bin/bash

# Signal Terminate (allow processes to terminate correctly)
cleanup() {
  echo ">>> Cleaning up processes... <<<"
  kill -SIGTERM $client1_pid $client2_pid
  kill -SIGTERM $server_pid
}

trap cleanup SIGINT

# 1. Start the server
echo ">>> Starting server <<<"
./server &
server_pid=$!

sleep 3

# 2. Run the first client
echo ">>> Running client 1 <<<"
./client <<EOF &
From client 1
EOF
client1_pid=$!

sleep 3

# 3. Run the second client
echo ">>> Running client 2 <<<"
./client <<EOF &
From client 2
EOF
client2_pid=$!

sleep 3

cleanup

echo ">>> Test completed. Check the log file. <<<"
