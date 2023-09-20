#OUTPUT
#True
#True
#False
#False
#False
#END

#RET 0

func test1(x:i32) is
    if x > 20 and x < 30 then
        println("True");
    else
        println("False");
    end
end

func main -> i32 is
    test1(21);
    test1(25);
    test1(30);
    test1(20);
    test1(100);
    return 0;
end
