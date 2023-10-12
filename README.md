## tiny redis

### A simple and stupid tiny redis server implementation, only supports string as value,used for learning purpose

### What did I do
- No persistence support
- TTL support(lazy deletion(when access) / timer deletion(scaning whole the search tree periodically)
- Left-Leaning RB Tree structure
- Basic socket programming
- Http request as commands(url decode support)
- Hybird programming based on C and C++
- Basic gtest unit tests (very limited coverage)
- Single thread, no race condition
- Event loop based on io uring
- Basic io uring using

### specail dependencies
- IO\_Uring
- gtest

### examples

upload a string as the value
> curl -X POST -d "value" "localhost:8000/key"

upload a file content as the value 
> curl -X POST -T 200M.txt "localhost:8000/200M"

TTL/s 
> curl -X POST -d "value" "localhost:8000/key?TTL=10"

delete
> curl -X DELETE "localhost:8000/key"


### limits
- MAX KEY SIZE: less than 2048 byte (design bug)
- MAX VALUE SIZE: less than 512 MB
- **NO PERSISTENCE SUPPORT**
