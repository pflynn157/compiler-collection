
#OUTPUT
#Hello!
# How are you?
#Hello! How are you?
#Hello!!
#END

#RET 0

import std.fs;

func main() -> i32 is
    var fd : i32 := fs_open("../first.ok", 1);
    if fd = 0 then
        println("Unable to open.");
        return 1;
    end
    
    while fs_eof(fd) = false do
        var c : char := fs_get(fd);
        
        if fs_eof(fd) = false then
            print("%c", c);
        end
    end
    
    fs_close(fd);
    
    #
    # Now, write
    #
    fd := fs_open("/tmp/test.txt", 2);
    if fd = 0 then
        println("Unable to open for write.");
        return 1;
    end
    
    fs_writeln(fd, "Hello!");
    fs_writeln(fd, "How are you?");
    fs_close(fd);
    
    #
    # Try binary writing
    #
    fd := fs_open("/tmp/first.bin", 2);
    if fd = 0 then
        println("Unable to open test2.");
        return 1;
    end
    
    var bytes : u8[5];
    bytes[0] := 0xA1;
    bytes[1] := 0xA2;
    bytes[2] := 0xA3;
    bytes[3] := 0xA4;
    bytes[4] := 0xA5;
    
    fs_write(fd, bytes, 1, 5);
    fs_close(fd);
    
    return 0;
end

