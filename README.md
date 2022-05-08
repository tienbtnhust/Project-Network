# C Chess
> Online chess game written in C, using sockets, threads, dynamic memory and more

## Group Members
Le Thanh Hai - 20184254 <br>
Bui Thuc Nguyen Tien - 20180180 <br>
Le Ba Vinh - 20184331 <br>


## Build
**Server**
```
> gcc cchess-server.c board.c -o server -pthread
```
**Client**
```
> gcc cchess-client.c -o client -pthread
```

## Run
**Server**
```
> ./server
```
**Client**
```
> ./client localhost 8080
```

## How to move
```
> current position-next position (Ex: a2-a3)
