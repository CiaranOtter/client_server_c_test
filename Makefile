all:
	protoc -I=../proto --java_out=. ../proto/game_comm.proto

compile:
	javac 