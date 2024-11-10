all:
	protoc -I=../proto --java_out=. ../proto/game_comm.proto

compile:
	javac 

build-docker:
	docker build -t test-ref -f dep_docker/Dockerfile .