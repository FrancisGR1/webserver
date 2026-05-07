# multiple iterations of go tests
for i in $(seq 1 5); do go test && echo "run $i ok"; done

# siege
