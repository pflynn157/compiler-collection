
#OUTPUT
#Hello!
#l
#END

#RET 0

func main -> i32 is
    var str : string := "Hello!";
    var c : char := str[3];
    
    println(str);
    println("%c", c);
    
    return 0;
end

