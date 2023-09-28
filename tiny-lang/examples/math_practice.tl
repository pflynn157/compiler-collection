
#
# A simple math practice program written in Tiny
#

import std.io;
import std.num;

func practice(min:i32, max:i32) is
    var i : i32 := 0;
    while i < 10 do
        var n1 : i32 := rand_int(min, max);
        var n2 : i32 := rand_int(min, max);
        var op_sym : i32 := rand_int(1, 3);
        var op : char := '+';
        var answer, user_answer : i32 := 0;
        
        if op_sym = 1 then
            answer := n1 + n2;
            op := '+';
        elif op_sym = 2 then
            answer := n1 - n2;
            op := '-';
        elif op_sym = 3 then
            answer := n1 * n2;
            op := '*';
        end
        
        # Print the problem
        println("");
        println("%d %c %d = ", n1, op, n2);
        
        # Prompt the user
        var tries : i32 := 0;
        while true do
            user_answer := readint();
            if user_answer = answer then
                println("Correct!!!");
                break;
            else
                println("Wrong :(");
                if tries = 3 then
                    println("The correct answer is: %d", answer);
                    break;
                end
            end
            tries := tries + 1;
        end
        
        i := i + 1;
    end
end

func main(args:string[], argc:i32) -> i32 is
    rand_init();
    
    var level : i32 := 0;
    
    println("\tWelcome to Tiny Math!\n");
    
    while true do
        print("Enter your level (1-3): ");
        level := readint();
        
        if level >= 1 and level <= 3 then
            break;
        else
            println("Invalid level! Please try again.");
        end
    end
    
    # Assign levels
    var min, max : i32 := 0;
    
    if level = 1 then
        min := 0;
        max := 10;
    elif level = 2 then
        min := -20;
        max := 100;
    elif level = 3 then
        min := -1000;
        max := 1000;
    end
    
    practice(min, max);

    return 0;
end
