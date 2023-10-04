## My Plan
- try gtest 
- using LLRBtree to store and search data
- persistence
- memory snapshort (little CRIU)
- logs recovery
- query [done]
- SOCKET and data structure training && programming [done]
- http request support [done]
- io uring [done]
- simple log [done]
- try to program with moderncpp [not necessary]
## action
**PUT**
```
POST /[key](?ttl=[secs]) HTTP/x.x\r\n
...(skip those headers)
Content-Length: [length]\r\n
...
[val]\r\n
```

**DELETE** 
```
DELETE /[key] HTTP/x.x\r\n
```

**MODIFY**
```
POST /[key](?ttl=[secs]) HTTP/x.x\r\n
...(skip those headers)
Content-Length: [length]\r\n
...
[val]\r\n
```

**GET**
```
GET /[key] HTTP/x.x\r\n
```

When using '' or "", don't escape value.

**TODO**

~~maybe list is not necessary~~

### list (atomic)

**append**

**lpop**

**rpop**

**index**

**llen**

****

~~a little restful style~~

for invisible byte, using url encode, 0x08 -> %08

You cannot include spcial URL char directly(%#&=).Instead, encode them.