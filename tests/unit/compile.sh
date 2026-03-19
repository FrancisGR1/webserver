cd ../../ 
make && make lib 
cd -
cp ../../libwebserv.a .
c++ -std=c++23 -I../../src/ resolve_target.cpp libwebserv.a -o resolve_target
