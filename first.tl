
struct S1 is
    x : i32 := 10;
    y : i32 := 20;
end

func main -> i32 is
    struct s : S1;
    
    var x : i32 := s.x;
    print(x);
    
    print(s.x);
    print(s.y);
    
    return 0;
end
