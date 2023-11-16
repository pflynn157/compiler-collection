func main -> i32 is
    var str1 : string := "Hello!";
    var str2 : string := "Hello!";
    var str3 : string := "Hello!!!";
    
    if str1 = str2 then
        print("Pass1");
    else
        print("Fail1");
    end
    
    if str1 != str3 then
        print("Pass2");
    else
        print("Fail2");
    end
    
    var str4 : string := str1 + str2;
    var str5 : string := str1 + '!';
    
    return 0;
end

