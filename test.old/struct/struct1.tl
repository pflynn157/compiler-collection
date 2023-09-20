
#OUTPUT
#X: 10
#Y: 20
#Changed X: 25
#END

#RET 0

struct S1 is
    x : i32 := 10;
    y : i32 := 20;
end

func main -> i32 is
    struct s : S1;

    var v1 : i32 := s.x;
    
    println("X: %d", v1);
    println("Y: %d", s.y);
    
    s.x := 25;
    println("Changed X: %d", s.x);
    
    return 0;
end
