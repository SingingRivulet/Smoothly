# Smoothly  
An Open-Source Sandbox Strategy Game  
## Install：  
Please install irrlicht,bullet3,lua5.1 and raknet at first,and then run `make` in terminal.  
## Run:  
### run server:  
`./server`  
default port is `39065`  
### create a user:  
run `./admin --adminName admin --password 1234 --mode createUser --X 0 --Y 100 --Z 0 --nsid 1 --npwd userPassword`  
This commond will create a new user and show the UUID of new user and it will be used in next step.  
### run client:  
`./client 127.0.0.1 39065  yourUUID yourPassword`  
## View：  
![img](./screenshot/2019-03-12-23-09-36.png)  
