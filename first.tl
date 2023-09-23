extern debug(s:string, len:i32);
extern realloc(old_addr:string, old_len:i32, new_len:i32) -> string;

func main -> i32 is
    var s : string := "Hello!\n";
    debug(s, 7);
    
    var s2 : u8[4];
    s2[0] := 'H';
    s2[1] := 'i';
    s2[2] := '!';
    s2[3] := '\n';
    debug(s2, 4);
    
    var nums : i32[10];
    
    s2 := realloc(s2, 4, 5);
    
    s2[4] := '\n';
    debug(s2, 5);
    
    return 0;
end

