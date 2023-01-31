
#OUTPUT
#Before:
#X: 10
#Y: 20
#After:
#X: 25
#Y: 35
#END

#RET 0

struct S1 is
    x : i32 := 10;
    y : i32 := 20;
end

func printStruct(s:S1) is
    println("X: %d", s.x);
    println("Y: %d", s.y);
end

func updateStruct(s:S1) is
    s.x := 25;
    s.y := 35;
end

func main -> i32 is
    struct s : S1;
    
    println("Before:");
    printStruct(s);
    
    updateStruct(s);
    
    println("After:");
    printStruct(s);
    
    return 0;
end
