
#OUTPUT
#Equal
#NEQ
#GT
#Not
#LT
#Not
#NEQ
#Not
#GTE
#GTE
#Not
#LTE
#LTE
#Not
#END

#RET 0

extern printf(ln:string, x:i32);
extern println(ln:string);

func cmp1(x:i32) is
    if x = 10 then
        println("Equal");
    else
        println("NEQ");
    end
end

func cmp2(x:i32) is
    if x > 10 then
        println("GT");
    else
        println("Not");
    end
end

func cmp3(x:i32) is
    if x < 10 then
        println("LT");
    else
        println("Not");
    end
end

func cmp4(x:i32) is
    if x != 10 then
        println("NEQ");
    else
        println("Not");
    end
end

func cmp5(x:i32) is
    if x >= 10 then
        println("GTE");
    else
        println("Not");
    end
end

func cmp6(x:i32) is
    if x <= 10 then
        println("LTE");
    else
        println("Not");
    end
end

func main -> i32 is
    cmp1(10);
    cmp1(0);
    
    cmp2(20);
    cmp2(0);
    
    cmp3(5);
    cmp3(50);
    
    cmp4(11);
    cmp4(10);
    
    cmp5(20);
    cmp5(10);
    cmp5(0);
    
    cmp6(10);
    cmp6(0);
    cmp6(20);
    
    return 0;
end

