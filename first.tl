
#OUTPUT
#Hello!
# How are you?
#Hello! How are you?
#Hello!!
#END

#RET 0

func main(args:string[]) -> i32 is
    var str1 : string := "Hello!";
    var str2 : string := " How are you?";
    
    println(str1);
    println(str2);
    
    var str3 : string := str1 + str2;
    println(str3);
    
    var str4 : string := str1 + '!';
    println(str4);
    
    return 0;
end
