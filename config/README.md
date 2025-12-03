configuration format:

---------------------------------------

<block> {
    <key> <value> <value> ...;
    ...
}

---------------------------------------

blocks:
server{...}
local /<path> {...}


keys:          expected:
methods        [GET, POST, DELETE]
root           <path_to_dir>
cgi            <extension> <path>
upload         <on|off>
upload_dir     <path_to_file> 
listing        <on|off>  
listen         <interface:port>
server_name    <name>
