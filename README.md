## My Plan
- using LLRBtree to store and search data
- TTL support(scan thread and dead mark when access)
- persistence(data serialization)
- logs recovery(action serialization)
- memory snapshort (little CRIU)
- try gtest [done]
- query [done]
- for invisible byte, using url encode, 0x08 -> %08 [done]
- SOCKET and data structure training && programming [done]
- http request support [done]
- io uring [done]
- simple log [done]
- try to program with cpp [done]
## action

**PUT**
```
PUT /[key](?TTL=[secs?0=ferver]&TYPE=[...]) HTTP/x.x\r\n
...(skip those headers)
Content-Length: [length]\r\n
...
[val]\r\n
```


**DELETE** 
```
DELETE /[key] HTTP/x.x\r\n
```

**MODIFY** (for LIST/SET)
```
POST /[key](?ttl=[secs]) HTTP/x.x\r\n
...(skip those headers)
Content-Length: [length]\r\n
...
[val]\r\n
```
special key(key/advanced_act) for list 
if adv_act exists,reset ttl

**GET**
```
GET /[key] HTTP/x.x\r\n
```

When using '' or "", don't escape value.

**TODO**

~~maybe list is not necessary~~

## content type
    "STRING", ziplist
    "SET", rbtree store val
    "LIST", zip/link list
    <!-- "HASH", -->

### STRING
unnecessary

### SET

sadd -> add item

> POST /[key]/sadd ...

srm -> ...
> POST /[key]/srm
exists
> POST /[key]/exists ... [val]

GET -> all members
### LIST
all POST

lpop

rpop

key/insert/idx

key/idx_get/idx

llen




