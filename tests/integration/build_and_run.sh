tmux new-session -d -s webserv './webserv.out config/default.conf'
echo "do: 'tmux attach -t webserv'"
SERVER_PID=$!
docker build -t webserv-tests .
docker run --rm --network host webserv-tests
kill $SERVER_PID
