
#OUTPUT
#True
#20
#False
#31
#END

#RET 0

func test1(x:i8) is
    if x = 20 then println("True");
    else println("False");
    end
    
    println("%d", x);
end

func test2(x:i8) is
    if x = 20 then println("True");
    else println("False");
    end
    
    println("%d", x);
end

func main -> i32 is
    var x1 : i8 := 20;
    var x2 : i8 := 31;
    
    test1(x1);
    test2(x2);
    return 0;
end
