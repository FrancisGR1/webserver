source .env

for i in $(seq 1 ${NUM_DELETE_FILES}); do
    touch "$ROOT_DIR/delete/delete_$i.txt"
done

# populate delete/ with files to delete
cd config/files/delete
for i in $(seq 1 ${NUM_OF_FILES_TO_DELETE}); do
	echo "touch delete_me_$i.txt at $PWD"
	touch "delete_$i.txt"
done
