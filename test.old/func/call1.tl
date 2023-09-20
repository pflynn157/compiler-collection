
#OUTPUT
#X: 53
#END

#RET 0

func getNumber(x:i32, y:i32) -> i32 is
    return 20 + x + y;
end

func main -> i32 is
    var x : i32 := getNumber(23, 10);
    println("X: %d", x);
    
    return 0;
end

