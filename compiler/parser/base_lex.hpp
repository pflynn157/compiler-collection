#pragma once

struct BaseLex {
    virtual void unget(int t) {}
    virtual int get_next() { return 0; }
    virtual void debug_token(int t) {}
    
    std::string value = "";
    int i_value = 0;
    double f_value = 0.0;
    int line_number = 0;
};

