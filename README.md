# Smoothly  
An Open-Source Sandbox Strategy Game  
## Install：  
Please install irrlicht,bullet3,lua5.1 and raknet at first,and then run `make` in terminal.  
## Run:  
### run server:  
`./server`  
And then server will run on `39065` port  
### create a user:  
run `./admin --adminName admin --password 1234 --mode createUser --X 0 --Y 100 --Z 0 --nsid 1 --npwd userPassword`  
This commond will create a new user and show the UUID of new user and it will be used in next step.  
### run client:  
`./client 127.0.0.1 39065 [yourUUID] [yourPassword]`  
### teleport player:  
`./admin --adminName admin --password 1234 --mode teleport --X 2000 --Y 900 --Z 2000 --substance [substance uuid]`  
## View：  
![img](./screenshot/2019-05-10-17-26-15.png)  
