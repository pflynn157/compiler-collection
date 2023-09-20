
#OUTPUT
#Main!
#Func1
#Func2
#Func3: 20
#Func4: 20, 30
#Func5: 20
#Func6: 20, 30
#END

#RET 0

func func1 is
    println("Func1");
end

func func2 -> i32 is
    println("Func2");
    return 0;
end

func func3(x:i32) is
    println("Func3: %d", x);
end

func func4(x:i32, y:i32) is
    println("Func4: %d, %d", x, y);
end

func func5(x:i32) -> i32 is
    println("Func5: %d", x);
    return 0;
end

func func6(x:i32, y:i32) -> i32 is
    println("Func6: %d, %d", x, y);
    return 0;
end

func main -> i32 is
    println("Main!");
    
    func1();
    func2();
    func3(20);
    func4(20, 30);
    func5(20);
    func6(20, 30);
    
    return 0;
end
