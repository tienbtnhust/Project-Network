# C Chess
> Online chess game written in C, using sockets, threads, dynamic memory and more

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
current position-next position (Ex: a2-a3)