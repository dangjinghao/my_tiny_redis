## My Plan
- training SOCKET and data structure programming
- using LLRBtree to store and search data
- trying to program with moderncpp
- http request support
- try ctest
## action
**PUT**
```
POST /[key] HTTP/x.x\r\n
...(skip those headers)
Content-Length: [length]\r\n
...
val: [val]\r\n
```

**DELETE** 
```
DELETE /[key] HTTP/x.x\r\n
```

**MODIFY**
```
POST /[key] HTTP/x.x\r\n
...(skip those headers)
Content-Length: [length]\r\n
...
val: [val]\r\n
```

**GET**
```
GET /[key] HTTP/x.x\r\n
```

When using '' or "", don't escape value.

**TODO**~~maybe list is not necessary~~

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