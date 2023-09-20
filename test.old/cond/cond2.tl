
#OUTPUT
#X greater than 20.
#IDK
#X greater than 10.
#X greater than 5.
#IDK
#END

#RET 0

extern printf(ln:string, x:i32);
extern println(ln:string);

func cmp(x:i32) is
    if x > 20 then
        println("X greater than 20.");
    elif x > 10 then
        println("X greater than 10.");
    elif x > 5 then
        println("X greater than 5.");
    else
        println("IDK");
    end
end

func main -> i32 is
    cmp(25);
    cmp(4);
    cmp(15);
    cmp(7);
    cmp(1);
    return 0;
end

