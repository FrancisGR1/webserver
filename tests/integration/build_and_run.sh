tmux new-session -d -s webserv './webserv.out config/default.conf'
echo "do: 'tmux attach -t webserv'"
docker build -t webserv-tests .
docker run --rm --network host webserv-tests
