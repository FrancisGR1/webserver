# stop:
nginx -s stop

# start
nginx -c $(pwd)/nginx.conf

# verifica se está a funcionar:
# curl -v http://localhost:8080/

# enviar pedidos:
# nc localhost 8080
# <http request>

