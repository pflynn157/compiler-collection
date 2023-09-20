
struct S1 is
    x : i32 := 10;
    y : i32 := 20;
end

func printStruct(s:S1) is
    println("X: %d", s.x);
    println("Y: %d", s.y);
end

func main -> i32 is
    struct s : S1;
    
    println("Before:");
    printStruct(s);
    
    s.x := 25;
    
    println("After:");
    printStruct(s);
    
    return 0;
end
