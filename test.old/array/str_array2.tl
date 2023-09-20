
#OUTPUT
#Hi!
#How are you?
#Good!
#END

#RET 0

func main -> i32 is
    var args : string[3];
    args[0] := "Hi!";
    args[1] := "How are you?";
    args[2] := "Good!";
    
    var i : i32 := 0;
    while i < 3 do
        println(args[i]);
        i := i + 1;
    end
    
    return 0;
end
