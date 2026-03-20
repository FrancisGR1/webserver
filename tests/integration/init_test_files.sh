source .env

# populate delete/ with files to delete

cd server_files/delete
for i in $(seq 1 ${NUM_OF_FILES_TO_DELETE}); do
	echo "touch delete_me_$i.txt at $PWD/server_files/delete"
	touch "delete_$i.txt"
done
