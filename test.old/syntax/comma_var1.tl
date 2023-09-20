
#OUTPUT
#X: 20 | Y: 20
#X: 22 | Y: 66
#END

#RET 0

func main -> i32 is
    var x, y : i32 := 20;
    
    println("X: %d | Y: %d", x, y);
    
    x := 22;
    y := x * 3;
    
    println("X: %d | Y: %d", x, y);
    
    return 0;
end
