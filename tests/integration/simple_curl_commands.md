# create file
curl -X POST http://127.0.0.1:8080/upload -d hello=world
# get file
curl -X GET http://127.0.0.1:8080/upload/0.data
# delete file created by POST
curl -X DELETE http://127.0.0.1:8080/upload/0.data
