
#OUTPUT
#Before:
#X: 40
#Y: 50
#After:
#X: 25
#Y: 35
#END

#RET 0

struct S1 is
    x : i32 := 10;
    y : i32 := 20;
end

func initStruct -> S1 is
    struct s : S1;
    s.x := 40;
    s.y := 50;
    return s;
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
    struct s : S1 := initStruct();
    
    println("Before:");
    printStruct(s);
    
    updateStruct(s);
    
    println("After:");
    printStruct(s);
    
    return 0;
end
