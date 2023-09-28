#
# A simple example of the "echo" core utility written in Tiny
#

func help is
    println("echo\n");
    println("-n\t\tDo not output a trailing newline.");
    println("--help\t\tDisplay this message and exit.");
    println("--version\tDisplay version information.");
end

func version is
    println("simple echo, version 1\n");
    println("This is a simple implementation of echo written in Tiny Lang");
    println("It provides a simple example of what the language can do.");
    println("\nWritten by Patrick Flynn");
end

func main(args:string[], argc:i32) -> i32 is
    if argc = 0 then
        println("");
        return 0;
    end
    
    # Check arguments
    # Only the first parameter is an argument
    var arg : string := args[1];
    var nl : bool := true;
    if arg = "--help" then
        help();
        return 0;
    elif arg = "--version" then
        version();
        return 0;
    elif arg = "-n" then
        nl := false;
    end

    # Now, we can echo
    var i : i32 := 1;
    if nl = false then
        i := 2;
    end
    
    while i < argc do
        print(args[i]);
        print(" ");
        i := i + 1;
    end
    
    if nl then
        println("");
    end

    return 0;
end

