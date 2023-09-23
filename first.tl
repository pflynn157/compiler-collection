extern output(s:string, len:i32);
extern realloc(old_addr:string, old_len:i32, new_len:i32) -> string;

func main -> i32 is
    var s : string := "Hello!\n";
    output(s, 7);
    
    var s2 : u8[5];
    s2[0] := 'H';
    s2[1] := 'i';
    s2[2] := '!';
    s2[3] := '\n';
    s2[4] := 0;
    
    var len1 : i32 := strlen(s2);
    output(s2, 5);
    
    var nums : i32[10];
    
    s2 := realloc(s2, 5, 6);
    
    s2[4] := '\n';
    s2[5] := 0;
    output(s2, 6);
    
    print("sd", s, 19);
    
    return len1;
end

