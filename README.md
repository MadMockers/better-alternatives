This is a proof of concept for better solution of 'update-alternatives'

Code is messy / buggy / hodgepodged / will likely segfault  
Written between the hours of 10PM and 3AM

'alternatives' file format:
```
<alternative_name> <path/to/option> <priority>
```

e.g:
```
editor /usr/bin/vim 5
```

Search path:
```
$HOME/.alternatives
/etc/balternatives/$GROUP_ID.alternatives
/etc/balternatives/.alternatives
```

Default alternative settings go in /etc/balternatives/.alternatives  
User customizations go in their home directory

Compile with:
```
cmake .
make
```

Run with:
```
# bin/better-alternatives -oallow_other path/to/alternatives/directory
```

e.g:
```
# mkdir /etc/balternatives/alts/
# bin/better-alternatives -oallow_other /etc/balternatives/alts/
```

When listing the alternatives directory, you will see the alternatives for your user. Here's an example:
```
$ ls -l test/
total 0
lr--r--r-- 1 root root 14 Jan  1  1970 editor -> /usr/bin/gedit

$ echo "editor /usr/bin/vim 5" > ~/.alternatives

$ ls -l test
total 0
lr--r--r-- 1 root root 12 Jan  1  1970 editor -> /usr/bin/vim

$ sudo su -c "ls -l `pwd`/test"
total 0
lr--r--r-- 1 root root 14 Jan  1  1970 editor -> /usr/bin/gedit
```
