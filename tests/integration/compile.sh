cd ../../ && make
cd -
cp ../../webserv .
CGO_ENABLED=0 go build
